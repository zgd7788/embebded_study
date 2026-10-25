#ifndef TASK21_PROTOCOL_INCLUDED
#define TASK21_PROTOCOL_INCLUDED

#include "task21_ring.h"
#include "task21_parser.h"

typedef struct {
    rx_ring_t ring;
    frame_parser_t parser;
    uint32_t last_byte_ms;
    uint32_t timeout_ms;
} protocol_rx_t;

protocol_rx_t rx = {
    .timeout_ms = 20U
};

int protocol_rx_process(protocol_rx_t *rx, frame_t *frame_out);

#endif
