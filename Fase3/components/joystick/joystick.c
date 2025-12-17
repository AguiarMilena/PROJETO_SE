#include "joystick.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>

static adc_oneshot_unit_handle_t adc;
static adc_channel_t ch_x, ch_y;

/* Calibração */
static int xmin = 4095, xmax = 0, xcen = 2048;
static int ymin = 4095, ymax = 0, ycen = 2048;

/* ESP32-S3 ADC1 GPIO1..10 */
static adc_channel_t gpio_to_adc(int gpio)
{
    switch (gpio) {
        case 1:  return ADC_CHANNEL_0;
        case 2:  return ADC_CHANNEL_1;
        case 3:  return ADC_CHANNEL_2;
        case 4:  return ADC_CHANNEL_3;
        case 5:  return ADC_CHANNEL_4;
        case 6:  return ADC_CHANNEL_5;
        case 7:  return ADC_CHANNEL_6;
        case 8:  return ADC_CHANNEL_7;
        case 9:  return ADC_CHANNEL_8;
        case 10: return ADC_CHANNEL_9;
        default:
            ESP_LOGE("JOYSTICK", "GPIO %d nao suportado no ADC1", gpio);
            return ADC_CHANNEL_0; // fallback seguro
    }
}

void joystick_init(const joystick_config_t *cfg)
{
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1
    };
    adc_oneshot_new_unit(&unit_cfg, &adc);

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12
    };

    ch_x = gpio_to_adc(cfg->gpio_x);
    ch_y = gpio_to_adc(cfg->gpio_y);

    adc_oneshot_config_channel(adc, ch_x, &chan_cfg);
    adc_oneshot_config_channel(adc, ch_y, &chan_cfg);
}

void joystick_calibrate(uint32_t ms)
{
    int rx, ry;
    xmin = ymin = 4095;
    xmax = ymax = 0;

    TickType_t start = xTaskGetTickCount();

    while (xTaskGetTickCount() - start < pdMS_TO_TICKS(ms)) {
        adc_oneshot_read(adc, ch_x, &rx);
        adc_oneshot_read(adc, ch_y, &ry);

        if (rx < xmin) xmin = rx;
        if (rx > xmax) xmax = rx;
        if (ry < ymin) ymin = ry;
        if (ry > ymax) ymax = ry;
    }

    /* Centro = média de leituras estáveis */
    int sx = 0, sy = 0, n = 50;
    for (int i = 0; i < n; i++) {
        adc_oneshot_read(adc, ch_x, &rx);
        adc_oneshot_read(adc, ch_y, &ry);
        sx += rx;
        sy += ry;
    }

    xcen = sx / n;
    ycen = sy / n;

    /* Fallback de segurança */
    if ((xmax - xmin) < 500)
        xcen = (xmin + xmax) / 2;

    if ((ymax - ymin) < 500)
        ycen = (ymin + ymax) / 2;
}

/* NORMALIZAÇÃO CORRIGIDA */
static float normalize(int raw, int min, int center, int max)
{
    if (raw > center)
        raw = (raw > max) ? max : raw;
    else
        raw = (raw < min) ? min : raw;

    float v;
    if (raw >= center)
        v = (float)(raw - center) / (float)(max - center);
    else
        v = (float)(raw - center) / (float)(center - min);

    /* zona morta */
    if (fabsf(v) < 0.08f)
        v = 0.0f;

    if (v > 1.0f) v = 1.0f;
    if (v < -1.0f) v = -1.0f;

    return v;
}

void joystick_read_norm(float *x, float *y)
{
    int rx, ry;
    adc_oneshot_read(adc, ch_x, &rx);
    adc_oneshot_read(adc, ch_y, &ry);

    *x = normalize(rx, xmin, xcen, xmax);
    *y = normalize(ry, ymin, ycen, ymax);
}
