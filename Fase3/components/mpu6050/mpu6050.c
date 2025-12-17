#include "mpu6050.h"

#include <math.h>
#include "driver/i2c.h"

#define MPU6050_ADDR      0x68
#define REG_PWR_MGMT_1    0x6B
#define REG_ACCEL_XOUT_H  0x3B

#define I2C_PORT I2C_NUM_0
#define SDA_PIN  8
#define SCL_PIN  18
#define I2C_FREQ 100000

static esp_err_t i2c_write(uint8_t reg, uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1), true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t i2c_read(uint8_t reg, uint8_t *buf, size_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1), true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | 1, true);
    i2c_master_read(cmd, buf, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t mpu6050_init(void)
{
    i2c_config_t cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQ
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_PORT, &cfg));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_PORT, cfg.mode, 0, 0, 0));

    // Wake up MPU6050
    return i2c_write(REG_PWR_MGMT_1, 0x00);
}

esp_err_t mpu6050_read(mpu6050_t *imu)
{
    uint8_t raw[6];

    if (i2c_read(REG_ACCEL_XOUT_H, raw, 6) != ESP_OK)
        return ESP_FAIL;

    int16_t ax = (raw[0] << 8) | raw[1];
    int16_t ay = (raw[2] << 8) | raw[3];
    int16_t az = (raw[4] << 8) | raw[5];

    imu->accel_x = ax / 16384.0f;
    imu->accel_y = ay / 16384.0f;
    imu->accel_z = az / 16384.0f;

    imu->roll  = atan2f(imu->accel_y, imu->accel_z) * 180.0f / M_PI;
    imu->pitch = atan2f(-imu->accel_x,
                         sqrtf(imu->accel_y * imu->accel_y +
                               imu->accel_z * imu->accel_z))
                 * 180.0f / M_PI;

    return ESP_OK;
}
