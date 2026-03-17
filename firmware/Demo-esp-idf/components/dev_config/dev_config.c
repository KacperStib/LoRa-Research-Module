#include <stdio.h>
#include "include/dev_config.h"

volatile radio_config_t radio_cfg = {
    .tech = RADIO_TECH_LORA,
    .dir  = RADIO_DIR_RX,
};