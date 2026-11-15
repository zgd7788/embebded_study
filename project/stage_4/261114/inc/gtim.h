#ifndef GTIM_H
#define GTIM_H

#include <stdint.h>

#define TIM2 ((gtim_t *)0x40000000U)
#define TIM3 ((gtim_t *)0x40001000U)
#define TIM4 ((gtim_t *)0x40002000U)
#define TIM5 ((gtim_t *)0x40003000U)
/* CR1 */
#define GTIM_CR1_CEN_Msk          (1U << 0U)
#define GTIM_CR1_UDIS_Msk         (1U << 1U)
#define GTIM_CR1_URS_Msk          (1U << 2U)
#define GTIM_CR1_OPM_Msk          (1U << 3U)
#define GTIM_CR1_DIR_Msk          (1U << 4U)
#define GTIM_CR1_CMS_Msk          (0x3U << 5U)
#define GTIM_CR1_ARPE_Msk         (1U << 7U)
#define GTIM_CR1_UIFREMAP_Msk     (1U << 11U)

/* SMCR、SR、EGR：SMS 是 bits2:0 与 bit16 组成的不连续字段。 */
#define GTIM_SMCR_SMS_Msk         ((0x7U << 0U) | (1U << 16U))
#define GTIM_SMCR_ECE_Msk         (1U << 14U)
#define GTIM_SR_UIF_Msk           (1U << 0U)
#define GTIM_EGR_UG_Msk           (1U << 0U)

/* CCMR：以下以第一个通道为基准，第二个通道整体左移 8 位。 */
#define GTIM_CCMR1_CC1S_Msk       (0x3U << 0U)
#define GTIM_CCMR1_OC1FE_Msk      (1U << 2U)
#define GTIM_CCMR1_OC1PE_Msk      (1U << 3U)
#define GTIM_CCMR1_OC1M_Pos      4U
#define GTIM_CCMR1_OC1M_Msk      ((0x7U << 4U) | (1U << 16U))
#define GTIM_CCMR1_OC1M_PWM1     (0x6U << 4U)
#define GTIM_CCMR1_OC1M_PWM2     (0x7U << 4U)
#define GTIM_CCMR1_OC1CE_Msk      (1U << 7U)

/* CCER：各通道相对于 CH1 分别左移 0、4、8、12 位。 */
#define GTIM_CCER_CC1E_Msk        (1U << 0U)
#define GTIM_CCER_CC1P_Msk        (1U << 1U)
#define GTIM_CCER_CC1NP_Msk       (1U << 3U)

typedef enum
{
    TIM_CHANNEL_1 = 0U,
    TIM_CHANNEL_2,
    TIM_CHANNEL_3,
    TIM_CHANNEL_4,
}gtim_channel_e;

typedef enum
{
    TIM_PWM_MODE_1 = 0U,
    TIM_PWM_MODE_2
}gtim_pwm_mode_e;

typedef enum
{
    TIM_POLARITY_HIGH = 0U,    /* 高电平有效 */
    TIM_POLARITY_LOW,          /* 低电平有效 */
}gtim_polarity_e;

typedef struct
{
    gtim_pwm_mode_e Mode;
    gtim_polarity_e Polarity;
    uint32_t CompareValue;
}gtim_pwm_config_t;

typedef struct
{
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
    uint32_t reserved0;      /* Offset 0x30 is reserved. */
    volatile uint32_t CCR1;
    volatile uint32_t CCR2;
    volatile uint32_t CCR3;
    volatile uint32_t CCR4;
    uint32_t reserved1;      /* Offset 0x44 is reserved. */
    volatile uint32_t DCR;   
    volatile uint32_t DMAR;  
}gtim_t; 

int GTIM_BaseInit(gtim_t *tim, uint16_t psc, uint32_t arr);
void GTIMx_Start(gtim_t *TIMx);
void GTIMx_Stop(gtim_t *TIMx);
uint8_t GTIMx_PollUpdate(gtim_t *TIMx);
uint32_t GTIMx_GetCounter(gtim_t *TIMx);
/*
    功能：显式改变共享计数位置；须开启目标时钟，并允许影响全部通道
    参数：
        @TIMx：TIM2～5 实例指针
        @cnt：CNT 原始值，TIM3/4 最大 65535，TIM2/5 最大 UINT32_MAX
    返回值：成功返回 0；实例或数值位宽无效返回 -1，失败不写寄存器
*/
int TIM_SetCounter(gtim_t *TIMx, uint32_t cnt);

/*
    功能：显式产生 UG，影响整个定时器的预装载及计数；须先开启目标时钟
    参数：
        @TIMx：TIM2～5 实例指针
    返回值：写入 UG 后返回 0，不等待装载完成；实例无效返回 -1
*/
int TIM_GenerateUpdate(gtim_t *TIMx);

/*
    功能：清 UIF 并保留其他状态；须先开启目标时钟
    参数：
        @TIMx：TIM2～5 实例指针
    返回值：写入清除值后返回 0；实例无效返回 -1，失败不写寄存器
*/
int TIM_ClearUpdateFlag(gtim_t *TIMx);

/*
    功能：配置指定 PWM 通道，保持输出关闭；须开启目标时钟、完成 BaseInit、
          停止计数并独占配置；不产生 UG，不改变其他通道或共享计时配置
    参数：
        @TIMx：TIM2～5 实例指针
        @channel：TIM_CHANNEL_1～TIM_CHANNEL_4
        @config：非空只读配置，含 PWM1/2、高/低有效及原始 CCR 值；
                 CCR 可大于 ARR，TIM3/4 最大 65535，TIM2/5 最大 UINT32_MAX
    返回值：成功返回 0；参数无效返回 -1；参数合法但 CEN 为 1 返回 -2；
            失败不写寄存器
*/
int TIM_PWM_ConfigChannel(gtim_t *TIMx, gtim_channel_e channel, const gtim_pwm_config_t *config);

/*
    功能：开启指定 PWM 输出和共享计数器，不清 CNT、不产生 UG；须先开启
          时钟、配置 GPIO/PWM 并按需装载预装载，首段脉冲不保证完整
    参数：
        @TIMx：TIM2～5 实例指针
        @channel：TIM_CHANNEL_1～TIM_CHANNEL_4
    返回值：成功返回 0；实例/通道无效返回 -1；目标不是 PWM1/2 输出返回 -2；
            失败不写寄存器
*/
int TIM_PWM_Start(gtim_t *TIMx, gtim_channel_e channel);

/*
    功能：仅关闭指定通道，不停止共享计数器、不保证引脚低电平；须先开启目标时钟
    参数：
        @TIMx：TIM2～5 实例指针
        @channel：TIM_CHANNEL_1～TIM_CHANNEL_4
    返回值：成功返回 0；实例/通道无效返回 -1，失败不写寄存器
*/
int TIM_PWM_Stop(gtim_t *TIMx, gtim_channel_e channel);

/*
    功能：更新目标 CCR，不重置 CNT、不产生 UG；须先开启目标时钟并配置 PWM，
          开启 OCxPE 后在下次更新事件生效，操作期间不得并发修改配置
    参数：
        @TIMx：TIM2～5 实例指针
        @channel：TIM_CHANNEL_1～TIM_CHANNEL_4
        @compare_value：原始 CCR 值，可大于 ARR，不是百分数；
                        TIM3/4 最大 65535，TIM2/5 最大 UINT32_MAX
    返回值：成功写入返回 0；参数无效返回 -1；目标不是 PWM1/2 输出返回 -2；
            失败不写寄存器
*/
int TIM_PWM_SetCompare(gtim_t *TIMx, gtim_channel_e channel, uint32_t compare_value);

#endif
