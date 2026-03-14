#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "ina219.h"
#include "oled.h"
#include "sd_card.h"
#include "gps.h"
#include "lora.h"

void app_main(void)
{	
	SSD1306_t dev;
	// Inicjalizacja magistrali I2C
	if (i2c_master_init() != ESP_OK)
		ESP_LOGE("I2c", "I2C init: ERROR");
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
	
	// Inicjalizacja i start taska GPS
    nmea_parser_config_t config = NMEA_PARSER_CONFIG_DEFAULT();
    nmea_parser_handle_t nmea_hdl = nmea_parser_init(&config);
    nmea_parser_add_handler(nmea_hdl, gps_event_handler, NULL);
	
	// Inicjalizacja LoRa
	int cr = 1;
	int bw = 7;
	int sf = 7;
	
	if (lora_init() == 0)
		ESP_LOGE(pcTaskGetName(NULL), "Does not recognize the module");
		
	lora_set_coding_rate(cr);
	ESP_LOGI(pcTaskGetName(NULL), "coding_rate=%d", cr);
	
	lora_set_bandwidth(bw);
	ESP_LOGI(pcTaskGetName(NULL), "bandwidth=%d", bw);
	
	lora_set_spreading_factor(sf);
	ESP_LOGI(pcTaskGetName(NULL), "spreading_factor=%d", sf);
	
	// Task LoRa TX
	xTaskCreate(&task_tx, "TX", 1024*3, NULL, 5, NULL);
	// Task LoRa RX
	//xTaskCreate(&task_rx, "RX", 1024*3, NULL, 5, NULL);
	
    while (true) {
        printf("Hello from app_main!\n");
        // Odczyt pradu
		float current = ina219_read_current() * 1000;
		printf("CURRENT: %.2f\n", current);
		
		// Wyswietlanie na ekranie OLED
		char buf[12];
		sprintf(buf, "Prad: %.2f", current);
		ssd1306_display_text(&dev, 1, buf, 12, false);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
        // Zapis na karte SD
        snprintf(buf, sizeof(buf), "CURR: %.2f\n", current);
        s_example_append_file((const char*)LOG_FILE_NAME, buf);
    }
}
