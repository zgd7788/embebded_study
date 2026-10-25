#ifndef TASK21_PARSER_INCLUDED
#define TASK21_PARSER_INCLUDED

#include <stddef.h>
#include <stdint.h>

#include "task21_frame.h"

typedef struct
{
    uint8_t bytes[12];
    size_t used;
    size_t expected;
}frame_parser_t;

void frame_parser_init(frame_parser_t *parser);
int frame_parser_push(frame_parser_t *parser, uint8_t byte, frame_t *frame_out);

#endif