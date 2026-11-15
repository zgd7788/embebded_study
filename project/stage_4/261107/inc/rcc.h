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

#endif
