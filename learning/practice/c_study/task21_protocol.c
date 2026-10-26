#include "task21_protocol.h"

/*
    功能：初始化协议接收上下文，绑定接收数组，清空缓冲区和解析状态，
          设置半帧超时阈值。
    参数：
        @rx：接收上下文地址，不能为 NULL。
        @storage：指向至少 capacity 字节的可写数组，不能为 NULL；
                  数组须在整个接收期间保持有效。
        @capacity：接收数组容量，必须大于 0。
        @timeout_ms：半帧超时阈值，单位为毫秒，必须大于 0。
    返回值：
        0：初始化成功。
       -1：参数非法，不修改接收上下文。
*/
int protocol_rx_init(protocol_rx_t *rx, uint8_t *storage,
                     size_t capacity, uint32_t timeout_ms)
{
    if((rx == NULL)||(storage == NULL)||(capacity == 0)||(timeout_ms == 0))
    {
        return -1;
    }
    rx_ring_init(&rx->ring, storage, capacity);
    frame_parser_init(&rx->parser);
    rx->last_byte_ms = 0U;
    rx->timeout_ms = timeout_ms;
    return 0;
}

/*
    功能：最多从 RX 环形缓冲区取出一个字节，交给增量解析器。
    参数：
        @rx：接收上下文，不能为 NULL；
             内部指针须指向已初始化的环形缓冲区和解析器。
        @now_ms：调用者提供的当前毫秒计数。
        @frame_out：接收完整有效帧的输出地址，不能为 NULL；
                    仅返回 1 时写入。
    返回值：
        1：解析出一帧完整有效帧。
        0：缓冲区为空、帧尚未收齐，或错误候选帧已丢弃。
       -1：参数错误。
*/
int protocol_rx_process(protocol_rx_t *rx, uint32_t now_ms, frame_t *frame_out)
{
    if ((rx == NULL) || (frame_out == NULL))
    {
        return -1;
    }

    uint8_t byte;
    protocol_rx_check_timeout(rx, now_ms);
    int result = rx_ring_pop(&rx->ring, &byte);
    if (result != 0)
    {
        return result == -2 ? 0 : -1;
    }
    result = frame_parser_push(&rx->parser, byte, frame_out);
    rx->last_byte_ms = now_ms;
    return result == -2 ? 0 : result;
}


/*
    功能：检查半帧是否超时；超时则重置解析器。
    参数：
        @rx：接收上下文，不能为 NULL。
        @now_ms：调用者提供的当前毫秒计数，类型为 uint32_t。
    返回值：
        1：半帧已超时，解析器已重置。
        0：没有半帧，或尚未超时。
       -1：参数错误。
*/
int protocol_rx_check_timeout(protocol_rx_t *rx, uint32_t now_ms)
{
    if(rx == NULL)
    {
        return -1;
    }
    uint32_t elapsed_ms = now_ms - rx->last_byte_ms;
    if(rx->parser.used > 0U)
    {
        if(elapsed_ms >= rx->timeout_ms)
        {
            frame_parser_init(&rx->parser);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}