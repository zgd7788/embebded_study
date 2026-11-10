#include "uart.h"

#define UART_DEINIT_POLL_LIMIT 1000000U

/*
    功能：配置并启用 UART4
    参数：
        @config：串口配置，只读；包括波特率、字长、校验和停止位
        @uart_clock_hz：UART4 当前内核时钟频率，单位 Hz
    前置条件：UART4 外设访问时钟已开启，且 uart_clock_hz 与当前内核时钟一致
    返回值：成功返回 0；参数无效、BRR 超出支持范围或波特率误差超过 2% 返回 1
    说明：本实现固定使用 16 倍过采样、PRESC 不分频并关闭硬件流控；成功时使能收发和 UART4
*/
int UART4_Init(const uart_init_t *config, uint32_t uart_clock_hz)
{
    if(config ==NULL)
    {
        return 1;
    }
    if ((config->BaudRate == 0U) || (uart_clock_hz == 0U))
    {
        return 1;
    }

    uint32_t BaudRate = config->BaudRate;
    const uint32_t stop_mask = 3U;
    uint32_t cr1_config = 0U;
    uint32_t cr2_config = 0U;

    uint64_t brr =
    ((uint64_t)uart_clock_hz + BaudRate / 2U) / BaudRate;
    if ((brr < 0x10U) || (brr > 0xFFFFU))
    {
        return 1;
    }
    uint64_t expected_clock = (uint64_t)BaudRate * brr;
    uint64_t difference;

    if ((uint64_t)uart_clock_hz >= expected_clock)
    {
        difference = (uint64_t)uart_clock_hz - expected_clock;
    }
    else
    {
        difference = expected_clock - (uint64_t)uart_clock_hz;
    }

    if (difference * 100U > expected_clock * 2U)
    {
        return 1;
    }
    switch (config->WordLength)
    {
        /* 将字长转换成 CR1 的 M1、M0 配置 */
        /* default：返回参数错误 */
        case UART_WORDLENGTH_8BIT:
            cr1_config &= ~((1U << 12U)|(1U << 28U));
        break;
        case UART_WORDLENGTH_9BIT:
            cr1_config |= (1U << 12U);
        break;
        case UART_WORDLENGTH_7BIT:
            cr1_config |= (1U << 28U); 
        break;
        default:
            return 1;
        break;
    }

    switch (config->Paritysel)
    {
        /* 将校验方式转换成 CR1 的 PCE、PS 配置 */
        /* default：返回参数错误 */
        case UART_PARITY_ODD:
            /* PCE=1，PS=1 */
            cr1_config |= (1U << 10U);
            cr1_config |= (1U << 9U);
        break;
        case UART_PARITY_EVEN:
            /* PCE=1，PS=0 */
            cr1_config |= (1U << 10U);
        break;
        case UART_PARITY_NONE:
            /* PCE=0，PS=0 */
        break;
        default:
            return 1;
        break;
    }

    switch (config->Stopbits)
    {
        /* 将停止位转换成 CR2 的 STOP 配置 */
        /* default：返回参数错误 */
        case UART_STOP_1BIT:
            /* STOP=00 */
        break;
        case UART_STOP_0_5BIT:
            /* STOP=01 */
            cr2_config |= 0x1U << 12U;
        break;
        case UART_STOP_2BIT:
            /* STOP=10 */
            cr2_config |= 0x2U << 12U;
        break;
        case UART_STOP_1_5BIT:
            /* STOP=11 */
            cr2_config |= (stop_mask << 12U); 
        break;
        default:
            return 1;
        break;
    }
    /* 关闭 UART4 */
    UART4->CR1 &= ~(1U << 0U);
    /* 配置波特率 */
    UART4->BRR = (uint16_t)brr;
    /* 配置字长和校验 */
    UART4->CR1 &= ~((1U << 12U) | (1U << 28U) |
                   (1U << 10U) | (1U << 9U));
    UART4->CR1 |= cr1_config;
    /* 配置停止位 */
    UART4->CR2 &= ~(3U << 12U);
    UART4->CR2 |= cr2_config;
    /* 设置16倍过采样 */
    UART4->CR1 &= ~(1U << 15U);
    /* 设置不分频 */
    UART4->PRESC &= ~(0xFU);
    /* 关闭硬件流控 */
    UART4->CR3 &= ~(3U << 8U);
    /* 设置发送使能和接收使能 */
    UART4->CR1 |= (3U << 2U);
    /* 使能UART4 */
    UART4->CR1 |= (1U << 0U);
    return 0;
}

/*
    功能：停止 UART4 收发并禁用外设
    参数：无
    前置条件：UART4 外设访问时钟已开启
    返回值：成功返回 0；UART 正在发送且在有限轮询次数内未检测到 TC 返回 1
    说明：UE 和 TE 均已置位时先等待传输完成；轮询次数不是精确的时间单位
*/
int UART4_DeInit(void)
{
    const uint32_t ue_te_mask = (1U << 0U) | (1U << 3U);
    const uint32_t disable_mask = ue_te_mask | (1U << 2U);

    /* UART正在使能发送时，先等待移位寄存器中的最后一帧发送完成。 */
    if ((UART4->CR1 & ue_te_mask) == ue_te_mask)
    {
        uint32_t timeout = UART_DEINIT_POLL_LIMIT;

        while ((UART4->ISR & UART_ISR_TC) == 0U)
        {
            if (timeout == 0U)
            {
                return 1;
            }
            timeout--;
        }
    }

    /* TC超时会在上面提前返回；只有确认发送完成后才关闭UART。 */
    UART4->CR1 &= ~disable_mask;
    return 0U;
}

/*
    功能：阻塞发送一个字节
    参数：@data：要发送的数据
    前置条件：UART4 已完成初始化，且已使能发送功能
    返回值：无
    说明：等待 ISR bit7 表示发送寄存器/FIFO 可接收数据后写入 TDR；本函数无超时
*/
void HAL_UART4_PutChar(uint8_t data)
{
    while((UART4->ISR & UART_ISR_TXNF) == 0U);
    UART4->TDR = data;
}

/*
    功能：阻塞接收一个字节
    参数：@data_out：指向可写 uint8_t 变量的指针，成功时保存收到的数据
    前置条件：UART4 已完成初始化，且已使能接收功能
    返回值：成功接收并写入数据返回 0；data_out 为空返回 1
    说明：等待 ISR.RXNE（bit5）置位后读取 RDR；等待过程无超时
*/
int HAL_UART4_GetChar(uint8_t *data_out)
{
    if(data_out == NULL)
    {
        return 1U;
    }
    while((UART4->ISR & UART_ISR_RXNE) == 0U);
    *data_out = UART4->RDR;
    return 0U;
}

/*
    功能：阻塞发送一个以 '\0' 结尾的字符串
    参数：@str：指向只读、以 '\0' 结尾的字符串
    前置条件：UART4 已完成初始化，且已使能发送功能
    返回值：字符串发送完成返回 0；str 为空返回 1
    说明：空字符串发送成功；不发送结尾 '\0'，也不自动追加换行
*/
int HAL_UART4_PutString(const char *str)
{
    if (str == NULL)
    {
        return 1U;
    }
    while (*str != '\0')
    {
        HAL_UART4_PutChar(*str++);
    }
    return 0U;
}

/*
    功能：阻塞接收一行字符并保存为以 '\0' 结尾的字符串
    参数：
        @buf：接收缓冲区
        @buf_size：缓冲区容量，包含结尾 '\0' 所占空间
    前置条件：UART4 已完成初始化，且已使能接收功能
    返回值：正常收到一行返回 0；参数无效或底层接收失败返回 1；行内容过长返回 2
    说明：CR 或 LF 结束一行；CR 后的可选 LF 会在下次调用时跳过。
          最多保存 buf_size-1 个字符；超长时丢弃本行剩余字符直到行结束。
          内部 skip_lf 状态为 static，因此本函数不适合并发或重入调用
*/
int HAL_UART4_GetString(char *buf, uint8_t buf_size)
{
    if ((buf == NULL) || (buf_size == 0U))
    {
        return 1U;
    }

    uint8_t data;
    uint8_t index = 0U;
    static uint8_t skip_lf = 0U;
    uint8_t overflow = 0U;

    for (;;)
    {
        if (HAL_UART4_GetChar(&data) != 0U)
        {
            buf[index] = '\0';
            return 1U;
        }

        if (skip_lf != 0U)
        {
            skip_lf = 0U;
            if (data == '\n')
            {
                continue;
            }
        }

        if ((data == '\r') || (data == '\n'))
        {
            skip_lf = (data == '\r');
            break;
        }

        if (index < (uint8_t)(buf_size - 1U))
        {
            buf[index++] = (char)data;
        }
        else
        {
            overflow = 1U;
        }
    }

    buf[index] = '\0';
    return (overflow != 0U) ? 2 : 0;
}
