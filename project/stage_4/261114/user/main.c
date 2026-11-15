/* brief: TIM4 蜂鸣器间歇鸣响，TIM2 计时，UART4 保留字符回显 */

#include "gpio.h"
#include "uart.h"
#include "rcc.h"
#include "gtim.h"
#include "timebase.h"
#include "buzzer.h"

#define BUZZER_INTERVAL_TICKS 5000U /* 当前 TIM2 频率下约 500ms。 */

/* led1 */
#define LED1_PORT   GPIOZ
#define LED1_PIN    GPIO_PIN_5
/* rx:PB2 tx:PG11 */
#define UART4_TX_PORT GPIOG
#define UART4_TX_PIN GPIO_PIN_11
#define UART4_RX_PORT GPIOB
#define UART4_RX_PIN GPIO_PIN_2

/*
    功能：尝试静音后停留在无限循环，不在 UART 故障路径继续打印
          尚未初始化蜂鸣器时，Buzzer_Stop 不操作寄存器；静音返回值被忽略
    参数：无
    返回值：无；本函数不会返回调用方
*/
static void application_stop(void)
{
    (void)Buzzer_Stop();
    for (;;)
    {
    }
}

/*
    功能：用 TIM4_CH1 驱动蜂鸣器，每约 500ms 切换鸣响/静音
          调用前由启动代码设置好栈、清零 BSS，并提供可访问的运行内存。
          TIM2 独立自由运行，UART 每轮只立即检查一次接收状态，
          避免原来最长 5 秒的接收等待延后鸣响切换；正常回显完成后翻转 LED。
          发送提交和 TC 等待仍各自有超时，主循环调度不是硬实时保证。
    参数：无
    返回值：无正常返回；初始化失败或进入主循环后均停留在循环中
*/
int main(void)
{
    uint32_t clock_hz;
    const gpioinit_t uart4_tx_config = {
        .Mode = GPIO_AF_MODE,
        .Otype = GPIO_OUTPUT_PP,
        .Speed = GPIO_LOW_SPEED,
        .Pin = UART4_TX_PIN,
        .Pupd = GPIO_NOPPNOPD,
        .State = GPIO_PIN_RESET,
        .AlternateFunc = GPIO_AF_6
    };

    const gpioinit_t uart4_rx_config = {
        .Mode = GPIO_AF_MODE,
        .Speed = GPIO_LOW_SPEED,
        .Pin = UART4_RX_PIN,
        .Pupd = GPIO_PULLUP,
        .State = GPIO_PIN_RESET,
        .AlternateFunc = GPIO_AF_8
    };

    const gpioinit_t led1_config = {
        .Mode = GPIO_OUTPUT_MODE,
        .Otype = GPIO_OUTPUT_PP,
        .Pin = GPIO_PIN_5,
        .Speed = GPIO_LOW_SPEED,
        .State = GPIO_PIN_RESET,
    
    };
    RCC_MP_AHB4ENSETR = (1U << 1U)|((1U << 6U));

    RCC_MP_AHB5ENSETR = (1U << 0);
    RCC_TIM2_EnableClock();
    HAL_GPIO_Init(&LED1_PORT, &led1_config);
    HAL_GPIO_Init(&UART4_TX_PORT, &uart4_tx_config);
    HAL_GPIO_Init(&UART4_RX_PORT, &uart4_rx_config);

    const uart_init_t uart4_config = {
        .Paritysel = UART_PARITY_NONE,
        .Stopbits = UART_STOP_1BIT,
        .WordLength = UART_WORDLENGTH_8BIT,
        .BaudRate = 115200U,
        .FifoMode = UART_FIFO_ENABLE,
    };
  
    if (RCC_UART4_EnableClock() != 0)
    {
        for (;;)
        {
        }
    }
    if (UART4_DeInit() != 0)
    {
        for (;;)
        {
        }
    }
    if (RCC_HSI_EnableAndWaitReady() != 0)
    {
        for (;;)
        {
        }
    }
    if (RCC_UART4_SelectClock(UART_HSI) != 0)
    {
        for (;;)
        {
        }
    }
    if (RCC_UART4_GetKernelClockHz(&clock_hz) != 0)
    {
        for (;;)
        {
        }
    }
    if (UART4_Init(&uart4_config, clock_hz) != 0)
    {
        for (;;)
        {
        }
    }
    uint8_t data;
    int ret;
    if (GTIM_BaseInit(TIM2, 20887U, UINT32_MAX) != 0)
    {
        application_stop();
    }
    GTIMx_Start(TIM2);

    /* GPIOB 已为 UART4_RX 开启时钟；PB6 与 PB2 可以独立配置。 */
    RCC_TIM4_EnableClock();
    if (Buzzer_Init() != 0)
    {
        application_stop();
    }
    if (Buzzer_Start() != 0)
    {
        application_stop();
    }
    uint8_t buzzer_on = 1U;
    uint32_t last_buzzer_tick = Timebase_GetTicks(TIM2);

    for (;;)
    {
        uint32_t now = Timebase_GetTicks(TIM2);
        if ((uint32_t)(now - last_buzzer_tick) >= BUZZER_INTERVAL_TICKS)
        {
            ret = (buzzer_on != 0U) ? Buzzer_Stop() : Buzzer_Start();
            if (ret != 0)
            {
                application_stop();
            }
            buzzer_on ^= 1U;
            last_buzzer_tick = now;
        }

        ret = HAL_UART4_GetCharTimeout(&data, 0U);
        if(ret == 0U)
        {
            ret = HAL_UART4_PutCharTimeout(data, 10000U);
            if (ret != 0)
            {
                /* 提交失败：本次未写入 TDR，停止处理，不再调用串口打印。 */
                application_stop();
            }

            ret = HAL_UART4_WaitTxComplete(10000U);
            if (ret != 0)
            {
                /* TC 等待超时：不保证此前提交的数据已经全部发完。 */
                application_stop();
            }

            HAL_GPIO_Toggle(&LED1_PORT, LED1_PIN);
        }
        else if (ret == 2)
        {
            /* 立即检查时没有数据，继续主循环调度蜂鸣器。 */
        }
        else
        {
            /* 接收接口返回参数错误或其他非预期状态，停止本次实验。 */
            application_stop();
        }
    }
}
