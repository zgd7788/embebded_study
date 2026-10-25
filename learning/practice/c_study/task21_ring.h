#ifndef TASK21_RING_INCLUDED
#define TASK21_RING_INCLUDED

#include <stddef.h>
#include <stdint.h>
#include "task21_parser.h"

typedef struct {
    uint8_t *data;
    size_t capacity;
    size_t read_pos;
    size_t write_pos;
    size_t count;
} rx_ring_t;

int rx_ring_init(rx_ring_t *ring, uint8_t *storage, size_t capacity);
int rx_ring_push(rx_ring_t *ring, uint8_t byte);
int rx_ring_pop(rx_ring_t *ring, uint8_t *byte_out);

#endif