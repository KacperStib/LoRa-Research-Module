#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "ina219.h"

void app_main(void)
{
	// Inicjalizacja magistrali I2C
	if (i2c_master_init() != ESP_OK)
		ESP_LOGE("I2c", "I2C init: ERROR");
	// Urchomienie i config ina219
	ina219_power_on(0.05, 10);
	if (err != ESP_OK)
    	ESP_LOGE("INA219", "I2C CMD ERROR: 0x%x", err);	
	vTaskDelay(200 / portTICK_PERIOD_MS);
	
    while (true) {
        printf("Hello from app_main!\n");
        // Odczyt pradu i mocy
		float current = ina219_read_current() * 1000;
		float power = ina219_read_power() * 1000;
		printf("CURRENT: %.2f\n", current);
        sleep(1);
    }
}
