#pragma once

#include "esp_err.h"

/* Dados do sensor */
typedef struct {
    float accel_x;   // g
    float accel_y;   // g
    float accel_z;   // g
    float pitch;     // graus
    float roll;      // graus
} mpu6050_t;

/* Inicializa I2C + MPU6050 */
esp_err_t mpu6050_init(void);

/* Lê acelerômetro e calcula pitch/roll */
esp_err_t mpu6050_read(mpu6050_t *imu);
