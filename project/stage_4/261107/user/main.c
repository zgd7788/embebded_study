/* brief: 串口 */

#include "gpio.h"
#include "uart.h"
#include "rcc.h"

#define DELAY_LOOP_COUNT 80000000U

/* led1 */
#define LED1_PORT   GPIOZ
#define LED1_PIN    GPIO_PIN_5
/* rx:PB2 tx:PG11 */
#define UART4_TX_PORT GPIOG
#define UART4_TX_PIN GPIO_PIN_11
#define UART4_RX_PORT GPIOB
#define UART4_RX_PIN GPIO_PIN_2

/*
    功能：执行一段粗略忙等待
    参数：无
    返回值：无
    说明：延时受 CPU 时钟和编译结果影响，不是精确计时接口
*/
// static void delay_roughly(void)
// {
//     volatile uint32_t count = DELAY_LOOP_COUNT;

//     while (count > 0U)
//     {
//         count--;
//     }
// }

/*
    功能：比较两个以 '\0' 结尾的字符串是否完全相同
    参数：
        @a：第一个有效的 NUL 结尾字符串
        @b：第二个有效的 NUL 结尾字符串
    前置条件：a 和 b 均非空指针，并指向可读的 NUL 结尾字符串
    返回值：完全相同返回 1，否则返回 0
*/
// static int strings_equal(const char *a, const char *b)
// {
//     while ((*a != '\0') && (*a == *b))
//     {
//         a++;
//         b++;
//     }

//     return (*a == '\0') && (*b == '\0');
// }

/*
    功能：初始化 GPIO、UART4 和时钟，并处理串口命令
    参数：无
    前置条件：由启动代码设置好栈、清零 BSS，并在可访问的内存环境中调用
    返回值：无正常返回；初始化失败或进入主循环后停留在循环中
    说明：批量接收回显
*/
int main(void)
{
    uint8_t buf[20];
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
    size_t count;
    size_t cnt;
    size_t offset = 0U;
    int ret;
    for (;;)
    {
        int result = HAL_UART4_ReadAvailable(buf, sizeof(buf), &count);
        if(result == 0U)
        {
            while (offset < count)
            {
                ret = HAL_UART4_WriteAvailable(buf + offset, count - offset, &cnt);
                if(ret != 0U)
                {
                    for(;;);
                }
                offset += cnt;
            }
            offset = 0U;
        }
    }
}
