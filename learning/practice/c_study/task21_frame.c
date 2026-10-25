#include "task21_frame.h"
#include "task21_crc.h"

/*
    功能：编码一帧并写入 out
    参数：
        @out：接收完整帧的缓冲区，不能为 NULL。
        @out_capacity：缓冲区容量。
        @command：命令码。
        @payload：payload 数据；长度为 0 时允许为 NULL，否则不能为 NULL。
        @payload_length：payload 长度，范围为 0~8。
        @frame_length_out：接收编码后的总帧长，不能为 NULL。
    返回值：成功返回 0，参数非法或容量不足返回 -1。
*/
int frame_encode(uint8_t *out,
                 size_t out_capacity,
                 uint8_t command,
                 const uint8_t *payload,
                 uint8_t payload_length,
                 size_t *frame_length_out)
{
    if ((out == NULL) || (frame_length_out == NULL) ||
        ((payload == NULL) && (payload_length != 0U)) ||
        (payload_length > 8U))
    {
        return -1;
    }
    
    size_t required_length = (size_t)payload_length + 4u;
    
    if (out_capacity < required_length)
    {
        return -1;
    }
    uint8_t crc_input[10];
    crc_input[0] = payload_length;
    crc_input[1] = command;
    for (size_t i = 0; i < (size_t)payload_length; i++)
    {
        crc_input[i + 2U] = payload[i];
    }

    uint8_t checkcrc = 0;
    const size_t crc_input_length = (size_t)payload_length + 2U;
    if (crc8_atm(crc_input, crc_input_length, &checkcrc) != 0)
    {
        return -1;
    }

    out[0] = 0xAAU;
    out[1] = payload_length;
    out[2] = command;
    for (size_t i = 0; i < (size_t)payload_length; i++)
    {
        out[i + 3U] = crc_input[i + 2U];
    }
    out[required_length - 1U] = checkcrc;
    *frame_length_out = required_length;

    return 0;
}
