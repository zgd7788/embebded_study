#ifndef TIMEBASE_H
#define TIMEBASE_H

#include "gtim.h"

/*
    功能：读取平台当前的 32 位自由运行计数
    参数：无
    前置条件：调用方已启动时间源，计时期间不能重置计数或改变频率
    返回值：原始 tick 数，不是毫秒数
    说明：当前使用 TIM2、PSC=20887、ARR=UINT32_MAX，约 10000tick/秒；
          以 uint32_t 差值计时，实际时间跨度须小于一次完整回绕周期。
          替换底层时间源时，应保持单位和回绕契约，并同步调用方的时间参数
*/
uint32_t Timebase_GetTicks(gtim_t *TIMx);


#endif
