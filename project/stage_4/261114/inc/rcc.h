#ifndef RCC_H
#define RCC_H

#include <stdint.h>
#include <stddef.h>

#define RCC_MP_APB1ENSETR (*(volatile uint32_t *)0x50000A00U)
#define RCC_OCENSETR      (*(volatile uint32_t *)0x5000000CU)
#define RCC_UART24CKSELR  (*(volatile uint32_t *)0x500008E8U)
#define RCC_HSICFGR       (*(volatile uint32_t *)0x50000018U)
#define RCC_OCRDYR        (*(volatile uint32_t *)0x50000808U)

typedef enum
{
    UART4_PCLK1 = 0U,
    UART4_PLL4_Q,
    UART_HSI,
    UART_CSI,
    UART_HSE
}uart4_clock_source_e;

int RCC_UART4_SelectClock(uart4_clock_source_e source);
int RCC_UART4_EnableClock(void);
int RCC_HSI_EnableAndWaitReady(void);
int RCC_UART4_GetKernelClockHz(uint32_t *clock_hz);
void RCC_TIM2_EnableClock(void);
/*
    功能：通过 MPU 侧 RCC 开启 TIM4 时钟，不改变共享时钟源或启动计数
    参数：无
    返回值：无；不检查时钟使能是否成功
*/
void RCC_TIM4_EnableClock(void);

#endif
