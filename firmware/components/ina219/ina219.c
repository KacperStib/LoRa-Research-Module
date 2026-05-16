#include "ina219.h"

// Globalne zmienne pomiarowe
float current_mA = 0;
float current_mA_peak = 0;

// Podstawowe parametry konfiguracyjne ukladu
uint8_t ina_range = 0b1;     // Zakres napiecia bus: 32 V
uint8_t ina_gain = 0b11;     // Wzmocnienie PGA: 320 mV
uint8_t ina_b_res = 0b0000;  // Rozdzielczosc Bus: 12-bit
uint8_t ina_s_res = 0b0000;  // Rozdzielczosc Shunt: 12-bit, 1 probka
uint8_t ina_mode = 0b111;    // Tryb pracy: ciagly pomiar shunt i bus

float currentLSB = 0, powerLSB = 0;

// Inicjalizacja i kalibracja ukladu INA219
esp_err_t ina219_power_on(float shuntVAL, float iMAX)
{
    // Konfiguracja rejestru CONFIG
    uint16_t config = 0;
    config |= (ina_range << 13 | ina_gain << 11 | ina_b_res << 7 | ina_s_res << 3 | ina_mode);
    
    // Zapis konfiguracji do ukladu
    err = i2c_write_2byte(INA219_ADDR, INA219_REG_CONFIG, config);
	
    // Obliczenia kalibracyjne
    uint16_t calibrationValue;
    float iMaxPossible, minimumLSB;
    
    // Maksymalny mozliwy prad dla podanego bocznikowa (shunt)
    iMaxPossible = 0.32f / shuntVAL;
    minimumLSB = iMAX / 32767;
	
    // Zaokraglenie wartosci Current_LSB do najblizszej "rownej" liczby
    currentLSB = ceil(minimumLSB / 0.0001f) * 0.0001f;
    powerLSB = currentLSB * 20;
    
    // Wyznaczenie wartosci rejestru kalibracji zgodnie ze wzorem z noty katalogowej
    calibrationValue = (uint16_t)((0.04096) / (currentLSB * shuntVAL));
    
    // Zapis wartosci kalibracyjnej do ukladu
    err = i2c_write_2byte(INA219_ADDR, INA219_REG_CALIBRATION, calibrationValue);

    return err;
}

// Odczyt napiecia z szyny 
float ina219_read_voltage(void)
{
    uint8_t buf[2];
    i2c_write_reg(INA219_ADDR, INA219_REG_BUSVOLTAGE);
    i2c_read(INA219_ADDR, buf, 2);
    
    // Konwersja dwoch bajtow na uint16_t (MSB * 256 + LSB)
    uint16_t voltage = (uint16_t)buf[0] * 256 + (uint16_t)buf[1];
    
    // Przesuniecie o 3 bity w prawo (odrzucenie flag CNVR i OVF) oraz skalowanie do woltow
    return ((voltage >> 3) * 4 * 0.001);
}

// Odczyt aktualnego pradu
float ina219_read_current(void)
{
    uint8_t buf[2];
    i2c_write_reg(INA219_ADDR, INA219_REG_CURRENT);
    i2c_read(INA219_ADDR, buf, 2);
    
    // Konwersja bajtow na surowa wartosc pradu
    float current = (uint16_t)buf[0] * 256 + (uint16_t)buf[1];
    current = current * currentLSB;
    
    ESP_LOGD(TAG_PWR, "CURRENT: %.2f", current);
    return current;
}

// Odczyt aktualnego poboru mocy
float ina219_read_power(void)
{
    uint8_t buf[2];
    i2c_write_reg(INA219_ADDR, INA219_REG_POWER);
    i2c_read(INA219_ADDR, buf, 2);
    
    // Konwersja bajtow na surowa wartosc mocy
    float power = (uint16_t)buf[0] * 256 + (uint16_t)buf[1];
    power = power * powerLSB;
    
    ESP_LOGD(TAG_PWR, "POWER: %.2f", power);
    return power;
}

// Przeszukanie profilu pradowego w poszukiwaniu najwyzszego piku
// Predkosc magistrali I2C podniesiona ze 100 kHz do 400 kHz w celu zvwiekszenia czestotliwosci probkowania
float ina219_find_peak(void)
{
    float peak = 0, sample;
    TickType_t t_end = xTaskGetTickCount() + pdMS_TO_TICKS(100);
	
    while (xTaskGetTickCount() < t_end) 
    {
        sample = ina219_read_current();
        if (sample > peak)
        { 
            peak = sample; 
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    
    return peak;
}