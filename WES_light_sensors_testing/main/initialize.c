#include <stdio.h>
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_err.h"

// VEML7700 I2C address and register definitions
#define VEML7700_ADDR 0x10 // Default I2C address for VEML7700
#define VEML7700_ALS_CONF 0x00
#define VEML7700_ALS_DATA 0x04

#define I2C_MASTER_SCL_IO 21      /*!< GPIO number for I2C master clock */
#define I2C_MASTER_SDA_IO 22      /*!< GPIO number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0  /*!< I2C port number for master device */
#define I2C_MASTER_FREQ_HZ 100000 /*!< I2C master clock frequency */

// I2C Configuration
esp_err_t i2c_master_init()
{
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    return i2c_param_config(I2C_MASTER_NUM, &conf);
}

// I2C write function
esp_err_t i2c_write_byte(uint8_t reg_addr, uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (VEML7700_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// I2C read function
esp_err_t i2c_read_bytes(uint8_t reg_addr, uint8_t *data, size_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (VEML7700_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (VEML7700_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

void app_main(void)
{
    // Initialize I2C
    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE("I2C", "I2C initialization failed");
        return;
    }

    uint8_t als_data[2]; // Array to store ALS data
    while (1)
    {
        // Read ALS data from VEML7700
        ret = i2c_read_bytes(VEML7700_ALS_DATA, als_data, 2);
        if (ret != ESP_OK)
        {
            ESP_LOGE("I2C", "Failed to read ALS data");
            return;
        }

        // Combine the 2 bytes to form the 16-bit lux value
        uint16_t lux = ((uint16_t)als_data[0] << 8) | als_data[1];

        // Output based on lux value
        if (lux < 500)
        {
            printf("Lux value: %d -> Output: 1\n", lux);
        }
        else
        {
            printf("Lux value: %d -> Output: 0\n", lux);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay 1 second before reading again
    }
}
