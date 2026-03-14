#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "../../main/bsp.h"

#define TAG_INIT "SPI_INIT"

esp_err_t spi_init(void);
