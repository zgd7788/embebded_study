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

/* SMCR: SMS 是 bits2:0 与 bit16 组成的不连续字段。 */
#define GTIM_SMCR_SMS_Msk         ((0x7U << 0U) | (1U << 16U))
#define GTIM_SMCR_ECE_Msk         (1U << 14U)
/* SR: 以第一个通道为基准，第二个通道整体左移1位 */
#define GTIM_SR_UIF_Pos           0U
#define GTIM_SR_UIF_Msk           (1U << GTIM_SR_UIF_Pos)
#define GTIM_SR_CC1IF_Pos         1U
#define GTIM_SR_CC1IF_Msk         (1U << GTIM_SR_CC1IF_Pos)
#define GTIM_SR_CC1OF_Pos         9U
#define GTIM_SR_CC1OF_Msk         (1U << GTIM_SR_CC1OF_Pos)
/* EGR */
#define GTIM_EGR_UG_Msk           (1U << 0U)

/* CCMR(input capture mode):以第一个通道为基准，第二个通道整体左移8位，CC1S同输出比较功能 */
#define GTIM_CCMR1_IC1PSC_Pos   2U
#define GTIM_CCMR1_IC1PSC_Msk   (0x3U << GTIM_CCMR1_IC1PSC_Pos)
#define GTIM_CCMR1_IC1F_Pos     4U
#define GTIM_CCMR1_IC1F_Msk     (0xFU << GTIM_CCMR1_IC1F_Pos)

/* CCMR(output compare mode)：以下以第一个通道为基准，第二个通道整体左移 8 位。 */
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
#define GTIM_CCER_CC1P_Pos        1U
#define GTIM_CCER_CC1P_Msk        (1U << GTIM_CCER_CC1P_Pos)
#define GTIM_CCER_CC1NP_Pos       3U
#define GTIM_CCER_CC1NP_Msk       (1U << GTIM_CCER_CC1NP_Pos)

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

/* pwm 配置结构体 */
typedef struct
{
    gtim_pwm_mode_e Mode;
    gtim_polarity_e Polarity;
    uint32_t CompareValue;
}gtim_pwm_config_t;

/* 输入源选择，对应 CCxS 的未移位编码 */
typedef enum
{
    TIM_IC_SELECT_DIRECT   = 1U, /* 01：直接映射 */
    TIM_IC_SELECT_INDIRECT = 2U, /* 10：间接映射 */
} gtim_select_e;

/* 捕获边沿：这两种配置都要求 CCxNP = 0 */
typedef enum
{
    TIM_IC_EDGE_RISING  = 0U, /* CCxP = 0：上升沿 */
    TIM_IC_EDGE_FALLING = 1U, /* CCxP = 1：下降沿 */
} gtim_captureedge_e;

/* 捕获预分频，对应 ICxPSC 的未移位编码 */
typedef enum
{
    TIM_IC_PSC_DIV1 = 0U, /* 00：每个有效边沿捕获 */
    TIM_IC_PSC_DIV2 = 1U, /* 01：每2个有效边沿捕获 */
    TIM_IC_PSC_DIV4 = 2U, /* 10：每4个有效边沿捕获 */
    TIM_IC_PSC_DIV8 = 3U, /* 11：每8个有效边沿捕获 */
} gtim_prescaler_e;

/* 输入捕获配置结构体 */
typedef struct
{
    gtim_select_e Selection;       /* 输入源映射 */
    gtim_captureedge_e Edge;       /* 捕获边沿 */
    gtim_prescaler_e Prescaler;    /* 捕获事件预分频 */
    uint8_t Filter;    
}gtim_ic_config_t;

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
int TIM_SetCounter(gtim_t *TIMx, uint32_t cnt);
int TIM_GenerateUpdate(gtim_t *TIMx);
int TIM_ClearUpdateFlag(gtim_t *TIMx);

int TIM_PWM_ConfigChannel(gtim_t *TIMx, gtim_channel_e channel, const gtim_pwm_config_t *config);
int TIM_PWM_Start(gtim_t *TIMx, gtim_channel_e channel);
int TIM_PWM_Stop(gtim_t *TIMx, gtim_channel_e channel);
int TIM_PWM_SetCompare(gtim_t *TIMx, gtim_channel_e channel, uint32_t compare_value);

int TIM_IC_ConfigChannel(gtim_t *TIMx, gtim_channel_e channel, const gtim_ic_config_t *config);
int TIM_IC_Start(gtim_t *TIMx, gtim_channel_e channel);
int TIM_IC_Stop(gtim_t *TIMx, gtim_channel_e channel);
int TIM_IC_ReadCapturedValue(gtim_t *TIMx, gtim_channel_e channel, uint32_t *capture);
#endif
