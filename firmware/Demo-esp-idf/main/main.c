#include <stdio.h>
#include <stdbool.h>
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

float current_mA = 0;
SSD1306_t dev;

// ===== TASK PWR =====
void vPowerTask(void *pv) {
	for (;;) {
    	current_mA = ina219_read_current() * 1000;
    	vTaskDelay(pdMS_TO_TICKS(200));
  	}
}

// ===== TASK OLED =====
void vOledTask(void *pv) {
	for (;;) {
		char buf[12];
		
		sprintf(buf, "Prad: %.2f", current_mA);
		ssd1306_display_text(&dev, 1, buf, 12, false);
		
		sprintf(buf, "GPS: %.2f", current_mA);
		ssd1306_display_text(&dev, 2, buf, 12, false);
		
		sprintf(buf, "Radio: %s", radio_mode ? "TX" : "RX");
		ssd1306_display_text(&dev, 3, buf, 12, false);
		
		vTaskDelay(pdMS_TO_TICKS(200));
	}
}

// ===== TASK SD =====
void vLogTask(void *pv) {
	for (;;) {
		char buf[50];
		snprintf(buf, sizeof(buf), "CURR: %.2f\n", current_mA);
        s_example_append_file((const char*)LOG_FILE_NAME, buf);
        vTaskDelay(pdMS_TO_TICKS(1000));
 	}
}

// ===== TASK LORA =====
void vRadioTask(void *pv) {
	uint8_t buf8[32];
	for (;;) {
		if (radio_mode){
			// LoRa send
	        int send_len = sprintf((char *)buf8,"Hello World!!");
	        lora_send_packet(buf8, send_len);
	        vTaskDelay(60000 / portTICK_PERIOD_MS);
	  	}
	  	else {
			lora_receive(); // put into receive mode
			if (lora_received()) {
				int rxLen = lora_receive_packet(buf8, sizeof(buf8));
				ESP_LOGI(pcTaskGetName(NULL), "%d byte packet received:[%.*s]", rxLen, rxLen, buf8);
			}
			vTaskDelay(1); // Avoid WatchDog alerts 
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
	
	// Urchomienie i config ina219
	ina219_power_on(0.05, 10);
	if (err != ESP_OK)
    	ESP_LOGE("INA219", "I2C CMD ERROR: 0x%x", err);	
	vTaskDelay(200 / portTICK_PERIOD_MS);
	
	// Inicjalizacja ekranu OLED
	ssd1306_init(&dev, 128, 64);
	ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff);
	ssd1306_display_text(&dev, 0, "Hello", 5, false);
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	
	// Inicjalizacja SD karty
	sd_card_init();
	snprintf(LOG_FILE_NAME, sizeof(LOG_FILE_NAME), "%s/log.txt", MOUNT_POINT);
	s_example_write_file((const char*)LOG_FILE_NAME, "HELLO FROM ESP IDF");
	
	// Inicjalizacja LoRa
	lora_init();
	lora_set_frequency(433e6);
	lora_set_coding_rate(1);
	lora_set_bandwidth(7);
	lora_set_spreading_factor(7);
	
	// Inicjalizacja handlera GPS
	/* NMEA parser configuration */
    nmea_parser_config_t config = NMEA_PARSER_CONFIG_DEFAULT();
    /* init NMEA parser library */
    nmea_parser_handle_t nmea_hdl = nmea_parser_init(&config);
    /* register event handler for NMEA parser library */
    nmea_parser_add_handler(nmea_hdl, gps_event_handler, NULL);
	
	// Tasks
	xTaskCreate(vPowerTask,  "PWR",  2048, NULL, 5, NULL);
	xTaskCreate(vOledTask,  "OLED",  4096, NULL, 6, NULL);
	xTaskCreate(vLogTask,  "SD",  4096, NULL, 3, NULL);
	xTaskCreate(vRadioTask,  "RADIO",  4096, NULL, 1, NULL);
	
	// Konsola
	shell_init();
}
