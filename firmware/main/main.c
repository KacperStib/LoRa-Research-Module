/**
 * @file main.c
 * @author Kacper Stiborski
 * @brief Główny moduł aplikacji urządzenia radiowego dla ESP32.
 *        Zarządza zadaniami FreeRTOS (Radio, SD, OLED, GPS, Power).
 * @version 1.0
 * @date 2026-05-16
 * 
 * @copyright Copyright (c) 2026
 * 
 */
 
#include <stdio.h>
#include <stdbool.h>
#include <sys/_intsup.h>
#include <unistd.h>

#include "esp_log.h"
#include "spi.h"
#include "ina219.h"
#include "oled.h"
#include "sd_card.h"
#include "lora.h"
#include "gps.h"
#include "shell_mng.h"
#include "dev_config.h"
#include "nvs_storage.h"

// Instancja wyswietlacza
SSD1306_t dev;

// ====== TASK PWR ======
// Cykliczny odczyt danych o zuzyciu pradu
void vPowerTask(void *pv) 
{
	for (;;) 
	{
    	current_mA = ina219_read_current() * 1000;
    	vTaskDelay(pdMS_TO_TICKS(1000));
  	}
}

// ====== TASK OLED ======
// Wyswietlanie informacji na ekranie
void vOledTask(void *pv) 
{
	for (;;) 
	{
		char buf[32];
		
		// Zuzycie chwilowe pradu
		memset(buf, ' ', sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
		sprintf(buf, "Prad: %.2f", current_mA);
		ssd1306_display_text(&dev, 1, buf,sizeof(buf) - 1, false);
		
		// FIX GPS
		memset(buf, ' ', sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
		sprintf(buf, "GPS FIX: %s (%d)", gps_fix ? "Y" : "N", gps_sats);
		ssd1306_display_text(&dev, 2, buf, sizeof(buf) - 1, false);
		// Wspolrzedne GPS
		if (gps_fix)
		{
			memset(buf, ' ', sizeof(buf) - 1);
        	buf[sizeof(buf) - 1] = '\0';
			sprintf(buf, "%.2f N %.2f E", gps_lat, gps_lon);
			ssd1306_display_text(&dev, 3, buf, sizeof(buf) - 1, false);
		}
		
		// Technologia radiowa
		memset(buf, ' ', sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
		sprintf(buf, "%s %s",
				radio_cfg.tech == RADIO_TECH_LORA ? "LORA" : "ESPNOW",
				radio_cfg.dir ? "TX" : "RX");
		ssd1306_display_text(&dev, 4, buf, sizeof(buf) - 1, false);
		
		// Informacje o transmisji
		memset(buf, ' ', sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        // RSSI tylko w trybie RX
        if (radio_cfg.dir == RADIO_DIR_RX) 
        {
            sprintf(buf, "RSSI: %d dBm", rssi);
            ssd1306_display_text(&dev, 5, buf, sizeof(buf) - 1, false);
        } 
        // Pik pradowy w trybie TX
        else 
        {
			sprintf(buf, "Peak: %.2f mA", current_mA_peak);
            ssd1306_display_text(&dev, 5, buf, sizeof(buf) - 1, false); // czyść linię
        }
        
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

// ====== TASK SD ======
// Zapis logow na karte SD
void vLogTask(void *pv) 
{
	// Instancja zdarzenia
	log_event_t ev;
    for (;;) 
    {
		// Odczytaj z kolejki
		if (xQueueReceive(xLogQueue, &ev, portMAX_DELAY) == pdTRUE) 
		{	
			// Dodatkowo sprawdz czy karta jest zainicjalizowana
	        if (!sd_card_ready) 
	        {
	            sd_card_init();
	            snprintf(LOG_FILE_NAME, sizeof(LOG_FILE_NAME), "%s/log.txt", MOUNT_POINT);
	            s_example_write_file((const char*)LOG_FILE_NAME, "Measurements:");
	        }
	
	        char buf[80];
	        //const char *tech = radio_cfg.tech == RADIO_TECH_LORA ? "LORA" : "ESPNOW";
			// Wyluskaj informacje dla TX
	        if (!ev.is_tx) 
	        {
	            snprintf(buf, sizeof(buf), "%s RX RSSI:%d%s\n",
	                ev.tech,
	                ev.rssi,
	                ev.has_gps ? ({static char g[32]; snprintf(g,sizeof(g)," LAT:%.5f LON:%.5f",gps_lat,gps_lon); g;}) : "");
	        } 
	        // Wyluskaj informacje dla RX
	        else 
	        {
	            snprintf(buf, sizeof(buf), "%s TX CURR:%.2f%s\n",
	                ev.tech,
	                ev.peak_mA,
	                ev.has_gps ? ({static char g[32]; snprintf(g,sizeof(g)," LAT:%.5f LON:%.5f",gps_lat,gps_lon); g;}) : "");
	        }
			// Zapisz log na karcie SD
	        s_example_append_file((const char*)LOG_FILE_NAME, buf);
	    }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// ====== TASK RADIO ======
// Zarzadza przesylaniem lub odbieraniem danych w wybranej technologii
void vRadioTask(void *pv) 
{
	uint8_t buf8[32];
	for (;;) 
	{	
		// Technologia LoRa
		if (radio_cfg.tech == RADIO_TECH_LORA)
		{	
			// tryb TX
			if (radio_cfg.dir)
			{
		        int send_len = sprintf((char *)buf8,"Hello World!!");
		        lora_send_packet(buf8, send_len);
		        // Zmierz pik pradowy od razu po nadaniu
		        current_mA_peak = ina219_find_peak() * 1000;
		        // Wrzuc log do kolejki dla karty SD
		        sd_log_event (true, true, current_mA_peak, 0, gps_fix, gps_lat, gps_lon);
		        vTaskDelay(6000 / portTICK_PERIOD_MS);
		  	}
		  	// Tryb RX
		  	else 
		  	{
				lora_receive();
				if (lora_received()) 
				{
					int rxLen = lora_receive_packet(buf8, sizeof(buf8));
					// Zmierz RSSI
					rssi = (int)lora_packet_rssi();
					// Wrzuc log do kolejki dla karty SD
					sd_log_event (true, false, 0, rssi, gps_fix, gps_lat, gps_lon);
					ESP_LOGI(pcTaskGetName(NULL), "%d byte packet received:[%.*s]", rxLen, rxLen, buf8);
					
				}
				// W przypadku LoRa trzeba robic recznie polling wiadomosci, nie ma mechanizmu callbacku
				vTaskDelay(1); 
			}
		}
		// ESPNOW
		else 
		{ 
			// Tryb TX
			if (radio_cfg.dir)
			{
		        int len = sprintf((char *)buf8, "Hello ESP-NOW!");
                espnow_send(buf8, len);
                current_mA_peak = ina219_find_peak() * 1000;
                sd_log_event (false, true, current_mA_peak, 0, gps_fix, gps_lat, gps_lon);
                vTaskDelay(pdMS_TO_TICKS(6000));
		  	}
		  	// Tryb RX
		  	else 
		  	{	
				// Cala logika odbioru jest w callbacku, zarejestrowanym przy inicjalizacji
				vTaskDelay(pdMS_TO_TICKS(100));
			}
		}
	}
}

void app_main(void)
{	
	// Inicjalizacja magistrali I2C
	if (i2c_master_init() != ESP_OK)
		ESP_LOGE("I2c", "I2C init: ERROR");
	// Inicjalizacja magistrali SPI
	spi_init();
	
	// NVS — jedno wywołanie, tutaj, przed wszystkim co z NVS korzysta
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
	    ESP_ERROR_CHECK(nvs_flash_erase());
	    ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	
	// Wczytaj ostatni config z NVS (jeśli brak — zostaną defaulty z dev_config.c)
	nvs_cfg_load((radio_config_t *)&radio_cfg);
	
	// Zastosuj config sprzętowo dla radia
	radio_apply_config();

	// Urchomienie i config ina219
	ina219_power_on(0.05, 6.4);
	if (err != ESP_OK)
    	ESP_LOGE("INA219", "I2C CMD ERROR: 0x%x", err);	
	vTaskDelay(200 / portTICK_PERIOD_MS);
	
	// Inicjalizacja ekranu OLED
	ssd1306_init(&dev, 128, 64);
	ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff);
	ssd1306_display_text(&dev, 0, "Radio Module", 13, false);
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	
	// Inicjalizacja SD karty
	sd_card_init();
	snprintf(LOG_FILE_NAME, sizeof(LOG_FILE_NAME), "%s/log.txt", MOUNT_POINT);
	s_example_write_file((const char*)LOG_FILE_NAME, "Measurements:");
	
	// Inicjalizacja handlera GPS - tworzy task GPS
	/* NMEA parser configuration */
    nmea_parser_config_t config = NMEA_PARSER_CONFIG_DEFAULT(GPS_RX);
    /* init NMEA parser library */
    nmea_parser_handle_t nmea_hdl = nmea_parser_init(&config);
    /* register event handler for NMEA parser library */
    nmea_parser_add_handler(nmea_hdl, gps_event_handler, NULL);
	
	// Kolejka do zapisywania logow na SD
	xLogQueue = xQueueCreate(8, sizeof(log_event_t));
	
	// Rozpocznij Zadania
	xTaskCreate(vRadioTask,  "RADIO",  4096, NULL, 1, NULL);
	xTaskCreate(vLogTask,  "SD",  4096, NULL, 3, NULL);
	xTaskCreate(vPowerTask,  "PWR",  2048, NULL, 5, NULL);
	xTaskCreate(vOledTask,  "OLED",  4096, NULL, 6, NULL);
	
	// Konsola
	shell_init();
}