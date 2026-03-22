#ifndef ESP_NOW_C_H
#define ESP_NOW_C_H

// espnow.h
#pragma once
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_netif.h"
#include "esp_event.h"

extern int rssi;

// Callback wywoływany gdy przyjdzie pakiet ESP-NOW
typedef void (*espnow_rx_cb_t)(const uint8_t *data, int len);

esp_err_t espnow_init(espnow_rx_cb_t rx_callback);
esp_err_t espnow_set_peer(const uint8_t mac[6]);
esp_err_t espnow_send(const uint8_t *data, size_t len);
void      espnow_deinit(void);
void espnow_rx_handler(const uint8_t *data, int len);

#endif