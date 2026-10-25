#ifndef TASK21_CRC_INCLUDED
#define TASK21_CRC_INCLUDED

#include <stddef.h>
#include <stdint.h>

int crc8_atm(const uint8_t *data, size_t length, uint8_t *crc_out);

#endif