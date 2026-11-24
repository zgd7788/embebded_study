#include <stddef.h>

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
    I2Cx->CR2 &= ~(I2C_CR2_RD_WRN_Msk);
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

/*
    功能：以主机模式在一次 START/STOP 之间连续发送 1～255 个数据字节。
          调用前须完成 I2C5 初始化并启动 TIM2；当前用于单主机轮询访问，
          访问期间不能由其他代码操作 I2C5。驱动只按顺序发送缓冲区，
          寄存器地址和数据的含义由调用方按从机协议组织。
          每次 TXIS 置位后提交下一个字节，全部提交后继续等待 STOP。
          传输中超时会请求 STOP，但不等待恢复完成；失败前可能已经发送
          部分数据，不能据返回值认为从机寄存器完全没有改变。
    参数：
        @I2Cx：I2C 寄存器组地址，当前仅支持 I2C5。
        @addr7：从机的原始 7 位地址，不包含读写方向位。
        @data：待发送缓冲区，不能为 NULL，调用期间保持有效且内容不变。
        @length：发送字节数，范围为 1～255，不包含线上从机地址字节。
        @timeout_ticks：等待空闲、提交数据及等待 STOP 共用的超时预算，
                        单位为 TIM2 tick，0 表示立即超时且不发起传输。
    返回值：0 表示已提交全部数据、检测到 STOP 且未收到 NACK。
           -1 表示实例、地址、缓冲区或长度无效，-2 表示等待超时。
           -3 表示收到 NACK 后检测到 STOP，-4 表示数据未全部提交就出现 STOP。
*/
int I2C_MasterWrite(i2c_t *I2Cx, uint8_t addr7, const uint8_t *data,
                    size_t length, uint32_t timeout_ticks)
{
    if ((I2Cx != I2C5) || (addr7 > 0x7FU) || (data == NULL) ||
        (length == 0U) || (length > 255U))
    {
        return -1;
    }

    const uint32_t start_ticks = Timebase_GetTicks(TIM2);
    while ((I2Cx->ISR & I2C_ISR_BUSY_Msk) != 0U)
    {
        if ((Timebase_GetTicks(TIM2) - start_ticks) >= timeout_ticks)
        {
            return -2;
        }
    }
    if ((Timebase_GetTicks(TIM2) - start_ticks) >= timeout_ticks)
    {
        return -2;
    }

    I2Cx->ICR = I2C_ICR_NACKCF_Msk | I2C_ICR_STOPCF_Msk;

    /* NBYTES 包含缓冲区内的每个字节，例如寄存器索引和要写入的值；
       硬件自行发送的从机地址不计入 NBYTES。首版不使用 RELOAD。 */
    uint32_t cr2 = I2Cx->CR2;
    cr2 &= ~(I2C_CR2_SADD_Msk | I2C_CR2_RD_WRN_Msk |
             I2C_CR2_ADD10_Msk | I2C_CR2_START_Msk | I2C_CR2_STOP_Msk |
             I2C_CR2_NBYTES_Msk | I2C_CR2_RELOAD_Msk | I2C_CR2_AUTOEND_Msk);
    cr2 |= ((uint32_t)addr7 << 1U) |
           ((uint32_t)length << I2C_CR2_NBYTES_Pos) | I2C_CR2_AUTOEND_Msk;
    I2Cx->CR2 = cr2 | I2C_CR2_START_Msk;

    size_t sent = 0U;
    for (;;)
    {
        const uint32_t status = I2Cx->ISR;

        /* 最后一个字节也可能被 NACK；先排除 NACK 再判断成功。 */
        if ((status & I2C_ISR_STOPF_Msk) != 0U)
        {
            I2Cx->ICR = I2C_ICR_STOPCF_Msk | I2C_ICR_NACKCF_Msk;
            if ((status & I2C_ISR_NACKF_Msk) != 0U)
            {
                return -3;
            }
            if (sent != length)
            {
                return -4;
            }
            return 0;
        }

        if ((sent < length) && ((status & I2C_ISR_NACKF_Msk) == 0U) &&
            ((status & I2C_ISR_TXIS_Msk) != 0U))
        {
            I2Cx->TXDR = data[sent];
            ++sent;  /* 记录已提交数量，硬件传输完成仍需等待 STOP。 */
        }

        if ((Timebase_GetTicks(TIM2) - start_ticks) >= timeout_ticks)
        {
            I2Cx->CR2 |= I2C_CR2_STOP_Msk;
            return -2;
        }
    }
}

/*
    功能：以主机模式在一次 START/STOP 之间连续接收 1～255 个数据字节。
          调用前须完成 I2C5 初始化并启动 TIM2；当前用于单主机轮询访问，
          访问期间不能由其他代码操作 I2C5。本函数不发送寄存器索引，
          从机内部地址指针及地址自增规则由调用方按器件手册设置。
          每次 RXNE 置位读取一个字节，末字节 NACK 和 STOP 由硬件产生。
          超时后若已发起传输，会请求 STOP，但不等待恢复完成。
    参数：
        @I2Cx：I2C 寄存器组地址，当前仅支持 I2C5。
        @addr7：从机的原始 7 位地址，不包含读写方向位。
        @data：可写接收缓冲区，不能为 NULL，至少有 length 字节容量，
               调用期间保持有效。失败时可能已写入部分数据，内容不回滚；
               调用方只能在返回 0 后将其作为完整接收结果使用。
        @length：期望接收字节数，范围为 1～255，不包含线上从机地址字节。
        @timeout_ticks：等待空闲、接收数据及等待 STOP 共用的超时预算，
                        单位为 TIM2 tick，0 表示立即超时且不发起传输。
    返回值：0 表示读到全部数据、检测到 STOP 且未收到 NACK。
           -1 表示实例、地址、缓冲区或长度无效，-2 表示等待超时。
           -3 表示收到 NACK 后检测到 STOP，-4 表示 STOP 时接收数量不足。
*/
int I2C_MasterRead(i2c_t *I2Cx, uint8_t addr7, uint8_t *data,
                   size_t length, uint32_t timeout_ticks)
{
    if ((I2Cx != I2C5) || (addr7 > 0x7FU) || (data == NULL) ||
        (length == 0U) || (length > 255U))
    {
        return -1;
    }

    const uint32_t start_ticks = Timebase_GetTicks(TIM2);
    while ((I2Cx->ISR & I2C_ISR_BUSY_Msk) != 0U)
    {
        if ((Timebase_GetTicks(TIM2) - start_ticks) >= timeout_ticks)
        {
            return -2;
        }
    }
    if ((Timebase_GetTicks(TIM2) - start_ticks) >= timeout_ticks)
    {
        return -2;
    }

    I2Cx->ICR = I2C_ICR_NACKCF_Msk | I2C_ICR_STOPCF_Msk;

    /* length 控制主机接收数量；从机是否地址自增与此配置相互独立。
       首版不用 RELOAD，AUTOEND 让硬件在末字节 NACK 后自动发送 STOP。 */
    uint32_t cr2 = I2Cx->CR2;
    cr2 &= ~(I2C_CR2_SADD_Msk | I2C_CR2_RD_WRN_Msk |
             I2C_CR2_ADD10_Msk | I2C_CR2_START_Msk | I2C_CR2_STOP_Msk |
             I2C_CR2_NBYTES_Msk | I2C_CR2_RELOAD_Msk | I2C_CR2_AUTOEND_Msk);
    cr2 |= ((uint32_t)addr7 << 1U) | ((uint32_t)length << I2C_CR2_NBYTES_Pos) |
           I2C_CR2_RD_WRN_Msk | I2C_CR2_AUTOEND_Msk;
    I2Cx->CR2 = cr2 | I2C_CR2_START_Msk;

    size_t received = 0U;
    for (;;)
    {
        const uint32_t status = I2Cx->ISR;

        /* 最后一个字节和 STOPF 可能同时到来，必须先从 RXDR 取数据。
           received < length 确保软件只在调用方约定的容量范围内写入。 */
        if ((received < length) && ((status & I2C_ISR_RXNE_Msk) != 0U))
        {
            data[received] = (uint8_t)I2Cx->RXDR;
            ++received;
        }

        if ((status & I2C_ISR_STOPF_Msk) != 0U)
        {
            I2Cx->ICR = I2C_ICR_STOPCF_Msk | I2C_ICR_NACKCF_Msk;
            if ((status & I2C_ISR_NACKF_Msk) != 0U)
            {
                return -3;
            }
            if (received != length)
            {
                return -4;
            }
            return 0;
        }

        /* 收到全部数据也要继续等 STOP；整笔访问只使用同一个起始 tick。 */
        if ((Timebase_GetTicks(TIM2) - start_ticks) >= timeout_ticks)
        {
            I2Cx->CR2 |= I2C_CR2_STOP_Msk;
            return -2;
        }
    }
}

/*
    功能：以主机模式发起独立的单字节读事务，由硬件发送末字节 NACK 和 STOP。
          调用前须完成 I2C5 初始化并启动 TIM2。读取 RXDR 后先暂存数据，
          正常 STOP 后才交付给调用方；RXNE 和 STOPF 同时出现时先取数据。
          本函数不发送寄存器索引，访问寄存器前须先设置从机内部地址指针。
    参数：
        @I2Cx：I2C 寄存器组地址，当前仅支持 I2C5。
        @addr7：从机的原始 7 位地址，例如 0x4A。
        @data_out：输出指针，不能为 NULL，仅成功时修改其指向的一个字节。
        @timeout_ticks：等待空闲和接收结束共用的超时预算，单位为 TIM2 tick。
    返回值：0 表示读到数据、检测到 STOP 且未收到 NACK。
           -1 表示实例、地址或输出指针无效，-2 表示等待超时。
           -3 表示收到 NACK 后检测到 STOP，-4 表示 STOP 时尚未取得数据。
*/
int I2C_MasterReadByte(i2c_t *I2Cx, uint8_t addr7, uint8_t *data_out, uint32_t timeout_ticks)
{
    if((I2Cx != I2C5) || (addr7 > 0x7F)||(data_out == NULL))
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
    I2Cx->CR2 |= (I2C_CR2_RD_WRN_Msk);
    I2Cx->CR2 |= (I2C_CR2_START_Msk);
    uint8_t data_received = 0U;
    uint8_t data;
    for (;;)
    {
        uint32_t status = I2Cx->ISR;
    
        if ((data_received == 0U) &&
            ((status & I2C_ISR_RXNE_Msk) != 0U))
        {
            data = (uint8_t)I2Cx->RXDR;
            data_received = 1U;
        }
        if ((status & I2C_ISR_STOPF_Msk) != 0U)
        {
            /* 向 ICR 写 1，清除 STOPF 和 NACKF。 */
            I2Cx->ICR = I2C_ICR_STOPCF_Msk | I2C_ICR_NACKCF_Msk;
        
            if ((status & I2C_ISR_NACKF_Msk) != 0U)
            {
                return -3;  /* 从机回复了 NACK */
            }
            if (data_received == 0U)
            {
                return -4;  /* 尚未收到数据就出现了 STOP。 */
            }
            *data_out = data;
            return 0;
        }
    
        now_ticks = Timebase_GetTicks(TIM2);
        if ((now_ticks - start_ticks) >= timeout_ticks)
        {
            return -2;
        }
    }
}

/*
    功能：先发送一个字节的寄存器地址，再用重复 START 切换到读方向，
          接收一个字节后由硬件发送 NACK 和 STOP，两段传输之间不产生 STOP。
          调用前须完成 I2C5 初始化并启动 TIM2；当前用于单主机、轮询访问，
          从机须支持这种寄存器读取流程，访问期间不能由其他代码操作 I2C5。
          超时后若已经发起传输，会请求 STOP，但不等待恢复完成；调用方
          再次访问前须确认总线已空闲，必要时另行恢复外设或总线。
    参数：
        @I2Cx：I2C 寄存器组地址，当前仅支持 I2C5。
        @addr7：从机的原始 7 位地址，不包含读写方向位。
        @reg：要读取的寄存器地址，当前仅支持一个字节的寄存器地址。
        @data_out：接收结果的输出指针，不能为 NULL，仅成功时修改其内容。
        @timeout_ticks：等待总线空闲、发送寄存器地址及接收数据共用的
                        超时预算，单位为 TIM2 tick，0 表示立即超时。
    返回值：0 表示读到一个字节、检测到 STOP 且未收到 NACK。
           -1 表示实例、从机地址或输出指针无效，-2 表示等待超时。
           -3 表示任一阶段收到 NACK 后检测到 STOP。
           -4 表示发送阶段意外结束，或接收阶段 STOP 时尚未取得数据。
*/
int I2C_ReadRegisterByte(i2c_t *I2Cx, uint8_t addr7, uint8_t reg,
                         uint8_t *data_out, uint32_t timeout_ticks)
{
    if ((I2Cx != I2C5) || (addr7 > 0x7FU) || (data_out == NULL))
    {
        return -1;
    }

    const uint32_t start_ticks = Timebase_GetTicks(TIM2);

    /* 只在整笔事务开始前等待空闲；重复 START 前总线仍属于本次访问。 */
    while ((I2Cx->ISR & I2C_ISR_BUSY_Msk) != 0U)
    {
        if ((Timebase_GetTicks(TIM2) - start_ticks) >= timeout_ticks)
        {
            return -2;
        }
    }
    if ((Timebase_GetTicks(TIM2) - start_ticks) >= timeout_ticks)
    {
        return -2;
    }

    I2Cx->ICR = I2C_ICR_NACKCF_Msk | I2C_ICR_STOPCF_Msk;

    /* 第一段：7 位地址、写方向、NBYTES=1、RELOAD=0、AUTOEND=0。
       先在局部变量中拼好配置，再和 START 一次写入 CR2。 */
    uint32_t cr2 = I2Cx->CR2;
    cr2 &= ~(I2C_CR2_SADD_Msk | I2C_CR2_RD_WRN_Msk |
             I2C_CR2_ADD10_Msk | I2C_CR2_START_Msk | I2C_CR2_STOP_Msk |
             I2C_CR2_NBYTES_Msk | I2C_CR2_RELOAD_Msk | I2C_CR2_AUTOEND_Msk);
    cr2 |= ((uint32_t)addr7 << 1U) | (1U << I2C_CR2_NBYTES_Pos);
    I2Cx->CR2 = cr2 | I2C_CR2_START_Msk;

    uint8_t reg_written = 0U;
    for (;;)
    {
        const uint32_t status = I2Cx->ISR;

        /* 正常写阶段应到达 TC；收到 NACK 时硬件仍会自动发送 STOP。 */
        if ((status & I2C_ISR_STOPF_Msk) != 0U)
        {
            I2Cx->ICR = I2C_ICR_STOPCF_Msk | I2C_ICR_NACKCF_Msk;
            if ((status & I2C_ISR_NACKF_Msk) != 0U)
            {
                return -3;
            }
            return -4;
        }

        if ((status & I2C_ISR_NACKF_Msk) == 0U)
        {
            if ((reg_written == 0U) && ((status & I2C_ISR_TXIS_Msk) != 0U))
            {
                I2Cx->TXDR = reg;
                reg_written = 1U;
            }

            /* TXDR 写入只代表提交；TC 才表示该字节传输完成。 */
            if ((reg_written != 0U) && ((status & I2C_ISR_TC_Msk) != 0U))
            {
                break;
            }
        }

        if ((Timebase_GetTicks(TIM2) - start_ticks) >= timeout_ticks)
        {
            I2Cx->CR2 |= I2C_CR2_STOP_Msk;
            return -2;
        }
    }

    /* 第二段：同一从机地址、读方向、NBYTES=1、AUTOEND=1。
       此处不等待 BUSY 清零。设置 START 会清 TC，并产生重复 START。 */
    I2Cx->CR2 = cr2 | I2C_CR2_RD_WRN_Msk |
                I2C_CR2_AUTOEND_Msk | I2C_CR2_START_Msk;

    uint8_t data_received = 0U;
    uint8_t data = 0U;
    for (;;)
    {
        const uint32_t status = I2Cx->ISR;

        /* RXNE 与 STOPF 可能同时置位，必须先取出最后一个字节。 */
        if ((data_received == 0U) && ((status & I2C_ISR_RXNE_Msk) != 0U))
        {
            data = (uint8_t)I2Cx->RXDR;
            data_received = 1U;
        }

        if ((status & I2C_ISR_STOPF_Msk) != 0U)
        {
            I2Cx->ICR = I2C_ICR_STOPCF_Msk | I2C_ICR_NACKCF_Msk;
            if ((status & I2C_ISR_NACKF_Msk) != 0U)
            {
                return -3;
            }
            if (data_received == 0U)
            {
                return -4;
            }
            *data_out = data;
            return 0;
        }

        /* 沿用同一起点，写阶段和读阶段不会各获得一份超时预算。 */
        if ((Timebase_GetTicks(TIM2) - start_ticks) >= timeout_ticks)
        {
            I2Cx->CR2 |= I2C_CR2_STOP_Msk;
            return -2;
        }
    }
}
