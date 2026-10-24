#include "task19_frame.h"

/*
    功能：把 data 中 length 个字节相加，将结果按 8 位保留后写入 *checksum_out。
    参数：
        @data：待累加的字节数组；本任务约定即使 length == 0 也不能为 NULL。
        @length：要累加的字节数。
        @checksum_out：接收校验和的输出地址。
    返回值：**成功返回 0；data == NULL 或 checksum_out == NULL 返回 -1。长度为 0 时成功写入校验和 0。
*/
int frame_checksum(const uint8_t *data, size_t length, uint8_t *checksum_out)
{
    if((data == NULL)||(checksum_out == NULL))
    {
        return -1;
    }
    if(length == 0)
    {
        *checksum_out = 0;
    }
    *checksum_out = 0;
    for(size_t i = 0; i < length; i++)
    {
        *checksum_out += data[i];
    }
    return 0;
}

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
    if((out == NULL)||((payload == NULL)&&(payload_length != 0))||(frame_length_out == NULL))
    {
        return -1;   
    }
    if((out_capacity >= payload_length + 4)||(payload_length > 8))
    {
        return -1;
    }
    uint8_t check_outsum = 0;
    if(payload_length == 0)
    {
        *out = 0xAA;
        *(out+1) = 0x00;
        *(out+2) = command;
        *(out+3) = frame_checksum(out+1,2,&check_outsum);
        *frame_length_out = 4;
    }
    else
    {
        *out = 0xAA;
        *(out+1) = payload_length;
        *(out+2) = command;
        for(size_t i=0; i < payload_length; i++)
        {
            *(out+i+3) = *(payload+i);
        }
        *(out+3+payload_length) = frame_checksum(out+1,2,&check_outsum);
        *frame_length_out = 4+payload_length;
    }
    return 0;
}
