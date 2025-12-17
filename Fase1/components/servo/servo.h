#ifndef SERVO_H
#define SERVO_H

typedef struct {
    int gpio_x;
    int gpio_y;
} servo_config_t;

/* Inicializa os servos */
void servo_init(const servo_config_t *cfg);

/* Set direto (baixo nível) */
void servo_set_norm_x(float v);
void servo_set_norm_y(float v);

/* ⭐ Set com suavização (USAR NO PROJETO) */
void servo_set_smooth(float x, float y);

#endif
