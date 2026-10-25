#include "task21_crc.h"

/*
    功能：计算 CRC-8/ATM 校验值。
    参数：
        @data：要计算的数据。
        @length：要计算数据的字节长度。
        @crc_out：计算得到的crc校验值。
    返回值：参数错误返回-1，成功返回0.
*/
int crc8_atm(const uint8_t *data, size_t length, uint8_t *crc_out)
{
    if(((data == NULL)&&(length != 0))||(crc_out == NULL))
    {
        return -1;
    }
    uint8_t crc = 0x00U;
    for(size_t j=0; j < length; j++)
    {
        crc ^= data[j];
        for(size_t i = 0; i < 8U; i++)
        {
            if((crc&0x80U) != 0U)
            {
                crc = (uint8_t)((crc << 1) ^ 0x07U);
            }
            else
            {
                crc = (uint8_t)(crc << 1);
            }
        }
    }
    *crc_out = crc;
    return 0;
}
