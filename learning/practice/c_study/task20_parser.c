#include "task20_parser.h"
#include "task19_frame.h"

/*
    功能：初始化帧解析器，清除当前缓存状态。
    参数：
        @parser：帧解析器地址；为 NULL 时不执行操作。
    返回值：无。
*/
void frame_parser_init(frame_parser_t *parser)
{
    if(parser == NULL)
    {
        return;
    }
    parser->used = 0;
    parser->expected = 0;
    for(int i=0; i<12; i++)
    {
        parser->bytes[i] = 0;
    }
}

/*
    功能：向解析器输入一个字节；完整帧校验成功时写入 frame_out。
    参数：
        @parser：帧解析器地址，不能为 NULL。
        @byte：本次接收到的一个字节。
        @frame_out：接收完整有效帧的输出地址，不能为 NULL；
                    返回 1 时写入，其余返回值不修改。
    返回值：
        0：尚无完整有效帧，或当前字节属于帧外数据。
        1：收到完整且校验正确的帧，frame_out 已写入。
       -1：parser 或 frame_out 为 NULL。
       -2：发现非法长度或校验失败，当前候选帧已丢弃。
*/
int frame_parser_push(frame_parser_t *parser, uint8_t byte, frame_t *frame_out)
{
    /* parser 或 frame_out 为空，返回 -1。 */
    if((parser == NULL)||(frame_out == NULL))
    {
        return -1;
    }
    /* 当前没有候选帧时，忽略非 0xAA 字节；收到 0xAA 就开始缓存。 */
    if(parser->used == 0)
    {
        if(byte != 0xAA)
        {
            return 0;
        }
        else
        {
            parser->bytes[0] = byte;
            parser->used++;
            return 0;
        }
        
    }
    /* 收到 LEN 后检查是否大于 8。非法时返回 -2 并重置候选帧。 */
    if(parser->used == 1)
    {
        if(byte > 8)
        {
            frame_parser_init(parser);
            return -2;
        }
        else
        {
            parser->expected = byte+4;
        }
    }

    uint8_t checksum = 0;
    parser->used++;
    parser->bytes[parser->used-1] = byte;
    
    if(parser->used < parser->expected)
    {
        return 0;
    }
    /* 等到 used == expected，再计算校验和。校验范围是从 bytes[1] 开始的 LEN + 2 个字节。 */
    if(parser->used == parser->expected)
    {
        if((frame_checksum(&parser->bytes[1], (size_t)parser->bytes[1]+2, &checksum) == 0)
            &&(checksum == parser->bytes[parser->used-1]))
        {
            frame_out->command = parser->bytes[2];
            frame_out->payload_length = parser->bytes[1];
            for(size_t i=0; i<parser->bytes[1]; i++)
            {
                frame_out->payload[i] = parser->bytes[3+i];
            }
            frame_parser_init(parser);
            return 1;
        }
        else
        {
            frame_parser_init(parser);
            return -2;
        }
    }
    return 0;
}