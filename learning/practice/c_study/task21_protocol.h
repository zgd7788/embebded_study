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


int protocol_rx_init(protocol_rx_t *rx,
                     uint8_t *storage,
                     size_t capacity,
                     uint32_t timeout_ms);
int protocol_rx_process(protocol_rx_t *rx, uint32_t now_ms, frame_t *frame_out);
int protocol_rx_check_timeout(protocol_rx_t *rx, uint32_t now_ms);

#endif
