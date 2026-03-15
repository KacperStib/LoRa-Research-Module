#include "ina219.h"

// Parametry do konfiguracji - podstawowe parametry
uint8_t ina_range = 0b1; // 32 V
uint8_t ina_gain = 0b11; // 320 mV
uint8_t ina_b_res = 0b0011; // 12 bit
uint8_t ina_s_res = 0b0011; // 12 bit 1S
uint8_t ina_mode = 0b111; // shunt and bus cont

float currentLSB = 0, powerLSB = 0;

esp_err_t ina219_power_on(float shuntVAL, float iMAX){
	// konfiguracja
	uint16_t config = 0;
	config |= (ina_range << 13 | ina_gain << 11 | ina_b_res << 7 | ina_s_res << 3 | ina_mode);
	// Zapis koniguracji
	err = i2c_write_2byte(INA219_ADDR, INA219_REG_CONFIG, config);
	
	// kalibracja
	uint16_t calibrationValue;

    float iMaxPossible, minimumLSB;
	// Obliczenie maksymalnego możliwego prądu dla danej rezystancji shunt
    iMaxPossible = 0.32f / shuntVAL;
    minimumLSB = iMAX / 32767;
	
	// While this value yields the highest resolution, it is common to select a value for
	// the Current_LSB to the nearest round number above this value to simplify the conversion of the Current Register
	// (04h) and Power Register (03h) to amperes and watts respectively
    
    // Mnozniki surowej wartosci z rejestrow
	currentLSB = ceil(minimumLSB / 0.0001f) * 0.0001f;
    powerLSB = currentLSB * 20;
	// Obliczenie wartości kalibracyjnej zgodnie ze wzorem z dokumentacji
    calibrationValue = (uint16_t)((0.04096) / (currentLSB * shuntVAL));
	// Zapis kalibracji
	err = i2c_write_2byte(INA219_ADDR, INA219_REG_CALIBRATION, calibrationValue);

    return err;
}

// Prosty odczyt napiecia
float ina219_read_voltage(){
	uint8_t buf[2];
	i2c_write_reg(INA219_ADDR, INA219_REG_BUSVOLTAGE);
	i2c_read(INA219_ADDR, buf, 2);
	// value = MSB * 256 + LSB
	uint16_t voltage = (uint16_t)buf[0] * 256 + (uint16_t)buf[1];
	// Shift to the right 3 to drop CNVR and OVF and multiply by LSB
  	return ((voltage >> 3) * 4 * 0.001);
}

// Prosty odzczyt pradu
float ina219_read_current(){
	uint8_t buf[2];
	i2c_write_reg(INA219_ADDR, INA219_REG_CURRENT);
	i2c_read(INA219_ADDR, buf, 2);
	// value = MSB * 256 + LSB
	float current = (uint16_t)buf[0] * 256 + (uint16_t)buf[1];
	current = current * currentLSB;
	ESP_LOGD(TAG_PWR,"CURRENT: %.2f", current);
	return current;
}

// Prostu odczyt mocy
float ina219_read_power(){
	uint8_t buf[2];
	i2c_write_reg(INA219_ADDR, INA219_REG_POWER);
	i2c_read(INA219_ADDR, buf, 2);
	// value = MSB * 256 + LSB
	float power = (uint16_t)buf[0] * 256 + (uint16_t)buf[1];
	power = power * powerLSB;
	ESP_LOGD(TAG_PWR,"POWER: %.2f", power);
	return power;
}