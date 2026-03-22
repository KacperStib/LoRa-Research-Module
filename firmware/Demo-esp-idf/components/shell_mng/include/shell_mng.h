#ifndef SHELL_MNG_H
#define SHELL_MNG_H

#include "dev_config.h"
#include "esp_now_c.h"
#include "lora.h"
#include "nvs_storage.h"

// w sdkconfig ustawic esp_console jakby zmieniac na UART a nie USB

/**
 * @brief Rejestruje komendy konsolowe do zarzadzania poziomem logow.
 *
 * Dostepne komendy:
 *   log none    [tag]
 *   log err     [tag]
 *   log wrn     [tag]
 *   log inf     [tag]
 *   log dbg     [tag]
 *
 * Parametr [tag] jest opcjonalny — domyslnie "*" (wszystkie tagi).
 */

void shell_init(void);
void console_log_register(void);
void console_radio_register(void);

#endif