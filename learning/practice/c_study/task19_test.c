#include "task19_frame.h"
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

static int all_bytes_equal(const uint8_t *data, size_t length, uint8_t value)
{
    for (size_t i = 0; i < length; i++)
    {
        if (data[i] != value)
        {
            return 0;
        }
    }
    return 1;
}

int main(void)
{
    int failures = 0;
    uint8_t checksum = 0xA5u;
    const uint8_t sample[] = {0x03u, 0x20u, 0x05u, 0x02u, 0xFFu};
    const uint8_t overflow_sample[] = {0xFFu, 0x01u};
    uint8_t frame[16];
    size_t frame_length = 999u;

    failures += !check(frame_checksum(sample, sizeof(sample), &checksum) == 0 &&
                       checksum == 0x29u,
                       "checksum example vector");
    failures += !check(frame_checksum(overflow_sample,
                                      sizeof(overflow_sample),
                                      &checksum) == 0 && checksum == 0x00u,
                       "checksum wraps to 8 bits");
    failures += !check(frame_checksum(sample, 0u, &checksum) == 0 &&
                       checksum == 0u,
                       "checksum empty input");
    failures += !check(frame_checksum(NULL, 1u, &checksum) == -1,
                       "checksum NULL data");
    failures += !check(frame_checksum(sample, sizeof(sample), NULL) == -1,
                       "checksum NULL output");

    memset(frame, 0xCC, sizeof(frame));
    frame_length = 999u;
    failures += !check(frame_encode(frame, sizeof(frame), 0x10u,
                                    (const uint8_t[]){0x01u, 0x02u}, 2u,
                                    &frame_length) == 0 &&
                       frame_length == 6u &&
                       memcmp(frame,
                              (const uint8_t[]){0xAAu, 0x02u, 0x10u,
                                                0x01u, 0x02u, 0x15u},
                              6u) == 0,
                       "encode example frame");

    memset(frame, 0xCC, sizeof(frame));
    frame_length = 999u;
    failures += !check(frame_encode(frame, sizeof(frame), 0x20u, NULL, 0u,
                                    &frame_length) == 0 &&
                       frame_length == 4u && frame[0] == 0xAAu &&
                       frame[1] == 0u && frame[2] == 0x20u &&
                       frame[3] == 0x20u,
                       "encode empty payload");

    {
        const uint8_t payload[8] = {
            0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u
        };
        memset(frame, 0xCC, sizeof(frame));
        frame_length = 999u;
        failures += !check(frame_encode(frame, sizeof(frame), 0x30u,
                                        payload, sizeof(payload),
                                        &frame_length) == 0 &&
                           frame_length == 12u && frame[0] == 0xAAu &&
                           frame[1] == 8u && frame[2] == 0x30u &&
                           memcmp(&frame[3], payload, sizeof(payload)) == 0 &&
                           frame[11] == 0x54u,
                           "encode maximum payload");
    }

    memset(frame, 0xCC, sizeof(frame));
    frame_length = 999u;
    failures += !check(frame_encode(frame, sizeof(frame), 0x10u,
                                    (const uint8_t[]){1u}, 9u,
                                    &frame_length) == -1 &&
                       all_bytes_equal(frame, sizeof(frame), 0xCCu) &&
                       frame_length == 999u,
                       "reject payload length above maximum");

    memset(frame, 0xCC, sizeof(frame));
    frame_length = 999u;
    failures += !check(frame_encode(frame, 5u, 0x10u,
                                    (const uint8_t[]){1u, 2u}, 2u,
                                    &frame_length) == -1 &&
                       all_bytes_equal(frame, sizeof(frame), 0xCCu) &&
                       frame_length == 999u,
                       "reject insufficient capacity without writes");

    failures += !check(frame_encode(NULL, sizeof(frame), 0u, NULL, 0u,
                                    &frame_length) == -1,
                       "encode NULL output");
    failures += !check(frame_encode(frame, sizeof(frame), 0u, NULL, 0u,
                                    NULL) == -1,
                       "encode NULL length output");
    failures += !check(frame_encode(frame, sizeof(frame), 0u, NULL, 1u,
                                    &frame_length) == -1,
                       "encode NULL nonempty payload");

    printf("%s\n", failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED");
    return failures == 0 ? 0 : 1;
}
