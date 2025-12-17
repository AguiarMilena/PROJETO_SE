#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "joystick.h"
#include "servo.h"

/* ===================== Estado Global ===================== */

typedef struct {
    float joy_x;
    float joy_y;
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
    joystick_calibrate(3000);

    float x, y;

    while (1) {
        joystick_read_norm(&x, &y);

        xSemaphoreTake(state_mutex, portMAX_DELAY);
        state.joy_x = x;
        state.joy_y = y;
        xSemaphoreGive(state_mutex);

        vTaskDelay(pdMS_TO_TICKS(20)); // 50 Hz
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

        servo_set_smooth(x, -y); // mantém inversão do eixo Y

        vTaskDelay(pdMS_TO_TICKS(20)); // 50 Hz
    }
}

/* ===================== Task 3 – Monitor (UART / JSON) ===================== */

static void task_monitor(void *arg)
{
    char json[96];
    int seq = 0;

    while (1) {
        xSemaphoreTake(state_mutex, portMAX_DELAY);

        snprintf(json, sizeof(json),
            "{\"seq\":%d,\"joy_x\":%.2f,\"joy_y\":%.2f}\n",
            seq++,
            state.joy_x,
            state.joy_y
        );

        xSemaphoreGive(state_mutex);

        printf("%s", json);

        vTaskDelay(pdMS_TO_TICKS(200)); // 5 Hz
    }
}

/* ===================== app_main ===================== */

void app_main(void)
{
    state_mutex = xSemaphoreCreateMutex();

    xTaskCreate(task_joystick, "task_joystick", 4096, NULL, 5, NULL);
    xTaskCreate(task_servo,    "task_servo",    4096, NULL, 5, NULL);
    xTaskCreate(task_monitor, "task_monitor",  4096, NULL, 3, NULL);
}
