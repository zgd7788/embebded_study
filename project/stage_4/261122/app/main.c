/* brief: I2C5 验证 CS42L51 的寄存器写入、原值恢复和多字节连续接收。 */

#include "gpio.h"
#include "uart.h"
#include "rcc.h"
#include "gtim.h"
#include "timebase.h"
#include "i2c.h"

#define AUDIO_RESET_WAIT_TICKS  2U
#define I2C_WRITE_TIMEOUT_TICKS 1000U
#define I2C_READ_TIMEOUT_TICKS  1000U
#define UART_TX_TIMEOUT_TICKS   1000U

#define CS42L51_ADDR7          0x4AU
#define CS42L51_ID_REG         0x01U
#define CS42L51_ID_MASK        0xF8U
#define CS42L51_ID_EXPECTED    0xD8U
/* DS679F1 第 45 页：MAP.bit7=1 使寄存器指针每传输一个数据字节后自增。 */
#define CS42L51_MAP_INCR       0x80U
#define CS42L51_READ_TEST_LENGTH 3U
/* DS679F1 第 60 页：0x0C 八位均为 ADCA 衰减量，FF/FE 分别为 -1/-2 dB。 */
#define CS42L51_ADCA_ATT_REG   0x0CU
#define CS42L51_ATT_TEST_A     0xFFU
#define CS42L51_ATT_TEST_B     0xFEU

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
    功能：把一个字节转换为带 0x 前缀的两位大写十六进制字符串，通过 UART4 输出。
          调用前须初始化 UART4 并启动 TIM2，换行由调用方单独输出。
    参数：
        @value：要显示的一个字节，取值范围为 0～255。
    返回值：无。串口超时时沿用 console_write 的提前结束策略。
*/
static void console_write_hex8(uint8_t value)
{
    const char hex[] = "0123456789ABCDEF";
    char message[] = "0x00";
    message[2] = hex[value >> 4U];
    message[3] = hex[value & 0x0FU];
    console_write(message);
}

/*
    功能：通过 UART4 输出失败操作的名称以及 I2C 返回码对应的错误原因。
          调用前须初始化 UART4 并启动 TIM2。
    参数：
        @operation：失败操作的名称，必须是非空、以 '\0' 结尾的字符串。
        @result：I2C 接口返回的非零错误码，当前解释 -1/-2/-3/-4。
    返回值：无。未知错误码显示通用失败消息。
*/
static void console_report_i2c_error(const char *operation, int result)
{
    console_write(operation);
    console_write(": ");
    switch (result)
    {
        case -1:
            console_write("I2C invalid argument (-1)\r\n");
            break;
        case -2:
            console_write("I2C timeout (-2)\r\n");
            break;
        case -3:
            console_write("I2C NACK (-3)\r\n");
            break;
        case -4:
            console_write("I2C unexpected STOP (-4)\r\n");
            break;
        default:
            console_write("I2C failed\r\n");
            break;
    }
}

/*
    功能：分别单字节读取 CS42L51 的 0x01～0x03 作为参考，再设置 MAP.INCR，
          通过 I2C_MasterRead 一次接收三个字节，与参考值逐项比较并输出结果。
          调用前须完成 UART4、I2C5、TIM2 初始化并释放音频芯片复位。
          此过程只写寄存器地址指针，不修改三个寄存器的内容；验证期间
          不允许其他代码修改这些寄存器，以保证前后两次读取可比较。
    参数：无
    返回值：所有访问成功且三个字节与逐个读取的结果一致返回 0，
            I2C 访问失败或任一字节比较不一致返回 1。
*/
static int cs42l51_verify_multibyte_read(void)
{
    uint8_t reference[CS42L51_READ_TEST_LENGTH];
    uint8_t received[CS42L51_READ_TEST_LENGTH];

    /* 参考值来自当前芯片，不把电源/时钟控制寄存器的复位默认值写死。 */
    for (size_t i = 0U; i < sizeof(reference); ++i)
    {
        const uint8_t reg = (uint8_t)(CS42L51_ID_REG + i);
        const int result = I2C_ReadRegisterByte(I2C5, CS42L51_ADDR7, reg,
                                                &reference[i], I2C_READ_TIMEOUT_TICKS);
        if (result != 0)
        {
            console_report_i2c_error("single register reference read", result);
            return 1;
        }
    }

    /* 0x81 = INCR=1 + 起始寄存器0x01。MAP 是发送的数据，从机地址仍为0x4A。
       这里采用手册中的 STOP → START：先发 MAP，再发起独立的读事务。 */
    const uint8_t map = CS42L51_MAP_INCR | CS42L51_ID_REG;
    int result = I2C_MasterWrite(I2C5, CS42L51_ADDR7, &map, sizeof(map),
                                  I2C_WRITE_TIMEOUT_TICKS);
    if (result != 0)
    {
        console_report_i2c_error("auto-increment MAP write", result);
        return 1;
    }
    result = I2C_MasterRead(I2C5, CS42L51_ADDR7, received, sizeof(received),
                             I2C_READ_TIMEOUT_TICKS);
    if (result != 0)
    {
        /* 失败时 received 可能只有部分数据，不显示或比较不完整结果。 */
        console_report_i2c_error("multi-byte read", result);
        return 1;
    }

    int status = 0;
    for (size_t i = 0U; i < sizeof(received); ++i)
    {
        console_write("REG");
        console_write_hex8((uint8_t)(CS42L51_ID_REG + i));
        console_write(" single=");
        console_write_hex8(reference[i]);
        console_write(" burst=");
        console_write_hex8(received[i]);
        console_write("\r\n");
        if (received[i] != reference[i])
        {
            status = 1;
        }
    }
    console_write((status == 0) ? "I2C multi-byte read PASS\r\n" :
                                  "I2C multi-byte read FAIL\r\n");
    return status;
}

/*
    功能：初始化 TIM2、UART4 和 I2C5，释放 CS42L51 复位并等待两个 TIM2 tick，
          读取并核对芯片 ID，再对 ADCA 衰减寄存器 0x0C 做多字节写入验证。
          验证顺序为读取原值、写入不同测试值、读回比较、恢复原值并再次读回。
          尝试写测试值之后，即使写入或读回失败也会尝试恢复原值；恢复失败
          会明确报告，不能视为配置已经恢复。写入测试和恢复均成功后，
          比较 0x01～0x03 的单字节参考值与多字节接收结果。
          本例只验证控制寄存器访问，不开启音频数据传输。
    参数：无
    返回值：初始化、ID 核对、写入与恢复、多字节接收比较全部成功返回 0，
            任一步失败返回 1。
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

    /* 先确认目标器件身份，只核对高五位，允许不同的低三位修订号。 */
    uint8_t id;
    int result = I2C_ReadRegisterByte(I2C5, CS42L51_ADDR7, CS42L51_ID_REG,
                                      &id, I2C_READ_TIMEOUT_TICKS);
    if (result != 0)
    {
        console_report_i2c_error("ID read", result);
        return 1;
    }
    console_write("ID=");
    console_write_hex8(id);
    console_write("\r\n");
    if ((id & CS42L51_ID_MASK) != CS42L51_ID_EXPECTED)
    {
        console_write("unexpected chip ID, write test skipped\r\n");
        return 1;
    }

    /* 0x01 是只读 ID；本次选择可写的 0x0C，并先保存完整原值。 */
    uint8_t original;
    result = I2C_ReadRegisterByte(I2C5, CS42L51_ADDR7, CS42L51_ADCA_ATT_REG,
                                  &original, I2C_READ_TIMEOUT_TICKS);
    if (result != 0)
    {
        console_report_i2c_error("original value read", result);
        return 1;
    }
    const uint8_t test_value = (original != CS42L51_ATT_TEST_A) ?
                               CS42L51_ATT_TEST_A : CS42L51_ATT_TEST_B;
    console_write("REG0x0C original=");
    console_write_hex8(original);
    console_write("\r\nREG0x0C test=");
    console_write_hex8(test_value);
    console_write("\r\n");

    /* 一个 START 中依次发送“寄存器索引 + 数据”，所以 NBYTES=2。
       MAP 的 INCR 位保持 0，此处只修改 0x0C，不访问后面的寄存器。 */
    uint8_t write_buffer[2] = {CS42L51_ADCA_ATT_REG, test_value};
    int status = 0;
    result = I2C_MasterWrite(I2C5, CS42L51_ADDR7, write_buffer,
                             sizeof(write_buffer), I2C_WRITE_TIMEOUT_TICKS);
    if (result != 0)
    {
        console_report_i2c_error("test value write", result);
        status = 1;
    }
    else
    {
        uint8_t readback;
        result = I2C_ReadRegisterByte(I2C5, CS42L51_ADDR7, CS42L51_ADCA_ATT_REG,
                                      &readback, I2C_READ_TIMEOUT_TICKS);
        if (result != 0)
        {
            console_report_i2c_error("test value readback", result);
            status = 1;
        }
        else
        {
            console_write("REG0x0C readback=");
            console_write_hex8(readback);
            console_write("\r\n");
            if (readback != test_value)
            {
                console_write("write/readback FAIL\r\n");
                status = 1;
            }
            else
            {
                console_write("write/readback PASS\r\n");
            }
        }
    }

    /* 写失败也可能已有部分数据到达从机，仍尝试写回保存的原值。
       每次接口调用会先等待 BUSY 清零；恢复失败时停止并报告。 */
    write_buffer[1] = original;
    result = I2C_MasterWrite(I2C5, CS42L51_ADDR7, write_buffer,
                             sizeof(write_buffer), I2C_WRITE_TIMEOUT_TICKS);
    if (result != 0)
    {
        console_report_i2c_error("original value restore", result);
        return 1;
    }

    uint8_t restored;
    result = I2C_ReadRegisterByte(I2C5, CS42L51_ADDR7, CS42L51_ADCA_ATT_REG,
                                  &restored, I2C_READ_TIMEOUT_TICKS);
    if (result != 0)
    {
        console_report_i2c_error("restore readback", result);
        return 1;
    }
    console_write("REG0x0C restored=");
    console_write_hex8(restored);
    console_write("\r\n");
    if (restored != original)
    {
        console_write("restore FAIL\r\n");
        return 1;
    }
    console_write("restore PASS\r\n");
    console_write((status == 0) ? "I2C multi-byte write PASS\r\n" :
                                  "I2C multi-byte write FAIL\r\n");
    if (status != 0)
    {
        return 1;
    }
    return cs42l51_verify_multibyte_read();
}
