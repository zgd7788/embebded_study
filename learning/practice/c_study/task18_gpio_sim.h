#ifndef TASK18_GPIO_SIM_H_INCLUDED
#define TASK18_GPIO_SIM_H_INCLUDED

#include <stdint.h>

typedef struct
{
    volatile uint32_t control;
    volatile uint32_t status;
} gpio_regs_t;

typedef struct
{
    volatile gpio_regs_t *regs;
} gpio_t;

int gpio_init(gpio_t *gpio, volatile gpio_regs_t *regs);
int gpio_set_enable(gpio_t *gpio, int enable);
int gpio_set_mode(gpio_t *gpio, uint32_t mode);
int gpio_get_ready(const gpio_t *gpio, int *ready_out);

#endif
