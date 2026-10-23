/*
    模拟一个 32 位控制寄存器：

    | 位段 | 含义 |
    |---|---|
    | bit 0~1 | 工作模式 MODE |
    | bit 4 | 使能 ENABLE |
    | bit 8 | 中断使能 IRQ_EN |
    | bit 12~15 | 分频值 PRESCALER |
*/

#include "reg_sim.h"

/*
    功能：修改寄存器中的指定字段。
    参数：
        @reg：原始 32 位寄存器值。
        @mask：字段掩码，例如 0x3。
        @shift：字段最低位的位置。
        @value：未左移的字段值。
    返回值：修改后的寄存器值
*/
uint32_t reg_set_field(uint32_t reg, uint32_t mask, unsigned int shift, uint32_t value)
{
    uint32_t field_mask = mask << shift;
    reg &= ~field_mask;
    reg |= (value & mask) << shift;
    return reg;
}

/*
    功能：读取指定寄存器字段。
    参数：
        reg：寄存器值。
        mask：字段掩码。
        shift：字段最低位的位置。
    返回值：右移后的字段值，不带其他位。
*/
uint32_t reg_get_field(uint32_t reg, uint32_t mask, unsigned int shift)
{
    uint32_t field_mask = mask << shift;
    reg &= field_mask;
    return (reg >> shift) & mask;
}

/*
    功能：设置 MODE 字段 bit 0~1。
    参数：
        @reg：寄存器地址，不能为 NULL。
        @mode：工作模式：
            MODE_OFF = 0
            MODE_RUN = 1
            MODE_SLEEP = 2
    返回值：
        0：成功
        -1：空指针或非法模式
*/
int reg_set_mode(uint32_t *reg, MODE_E mode)
{
    if(reg == NULL)
    {
        printf("reg addr error\n");
        return -1;
    }
    if(mode == MODE_OFF)
    {
        *reg = reg_set_field(*reg, 0x3, 0, 0);
    }
    else if(mode == MODE_RUN)
    {
        *reg = reg_set_field(*reg, 0x3, 0, 1);
    }
    else if(mode ==MODE_SLEEP)
    {
        *reg = reg_set_field(*reg, 0x3, 0, 2);
    }
    else
    {
        printf("mode error\n");
        return -1;
    }
    return 0;
}

/*
    功能：设置 ENABLE 位 bit 4。
    参数：
        @reg：寄存器地址，不能为 NULL。
        @enable：只能为 0 或 1。
    返回值：
        0：成功
        -1：空指针或参数非法
*/
int reg_set_enable(uint32_t *reg, int enable)
{
    if(reg == NULL)
    {
        printf("reg addr error\n");
        return -1;
    }
    if(enable == 1)
    {
        *reg |= 0x0010;
    }
    else if(enable == 0)
    {
        *reg &= ~0x0010;
    }
    else
    {
        printf("parameter error\n");
        return -1;
    }
    return 0;
}

/*
    功能：设置 IRQ_EN 位 bit 8。
    参数：
        @reg：寄存器地址，不能为 NULL。
        @enable：只能为 0 或 1。
    返回值：
        0：成功
        -1：空指针或参数非法
*/
int reg_set_irq_enable(uint32_t *reg, int enable)
{
    if(reg == NULL)
    {
        printf("reg addr error\n");
        return -1;
    }
    if(enable == 1)
    {
        *reg |= 0x0100;
    }
    else if(enable == 0)
    {
        *reg &= ~0x0100;
    }
    else
    {
        printf("parameter error\n");
        return -1;
    }
    return 0;
}

/*
    功能：设置 PRESCALER 字段 bit 12~15。
    参数：
        @reg：寄存器地址，不能为 NULL。
        @value：有效范围为 0~15。
    返回值：
        0：成功
        -1：空指针或数值超范围
*/
int reg_set_prescaler(uint32_t *reg, uint32_t value)
{
     if(reg == NULL)
    {
        printf("reg addr error\n");
        return -1;
    }
    if(value > 15)
    {
        printf("value error\n");
        return -1;
    }
    value = value << 12;
    uint32_t mask = 0xF << 12;
    *reg &= ~mask;
    *reg |= value;
    return 0;
}