#ifndef TASK21_FRAME_INCLUDED
#define TASK21_FRAME_INCLUDED

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    uint8_t command;
    uint8_t payload_length;
    uint8_t payload[8];
}frame_t;

int frame_encode(uint8_t *out,
                 size_t out_capacity,
                 uint8_t command,
                 const uint8_t *payload,
                 uint8_t payload_length,
                 size_t *frame_length_out);

#endif