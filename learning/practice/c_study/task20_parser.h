#ifndef TASK20_PARSER_INCLUDED
#define TASK20_PARSER_INCLUDED

#include <stdint.h>
#include <stddef.h>
typedef struct 
{
    uint8_t command;
    uint8_t payload_length;
    uint8_t payload[8];
}frame_t;

typedef struct 
{
    uint8_t bytes[12];
    size_t used;
    size_t expected;
} frame_parser_t;

void frame_parser_init(frame_parser_t *parser);
int frame_parser_push(frame_parser_t *parser, uint8_t byte, frame_t *frame_out);

#endif
