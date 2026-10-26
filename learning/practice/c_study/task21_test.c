#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "task21_protocol.h"
#include "task21_crc.h"

#define TEST_CAPACITY  64U
#define TEST_TIMEOUT   20U

static uint8_t g_storage[TEST_CAPACITY];

static unsigned int g_pass = 0U;
static unsigned int g_fail = 0U;

/* 检查测试结果 */
static void check(const char *name, int condition)
{
    if (condition)
    {
        printf("[PASS] %s\n", name);
        g_pass++;
    }
    else
    {
        printf("[FAIL] %s\n", name);
        g_fail++;
    }
}

/* 初始化接收对象 */
static int test_init(protocol_rx_t *rx)
{
    return protocol_rx_init(rx,
                            g_storage,
                            sizeof(g_storage),
                            TEST_TIMEOUT);
}

/*
 * 构造一帧合法数据：
 *
 * AA LEN CMD PAYLOAD CRC
 *
 * LEN     = 2
 * CMD     = 0x20
 * PAYLOAD = FF 01
 *
 * CRC 覆盖 LEN、CMD、PAYLOAD，不包含帧头 AA。
 */
static size_t make_valid_frame(uint8_t *out)
{
    uint8_t crc_input[4] = {
        0x02U,
        0x20U,
        0xFFU,
        0x01U
    };

    uint8_t crc = 0U;

    if (crc8_atm(crc_input, sizeof(crc_input), &crc) != 0)
    {
        return 0U;
    }

    out[0] = 0xAAU;
    out[1] = 0x02U;
    out[2] = 0x20U;
    out[3] = 0xFFU;
    out[4] = 0x01U;
    out[5] = crc;

    return 6U;
}

/* 向环形缓冲区逐字节写入一帧 */
static int enqueue_frame(protocol_rx_t *rx,
                         const uint8_t *frame,
                         size_t length)
{
    size_t i;

    for (i = 0U; i < length; i++)
    {
        if (rx_ring_push(&rx->ring, frame[i]) != 0)
        {
            return -1;
        }
    }

    return 0;
}

/* 检查解析出的帧内容 */
static int frame_is_correct(const frame_t *frame)
{
    return (frame->command == 0x20U) &&
           (frame->payload_length == 2U) &&
           (frame->payload[0] == 0xFFU) &&
           (frame->payload[1] == 0x01U);
}

/*
 * 测试 1：
 * 完整合法帧逐字节入队，再循环处理。
 *
 * 预期：
 * 1. 恰好解析出一帧。
 * 2. 帧内容正确。
 * 3. 环形缓冲区最终为空。
 */
static void test_1_complete_frame(void)
{
    protocol_rx_t rx;
    frame_t frame_out = {0};
    uint8_t frame[6];
    size_t frame_len;
    unsigned int frame_count = 0U;
    unsigned int attempts = 0U;
    int ret;

    printf("\n========== Test 1: Complete frame ==========\n");

    check("Initialize RX",
          test_init(&rx) == 0);

    frame_len = make_valid_frame(frame);

    check("Build valid frame",
          frame_len == sizeof(frame));

    check("Enqueue complete frame",
          enqueue_frame(&rx, frame, frame_len) == 0);

    /* 队列有数据时，每次 process 处理一个字节。 */
    while ((rx.ring.count > 0U) && (attempts < 16U))
    {
        attempts++;

        ret = protocol_rx_process(&rx, 100U, &frame_out);

        if (ret == 1)
        {
            frame_count++;
        }
        else if (ret < 0)
        {
            check("Process returns no error", 0);
            break;
        }
    }

    check("Exactly one frame output",
          frame_count == 1U);

    check("Frame content correct",
          frame_is_correct(&frame_out));

    check("Ring buffer empty",
          rx.ring.count == 0U);
}

/*
 * 测试 2：
 * 100 ms 处理帧头 AA。
 * 队列为空时，分别在 119 ms 和 120 ms 调用 process。
 *
 * 预期：
 * 119 ms：保留半帧，返回 0。
 * 120 ms：清除半帧，返回 0。
 */
static void test_2_timeout_boundary(void)
{
    protocol_rx_t rx;
    frame_t frame_out = {0};
    int ret;

    printf("\n========== Test 2: Timeout boundary ==========\n");

    check("Initialize RX",
          test_init(&rx) == 0);

    check("Enqueue header",
          rx_ring_push(&rx.ring, 0xAAU) == 0);

    /* 100 ms：处理帧头。 */
    ret = protocol_rx_process(&rx, 100U, &frame_out);

    check("Process header returns 0",
          ret == 0);

    check("Header consumed",
          rx.ring.count == 0U);

    check("Partial frame exists after header",
          rx.parser.used == 1U);

    check("Last byte timestamp is 100 ms",
          rx.last_byte_ms == 100U);

    /* 119 - 100 = 19 ms，小于 20 ms。 */
    ret = protocol_rx_process(&rx, 119U, &frame_out);

    check("119 ms returns 0",
          ret == 0);

    check("Partial frame retained at 119 ms",
          rx.parser.used == 1U);

    /* 120 - 100 = 20 ms，达到超时门限。 */
    ret = protocol_rx_process(&rx, 120U, &frame_out);

    check("120 ms returns 0",
          ret == 0);

    check("Partial frame cleared at 120 ms",
          rx.parser.used == 0U);

    check("Ring remains empty",
          rx.ring.count == 0U);
}

/*
 * 测试 3：
 * 100 ms 处理旧半帧的帧头。
 * 到 120 ms 时，旧半帧已经超时，同时队列中存在新合法帧。
 *
 * 按照当前 protocol_rx_process() 的实现：
 * 1. 先清除旧半帧。
 * 2. 同一次调用继续取出新帧第一个字节。
 * 3. 后续调用继续解析剩余字节。
 *
 * 预期：新帧不丢失，最终正确输出一帧。
 */
static void test_3_timeout_then_new_frame(void)
{
    protocol_rx_t rx;
    frame_t frame_out = {0};
    uint8_t frame[6];
    size_t frame_len;
    unsigned int frame_count = 0U;
    unsigned int attempts = 0U;
    int ret;

    printf("\n========== Test 3: Timeout then new frame ==========\n");

    check("Initialize RX",
          test_init(&rx) == 0);

    /* 100 ms：处理旧半帧的帧头 AA。 */
    check("Enqueue old header",
          rx_ring_push(&rx.ring, 0xAAU) == 0);

    ret = protocol_rx_process(&rx, 100U, &frame_out);

    check("Process old header returns 0",
          ret == 0);

    check("Old partial frame exists",
          rx.parser.used == 1U);

    /* 构造新合法帧，并全部写入环形缓冲区。 */
    frame_len = make_valid_frame(frame);

    check("Build new valid frame",
          frame_len == sizeof(frame));

    check("Enqueue new frame",
          enqueue_frame(&rx, frame, frame_len) == 0);

    /*
     * 120 ms：
     * elapsed_ms = 120 - 100 = 20 ms。
     *
     * 先清除旧半帧，然后继续处理队列中的第一个新字节。
     */
    ret = protocol_rx_process(&rx, 120U, &frame_out);

    check("Timeout call returns 0",
          ret == 0);

    check("New frame header processed",
          rx.parser.used == 1U);

    check("One new byte consumed",
          rx.ring.count == frame_len - 1U);

    /*
     * 继续处理剩余字节。
     * 固定使用 120 ms，避免测试中引入额外的时间变化。
     */
    while ((rx.ring.count > 0U) && (attempts < 16U))
    {
        attempts++;

        ret = protocol_rx_process(&rx, 120U, &frame_out);

        if (ret == 1)
        {
            frame_count++;
        }
        else if (ret < 0)
        {
            check("Continue processing without error", 0);
            break;
        }
    }

    /*
     * 第一次调用 120 ms 时只处理了帧头，
     * 因此后续循环应当输出一帧。
     */
    check("Exactly one new frame output",
          frame_count == 1U);

    check("New frame content correct",
          frame_is_correct(&frame_out));

    check("Ring empty after recovery",
          rx.ring.count == 0U);
}

/* ============================================================
 * Test 4：FIFO、满/空和下标回绕
 * ============================================================ */
static void test_4_ring_fifo_full_empty_wrap(void)
{
    rx_ring_t ring;
    uint8_t storage[4];
    uint8_t value = 0U;
    int ret;

    printf("\n========== Test 4: Ring FIFO/full/empty/wrap ==========\n");

    ret = rx_ring_init(&ring, storage, sizeof(storage));
    check("Ring init", ret == 0);

    /* 空队列读取应失败。 */
    ret = rx_ring_pop(&ring, &value);
    check("Pop empty ring returns -2", ret == -2);
    check("Empty ring count remains zero", ring.count == 0U);

    /* 填满容量为 4 的环形缓冲区。 */
    check("Push A", rx_ring_push(&ring, 'A') == 0);
    check("Push B", rx_ring_push(&ring, 'B') == 0);
    check("Push C", rx_ring_push(&ring, 'C') == 0);
    check("Push D", rx_ring_push(&ring, 'D') == 0);

    check("Ring reaches full capacity",
          ring.count == sizeof(storage));

    /* 满队列继续写入应失败，不能覆盖旧数据。 */
    ret = rx_ring_push(&ring, 'E');
    check("Push full ring returns -2", ret == -2);
    check("Full ring count unchanged",
          ring.count == sizeof(storage));

    /* 读取 A、B，使读下标向前移动。 */
    ret = rx_ring_pop(&ring, &value);
    check("Pop A succeeds", ret == 0 && value == 'A');

    ret = rx_ring_pop(&ring, &value);
    check("Pop B succeeds", ret == 0 && value == 'B');

    /* 此时写入 E、F，写下标将发生回绕。 */
    check("Push E after pop",
          rx_ring_push(&ring, 'E') == 0);

    check("Push F after pop",
          rx_ring_push(&ring, 'F') == 0);

    check("Ring full after wraparound",
          ring.count == sizeof(storage));

    /* FIFO 顺序应为 C、D、E、F。 */
    ret = rx_ring_pop(&ring, &value);
    check("FIFO returns C", ret == 0 && value == 'C');

    ret = rx_ring_pop(&ring, &value);
    check("FIFO returns D", ret == 0 && value == 'D');

    ret = rx_ring_pop(&ring, &value);
    check("FIFO returns E", ret == 0 && value == 'E');

    ret = rx_ring_pop(&ring, &value);
    check("FIFO returns F", ret == 0 && value == 'F');

    check("Ring empty after all pops", ring.count == 0U);

    ret = rx_ring_pop(&ring, &value);
    check("Pop empty ring again returns -2", ret == -2);
}


/* ============================================================
 * Test 5：经 ring -> process 拆包、连续两帧
 * ============================================================ */
static void test_5_split_input_two_frames(void)
{
    protocol_rx_t rx;
    frame_t frame_out = {0};
    uint8_t frame[6];
    size_t frame_len;
    unsigned int frame_count = 0U;
    unsigned int attempts = 0U;
    int ret;

    printf("\n========== Test 5: Split input/two frames ==========\n");

    check("Initialize RX", test_init(&rx) == 0);

    frame_len = make_valid_frame(frame);
    check("Build valid frame", frame_len == sizeof(frame));

    /*
     * 第一帧拆成两部分入队：
     * 第一次只入队前 2 个字节 AA LEN。
     */
    check("Enqueue first part",
          enqueue_frame(&rx, frame, 2U) == 0);

    while (rx.ring.count > 0U)
    {
        ret = protocol_rx_process(&rx, 100U, &frame_out);

        if (ret == 1)
        {
            frame_count++;
        }
        else if (ret < 0)
        {
            check("Process first part without error", 0);
            return;
        }
    }

    check("First part does not produce a frame",
          frame_count == 0U);

    check("Parser retains partial frame",
          rx.parser.used == 2U);

    /*
     * 将第一帧剩余字节和第二帧全部入队。
     * 两帧在字节流中连续排列。
     */
    check("Enqueue first frame remainder",
          enqueue_frame(&rx, &frame[2], frame_len - 2U) == 0);

    check("Enqueue second complete frame",
          enqueue_frame(&rx, frame, frame_len) == 0);

    while ((rx.ring.count > 0U) && (attempts < 32U))
    {
        attempts++;

        ret = protocol_rx_process(&rx, 101U, &frame_out);

        if (ret == 1)
        {
            frame_count++;

            check("Every output frame content correct",
                  frame_is_correct(&frame_out));
        }
        else if (ret < 0)
        {
            check("Process consecutive frames without error", 0);
            return;
        }
    }

    check("Exactly two frames output",
          frame_count == 2U);

    check("Ring empty after two frames",
          rx.ring.count == 0U);

    check("Parser idle after two frames",
          rx.parser.used == 0U);
}


/* ============================================================
 * Test 6：非法 LEN、坏 CRC 后恢复
 * ============================================================ */
static void test_6_invalid_len_bad_crc_recovery(void)
{
    protocol_rx_t rx;
    frame_t frame_out = {0};
    uint8_t frame[6];
    size_t frame_len;
    unsigned int frame_count = 0U;
    unsigned int attempts = 0U;
    int ret;

    printf("\n========== Test 6: Invalid LEN/CRC recovery ==========\n");

    check("Initialize RX", test_init(&rx) == 0);

    frame_len = make_valid_frame(frame);
    check("Build valid frame", frame_len == sizeof(frame));

    /*
     * 第一部分：非法 LEN。
     * LEN = 9，假设协议最大 payload 长度为 8。
     *
     * AA 09
     */
    {
        const uint8_t invalid_len_data[] = {0xAAU, 0x09U};

        check("Enqueue invalid LEN bytes",
              enqueue_frame(&rx,
                            invalid_len_data,
                            sizeof(invalid_len_data)) == 0);
    }

    while (rx.ring.count > 0U)
    {
        ret = protocol_rx_process(&rx, 100U, &frame_out);

        check("Invalid LEN does not output a frame",
              ret != 1);

        if (ret < 0)
        {
            check("Invalid LEN processing has no fatal error", 0);
            return;
        }
    }

    check("Parser reset after invalid LEN",
          rx.parser.used == 0U);

    /*
     * 第二部分：坏 CRC。
     * 翻转正确帧的 CRC 字节，构造 CRC 错误帧。
     */
    frame[frame_len - 1U] ^= 0x01U;

    check("Enqueue bad CRC frame",
          enqueue_frame(&rx, frame, frame_len) == 0);

    while (rx.ring.count > 0U)
    {
        ret = protocol_rx_process(&rx, 101U, &frame_out);

        check("Bad CRC does not output a frame",
              ret != 1);

        if (ret < 0)
        {
            check("Bad CRC processing has no fatal error", 0);
            return;
        }
    }

    check("Parser reset after bad CRC",
          rx.parser.used == 0U);

    /*
     * 第三部分：紧接着输入合法帧。
     * 这可以验证坏 CRC 后，解析器能否恢复。
     */
    frame_len = make_valid_frame(frame);

    check("Enqueue valid frame after errors",
          enqueue_frame(&rx, frame, frame_len) == 0);

    while ((rx.ring.count > 0U) && (attempts < 16U))
    {
        attempts++;

        ret = protocol_rx_process(&rx, 102U, &frame_out);

        if (ret == 1)
        {
            frame_count++;
        }
        else if (ret < 0)
        {
            check("Recovery processing has no fatal error", 0);
            return;
        }
    }

    check("One valid frame output after errors",
          frame_count == 1U);

    check("Recovered frame content correct",
          frame_is_correct(&frame_out));

    check("Parser idle after recovery",
          rx.parser.used == 0U);

    check("Ring empty after recovery",
          rx.ring.count == 0U);
}


/* ============================================================
 * Test 7：uint32_t 毫秒计数回绕
 * ============================================================ */
static void test_7_millisecond_wraparound(void)
{
    protocol_rx_t rx;
    frame_t frame_out = {0};
    uint32_t start_ms = UINT32_MAX - 10U;
    int ret;

    printf("\n========== Test 7: Millisecond wraparound ==========\n");

    check("Initialize RX", test_init(&rx) == 0);

    /*
     * 在接近 UINT32_MAX 的时刻处理帧头。
     * last_byte_ms 会更新为 start_ms。
     */
    check("Enqueue header",
          rx_ring_push(&rx.ring, 0xAAU) == 0);

    ret = protocol_rx_process(&rx, start_ms, &frame_out);

    check("Header processed", ret == 0);
    check("Partial frame exists", rx.parser.used == 1U);
    check("Timestamp recorded correctly",
          rx.last_byte_ms == start_ms);

    /*
     * 从 UINT32_MAX - 10 到 8，经过 19 ms。
     * 计数器已经回绕，但尚未达到 20 ms 超时。
     */
    ret = protocol_rx_process(&rx, 8U, &frame_out);

    check("19 ms across wraparound does not timeout",
          ret == 0);

    check("Partial frame retained before timeout",
          rx.parser.used == 1U);

    /*
     * 从 UINT32_MAX - 10 到 9，经过 20 ms。
     * 应该触发超时。
     */
    ret = protocol_rx_process(&rx, 9U, &frame_out);

    check("20 ms across wraparound returns 0",
          ret == 0);

    check("Partial frame cleared at timeout",
          rx.parser.used == 0U);
}

int main(void)
{
    test_1_complete_frame();
    test_2_timeout_boundary();
    test_3_timeout_then_new_frame();
    test_4_ring_fifo_full_empty_wrap();
    test_5_split_input_two_frames();
    test_6_invalid_len_bad_crc_recovery();
    test_7_millisecond_wraparound();

    printf("\n========== Test summary ==========\n");
    printf("PASS: %u\n", g_pass);
    printf("FAIL: %u\n", g_fail);

    return (g_fail == 0U) ? 0 : 1;
}