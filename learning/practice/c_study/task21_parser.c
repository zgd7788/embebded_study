#include "task21_parser.h"
#include "task21_crc.h"

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
    parser->expected = 0U;
    parser->used = 0U;
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
    返回值：0：尚无完整有效帧，或当前字节属于帧外数据。
            1：收到完整且校验正确的帧，frame_out 已写入。
           -1：parser 或 frame_out 为 NULL。
           -2：发现非法长度或校验失败，当前候选帧已丢弃。
*/
int frame_parser_push(frame_parser_t *parser, uint8_t byte, frame_t *frame_out)
{
    if((parser == NULL)||(frame_out==NULL))
    {
        return -1;
    }
    if(parser->used == 0U)
    {
        if(byte == 0xAAU)
        {
            parser->bytes[0] = byte;
            parser->used = 1;
            return 0;
        }
        else
        {
            return 0;
        }
    }
    if(parser->used == 1U)
    {
        if(byte > 8U)
        {
            frame_parser_init(parser);
            return -2;
        }
        else
        {
            parser->bytes[1] = byte;
            parser->expected = byte+4;
            parser->used = 2;
            return 0;
        }
    }
    uint8_t checkcrc = 0;
    parser->bytes[parser->used] = byte;
    parser->used++;
    if(parser->used < parser->expected)
    {
        return 0;
    }
    if(parser->used == parser->expected)
    {
        if(crc8_atm(&parser->bytes[1], parser->expected-2U, &checkcrc) == 0U
            && (checkcrc == parser->bytes[parser->used-1U]))
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