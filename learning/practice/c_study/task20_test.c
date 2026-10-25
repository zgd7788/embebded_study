#include "task20_parser.h"
#include <stdio.h>
#include <string.h>

static int check(int condition, const char *name)
{
    if (!condition)
    {
        printf("FAIL: %s\n", name);
        return 0;
    }

    printf("PASS: %s\n", name);
    return 1;
}

static int frame_matches(const frame_t *frame,
                         uint8_t command,
                         const uint8_t *payload,
                         uint8_t payload_length)
{
    return frame->command == command &&
           frame->payload_length == payload_length &&
           (payload_length == 0u ||
            memcmp(frame->payload, payload, payload_length) == 0);
}

int main(void)
{
    int failures = 0;
    frame_parser_t parser;
    frame_t frame = {
        .command = 0xEEu,
        .payload_length = 7u,
        .payload = {0xEEu}
    };
    const uint8_t first_frame[] = {
        0xAAu, 0x02u, 0x10u, 0x01u, 0x02u, 0x15u
    };
    const uint8_t empty_payload_frame[] = {
        0xAAu, 0x00u, 0x20u, 0x20u
    };

    frame_parser_init(&parser);
    failures += !check(frame_parser_push(NULL, 0xAAu, &frame) == -1,
                       "NULL parser");
    failures += !check(frame_parser_push(&parser, 0xAAu, NULL) == -1,
                       "NULL frame output");
    frame_parser_init(NULL);
    failures += !check(1, "init accepts NULL");

    failures += !check(frame_parser_push(&parser, 0x55u, &frame) == 0 &&
                       parser.used == 0u,
                       "ignore garbage before SOF");

    for (size_t i = 0; i < 3u; i++)
    {
        failures += !check(frame_parser_push(&parser, first_frame[i],
                                            &frame) == 0,
                           "partial frame produces no output");
    }
    failures += !check(parser.used == 3u && frame.command == 0xEEu,
                       "partial frame state is retained");

    int result = 0;
    for (size_t i = 3u; i < sizeof(first_frame); i++)
    {
        result = frame_parser_push(&parser, first_frame[i], &frame);
    }
    failures += !check(result == 1 &&
                       frame_matches(&frame, 0x10u,
                                     (const uint8_t[]){0x01u, 0x02u}, 2u),
                       "complete split frame");
    failures += !check(parser.used == 0u && parser.expected == 0u,
                       "parser resets after valid frame");

    {
        const uint8_t stream[] = {
            0xAAu, 0x02u, 0x10u, 0x01u, 0x02u, 0x15u,
            0xAAu, 0x00u, 0x20u, 0x20u
        };
        frame_t output[2] = {{0}};
        size_t output_count = 0u;
        int stream_ok = 1;

        frame_parser_init(&parser);
        for (size_t i = 0; i < sizeof(stream); i++)
        {
            result = frame_parser_push(&parser, stream[i], &frame);
            if (result == 1)
            {
                if (output_count >= 2u)
                {
                    stream_ok = 0;
                }
                else
                {
                    output[output_count++] = frame;
                }
            }
            else if (result != 0)
            {
                stream_ok = 0;
            }
        }

        failures += !check(stream_ok && output_count == 2u &&
                           frame_matches(&output[0], 0x10u,
                                         (const uint8_t[]){0x01u, 0x02u}, 2u) &&
                           frame_matches(&output[1], 0x20u, NULL, 0u),
                           "two concatenated frames");
    }

    frame_parser_init(&parser);
    result = 0;
    for (size_t i = 0; i < sizeof(empty_payload_frame); i++)
    {
        result = frame_parser_push(&parser, empty_payload_frame[i], &frame);
    }
    failures += !check(result == 1 &&
                       frame_matches(&frame, 0x20u, NULL, 0u),
                       "zero-length payload frame");

    {
        const uint8_t maximum_frame[] = {
            0xAAu, 0x08u, 0x30u, 1u, 2u, 3u,
            4u, 5u, 6u, 7u, 8u, 0x5Cu
        };
        const uint8_t maximum_payload[] = {
            1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u
        };

        frame_parser_init(&parser);
        result = 0;
        for (size_t i = 0; i < sizeof(maximum_frame); i++)
        {
            result = frame_parser_push(&parser, maximum_frame[i], &frame);
        }
        failures += !check(result == 1 &&
                           frame_matches(&frame, 0x30u, maximum_payload, 8u),
                           "maximum payload frame");
    }

    frame_parser_init(&parser);
    failures += !check(frame_parser_push(&parser, 0xAAu, &frame) == 0 &&
                       frame_parser_push(&parser, 9u, &frame) == -2 &&
                       parser.used == 0u,
                       "reject invalid LEN and reset");
    result = 0;
    for (size_t i = 0; i < sizeof(empty_payload_frame); i++)
    {
        result = frame_parser_push(&parser, empty_payload_frame[i], &frame);
    }
    failures += !check(result == 1 && frame.command == 0x20u,
                       "recover after invalid LEN");

    {
        const uint8_t bad_frame[] = {
            0xAAu, 0x02u, 0x10u, 0x01u, 0x02u, 0x14u
        };
        frame_parser_init(&parser);
        result = 0;
        for (size_t i = 0; i < sizeof(bad_frame); i++)
        {
            result = frame_parser_push(&parser, bad_frame[i], &frame);
        }
        failures += !check(result == -2 && parser.used == 0u,
                           "reject bad checksum and reset");

        result = 0;
        for (size_t i = 0; i < sizeof(first_frame); i++)
        {
            result = frame_parser_push(&parser, first_frame[i], &frame);
        }
        failures += !check(result == 1 && frame.command == 0x10u,
                           "recover after bad checksum");
    }

    printf("%s\n", failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED");
    return failures == 0 ? 0 : 1;
}
