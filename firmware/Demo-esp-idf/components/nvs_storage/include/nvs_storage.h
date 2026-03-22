#ifndef NVS_STORAGE_H
#define NVS_STORAGE_H

#include "dev_config.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

#define NVS_NAMESPACE  "radio"
#define NVS_KEY        "cfg"

esp_err_t nvs_cfg_save(const radio_config_t *cfg);
esp_err_t nvs_cfg_load(radio_config_t *cfg);

#endif