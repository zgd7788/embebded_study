#ifndef RCC_H
#define RCC_H

#include <stdint.h>
#include <stddef.h>
#include "gpio.h"

#define RCC_MP_APB1ENSETR (*(volatile uint32_t *)0x50000A00U)
#define RCC_OCENSETR      (*(volatile uint32_t *)0x5000000CU)
#define RCC_UART24CKSELR  (*(volatile uint32_t *)0x500008E8U)
#define RCC_HSICFGR       (*(volatile uint32_t *)0x50000018U)
#define RCC_OCRDYR        (*(volatile uint32_t *)0x50000808U)

/* RCC_MP_AHB4ENSETR */
#define GPIOA_EN_Msk     (1U)
#define GPIOB_EN_Pos     (1U)
#define GPIOB_EN_Msk     (1U << GPIOB_EN_Pos)
#define GPIOC_EN_Pos     (2U)
#define GPIOC_EN_Msk     (1U << GPIOC_EN_Pos)
#define GPIOD_EN_Pos     (3U)
#define GPIOD_EN_Msk     (1U << GPIOD_EN_Pos)
#define GPIOE_EN_Pos     (4U)
#define GPIOE_EN_Msk     (1U << GPIOE_EN_Pos)
#define GPIOF_EN_Pos     (5U)
#define GPIOF_EN_Msk     (1U << GPIOF_EN_Pos)
#define GPIOG_EN_Pos     (6U)
#define GPIOG_EN_Msk     (1U << GPIOG_EN_Pos)
#define GPIOH_EN_Pos     (7U)
#define GPIOH_EN_Msk     (1U << GPIOH_EN_Pos)
#define GPIOI_EN_Pos     (8U)
#define GPIOI_EN_Msk     (1U << GPIOI_EN_Pos)
#define GPIOJ_EN_Pos     (9U)
#define GPIOJ_EN_Msk     (1U << GPIOJ_EN_Pos)
#define GPIOK_EN_Pos     (10U)
#define GPIOK_EN_Msk     (1U << GPIOK_EN_Pos)

/* RCC_MP_AHB5ENSETR */
#define GPIOZ_EN_Msk     (1U)

/* RCC_MP_APB1ENSETR */
#define TIM2_EN_Msk     (1U << 0U)
#define TIM4_EN_Pos     (2U)
#define TIM4_EN_Msk     (1U << TIM4_EN_Pos)
#define UART4_EN_Pos    (16U)
#define UART4_EN_Msk    (1U << UART4_EN_Pos)
#define I2C5_EN_Pos     (24U)
#define IIC5_EN_Msk     (1U << I2C5_EN_Pos)

typedef enum
{
    UART4_PCLK1 = 0U,
    UART4_PLL4_Q,
    UART_HSI,
    UART_CSI,
    UART_HSE
}uart4_clock_source_e;

int RCC_UART4_SelectClock(uart4_clock_source_e source);
void RCC_UART4_EnableClock(void);
int RCC_HSI_EnableAndWaitReady(void);
int RCC_UART4_GetKernelClockHz(uint32_t *clock_hz);

int RCC_GPIO_EnableClock(volatile gpio_t *GPIOx);
void RCC_TIM2_EnableClock(void);
/*
    功能：通过 MPU 侧 RCC 开启 TIM4 时钟，不改变共享时钟源或启动计数
    参数：无
    返回值：无；不检查时钟使能是否成功
*/
void RCC_TIM4_EnableClock(void);
void RCC_I2C5_EnableClock(void);
#endif
