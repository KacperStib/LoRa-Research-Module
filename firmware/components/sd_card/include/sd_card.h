/*
 * sd_card.h
 *
 * Modul obslugi karty SD w trybie SPI oraz kolejkowania logow systemowych.
 * Definiuje struktury danych dla zdarzen, parametry montowania i funkcje plikowe.
 */

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

#include "../../main/bsp.h"

#define SD_TAG "SD_CARD"

#define EXAMPLE_MAX_CHAR_SIZE    64
#define MOUNT_POINT "/sdcard"

// Struktura do zapisywania logow (RX/TX z danymi RSSI, pradu i GPS)
typedef struct {
    char tech[8];       
    bool is_tx;
    float peak_mA;      
    int rssi;           
    bool has_gps;
    float lat, lon;
} log_event_t;

// Globalna kolejka logow systemowych
extern QueueHandle_t xLogQueue;

extern char LOG_FILE_NAME[64];
extern bool sd_card_ready;

// Inicjalizacja sprzetowa i montowanie karty SD w systemie plikow FAT
esp_err_t sd_card_init(void);

// Podstawowe operacje na plikach
esp_err_t s_example_write_file(const char *path, char *data);
esp_err_t s_example_append_file(const char *path, char *data);
esp_err_t s_example_read_file(const char *path);
bool file_exists(const char *path);

// Funkcja pomocnicza do zapisu zdarzenia do kolejki logow
esp_err_t sd_log_event(bool tech, bool is_tx, float current_mA_peak, int rssi, bool gps_fix, float gps_lat, float gps_lon);

#endif // SD_CARD_H