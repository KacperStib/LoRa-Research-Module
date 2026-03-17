#include <stdio.h>
#include "include/sd_card.h"

char LOG_FILE_NAME[64] = {0,};
bool sd_card_ready = 0;

esp_err_t sd_card_init()
{	
	sdmmc_card_t *card;
    /* 2. Konfiguracja hosta SD-SPI (używa już istniejącej magistrali) */
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    /* WAŻNE: nie wywołujemy host.init() — magistrala już gotowa      */
 
    /* 3. Konfiguracja slotu SD */
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SD_CS;
    slot_config.host_id = (spi_host_device_t)host.slot; /* SPI2_HOST */
 
    /* 4. Montowanie systemu plików FAT */
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files              = 5,
        .allocation_unit_size   = 16 * 1024,
    };
 
    ESP_LOGI(SD_TAG, "Montowanie karty SD (CS=GPIO%d)...", SD_CS);
    esp_err_t ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config,
                                   &mount_config, &card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(SD_TAG, "Błąd montowania systemu plików.");
        } else {
            ESP_LOGE(SD_TAG, "Błąd inicjalizacji karty SD: %s",
                     esp_err_to_name(ret));
        }
        return ret;
    }
 
    sdmmc_card_print_info(stdout, card);
    ESP_LOGI(SD_TAG, "Karta SD zamontowana w %s", MOUNT_POINT);
    sd_card_ready = 1;
    return ESP_OK;
}

esp_err_t s_example_write_file(const char *path, char *data)
{
    ESP_LOGI("SD", "Opening file %s", path);
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        ESP_LOGE("SD", "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, "%s", data);
    fclose(f);
    ESP_LOGI("SD", "File written");

    return ESP_OK;
}

esp_err_t s_example_append_file(const char *path, char *data)
{
    FILE *f = fopen(path, "a");
    if (f == NULL) {
        ESP_LOGE("SD", "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, "%s", data);
    fclose(f);
    return ESP_OK;
}

esp_err_t s_example_read_file(const char *path)
{
    ESP_LOGI("SD", "Reading file %s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        ESP_LOGE("SD", "Failed to open file for reading");
        return ESP_FAIL;
    }
    char line[EXAMPLE_MAX_CHAR_SIZE];
    fgets(line, sizeof(line), f);
    fclose(f);

    // strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI("SD", "Read from file: '%s'", line);

    return ESP_OK;
}

bool file_exists(const char *path) {
	FILE * fd = fopen(path, "r");
    if (fd == 0) {
        return false;  // File does not exist
    }
    fclose(fd);
    return true;  // File exists
}