#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "ina219.h"
#include "oled.h"

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
    }
}
