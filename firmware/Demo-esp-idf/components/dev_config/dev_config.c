#include <stdio.h>
#include "include/dev_config.h"

static const char *TAG = "radio_ctrl";

volatile radio_config_t radio_cfg = {
    .tech = RADIO_TECH_LORA,
    .dir  = RADIO_DIR_RX,
};

void radio_apply_config(void)
{
    if (radio_cfg.tech == RADIO_TECH_LORA) {
        espnow_deinit();
        lora_init();
        lora_set_frequency(433e6);
        lora_set_coding_rate(1);
        lora_set_bandwidth(7);
        lora_set_spreading_factor(7);
        ESP_LOGI(TAG, "Aktywna: LORA, dir=%s",
                 radio_cfg.dir == RADIO_DIR_TX ? "TX" : "RX");
    } else {
        gpio_set_level(LORA_PWR, 0);
        espnow_init(espnow_rx_handler);
        espnow_set_peer((uint8_t *)radio_cfg.peer_mac);
        ESP_LOGI(TAG, "Aktywna: ESP-NOW, dir=%s, peer=" MACSTR,
                 radio_cfg.dir == RADIO_DIR_TX ? "TX" : "RX",
                 MAC2STR(radio_cfg.peer_mac));
    }
}