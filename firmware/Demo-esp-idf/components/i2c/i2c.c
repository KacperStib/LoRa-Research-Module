#include "include/i2c.h"

// infomracja czy odczyt czy zapis przez mastera jest w LSB adresu slavea

esp_err_t err = ESP_OK;

// inicjalizacja
esp_err_t i2c_master_init(void)
{
	// konfiguracja magistrali
	int i2c_master_port = I2C_MASTER_NUM;
	i2c_config_t conf = {
	.mode = I2C_MODE_MASTER,
	.sda_io_num = I2C_MASTER_SDA_IO,
	.sda_pullup_en = GPIO_PULLUP_DISABLE,
	.scl_io_num = I2C_MASTER_SCL_IO,
	.scl_pullup_en = GPIO_PULLUP_DISABLE,
	.master.clk_speed = I2C_MASTER_FREQ_HZ,
	// .clk_flags = 0, /*!< Optional, you can use I2C_SCLK_SRC_FLAG_*flags to choose i2c source clock here. */= I2
	};
	
	// sprawdzenie errorow
	err = i2c_param_config(i2c_master_port, &conf);
	if (err != ESP_OK) {
		return err;
	}
	
	// instalacja magistrali
	return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

// zapis jednego bajtu
esp_err_t i2c_write_reg(uint8_t ADDR, uint8_t REG){
	// zabranie zasobow
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	// bit startu
	i2c_master_start(cmd);
	// adres slave'a
	i2c_master_write_byte(cmd, (ADDR << 1) | I2C_MASTER_WRITE, ACK_EN);
	// rejestr
	i2c_master_write_byte(cmd, REG, ACK_EN);
	// bit stopu
	i2c_master_stop(cmd);
	// odlozenie zasobow
	err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
	i2c_cmd_link_delete(cmd);
	return err;
}

// zapis jednego bajtu do rejestru
esp_err_t i2c_write_val(uint8_t ADDR, uint8_t REG, uint8_t VAL){
	// zabranie zasobow
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	// bit startu
	i2c_master_start(cmd);
	// adres slave'a
	i2c_master_write_byte(cmd, (ADDR << 1) | I2C_MASTER_WRITE, ACK_EN);
	// rejestr
	i2c_master_write_byte(cmd, REG, ACK_EN);
	// dane
	i2c_master_write_byte(cmd, VAL, ACK_EN);
	// bit stopu
	i2c_master_stop(cmd);
	// odlozenie zasobow
	err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
	i2c_cmd_link_delete(cmd);
	return err;
}

// odczyt do bufora
esp_err_t i2c_read(uint8_t ADDR, uint8_t *buf, uint8_t bytesToReceive){
	// zabranie zasobow
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	// bit startu
	i2c_master_start(cmd);
	// adres slave'a
	i2c_master_write_byte(cmd, (ADDR << 1) | I2C_MASTER_READ, ACK_EN);
	// odczytaj do bufora
	i2c_master_read(cmd, buf, bytesToReceive, I2C_MASTER_LAST_NACK);
	// NACK
	//i2c_master_read_byte(cmd, buf + bytesToReceive - 1, ACK_DIS);
	// bit stopu
	i2c_master_stop(cmd);
	// odlozenie zasobow 
	err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
	i2c_cmd_link_delete(cmd);
	return err;
}

// zapis dwoch bajtow 
esp_err_t i2c_write_2byte(uint8_t ADDR, uint8_t REG, uint16_t VAL){
	// Podział 2 bajtow uint16_t do przeslania na 2 odzielne bajty uint8_t
	uint8_t bytes[2];
  	bytes[0] = VAL >> 8;
  	bytes[1] = VAL & 0xFF;
	// zabranie zasobow
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	// bit startu
	i2c_master_start(cmd);
	// adres slave'a
	i2c_master_write_byte(cmd, (ADDR << 1) | I2C_MASTER_WRITE, ACK_EN);
	// rejestr
	i2c_master_write_byte(cmd, REG, ACK_EN);
	// dane
	i2c_master_write_byte(cmd, bytes[0], ACK_EN);
	i2c_master_write_byte(cmd, bytes[1], ACK_EN);
	// bit stopu
	i2c_master_stop(cmd);
	// odlozenie zasobow
	err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
	i2c_cmd_link_delete(cmd);
	return err;
}