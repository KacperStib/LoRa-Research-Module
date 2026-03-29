#ifndef DEV_CONFIG_H
#define DEV_CONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include "lora.h"#include "esp_now_c.h"
#include "esp_log.h"

typedef enum {
    RADIO_TECH_LORA   = 0,
    RADIO_TECH_ESPNOW = 1,
} radio_tech_t;

typedef enum {
    RADIO_DIR_RX = 0,
    RADIO_DIR_TX = 1,
} radio_dir_t;

typedef struct {
    uint8_t sf;
    uint8_t bw;
    uint8_t cr;
    uint8_t pwr;
} lora_config_t;

// Główna struktura konfiguracyjna — będzie szła do NVS
typedef struct {
    radio_tech_t tech;
    radio_dir_t  dir;
    uint8_t      peer_mac[6];
    // tu później częstotliwość LoRa, MAC ESP-NOW, itp.
    lora_config_t lora;
} radio_config_t;

extern volatile radio_config_t radio_cfg;

void radio_apply_config(void);

#endif