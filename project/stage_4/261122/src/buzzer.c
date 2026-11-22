#include "buzzer.h"
#include "gpio.h"
#include "gtim.h"

/* TIM4 与 TIM2 同属 APB1 定时器组，沿用已核对的约 208.878MHz。
 * f_PWM = 208877929.6875 / (10444 * 10) ≈ 1999.98Hz。
 * PWM1、高有效：5/10 = 50%。三个参数均为寄存器原始值。
 */
#define BUZZER_PSC       10443U
#define BUZZER_ARR       9U
#define BUZZER_COMPARE   5U

static uint8_t buzzer_initialized;

static const gpioinit_t buzzer_output_low = {
    .Mode = GPIO_OUTPUT_MODE,
    .Otype = GPIO_OUTPUT_PP,
    .Speed = GPIO_LOW_SPEED,
    .Pin = GPIO_PIN_6,
    .Pupd = GPIO_NOPPNOPD,
    .State = GPIO_PIN_RESET,
    .AlternateFunc = GPIO_AF_2
};

static const gpioinit_t buzzer_alternate = {
    .Mode = GPIO_AF_MODE,
    .Otype = GPIO_OUTPUT_PP,
    .Speed = GPIO_LOW_SPEED,
    .Pin = GPIO_PIN_6,
    .Pupd = GPIO_PULLDOWN,
    .State = GPIO_PIN_RESET,
    .AlternateFunc = GPIO_AF_2
};

/*
    功能：准备固定音调，完成后保持静音
          调用前须开启 GPIOB/TIM4 时钟，当前程序独占 PB6/TIM4。
          先将负载关断，再配计数基础与 CH1；装载完成后清初始化 UIF。
          GPIO 驱动无错误返回，本函数使用经核对的固定 GPIO 配置。
          可重复初始化，但会重置 TIM4，频率依赖当前板级时钟配置。
    参数：无；内部固定使用 PB6/AF2/TIM4_CH1，约 2kHz、50% 占空比
    返回值：成功返回 0；底层定时器配置失败返回 -1，且保持未初始化状态；
            正确供钟和 GPIO 配置生效的前提下，PB6 保持普通输出低
*/
int Buzzer_Init(void)
{
    const gtim_pwm_config_t config = {
        .Mode = TIM_PWM_MODE_1,
        .Polarity = TIM_POLARITY_HIGH,
        .CompareValue = BUZZER_COMPARE
    };

    buzzer_initialized = 0U;
    HAL_GPIO_Init(&GPIOB, &buzzer_output_low);
    if (GTIM_BaseInit(TIM4, BUZZER_PSC, BUZZER_ARR) != 0)
    {
        return -1;
    }
    if (TIM_PWM_ConfigChannel(TIM4, TIM_CHANNEL_1, &config) != 0)
    {
        return -1;
    }
    if ((TIM_GenerateUpdate(TIM4) != 0) || (TIM_ClearUpdateFlag(TIM4) != 0))
    {
        return -1;
    }
    buzzer_initialized = 1U;
    return 0;
}

/*
    功能：启动固定音调，先让 PWM 运行，再通过 AF2 将信号输出到 PB6
          调用前须成功完成 Buzzer_Init，且 GPIOB/TIM4 时钟保持开启。
          可重复调用，不产生更新事件、不重置 CNT，不保证首段脉冲完整。
    参数：无
    返回值：成功返回 0；底层 PWM 启动失败返回 -1；尚未初始化返回 -2
*/
int Buzzer_Start(void)
{
    if (buzzer_initialized == 0U)
    {
        return -2;
    }
    if (TIM_PWM_Start(TIM4, TIM_CHANNEL_1) != 0)
    {
        return -1;
    }
    HAL_GPIO_Init(&GPIOB, &buzzer_alternate);
    return 0;
}

/*
    功能：关闭 TIM4_CH1，再将 PB6 切为普通推挽输出低以实现静音
          调用前须成功完成 Buzzer_Init，且 GPIOB/TIM4 时钟保持开启。
          不停止 TIM4 计数；关通道与引脚电平处理分开，可重复调用。
    参数：无
    返回值：成功返回 0；底层关闭通道失败返回 -1，但仍尝试将 PB6 输出低；
            尚未初始化返回 -2，不操作寄存器
*/
int Buzzer_Stop(void)
{
    if (buzzer_initialized == 0U)
    {
        return -2;
    }
    int status = TIM_PWM_Stop(TIM4, TIM_CHANNEL_1);
    HAL_GPIO_Init(&GPIOB, &buzzer_output_low);
    return (status == 0) ? 0 : -1;
}
