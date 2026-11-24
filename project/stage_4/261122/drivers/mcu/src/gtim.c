#include "gtim.h"
#include <stddef.h>

/* 私有映射：选定寄存器及字段位置后，各通道共用同一套操作。 */
typedef struct
{
    volatile uint32_t *ccmr;
    volatile uint32_t *ccr;
    uint32_t ccmr_shift;
    uint32_t ccer_shift;
} tim_channel_registers_t;

/*
    功能：判断指针是否对应本驱动支持的定时器实例，不访问硬件寄存器
    参数：
        @TIMx：待检查的定时器寄存器组指针，可为空
    返回值：TIMx 为 TIM2、TIM3、TIM4 或 TIM5 时返回 1，否则返回 0
*/
static int tim_instance_valid(const gtim_t *TIMx)
{
    return (TIMx == TIM2) || (TIMx == TIM3) ||
           (TIMx == TIM4) || (TIMx == TIM5);
}

/*
    功能：检查计数值是否符合目标实例 CNT、ARR 或 CCR 的寄存器位宽
          调用方须先确认 TIMx 是有效实例；本函数不检查 ARR 等业务约束
    参数：
        @TIMx：已确认有效的 TIM2、TIM3、TIM4 或 TIM5 实例
        @value：待检查的原始寄存器数值，不是时间或占空比
    返回值：TIM3/4 的 value 不超过 65535，或实例为 TIM2/5 时返回 1；
            TIM3/4 的 value 超出 16 位范围时返回 0
*/
static int tim_count_valid(const gtim_t *TIMx, uint32_t value)
{
    return ((TIMx != TIM3) && (TIMx != TIM4)) || (value <= UINT16_MAX);
}

/*
    功能：查找指定通道的 CCMR、CCR 地址及字段移位，不读写硬件寄存器
          逐个取得成员地址，不将结构体成员当作数组进行指针运算
    参数：
        @TIMx：目标定时器寄存器组指针，仅支持 TIM2～5
        @channel：目标通道，取 TIM_CHANNEL_1～TIM_CHANNEL_4
        @registers：接收映射结果的结构体指针，由内部调用方保证非空可写
    返回值：成功返回 0；实例或通道无效返回 -1，且不修改映射结果
*/
static int tim_channel_registers(gtim_t *TIMx, gtim_channel_e channel,
                                 tim_channel_registers_t *registers)
{
    if (!tim_instance_valid(TIMx))
    {
        return -1;
    }

    switch (channel)
    {
        case TIM_CHANNEL_1:
            registers->ccmr = &TIMx->CCMR1;
            registers->ccr = &TIMx->CCR1;
            registers->ccmr_shift = 0U;
            registers->ccer_shift = 0U;
            break;
        case TIM_CHANNEL_2:
            registers->ccmr = &TIMx->CCMR1;
            registers->ccr = &TIMx->CCR2;
            registers->ccmr_shift = 8U;
            registers->ccer_shift = 4U;
            break;
        case TIM_CHANNEL_3:
            registers->ccmr = &TIMx->CCMR2;
            registers->ccr = &TIMx->CCR3;
            registers->ccmr_shift = 0U;
            registers->ccer_shift = 8U;
            break;
        case TIM_CHANNEL_4:
            registers->ccmr = &TIMx->CCMR2;
            registers->ccr = &TIMx->CCR4;
            registers->ccmr_shift = 8U;
            registers->ccer_shift = 12U;
            break;
        default:
            return -1;
    }
    return 0;
}

/*
    功能：读取通道配置，判断其是否为 PWM1 或 PWM2 输出模式
          调用前须开启目标时钟，并保证检查期间配置不被并发修改
    参数：
        @registers：指向已成功取得的通道映射，只读且不能为空
    返回值：CCxS 为输出且 OCxM 为 PWM1/2 时返回 1，否则返回 0；
            不检查输出使能、计数使能或 GPIO 复用状态
*/
static int tim_channel_is_pwm(const tim_channel_registers_t *registers)
{
    uint32_t value = *registers->ccmr >> registers->ccmr_shift;
    uint32_t mode = value & GTIM_CCMR1_OC1M_Msk;

    return ((value & GTIM_CCMR1_CC1S_Msk) == 0U) &&
           ((mode == GTIM_CCMR1_OC1M_PWM1) || (mode == GTIM_CCMR1_OC1M_PWM2));
}

/*
    功能：读取通道配置，判断其是否为当前支持的直接或间接输入捕获模式
          调用前须开启目标时钟，并保证检查期间配置不被并发修改
    参数：
        @registers：指向已成功取得的通道映射，只读且不能为空
    返回值：CCxS 为 01（直接映射）或 10（间接映射）时返回 1，否则返回 0；
            不识别 TRC 输入，不检查捕获使能、计数使能或 GPIO 复用状态
*/
static int tim_channel_is_capture(const tim_channel_registers_t *registers)
{
    uint32_t value = *registers->ccmr >> registers->ccmr_shift;

    return ((value & GTIM_CCMR1_CC1S_Msk) == 1U) || ((value & GTIM_CCMR1_CC1S_Msk) == 2U);
}
/*
    功能：将 TIM2～5 配置为内部时钟、向上连续计数，初始化后保持停止
          调用前须开启目标时钟，且调用方独占整个定时器的配置。
          此操作改变所有通道共享的计时条件；关闭中断/DMA、装载配置、
          重置计数并清旧状态，但不配置通道或 GPIO。
    参数：
        @TIMx：目标定时器寄存器组指针，仅支持 TIM2～5
        @psc：写入 PSC 的原始值，范围 0～65535，实际分频系数为 psc + 1
        @arr：写入 ARR 的原始值，不可为 0；TIM3/4 最大 65535，
              TIM2/5 最大 UINT32_MAX；计数范围为 0～arr
    返回值：成功返回 0；实例无效、arr 为 0 或超过实例位宽返回 -1；
            失败时不写寄存器
*/
int GTIM_BaseInit(gtim_t *TIMx, uint16_t psc, uint32_t arr)
{
    if (!tim_instance_valid(TIMx) || (arr == 0U) || !tim_count_valid(TIMx, arr))
    {
        return -1;
    }

    TIMx->CR1 &= ~(GTIM_CR1_CEN_Msk | GTIM_CR1_UIFREMAP_Msk);
    TIMx->SMCR &= ~(GTIM_SMCR_ECE_Msk | GTIM_SMCR_SMS_Msk);
    TIMx->CR1 &= ~(GTIM_CR1_CMS_Msk | GTIM_CR1_DIR_Msk | GTIM_CR1_OPM_Msk |
                   GTIM_CR1_UDIS_Msk | GTIM_CR1_URS_Msk);
    TIMx->DIER = 0U;
    TIMx->CR1 |= GTIM_CR1_ARPE_Msk;
    TIMx->PSC = psc;
    TIMx->ARR = arr;
    TIMx->EGR = GTIM_EGR_UG_Msk;
    TIMx->SR = 0U; /* 独占初始化，清除全部旧状态。 */
    return 0;
}

/*
    功能：设置 CR1.CEN，启动共享计数器，不清 CNT、不产生更新事件
          调用方须保证实例有效、时钟已开启且计数基础已初始化
    参数：
        @TIMx：目标定时器寄存器组指针，仅支持 TIM2～5，本函数不做校验
    返回值：无
*/
void GTIMx_Start(gtim_t *TIMx)
{
    TIMx->CR1 |= GTIM_CR1_CEN_Msk;
}

/*
    功能：清除 CR1.CEN，停止共享计数器，保留 CNT 和其他配置
          会影响该定时器全部通道；调用方须保证实例有效且时钟已开启
    参数：
        @TIMx：目标定时器寄存器组指针，仅支持 TIM2～5，本函数不做校验
    返回值：无
*/
void GTIMx_Stop(gtim_t *TIMx)
{
    TIMx->CR1 &= ~GTIM_CR1_CEN_Msk;
}

/*
    功能：非阻塞检查并清除更新标志 UIF；UIF 不累计更新次数
          调用方须保证实例有效、时钟已开，且独占 UIF 的处理；
          清除时向 UIF 写 0、其他状态位写 1，以保留其他状态
    参数：
        @TIMx：目标定时器寄存器组指针，仅支持 TIM2～5，本函数不做校验
    返回值：检测到 UIF 后清除并返回 1；未检测到 UIF 返回 0
*/
uint8_t GTIMx_PollUpdate(gtim_t *TIMx)
{
    if ((TIMx->SR & GTIM_SR_UIF_Msk) != 0U)
    {
        TIMx->SR = ~GTIM_SR_UIF_Msk;
        return 1U;
    }
    return 0U;
}

/*
    功能：读取原始 CNT，不清计数、不等待；调用方须保证实例有效且时钟已开
          跨回绕的 uint32_t 差值计时要求 TIM2/5、ARR=UINT32_MAX，
          计时期间不能重置计数或改频，实际跨度小于一次完整回绕。
    参数：
        @TIMx：目标定时器寄存器组指针，仅支持 TIM2～5，本函数不做校验
    返回值：当前 CNT 原始计数值，不是毫秒数；本函数不提供错误状态
*/
uint32_t GTIMx_GetCounter(gtim_t *TIMx)
{
    return TIMx->CNT;
}

/*
    功能：显式写 CNT，不修改 PSC/ARR；调用前须开启目标时钟，且调用方
          允许改变所有通道共享的计数位置，不能重置正在服务 UART 超时的 TIM2
    参数：
        @TIMx：目标定时器寄存器组指针，仅支持 TIM2～5
        @cnt：写入 CNT 的原始计数值；TIM3/4 不超过 65535，
              TIM2/5 不超过 UINT32_MAX，不额外按当前 ARR 限制
    返回值：成功返回 0；实例无效或数值超过实例位宽返回 -1；失败不写寄存器
*/
int TIM_SetCounter(gtim_t *TIMx, uint32_t cnt)
{
    if (!tim_instance_valid(TIMx) || !tim_count_valid(TIMx, cnt))
    {
        return -1;
    }
    TIMx->CNT = cnt;
    return 0;
}

/*
    功能：向 EGR.UG 写 1，显式产生更新事件；调用前须开启目标时钟，
          且调用方允许更新整个定时器。在本模块 BaseInit 的配置下，
          装载 PSC/ARR/CCR 预装载、重新
          初始化计数/预分频计数并置 UIF；不自动清 UIF，不隐式启动。
          会影响所有通道，不用于运行中仅调整某通道比较值。
    参数：
        @TIMx：目标定时器寄存器组指针，仅支持 TIM2～5
    返回值：写入 UG 后返回 0；实例无效返回 -1，且不写寄存器；
            返回 0 不表示已轮询确认硬件完成装载
*/
int TIM_GenerateUpdate(gtim_t *TIMx)
{
    if (!tim_instance_valid(TIMx))
    {
        return -1;
    }
    TIMx->EGR = GTIM_EGR_UG_Msk;
    return 0;
}

/*
    功能：按写 0 清除规则清 UIF，保留其他状态；调用前须开启目标时钟
    参数：
        @TIMx：目标定时器寄存器组指针，仅支持 TIM2～5
    返回值：写入清除值后返回 0；实例无效返回 -1，且不写寄存器
*/
int TIM_ClearUpdateFlag(gtim_t *TIMx)
{
    if (!tim_instance_valid(TIMx))
    {
        return -1;
    }
    TIMx->SR = ~GTIM_SR_UIF_Msk;
    return 0;
}

/*
    功能：配置 TIM2～5 指定 CH1～4 的普通 PWM，完成后目标通道保持关闭
          调用前须开启目标时钟、成功完成 BaseInit 并停止计数，且调用方
          独占配置；保留其他通道及 PSC/ARR/CNT，不产生 UG、不自动启动
    参数：
        @TIMx：目标定时器寄存器组指针，仅支持 TIM2～5
        @channel：目标通道，取 TIM_CHANNEL_1～TIM_CHANNEL_4
        @config：只读配置指针，不可为空；Mode 为 PWM1/2，Polarity 为
                 高/低有效；CompareValue 是原始 CCR 值，可大于 ARR，
                 TIM3/4 最大 65535，TIM2/5 最大 UINT32_MAX
    返回值：成功返回 0；实例、通道、配置指针/字段或比较值位宽非法返回 -1；
            参数合法但 CEN 仍为 1 返回 -2；所有失败路径均不写寄存器
*/
int TIM_PWM_ConfigChannel(gtim_t *TIMx, gtim_channel_e channel,
                          const gtim_pwm_config_t *config)
{
    tim_channel_registers_t registers;
    uint32_t mode;

    if ((config == NULL) || (tim_channel_registers(TIMx, channel, &registers) != 0))
    {
        return -1;
    }
    if (((config->Mode != TIM_PWM_MODE_1) && (config->Mode != TIM_PWM_MODE_2)) ||
        ((config->Polarity != TIM_POLARITY_HIGH) && (config->Polarity != TIM_POLARITY_LOW)) ||
        !tim_count_valid(TIMx, config->CompareValue))
    {
        return -1;
    }
    if ((TIMx->CR1 & GTIM_CR1_CEN_Msk) != 0U)
    {
        return -2;
    }

    mode = (config->Mode == TIM_PWM_MODE_1) ?
            GTIM_CCMR1_OC1M_PWM1 : GTIM_CCMR1_OC1M_PWM2;

    /* 修改 CCxS 前必须先关闭目标输出。 */
    TIMx->CCER &= ~(GTIM_CCER_CC1E_Msk << registers.ccer_shift);

    uint32_t ccmr_mask = (GTIM_CCMR1_CC1S_Msk | GTIM_CCMR1_OC1FE_Msk |
                          GTIM_CCMR1_OC1PE_Msk | GTIM_CCMR1_OC1M_Msk |
                          GTIM_CCMR1_OC1CE_Msk) << registers.ccmr_shift;
    uint32_t ccmr_value = *registers.ccmr;
    ccmr_value &= ~ccmr_mask;
    ccmr_value |= (mode | GTIM_CCMR1_OC1PE_Msk) << registers.ccmr_shift;
    *registers.ccmr = ccmr_value;

    uint32_t ccer_value = TIMx->CCER;
    ccer_value &= ~((GTIM_CCER_CC1P_Msk | GTIM_CCER_CC1NP_Msk) << registers.ccer_shift);
    if (config->Polarity == TIM_POLARITY_LOW)
    {
        ccer_value |= GTIM_CCER_CC1P_Msk << registers.ccer_shift;
    }
    TIMx->CCER = ccer_value;
    *registers.ccr = config->CompareValue;
    return 0;
}

/*
    功能：开启指定 PWM 输出并启动共享计数器
          调用前须开启目标时钟、配置 GPIO 与 PWM，并按需显式装载预装载。
          不清 CNT、不产生 UG；计数器已运行时允许其他通道加入，
          首个脉冲不保证从周期起点开始。不检查软件是否曾调用 Init。
    参数：
        @TIMx：目标定时器寄存器组指针，仅支持 TIM2～5
        @channel：待开启的通道，取 TIM_CHANNEL_1～TIM_CHANNEL_4
    返回值：成功返回 0；实例或通道无效返回 -1；目标通道不是 PWM1/2
            输出模式返回 -2；所有失败路径均不写寄存器
*/
int TIM_PWM_Start(gtim_t *TIMx, gtim_channel_e channel)
{
    tim_channel_registers_t registers;
    if (tim_channel_registers(TIMx, channel, &registers) != 0)
    {
        return -1;
    }
    if (!tim_channel_is_pwm(&registers))
    {
        return -2;
    }
    TIMx->CCER |= GTIM_CCER_CC1E_Msk << registers.ccer_shift;
    TIMx->CR1 |= GTIM_CR1_CEN_Msk;
    return 0;
}

/*
    功能：仅关闭指定通道输出；调用前须开启目标时钟，且调用方拥有该通道
          不停止共享计数器，也不承诺引脚变低；负载的静音/关断电平
          由 GPIO 或设备模块负责，不能把“关 CCxE”等同于“输出低”。
    参数：
        @TIMx：目标定时器寄存器组指针，仅支持 TIM2～5
        @channel：待关闭的通道，取 TIM_CHANNEL_1～TIM_CHANNEL_4
    返回值：成功返回 0；实例或通道无效返回 -1，且不写寄存器
*/
int TIM_PWM_Stop(gtim_t *TIMx, gtim_channel_e channel)
{
    tim_channel_registers_t registers;
    if (tim_channel_registers(TIMx, channel, &registers) != 0)
    {
        return -1;
    }
    TIMx->CCER &= ~(GTIM_CCER_CC1E_Msk << registers.ccer_shift);
    return 0;
}

/*
    功能：更新指定 PWM 通道的原始 CCR 比较值
          调用前须开启目标时钟并配置通道，操作期间无并发寄存器写入。
          开启 OCxPE 后在下次更新事件生效；不重置 CNT、不产生 UG、
          不更改 PSC/ARR 或其他通道，停止计数时不会自动装载新比较值。
    参数：
        @TIMx：目标定时器寄存器组指针，仅支持 TIM2～5
        @channel：目标通道，取 TIM_CHANNEL_1～TIM_CHANNEL_4
        @compare_value：原始 CCR 值，可大于 ARR，不是占空比百分数；
                        TIM3/4 最大 65535，TIM2/5 最大 UINT32_MAX
    返回值：成功写入 CCR 返回 0；实例、通道或比较值位宽非法返回 -1；
            目标通道不是 PWM1/2 输出模式返回 -2；失败时不写寄存器
*/
int TIM_PWM_SetCompare(gtim_t *TIMx, gtim_channel_e channel, uint32_t compare_value)
{
    tim_channel_registers_t registers;
    if ((tim_channel_registers(TIMx, channel, &registers) != 0) ||
        !tim_count_valid(TIMx, compare_value))
    {
        return -1;
    }
    if (!tim_channel_is_pwm(&registers))
    {
        return -2;
    }
    *registers.ccr = compare_value;
    return 0;
}

/*
    功能：约定配置指定输入捕获通道，完成后保持捕获关闭。
          计划要求先开启时钟、完成 BaseInit 并停止计数，
          由调用方独占配置；不更改其他通道或共享 PSC/ARR/CNT，不产生 UG。
    参数：
        @TIMx：目标定时器寄存器组指针，仅支持 TIM2～5
        @channel：目标通道，取 TIM_CHANNEL_1～TIM_CHANNEL_4
        @config：非空只读配置指针；Selection 为直接或间接映射，
                 Edge 为上升沿或下降沿，Prescaler 为每 1/2/4/8 个
                 有效边沿捕获一次，Filter 为 ICxF 原始编码，范围 0～15
    返回值：计划约定成功返回 0；参数无效返回 -1；参数合法但 CEN 为 1
            返回 -2；所有失败路径不写寄存器，具体行为待实现后审阅
*/
int TIM_IC_ConfigChannel(gtim_t *TIMx, gtim_channel_e channel, const gtim_ic_config_t *config)
{
    tim_channel_registers_t registers;

    if ((config == NULL) || (tim_channel_registers(TIMx, channel, &registers) != 0))
    {
        return -1;
    }
    if (((config->Edge != TIM_IC_EDGE_RISING) && (config->Edge != TIM_IC_EDGE_FALLING)) ||
        ((config->Prescaler != TIM_IC_PSC_DIV1) && (config->Prescaler != TIM_IC_PSC_DIV2)
        && (config->Prescaler != TIM_IC_PSC_DIV4) && (config->Prescaler != TIM_IC_PSC_DIV8))||
        ((config->Selection != TIM_IC_SELECT_DIRECT) && (config->Selection != TIM_IC_SELECT_INDIRECT)) ||
        (config->Filter > 15U))
    {
        return -1;
    }
    if((TIMx->CR1 & GTIM_CR1_CEN_Msk) != 0U)
    {
        return -2;
    }
    /* 修改 CCxS 前必须先关闭目标输出。 */
    TIMx->CCER &= ~(GTIM_CCER_CC1E_Msk << registers.ccer_shift);

    uint32_t ccmr_mask = (GTIM_CCMR1_CC1S_Msk | GTIM_CCMR1_IC1PSC_Msk |
                          GTIM_CCMR1_IC1F_Msk) << registers.ccmr_shift;
    uint32_t ccmr_value = *registers.ccmr;
    ccmr_value &= ~ccmr_mask;

    TIMx->CCER &= ~(GTIM_CCER_CC1NP_Msk << registers.ccer_shift);
    if(config->Edge == TIM_IC_EDGE_RISING)
    {
        TIMx->CCER &= ~(GTIM_CCER_CC1P_Msk << registers.ccer_shift);
    }
    else
    {
        TIMx->CCER |= (GTIM_CCER_CC1P_Msk << registers.ccer_shift);
    }
    ccmr_value |= ((config->Selection) | (config->Prescaler << GTIM_CCMR1_IC1PSC_Pos)
                    | (config->Filter << GTIM_CCMR1_IC1F_Pos)) << registers.ccmr_shift;
    *registers.ccmr = ccmr_value;
    return 0;
}
/*
    功能：开启指定输入捕获通道，并启动该定时器的共享计数器
          调用前须开启目标时钟，完成基础计时、GPIO 及捕获通道配置，
          且保证配置期间无并发修改；本函数不检查初始化调用历史。
          不清 CNT、不产生 UG，也不清除旧捕获值或状态标志；旧状态由
          调用方按测量流程处理。计数器已运行时允许捕获通道加入，
          首个捕获值仅记录一个边沿的位置，周期计算需取得连续有效样本。
    参数：
        @TIMx：目标定时器寄存器组指针，仅支持 TIM2～5
        @channel：待开启的通道，取 TIM_CHANNEL_1～TIM_CHANNEL_4
    返回值：成功返回 0；实例或通道无效返回 -1；目标通道的 CCxS
            不是 01（直接映射）或 10（间接映射）返回 -2；
            所有失败路径均不写寄存器
*/
int TIM_IC_Start(gtim_t *TIMx, gtim_channel_e channel)
{
    tim_channel_registers_t registers;
    if (tim_channel_registers(TIMx, channel, &registers) != 0)
    {
        return -1;
    }
    if(!tim_channel_is_capture(&registers))
    {
        return -2;
    }
    TIMx->CCER |= GTIM_CCER_CC1E_Msk << registers.ccer_shift;
    TIMx->CR1 |= GTIM_CR1_CEN_Msk;
    return 0;
}

/*
    功能：清除指定通道的 CCxE，关闭该通道的输入捕获
          调用前须开启目标时钟，且调用方拥有该通道；本函数仅校验
          实例和通道编号，不检查通道当前模式。保留共享计数器运行，
          不清 CCR 或状态标志，不修改其他通道的使能或 GPIO 配置。
    参数：
        @TIMx：目标定时器寄存器组指针，仅支持 TIM2～5
        @channel：待关闭的通道，取 TIM_CHANNEL_1～TIM_CHANNEL_4
    返回值：成功返回 0；实例或通道无效返回 -1，且不写寄存器
*/
int TIM_IC_Stop(gtim_t *TIMx, gtim_channel_e channel)
{
    tim_channel_registers_t registers;
    if (tim_channel_registers(TIMx, channel, &registers) != 0)
    {
        return -1;
    }
    TIMx->CCER &= ~(GTIM_CCER_CC1E_Msk << registers.ccer_shift);
    return 0;
}

/*
    功能：预留指定通道的非阻塞捕获读取接口。
          计划通过状态返回区分新捕获、无数据及错误情况。
    参数：
        @TIMx：目标定时器寄存器组指针，计划仅支持 TIM2～5
        @channel：目标通道，取 TIM_CHANNEL_1～TIM_CHANNEL_4
        @capture：接收原始捕获计数的非空可写指针，不是周期或频率；
                  计划仅在返回 1 时更新，TIM3/4 为 16 位计数、TIM2/5 为 32 位
    返回值：计划约定 1 为取得新捕获、0 为无新捕获、-1 为参数错误、
            -2 为通道模式不支持、-3 为过捕获。
*/
int TIM_IC_ReadCapturedValue(gtim_t *TIMx, gtim_channel_e channel, uint32_t *capture)
{
    tim_channel_registers_t registers;

    if ((capture == NULL) ||
        (tim_channel_registers(TIMx, channel, &registers) != 0))
    {
        return -1;
    }
    if (!tim_channel_is_capture(&registers))
    {
        return -2;
    }

    const uint32_t if_mask = GTIM_SR_CC1IF_Msk << (uint32_t)channel;
    const uint32_t of_mask = GTIM_SR_CC1OF_Msk << (uint32_t)channel;
    uint32_t status = TIMx->SR;
    const int has_capture = (status & if_mask) != 0U;
    uint32_t value = 0U;

    if (has_capture)
    {
        value = *registers.ccr;
        status |= TIMx->SR;  /* 保留读 CCR 前后看到的过捕获标志 */
    }

    if ((status & of_mask) != 0U)
    {
        TIMx->SR = ~of_mask;
        return -3;
    }
    if (!has_capture)
    {
        return 0;
    }

    *capture = value;
    return 1;
}
