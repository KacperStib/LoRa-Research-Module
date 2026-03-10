#ifndef SD_CARD_H
#define SD_CARD_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>
#include "esp_log.h"
#include <string.h>

#include <driver/sdspi_host.h>
#include "driver/spi_common.h"
#include <driver/sdmmc_host.h>
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"

#define EXAMPLE_MAX_CHAR_SIZE    64
#define MOUNT_POINT "/sdcard"

extern char LOG_FILE_NAME[64];

esp_err_t sd_card_init();
esp_err_t s_example_write_file(const char *path, char *data);
esp_err_t s_example_append_file(const char *path, char *data);
esp_err_t s_example_read_file(const char *path);
bool file_exists(const char *path);

#endif