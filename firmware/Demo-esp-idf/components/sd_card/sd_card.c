#include <stdio.h>
#include "include/sd_card.h"

char LOG_FILE_NAME[64] = {0,};

esp_err_t sd_card_init(){
	esp_vfs_fat_sdmmc_mount_config_t mount_config = { // @suppress("Invalid arguments")
	#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
	        .format_if_mount_failed = true,
	#else
	        .format_if_mount_failed = false,
	#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
	        .max_files = 5,
	        .allocation_unit_size = 16 * 1024
	};


	sdmmc_card_t *card;
	const char mount_point[] = MOUNT_POINT;

	ESP_LOGI("SD", "Initializing SD card");
	sdmmc_host_t host = SDSPI_HOST_DEFAULT(); // @suppress("Function cannot be resolved") // @suppress("Invalid arguments")
	
	#define SD_MOSI 19 //13
	#define SD_CLK  20  // 14
	#define SD_MISO 21 // 12
	#define SD_SS 18

	gpio_reset_pin(SD_SS);  // Reset GPIO settings to default
	gpio_set_direction(SD_SS, GPIO_MODE_OUTPUT); // Set as output
	gpio_set_level(SD_SS, 0); // Set initial level to LOW

	spi_bus_config_t bus_cfg = { // @suppress("Type cannot be resolved")
	        .mosi_io_num = SD_MOSI,
	        .miso_io_num = SD_MISO,
	        .sclk_io_num = SD_CLK,
	        .quadwp_io_num = -1,
	        .quadhd_io_num = -1,
	        .max_transfer_sz = 4000,
	 };


	esp_err_t ret = spi_bus_initialize((spi_host_device_t)host.slot, &bus_cfg, SDSPI_DEFAULT_DMA); // @suppress("Symbol is not resolved") // @suppress("Invalid arguments") // @suppress("Type cannot be resolved")
	if (ret != ESP_OK) {
		ESP_LOGE("SD", "Failed to initialize bus.");
		return -1;
	}

	// This initializes the slot without card detect (CD) and write protect (WP) signals.
	// Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
	sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT(); // @suppress("Function cannot be resolved") // @suppress("Type cannot be resolved")
	slot_config.gpio_cs = SD_SS; // @suppress("Field cannot be resolved")
	slot_config.host_id = (spi_host_device_t)host.slot; // @suppress("Field cannot be resolved") // @suppress("Type cannot be resolved")

	ESP_LOGI("SD", "Mounting filesystem");
	ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card); // @suppress("Invalid arguments")

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE("SD", "Failed to mount filesystem. "
					 "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
		} else {
			ESP_LOGE("SD", "Failed to initialize the card (%s). "
					 "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
		}
		return -1;
	} 
	ESP_LOGI("SD", "Filesystem mounted");

	// Card has been initialized, print its properties
	sdmmc_card_print_info(stdout, card);
	
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