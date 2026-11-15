#include "rcc.h"

#define RCC_HSI_READY_MASK ((1U << 0U) | (1U << 2U))
#define RCC_HSI_WAIT_LIMIT 1000000U

/*
    功能：选择 USART2 和 UART4 共用的内核时钟源
    参数：
        @source：时钟源，可选 UART4_PCLK1、UART4_PLL4_Q、
                 UART_HSI、UART_CSI 或 UART_HSE
    前置条件：
        调用方已保证目标时钟源开启并就绪，且已按要求停止并禁用
        受此次切换影响的 USART2/UART4；切换期间无其他代码修改该选源字段
    返回值：选源字段写入后返回 0；source 无效返回 1，且不修改寄存器
    说明：本函数只修改 RCC 选源字段，不开启振荡器、不检查源就绪状态，
          也不修改 UART 的 CR1.UE；改变此字段会同时影响 USART2 和 UART4
*/
int RCC_UART4_SelectClock(uart4_clock_source_e source)
{
    if((uint32_t)source > (uint32_t)UART_HSE)
    {
        return 1;
    }
    uint32_t value = RCC_UART24CKSELR;
    value = (value & ~0x7U) | (uint32_t)source;
    RCC_UART24CKSELR = value;
    return 0;
}
/*
    功能：通过 MPU 侧 RCC 寄存器使能 UART4 外设时钟
    参数：无
    返回值：写入使能位后返回 0；本函数未实现错误检测
    说明：向 RCC_MP_APB1ENSETR 的 UART4EN 位写 1，
          不改变其他外设的时钟使能状态；不选择 UART4 内核时钟源，
          也不设置 UART 的 CR1.UE 或配置波特率
*/
int RCC_UART4_EnableClock(void)
{
    RCC_MP_APB1ENSETR = (1U << 16U);
    return 0;
}

/*
    功能：开启 HSI 并等待时钟及其分频状态就绪
    参数：无
    返回值：HSIRDY 和 HSIDIVRDY 均置位时返回 0；等待超时返回 1
    说明：轮询上限不是精确的毫秒超时；本函数不修改 HSI 分频值
*/
int RCC_HSI_EnableAndWaitReady(void)
{
    RCC_OCENSETR = (1U << 0U);

    uint32_t timeout = RCC_HSI_WAIT_LIMIT;
    while ((RCC_OCRDYR & RCC_HSI_READY_MASK) != RCC_HSI_READY_MASK)
    {
        if (timeout == 0U)
        {
            return 1;
        }
        timeout--;
    }

    return 0;
}

/*
    功能：查询 UART4 在 HSI 时钟源下的标称内核时钟频率
    参数：
        @clock_hz：指向可写 uint32_t 变量的指针，成功时保存频率，单位 Hz
    前置条件：查询期间时钟源、HSI 分频及相关时钟状态保持稳定
    返回值：成功返回 0；以下任一条件成立时返回 1：
            clock_hz 为 NULL、当前选源不是 HSI、
            HSI 未就绪或 HSI 分频未就绪
            失败时不修改有效指针所指向的变量
    说明：当前仅支持 HSI，按标称 64 MHz 和 HSIDIV 配置计算频率；
          结果为 UART 内部 PRESC 预分频之前的频率，不是波特率或实测频率；
          本函数只读取时钟配置，不开启时钟或修改分频
*/
int RCC_UART4_GetKernelClockHz(uint32_t *clock_hz)
{
    if(clock_hz == NULL)
    {
        return 1;
    }
    if (((RCC_UART24CKSELR & 0x7U) != UART_HSI)||(RCC_OCRDYR & 0x05U) != 0x05U)
    {
        return 1;
    }
    uint8_t div_code = RCC_HSICFGR & 0x3U;
    *clock_hz = 64000000U >> div_code;
    return 0;
}
