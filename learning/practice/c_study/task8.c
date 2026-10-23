#include "reg_sim.h"

/*
    功能：向模拟的内存映射寄存器写入一个 32 位值。
    参数：
        @addr：寄存器地址，不能为 NULL；
        @value：要写入的 32 位值。
    返回值：null
*/
void reg_write(volatile uint32_t *addr, uint32_t value)
{
    *addr = value;
}
/*
    功能：读取一个 32 位寄存器的当前值。
    参数：
        @addr：寄存器地址，调用者保证有效；
    返回值：返回当前寄存器中的 32 位值。
*/
uint32_t reg_read(const volatile uint32_t *addr)
{
    return *addr;
}
/*
    功能：只更新寄存器中由 mask 指定的位。
    参数：
        @addr：寄存器地址，不能为 NULL；
        @mask：要修改的位，1 表示允许修改；
        @value：已经按寄存器位置对齐的新值。
    返回值：
        0：成功；
        -1：addr == NULL。
*/
int reg_update_bits(volatile uint32_t *addr, uint32_t mask, uint32_t value)
{
    if(addr == NULL)
    {
        return -1;
    }

    if (mask == 0)
    {
        return 0;
    }
    *addr = (*addr & ~mask) | (value & mask);
    return 0;
}
