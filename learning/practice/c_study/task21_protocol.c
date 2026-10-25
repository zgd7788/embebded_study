#include "task21_protocol.h"

/*
    功能：最多从 RX 环形缓冲区取出一个字节，交给增量解析器。
    参数：
        @rx：接收上下文，不能为 NULL；
             内部指针须指向已初始化的环形缓冲区和解析器。
        @frame_out：接收完整有效帧的输出地址，不能为 NULL；
                    仅返回 1 时写入。
    返回值：
        1：解析出一帧完整有效帧。
        0：缓冲区为空、帧尚未收齐，或错误候选帧已丢弃。
       -1：参数错误。
*/
int protocol_rx_process(protocol_rx_t *rx, frame_t *frame_out)
{
    if ((rx == NULL) || (frame_out == NULL))
    {
        return -1;
    }

    uint8_t byte;
    int result = rx_ring_pop(&rx->ring, &byte);
    if (result != 0)
    {
        return result == -2 ? 0 : -1;
    }

    result = frame_parser_push(&rx->parser, byte, frame_out);
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