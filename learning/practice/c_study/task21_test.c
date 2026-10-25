#include "task21_parser.h"
#include "task21_crc.h"
#include "task21_frame.h"

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
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

/*
 * 将一帧数据逐字节输入解析器。
 * 返回最后一个字节对应的解析结果。
 */
static int feed_frame(frame_parser_t *parser,
                      const uint8_t *data,
                      size_t length,
                      frame_t *frame_out)
{
    int result = 0;

    for (size_t i = 0U; i < length; i++)
    {
        result = frame_parser_push(parser, data[i], frame_out);
    }

    return result;
}

/*
 * 使用编码器生成合法测试帧。
 */
static int make_test_frame(uint8_t *buffer,
                           size_t capacity,
                           uint8_t command,
                           const uint8_t *payload,
                           uint8_t payload_length,
                           size_t *frame_length)
{
    return frame_encode(buffer,
                        capacity,
                        command,
                        payload,
                        payload_length,
                        frame_length);
}

int main(void)
{
    int failures = 0;

    frame_parser_t parser;
    frame_t frame_out;

    uint8_t buffer[16];
    size_t frame_length = 0U;
    int result;

    /*
     * 测试1：初始化解析器
     */
    memset(&parser, 0xAA, sizeof(parser));

    frame_parser_init(&parser);

    failures += !check(
        parser.used == 0U && parser.expected == 0U,
        "parser initialization"
    );

    /*
     * 测试2：忽略帧头之前的无效数据
     */
    result = frame_parser_push(&parser, 0x11U, &frame_out);

    failures += !check(
        result == 0 && parser.used == 0U,
        "ignore data before header"
    );

    /*
     * 测试3：接收帧头
     */
    result = frame_parser_push(&parser, 0xAAU, &frame_out);

    failures += !check(
        result == 0 &&
        parser.used == 1U &&
        parser.bytes[0] == 0xAAU,
        "receive frame header"
    );

    frame_parser_init(&parser);

    /*
     * 测试4：正常解析
     *
     * Command = 0x20
     * Payload = FF 01
     *
     * 完整帧：
     * AA 02 20 FF 01 CRC
     */
    {
        const uint8_t payload[] = {0xFFU, 0x01U};

        result = make_test_frame(
            buffer,
            sizeof(buffer),
            0x20U,
            payload,
            2U,
            &frame_length
        );

        failures += !check(
            result == 0,
            "prepare normal frame"
        );

        memset(&frame_out, 0, sizeof(frame_out));

        result = feed_frame(
            &parser,
            buffer,
            frame_length,
            &frame_out
        );

        failures += !check(
            result == 1,
            "normal frame parsing"
        );

        failures += !check(
            frame_out.command == 0x20U,
            "normal frame command"
        );

        failures += !check(
            frame_out.payload_length == 2U,
            "normal frame payload length"
        );

        failures += !check(
            frame_out.payload[0] == 0xFFU &&
            frame_out.payload[1] == 0x01U,
            "normal frame payload content"
        );

        failures += !check(
            parser.used == 0U && parser.expected == 0U,
            "parser reset after successful frame"
        );
    }

    /*
     * 测试5：空 Payload
     *
     * Command = 0x10
     * Payload length = 0
     */
    {
        result = make_test_frame(
            buffer,
            sizeof(buffer),
            0x10U,
            NULL,
            0U,
            &frame_length
        );

        failures += !check(
            result == 0,
            "prepare empty payload frame"
        );

        memset(&frame_out, 0, sizeof(frame_out));

        result = feed_frame(
            &parser,
            buffer,
            frame_length,
            &frame_out
        );

        failures += !check(
            result == 1,
            "empty payload parsing"
        );

        failures += !check(
            frame_out.command == 0x10U &&
            frame_out.payload_length == 0U,
            "empty payload frame content"
        );
    }

    /*
     * 测试6：最大 Payload，8字节
     */
    {
        const uint8_t payload[] = {
            0x00U, 0x11U, 0x22U, 0x33U,
            0x44U, 0x55U, 0x66U, 0x77U
        };

        result = make_test_frame(
            buffer,
            sizeof(buffer),
            0x30U,
            payload,
            8U,
            &frame_length
        );

        failures += !check(
            result == 0,
            "prepare maximum payload frame"
        );

        memset(&frame_out, 0, sizeof(frame_out));

        result = feed_frame(
            &parser,
            buffer,
            frame_length,
            &frame_out
        );

        failures += !check(
            result == 1,
            "maximum payload parsing"
        );

        failures += !check(
            frame_out.payload_length == 8U,
            "maximum payload length"
        );

        failures += !check(
            memcmp(frame_out.payload, payload, sizeof(payload)) == 0,
            "maximum payload content"
        );
    }

    /*
     * 测试7：非法长度，大于8
     */
    {
        frame_parser_init(&parser);

        result = frame_parser_push(&parser, 0xAAU, &frame_out);

        failures += !check(
            result == 0,
            "invalid length test: receive header"
        );

        result = frame_parser_push(&parser, 9U, &frame_out);

        failures += !check(
            result == -2,
            "reject payload length greater than 8"
        );

        failures += !check(
            parser.used == 0U && parser.expected == 0U,
            "reset after invalid length"
        );
    }

    /*
     * 测试8：CRC错误
     *
     * 先生成合法帧，再修改最后一个字节。
     */
    {
        const uint8_t payload[] = {0x12U, 0x34U};

        result = make_test_frame(
            buffer,
            sizeof(buffer),
            0x40U,
            payload,
            2U,
            &frame_length
        );

        failures += !check(
            result == 0,
            "prepare CRC error frame"
        );

        buffer[frame_length - 1U] ^= 0x01U;

        result = feed_frame(
            &parser,
            buffer,
            frame_length,
            &frame_out
        );

        failures += !check(
            result == -2,
            "reject frame with incorrect CRC"
        );

        failures += !check(
            parser.used == 0U && parser.expected == 0U,
            "reset after CRC error"
        );
    }

    /*
     * 测试9：不完整帧
     *
     * 只输入完整帧的前3个字节。
     */
    {
        const uint8_t payload[] = {0x12U, 0x34U};

        result = make_test_frame(
            buffer,
            sizeof(buffer),
            0x50U,
            payload,
            2U,
            &frame_length
        );

        failures += !check(
            result == 0,
            "prepare incomplete frame"
        );

        frame_parser_init(&parser);

        result = feed_frame(
            &parser,
            buffer,
            3U,
            &frame_out
        );

        failures += !check(
            result == 0,
            "incomplete frame not accepted"
        );

        failures += !check(
            parser.used == 3U &&
            parser.expected == frame_length,
            "incomplete frame retained in parser"
        );

        /* 继续输入剩余字节 */
        result = feed_frame(
            &parser,
            &buffer[3],
            frame_length - 3U,
            &frame_out
        );

        failures += !check(
            result == 1,
            "complete frame after receiving remaining bytes"
        );

        failures += !check(
            frame_out.command == 0x50U &&
            frame_out.payload_length == 2U &&
            frame_out.payload[0] == 0x12U &&
            frame_out.payload[1] == 0x34U,
            "reassembled frame content"
        );
    }

    /*
     * 测试10：连续接收两帧
     */
    {
        const uint8_t payload1[] = {0x11U};
        const uint8_t payload2[] = {0x22U, 0x33U};

        uint8_t frame1[16];
        uint8_t frame2[16];

        size_t length1 = 0U;
        size_t length2 = 0U;

        result = make_test_frame(
            frame1,
            sizeof(frame1),
            0x61U,
            payload1,
            1U,
            &length1
        );

        failures += !check(
            result == 0,
            "prepare first consecutive frame"
        );

        result = make_test_frame(
            frame2,
            sizeof(frame2),
            0x62U,
            payload2,
            2U,
            &length2
        );

        failures += !check(
            result == 0,
            "prepare second consecutive frame"
        );

        result = feed_frame(
            &parser,
            frame1,
            length1,
            &frame_out
        );

        failures += !check(
            result == 1 && frame_out.command == 0x61U,
            "receive first consecutive frame"
        );

        result = feed_frame(
            &parser,
            frame2,
            length2,
            &frame_out
        );

        failures += !check(
            result == 1 &&
            frame_out.command == 0x62U &&
            frame_out.payload_length == 2U &&
            frame_out.payload[0] == 0x22U &&
            frame_out.payload[1] == 0x33U,
            "receive second consecutive frame"
        );
    }

    /*
     * 测试11：parser为NULL
     */
    result = frame_parser_push(NULL, 0xAAU, &frame_out);

    failures += !check(
        result == -1,
        "NULL parser"
    );

    /*
     * 测试12：frame_out为NULL
     */
    result = frame_parser_push(&parser, 0xAAU, NULL);

    failures += !check(
        result == -1,
        "NULL frame output"
    );

    /*
     * 测试13：初始化函数接收NULL
     */
    frame_parser_init(NULL);

    failures += !check(
        1,
        "initialize with NULL parser does not crash"
    );

    /*
     * 汇总测试结果
     */
    printf("\n============================\n");

    if (failures == 0)
    {
        printf("ALL TESTS PASSED\n");
    }
    else
    {
        printf("TESTS FAILED: %d\n", failures);
    }

    printf("============================\n");

    return (failures == 0) ? 0 : 1;
}