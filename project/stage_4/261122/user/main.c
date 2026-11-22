/* brief: 通过 UART4 报告 I2C5 单字节写入结果 */

#include "gpio.h"
#include "uart.h"
#include "rcc.h"
#include "gtim.h"
#include "timebase.h"
#include "i2c.h"

#define AUDIO_RESET_WAIT_TICKS  2U
#define I2C_WRITE_TIMEOUT_TICKS 1000U
#define UART_TX_TIMEOUT_TICKS   1000U

/* UART4 接到板上的调试串口。 */
#define UART4_TX_PORT GPIOG
#define UART4_TX_PIN GPIO_PIN_11
#define UART4_RX_PORT GPIOB
#define UART4_RX_PIN GPIO_PIN_2

/* I2C5 SCL:PA11: SDA:PA12 */
#define I2C_SCL_PORT    GPIOA
#define I2C_SCL_PIN     GPIO_PIN_11
#define I2C_SDA_PORT    GPIOA
#define I2C_SDA_PIN     GPIO_PIN_12

/*
    功能：配置 PG11/AF6 和 PB2/AF8，用 HSI 时钟初始化 UART4 为 115200 8N1。
    参数：无
    返回值：成功返回 0，GPIO 时钟或 UART4 初始化失败返回 1。
*/
static int console_init(void)
{
    const gpioinit_t uart4_tx_config = {
        .Mode = GPIO_AF_MODE,
        .Otype = GPIO_OUTPUT_PP,
        .Speed = GPIO_LOW_SPEED,
        .Pin = UART4_TX_PIN,
        .Pupd = GPIO_NOPPNOPD,
        .AlternateFunc = GPIO_AF_6,
    };
    const gpioinit_t uart4_rx_config = {
        .Mode = GPIO_AF_MODE,
        .Otype = GPIO_OUTPUT_PP,
        .Speed = GPIO_LOW_SPEED,
        .Pin = UART4_RX_PIN,
        .Pupd = GPIO_PULLUP,
        .AlternateFunc = GPIO_AF_8,
    };
    const uart_init_t uart4_config = {
        .BaudRate = 115200U,
        .WordLength = UART_WORDLENGTH_8BIT,
        .Paritysel = UART_PARITY_NONE,
        .Stopbits = UART_STOP_1BIT,
        .FifoMode = UART_FIFO_ENABLE,
    };

    if (RCC_GPIO_EnableClock(&UART4_TX_PORT) != 0 ||
        RCC_GPIO_EnableClock(&UART4_RX_PORT) != 0)
    {
        return 1;
    }
    HAL_GPIO_Init(&UART4_TX_PORT, &uart4_tx_config);
    HAL_GPIO_Init(&UART4_RX_PORT, &uart4_rx_config);

    uint32_t uart_clock_hz;
    RCC_UART4_EnableClock();
    if (UART4_DeInit() != 0 ||
        RCC_HSI_EnableAndWaitReady() != 0 ||
        RCC_UART4_SelectClock(UART_HSI) != 0 ||
        RCC_UART4_GetKernelClockHz(&uart_clock_hz) != 0 ||
        UART4_Init(&uart4_config, uart_clock_hz) != 0)
    {
        return 1;
    }
    return 0;
}

/*
    功能：通过 UART4 逐字节输出诊断消息，提交完毕后等待最后一帧发完。
          调用前须初始化 UART4 并启动 TIM2，等待均设有超时。
    参数：
        @message：指向以 '\0' 结尾的字符串，需要换行时由调用方提供 '\r' 和 '\n'。
    返回值：无。提交字节超时时提前结束输出。
*/
static void console_write(const char *message)
{
    while (*message != '\0')
    {
        if (HAL_UART4_PutCharTimeout((uint8_t)*message, UART_TX_TIMEOUT_TICKS) != 0)
        {
            return;
        }
        ++message;
    }
    (void)HAL_UART4_WaitTxComplete(UART_TX_TIMEOUT_TICKS);
}

/*
    功能：初始化 TIM2、UART4 和 I2C5，释放 CS42L51 复位并等待两个 TIM2 tick，
          随后向 7 位地址 0x4A 发送寄存器索引 0x01，通过 UART4 报告结果。
    参数：无
    返回值：I2C 写入成功返回 0，初始化或写入失败返回 1。
*/
int main(void)
{
    const gpioinit_t audio_reset_config = {
        .Mode = GPIO_OUTPUT_MODE,
        .Otype = GPIO_OUTPUT_PP,
        .Speed = GPIO_LOW_SPEED,
        .Pin = GPIO_PIN_0,
        .Pupd = GPIO_NOPPNOPD,
        .State = GPIO_PIN_RESET,
    };

    const gpioinit_t i2c5_scl_config = {
        .Mode = GPIO_AF_MODE,
        .Otype = GPIO_OUTPUT_OD,
        .Speed = GPIO_LOW_SPEED,
        .Pin = I2C_SCL_PIN,
        .Pupd = GPIO_NOPPNOPD,
        .State = GPIO_PIN_RESET,
        .AlternateFunc = GPIO_AF_4
    };

    const gpioinit_t i2c5_sda_config = {
        .Mode = GPIO_AF_MODE,
        .Otype = GPIO_OUTPUT_OD,
        .Speed = GPIO_LOW_SPEED,
        .Pin = I2C_SDA_PIN,
        .Pupd = GPIO_NOPPNOPD,
        .State = GPIO_PIN_RESET,
        .AlternateFunc = GPIO_AF_4
    };

    RCC_TIM2_EnableClock();
    if (GTIM_BaseInit(TIM2, 20887U, UINT32_MAX) != 0)
    {
        return 1;
    }
    GTIMx_Start(TIM2);

    if (console_init() != 0)
    {
        return 1;
    }
    if (RCC_GPIO_EnableClock(&GPIOA) != 0 || RCC_GPIO_EnableClock(&GPIOC) != 0)
    {
        console_write("I2C5 GPIO clock setup failed\r\n");
        return 1;
    }
    HAL_GPIO_Init(&GPIOC, &audio_reset_config);
    HAL_GPIO_Init(&I2C_SCL_PORT, &i2c5_scl_config);
    HAL_GPIO_Init(&I2C_SDA_PORT, &i2c5_sda_config);

    RCC_I2C5_EnableClock();
    if (I2C_Init(I2C5, 0xF0421515U) != 0)
    {
        console_write("I2C5 init failed\r\n");
        return 1;
    }

    /* TIM2 约 10 kHz，跨过两个 tick 足以满足复位释放到 START 的 500 ns。 */
    HAL_GPIO_Write(&GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
    const uint32_t reset_released_ticks = Timebase_GetTicks(TIM2);
    while ((Timebase_GetTicks(TIM2) - reset_released_ticks) < AUDIO_RESET_WAIT_TICKS)
    {
    }

    const int result = I2C_MasterWriteByte(I2C5, 0x4AU, 0x01U, I2C_WRITE_TIMEOUT_TICKS);
    switch (result)
    {
        case 0:
            console_write("I2C5 write OK (0x4A, data 0x01)\r\n");
            return 0;
        case -1:
            console_write("I2C5 write invalid argument (-1)\r\n");
            break;
        case -2:
            console_write("I2C5 write timeout (-2)\r\n");
            break;
        case -3:
            console_write("I2C5 write NACK (-3)\r\n");
            break;
        case -4:
            console_write("I2C5 write unexpected STOP (-4)\r\n");
            break;
        default:
            console_write("I2C5 write failed\r\n");
            break;
    }
    return 1;
}
