#include "task18_gpio_sim.h"

/*
    功能：绑定 GPIO 模拟寄存器块。
    参数：
        @gpio：GPIO 上下文地址，不能为 NULL；
        @regs：模拟寄存器块地址，不能为 NULL。
    返回值：
        成功返回 0；
        任一指针为 NULL 返回 -1。
*/
int gpio_init(gpio_t *gpio, volatile gpio_regs_t *regs)
{
    if((gpio == NULL)||(regs == NULL))
    {
        return -1;
    }
    gpio->regs = regs;
    return 0;
}

/*
    功能：设置 control 寄存器的 ENABLE 位 bit 0。
    参数：
        @gpio：已初始化的 GPIO 上下文；
        @enable：只能为 0 或 1。
    返回值：
        成功返回 0；
        参数非法或寄存器地址为空返回 -1。
*/
int gpio_set_enable(gpio_t *gpio, int enable)
{
    if((gpio == NULL)||(gpio->regs == NULL)||((enable != 0)&&(enable != 1)))
    {
        return -1;
    }
    uint32_t mask = 0x1u;
    gpio->regs->control &= (~mask);
    gpio->regs->control |= enable; 
    return 0;
}

/*
    功能：设置 control 寄存器的 MODE 字段 bit 1~2。
    参数：
        @gpio：已初始化的 GPIO 上下文；
        @mode：有效范围为 0~3。
    返回值：
        成功返回 0；
        参数非法或寄存器地址为空返回 -1。
*/
int gpio_set_mode(gpio_t *gpio, uint32_t mode)
{
    if((gpio == NULL)||(gpio->regs == NULL)||(mode > 3))
    {
        return -1;
    }
    uint32_t mask = (0x3U << 1);
    gpio->regs->control &= (~mask);
    gpio->regs->control |= (mode << 1);
    return 0;
}

/*
    功能：读取 status 寄存器的 READY 位 bit 0。
    参数：
        @gpio：已初始化的 GPIO 上下文；
        @ready_out：用于接收 READY 值的输出地址。
    返回值：
        成功返回 0；
        参数非法或寄存器地址为空返回 -1。
*/
int gpio_get_ready(const gpio_t *gpio, int *ready_out)
{
    if((gpio == NULL)||(gpio->regs == NULL)||(ready_out == NULL))
    {
        return -1;
    }
    *ready_out = (gpio->regs->status & (0x01)) ? 1:0;
    return 0;
}