#ifndef __REG_SIM_H
#define __REG_SIM_H

#include <stdio.h>
#include <stdint.h>

typedef enum
{
    MODE_OFF = 0,
    MODE_RUN,
    MODE_SLEEP,
}MODE_E;

uint32_t reg_set_field(uint32_t reg,
                       uint32_t mask,
                       unsigned int shift,
                       uint32_t value);

uint32_t reg_get_field(uint32_t reg,
                       uint32_t mask,
                       unsigned int shift);

int reg_set_mode(uint32_t *reg, MODE_E mode);
int reg_set_enable(uint32_t *reg, int enable);
int reg_set_irq_enable(uint32_t *reg, int enable);
int reg_set_prescaler(uint32_t *reg, uint32_t value);

#endif
