#ifndef CS42L51_H
#define CS42L51_H

#include "i2c.h"

typedef struct
{
    i2c_t *i2c;
    uint8_t addr7;
} cs42l51_t;


int CS42L51_ReadReg(const cs42l51_t *dev, uint8_t reg,
                   uint8_t *value_out, uint32_t timeout_ticks);

int CS42L51_WriteReg(const cs42l51_t *dev, uint8_t reg,
                    uint8_t value, uint32_t timeout_ticks);

int CS42L51_UpdateBits(const cs42l51_t *dev, uint8_t reg,
                       uint8_t mask, uint8_t value,
                       uint32_t timeout_ticks);

#endif

