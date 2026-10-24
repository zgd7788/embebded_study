#ifndef TASK19_FRAME_INCLUDED
#define TASK19_FRAME_INCLUDED

#include <stdint.h>
#include <stddef.h>

int frame_checksum(const uint8_t *data, size_t length, uint8_t *checksum_out);

int frame_encode(uint8_t *out,
                 size_t out_capacity,
                 uint8_t command,
                 const uint8_t *payload,
                 uint8_t payload_length,
                 size_t *frame_length_out);

#endif
