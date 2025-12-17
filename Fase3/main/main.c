#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "joystick.h"
#include "servo.h"
#include "mpu6050.h"

/* ===================== Estado Global ===================== */

typedef struct {
    float joy_x;
    float joy_y;
    float pitch;
    float roll;
} system_state_t;

static system_state_t state;
static SemaphoreHandle_t state_mutex;

/* ===================== Task 1 – Joystick ===================== */

static void task_joystick(void *arg)
{
    joystick_config_t joy_cfg = {
        .gpio_x = 1,
        .gpio_y = 2
    };

    joystick_init(&joy_cfg);
    joystick_calibrate(2000);

    float x, y;

    while (1) {
        joystick_read_norm(&x, &y);

        xSemaphoreTake(state_mutex, portMAX_DELAY);
        state.joy_x = x;
        state.joy_y = y;
        xSemaphoreGive(state_mutex);

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/* ===================== Task 2 – Servo ===================== */

static void task_servo(void *arg)
{
    servo_config_t servo_cfg = {
        .gpio_x = 4,
        .gpio_y = 6
    };

    servo_init(&servo_cfg);

    float x, y;

    while (1) {
        xSemaphoreTake(state_mutex, portMAX_DELAY);
        x = state.joy_x;
        y = state.joy_y;
        xSemaphoreGive(state_mutex);

        servo_set_smooth(x, y);  // ✅ FUNÇÃO CORRETA

        vTaskDelay(pdMS_TO_TICKS(20)); // 50 Hz
    }
}


/* ===================== Task 3 – MPU6050 ===================== */

static void task_mpu6050(void *arg)
{
    mpu6050_t imu;

    mpu6050_init();

    while (1) {
        if (mpu6050_read(&imu) == ESP_OK) {
            xSemaphoreTake(state_mutex, portMAX_DELAY);
            state.pitch = imu.pitch;
            state.roll  = imu.roll;
            xSemaphoreGive(state_mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/* ===================== Task 4 – Monitor (UART / JSON) ===================== */

static void task_monitor(void *arg)
{
    char json[128];

    while (1) {
        xSemaphoreTake(state_mutex, portMAX_DELAY);

        snprintf(json, sizeof(json),
            "{\"joy_x\":%.2f,\"joy_y\":%.2f,\"pitch\":%.2f,\"roll\":%.2f}\n",
            state.joy_x,
            state.joy_y,
            state.pitch,
            state.roll
        );

        xSemaphoreGive(state_mutex);

        printf("%s", json);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* ===================== app_main ===================== */

void app_main(void)
{
    state_mutex = xSemaphoreCreateMutex();

    xTaskCreate(task_joystick, "task_joystick", 4096, NULL, 5, NULL);
    xTaskCreate(task_servo,    "task_servo",    4096, NULL, 5, NULL);
    xTaskCreate(task_mpu6050,  "task_mpu6050",  4096, NULL, 4, NULL);
    xTaskCreate(task_monitor, "task_monitor",  4096, NULL, 3, NULL);
}
