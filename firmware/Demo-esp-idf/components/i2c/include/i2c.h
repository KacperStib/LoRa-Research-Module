#include <stdio.h>
#include <driver/i2c.h>
#include <esp_err.h>
#include "esp_log.h"

#define I2C_MASTER_SCL_IO           3      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           2       /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                   /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ          100000 /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

#define ACK_EN 1
#define ACK_DIS 0

//extern i2c_cmd_handle_t cmd;
extern esp_err_t err;

// inicjalizacja magistrali
esp_err_t i2c_master_init(void);

// odpytanie rejestru
esp_err_t i2c_write_reg(uint8_t ADDR, uint8_t REG);

// zapis jednego bajtu do rejestru
esp_err_t i2c_write_val(uint8_t ADDR, uint8_t REG, uint8_t VAL);

// odczyt do bufora
esp_err_t i2c_read(uint8_t ADDR, uint8_t *buf, uint8_t bytesToReceive);

// zapis dwoch bajtow 
esp_err_t i2c_write_2byte(uint8_t ADDR, uint8_t REG, uint16_t VAL);