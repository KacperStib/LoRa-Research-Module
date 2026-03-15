#include "include/shell_mng.h"

#include <string.h>
#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_console.h"

// ─── Handler komendy "log" ────────────────────────────────────
static int cmd_log(int argc, char **argv)
{
    if (argc < 2) {
        printf("Uzycie: log <poziom> [tag]\n");
        printf("Poziomy: none  err  wrn  inf  dbg\n");
        printf("Tag:     opcjonalny, domyslnie * (wszystkie)\n");
        return 1;
    }

    esp_log_level_t level;

    if      (strcmp(argv[1], "none") == 0) level = ESP_LOG_NONE;
    else if (strcmp(argv[1], "err")  == 0) level = ESP_LOG_ERROR;
    else if (strcmp(argv[1], "wrn")  == 0) level = ESP_LOG_WARN;
    else if (strcmp(argv[1], "inf")  == 0) level = ESP_LOG_INFO;
    else if (strcmp(argv[1], "dbg")  == 0) level = ESP_LOG_DEBUG;
    else {
        printf("Nieznany poziom: '%s'\n", argv[1]);
        printf("Dostepne: none  err  wrn  inf  dbg\n");
        return 1;
    }

    // Jezeli podano tag — uzyj go, w przeciwnym razie "*" (wszystkie)
    const char *tag = (argc >= 3) ? argv[2] : "*";

    esp_log_level_set(tag, level);
    printf("Log [%s] ustawiony na: %s\n", tag, argv[1]);
    return 0;
}

// ─── Handler komendy "radio" ──────────────────────────────────
static int cmd_radio(int argc, char **argv)
{
    if (argc < 2) {
        printf("Tryb: %s\n", radio_mode ? "TX" : "RX");
        return 0;
    }

    if (strcmp(argv[1], "tx") == 0) {
        radio_mode = true;
        printf("Tryb ustawiony: TX\n");
    } else if (strcmp(argv[1], "rx") == 0) {
        radio_mode = false;
        printf("Tryb ustawiony: RX\n");
    } else {
        printf("Nieznany tryb: '%s'  (dostepne: rx  tx)\n", argv[1]);
        return 1;
    }
    return 0;
}

void shell_init(void)
{
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_cfg = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_cfg.prompt = "esp> ";
    repl_cfg.max_cmdline_length = 64;

    esp_console_dev_usb_serial_jtag_config_t usb_cfg =
        ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_console_new_repl_usb_serial_jtag(
        &usb_cfg, &repl_cfg, &repl));

    console_log_register();
    console_radio_register();

    ESP_ERROR_CHECK(esp_console_start_repl(repl));
}

// ─── Rejestracja ──────────────────────────────────────────────
void console_log_register(void)
{
    ESP_ERROR_CHECK(esp_console_cmd_register(&(esp_console_cmd_t){
        .command = "log",
        .help    = "Ustaw poziom logow: log <none|err|wrn|inf|dbg> [tag]",
        .hint    = "<none|err|wrn|inf|dbg> [tag]",
        .func    = cmd_log,
    }));
}

void console_radio_register(void)
{
    ESP_ERROR_CHECK(esp_console_cmd_register(&(esp_console_cmd_t){
        .command = "radio",
        .help    = "Ustaw tryb LoRa: radio <rx|tx>  (bez argumentu = pokaz aktualny)",
        .hint    = "<rx|tx>",
        .func    = cmd_radio,
    }));
}