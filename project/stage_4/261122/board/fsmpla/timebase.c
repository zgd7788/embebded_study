#include "timebase.h"

/*
    功能：使用 TIM2 实现 timebase.h 中约定的时间读取接口
    参数：无
    返回值：TIM2 当前 CNT 值
    说明：只负责读取，初始化和启动由调用方完成；接口单位与前提见 timebase.h
*/
uint32_t Timebase_GetTicks(gtim_t *TIMx)
{
    return GTIMx_GetCounter(TIMx);
}
