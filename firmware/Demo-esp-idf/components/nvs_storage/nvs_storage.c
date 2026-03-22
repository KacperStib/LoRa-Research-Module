#include "include/nvs_storage.h"

static const char *TAG = "nvs_cfg";

esp_err_t nvs_cfg_save(const radio_config_t *cfg)
{
    nvs_handle_t h;
    esp_err_t err;

    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open: %s", esp_err_to_name(err));
        return err;
    }

    err = nvs_set_blob(h, NVS_KEY, cfg, sizeof(*cfg));
    if (err == ESP_OK)
        err = nvs_commit(h);

    if (err != ESP_OK)
        ESP_LOGE(TAG, "nvs_set_blob/commit: %s", esp_err_to_name(err));

    nvs_close(h);
    return err;
}

esp_err_t nvs_cfg_load(radio_config_t *cfg)
{
    nvs_handle_t h;
    esp_err_t err;

    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &h);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open: %s", esp_err_to_name(err));
        return err;
    }

    size_t len = sizeof(*cfg);
    err = nvs_get_blob(h, NVS_KEY, cfg, &len);

    if (err == ESP_ERR_NVS_NOT_FOUND)
        ESP_LOGW(TAG, "Brak zapisanego configu w NVS");
    else if (err != ESP_OK)
        ESP_LOGE(TAG, "nvs_get_blob: %s", esp_err_to_name(err));
    else if (len != sizeof(*cfg)) {
        ESP_LOGW(TAG, "Rozmiar bloba (%u) != sizeof(radio_config_t) (%u) — ignoruję",
                 (unsigned)len, (unsigned)sizeof(*cfg));
        err = ESP_ERR_INVALID_SIZE;
    }

    nvs_close(h);
    return err;
}