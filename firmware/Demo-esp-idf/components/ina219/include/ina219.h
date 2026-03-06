#ifndef INA219_H
#define INA219_H

#include "i2c.h"
#include <math.h>

#define INA219_ADDR (0x40)
#define INA219_REG_CONFIG (0x00)
#define INA219_REG_SHUNTVOLTAGE (0x01)
#define INA219_REG_BUSVOLTAGE (0x02)
#define INA219_REG_POWER (0x03)
#define INA219_REG_CURRENT (0x04)
#define INA219_REG_CALIBRATION (0x05)

extern uint8_t ina_range;
extern uint8_t ina_gain;
extern uint8_t ina_b_res; 
extern uint8_t ina_s_res;
extern uint8_t ina_mode;

extern float currentLSB, powerLSB;

esp_err_t ina219_power_on(float shuntVAL, float iMAX);
float ina219_read_voltage();
float ina219_read_current();
float ina219_read_power();

#endif