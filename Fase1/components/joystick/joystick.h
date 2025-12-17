#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdint.h>

typedef struct {
    int gpio_x;
    int gpio_y;
} joystick_config_t;

void joystick_init(const joystick_config_t *cfg);
void joystick_calibrate(uint32_t ms);
void joystick_read_norm(float *x, float *y);

#endif
