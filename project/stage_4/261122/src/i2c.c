#include "i2c.h"
#include "timebase.h"

/*
    功能：配置 I2C5 的 TIMINGR 并置位 PE。调用前须开启 I2C5 时钟，
          并保持外设禁用（PE=0）。
    参数：
        @I2Cx：I2C 寄存器组地址，当前仅支持 I2C5。
        @timing：根据 I2C5 内核时钟和目标总线时序计算的 TIMINGR 寄存器值。
    返回值：成功返回 0，实例不支持返回 -1，外设已使能返回 -2。
*/
int I2C_Init(i2c_t *I2Cx, uint32_t timing)
{
    if(I2Cx != I2C5)
    {
        return -1;
    }
    if((I2Cx->CR1 & 0x1) != 0U)
    {
        return -2;
    }
    I2Cx->TIMINGR = timing;
    I2Cx->CR1 |= I2C_CR1_PE_Msk;
    return 0;
}

/*
    功能：以主机模式向 7 位地址的从机发送一个字节，由硬件产生 STOP。
          调用前须完成 I2C5 初始化并启动 TIM2 自由运行计数。
    参数：
        @I2Cx：I2C 寄存器组地址，当前仅支持 I2C5。
        @addr7：从机的原始 7 位地址，例如 0x4A。
        @data：要写入 TXDR 的一个字节。
        @timeout_ticks：等待总线空闲和发送结束共用的超时预算，单位为 TIM2 tick。
    返回值：0 表示已提交数据、检测到 STOP 且未收到 NACK。
           -1 表示实例或地址无效，-2 表示等待超时。
           -3 表示收到 NACK 后检测到 STOP，-4 表示写入 TXDR 前意外检测到 STOP。
*/
int I2C_MasterWriteByte(i2c_t *I2Cx, uint8_t addr7, uint8_t data, uint32_t timeout_ticks)
{
    if((I2Cx != I2C5) || (addr7 > 0x7F))
    {
        return -1;
    }
    uint32_t start_ticks = Timebase_GetTicks(TIM2);
    uint32_t now_ticks;

    /* 总线忙 */
    while ((I2Cx->ISR & I2C_ISR_BUSY_Msk) != 0U)
    {
        now_ticks = Timebase_GetTicks(TIM2);
        if ((now_ticks - start_ticks) >= timeout_ticks)
        {
            return -2;
        }
    }

    I2Cx->ICR = I2C_ICR_NACKCF_Msk | I2C_ICR_STOPCF_Msk;
    I2Cx->CR2 &= ~(I2C_CR2_ADD10_Msk);
    I2Cx->CR2 &= ~(I2C_CR2_NBYTES_Msk);
    I2Cx->CR2 |= (1U << I2C_CR2_NBYTES_Pos);
    I2Cx->CR2 &= ~(I2C_CR2_RELOAD_Msk);
    I2Cx->CR2 |= (I2C_CR2_AUTOEND_Msk);
    I2Cx->CR2 &= ~(I2C_CR2_SADD_Msk);
    I2Cx->CR2 |= (addr7 << 1U);
    I2Cx->CR2 &= ~(I2C_CR2_WD_WRN_Msk);
    I2Cx->CR2 |= (I2C_CR2_START_Msk);
    uint8_t data_written = 0U;
    for (;;)
    {
        uint32_t status = I2Cx->ISR;
    
        if ((status & I2C_ISR_STOPF_Msk) != 0U)
        {
            /* 向 ICR 写 1，清除 STOPF 和 NACKF。 */
            I2Cx->ICR = I2C_ICR_STOPCF_Msk | I2C_ICR_NACKCF_Msk;
        
            if ((status & I2C_ISR_NACKF_Msk) != 0U)
            {
                return -3;  /* 从机回复了 NACK */
            }
            if (data_written == 0U)
            {
                return -4;  /* 数据还没写入就出现了 STOP */
            }
            return 0;
        }
    
        now_ticks = Timebase_GetTicks(TIM2);
        if ((now_ticks - start_ticks) >= timeout_ticks)
        {
            return -2;
        }
    
        if ((data_written == 0U) &&
            ((status & I2C_ISR_NACKF_Msk) == 0U) &&
            ((status & I2C_ISR_TXIS_Msk) != 0U))
        {
            I2Cx->TXDR = data;
            data_written = 1U;
        }
    }

}
