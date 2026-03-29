#include <stdio.h>
#include "include/dev_config.h"

static const char *TAG = "radio_ctrl";

volatile radio_config_t radio_cfg = {
    .tech = RADIO_TECH_LORA,
    .dir  = RADIO_DIR_RX,
    .lora = {
        .sf  = 7,
        .bw  = 7,
        .cr  = 1,
        .pwr = 17,
    },
};

void radio_apply_config(void)
{
    if (radio_cfg.tech == RADIO_TECH_LORA) {
        espnow_deinit();
        lora_init();
        lora_set_frequency(433e6);
        lora_set_coding_rate(radio_cfg.lora.cr);
        lora_set_bandwidth(radio_cfg.lora.bw);
        lora_set_spreading_factor(radio_cfg.lora.sf);
        lora_set_tx_power(radio_cfg.lora.pwr);
           ESP_LOGI(TAG, "LORA SF%d BW%d CR%d PWR%d dir=%s",
                 radio_cfg.lora.sf, radio_cfg.lora.bw,
                 radio_cfg.lora.cr, radio_cfg.lora.pwr,
                 radio_cfg.dir == RADIO_DIR_TX ? "TX" : "RX");
    } else {
        gpio_set_level(LORA_PWR, 1);
        espnow_init(espnow_rx_handler);
        espnow_set_peer((uint8_t *)radio_cfg.peer_mac);
        ESP_LOGI(TAG, "ESP-NOW, dir=%s, peer=" MACSTR,
                 radio_cfg.dir == RADIO_DIR_TX ? "TX" : "RX",
                 MAC2STR(radio_cfg.peer_mac));
    }
}