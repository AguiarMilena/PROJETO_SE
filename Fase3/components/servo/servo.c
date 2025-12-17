#include "servo.h"
#include "driver/ledc.h"
#include <math.h>

/* PWM do servo */
#define SERVO_MIN_US 600
#define SERVO_MAX_US 2400
#define SERVO_FREQ   50

/* Suavização */
#define SERVO_STEP 0.02f   // ajuste fino aqui (0.01 bem suave, 0.05 rápido)

/* Estado atual */
static float cur_x = 0.0f;
static float cur_y = 0.0f;

/* ===================== Conversão ===================== */

static uint32_t norm_to_duty(float v)
{
    if (v >  1.0f) v =  1.0f;
    if (v < -1.0f) v = -1.0f;

    float t = (v + 1.0f) * 0.5f;
    float pulse = SERVO_MIN_US + t * (SERVO_MAX_US - SERVO_MIN_US);

    return (uint32_t)((pulse / 20000.0f) * 8192.0f);
}

/* ===================== Init ===================== */

void servo_init(const servo_config_t *cfg)
{
    ledc_timer_config_t timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_13_BIT,
        .freq_hz          = SERVO_FREQ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t ch_x = {
        .channel    = LEDC_CHANNEL_0,
        .gpio_num   = cfg->gpio_x,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = norm_to_duty(0.0f)
    };

    ledc_channel_config_t ch_y = {
        .channel    = LEDC_CHANNEL_1,
        .gpio_num   = cfg->gpio_y,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = norm_to_duty(0.0f)
    };

    ledc_channel_config(&ch_x);
    ledc_channel_config(&ch_y);

    /* Começa centralizado */
    cur_x = 0.0f;
    cur_y = 0.0f;
}

/* ===================== Set direto ===================== */

void servo_set_norm_x(float v)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, norm_to_duty(v));
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void servo_set_norm_y(float v)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, norm_to_duty(v));
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
}

/* ===================== Suavização ===================== */

static float smooth_step(float current, float target)
{
    if (target > current + SERVO_STEP)
        return current + SERVO_STEP;

    if (target < current - SERVO_STEP)
        return current - SERVO_STEP;

    return target;
}

void servo_set_smooth(float x, float y)
{
    /* Limites */
    if (x >  1.0f) x =  1.0f;
    if (x < -1.0f) x = -1.0f;
    if (y >  1.0f) y =  1.0f;
    if (y < -1.0f) y = -1.0f;

    /* Suaviza */
    cur_x = smooth_step(cur_x, x);
    cur_y = smooth_step(cur_y, y);

    /* Aplica */
    servo_set_norm_x(cur_x);
    servo_set_norm_y(cur_y);
}
