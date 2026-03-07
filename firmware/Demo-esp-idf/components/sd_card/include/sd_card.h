#ifndef SD_CARD_H
#define SD_CARD_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>
#include "esp_log.h"

esp_err_t sd_card_init();
esp_err_t s_example_write_file(const char *path, char *data);
esp_err_t s_example_append_file(const char *path, char *data);
esp_err_t s_example_read_file(const char *path);
bool file_exists(const char *path);

#endif