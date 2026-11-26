#include "cs42l51.h"

/*
    功能：读取 CS42L51 的一个寄存器。先发送 MAP，STOP 后另起读事务接收一个字节。
          调用前须初始化 I2C5、启动 TIM2，并释放芯片复位；读取失败不修改输出值。
    参数：
        @dev：器件绑定信息，不能为 NULL；包含 I2C 实例及原始 7 位地址。
              当前底层仅支持 I2C5，器件地址不能包含读写方向位。
        @reg：寄存器索引，范围为 0x00～0x7F，不携带 MAP.INCR 位。
              调用方须根据手册选择可读寄存器。
        @value_out：指向可写字节的输出指针，不能为 NULL，仅成功时写入读取结果。
        @timeout_ticks：每笔底层 I2C 事务分别使用的超时预算，单位为 TIM2 tick。
                        发送 MAP 和接收数据分别计时，不是整个函数共用一次预算。
    返回值：0 表示成功读取一个字节并正常结束传输。
           -1 表示器件指针、输出指针、实例、地址或寄存器索引无效。
           -2 表示超时，-3 表示收到 NACK，-4 表示意外 STOP 或数据不足。
           底层读写失败时立即返回原错误码，不继续后续传输。
*/
int CS42L51_ReadReg(const cs42l51_t *dev, uint8_t reg,
                   uint8_t *value_out, uint32_t timeout_ticks)
{
    if((dev == NULL) || (value_out == NULL) || (reg > 0x7FU))
    {
        return -1;
    }
    uint8_t val;
    int result = I2C_MasterWrite(dev->i2c, dev->addr7, &reg, sizeof(reg), timeout_ticks);
    if(result != 0U)
    {
        return result;
    }
    result = I2C_MasterReadByte(dev->i2c, dev->addr7, &val, timeout_ticks);
    if(result != 0U)
    {
        return result;
    }
    *value_out = val;
    return 0;
}

/*
    功能：向 CS42L51 的一个寄存器写入一个字节，在同一笔事务中依次发送 MAP 和数据。
          调用前须初始化 I2C5、启动 TIM2，并释放芯片复位；本函数不进行读回校验。
          写入失败前可能已有数据到达器件，不保证原值不变，也不自动恢复原值。
    参数：
        @dev：器件绑定信息，不能为 NULL；包含 I2C 实例及原始 7 位地址。
              当前底层仅支持 I2C5，器件地址不能包含读写方向位。
        @reg：寄存器索引，范围为 0x00～0x7F，不携带 MAP.INCR 位。
              调用方须根据手册选择可写寄存器并遵守保留位等访问规则。
        @value：写入该寄存器的完整一个字节；与 reg 组成两个字节的发送缓冲区。
        @timeout_ticks：本次底层 I2C 写事务的超时预算，单位为 TIM2 tick，
                        等待总线空闲和发送结束共用一次预算。
    返回值：0 表示两个字节已提交，检测到正常 STOP 且未收到 NACK。
           -1 表示器件指针、实例、地址或寄存器索引无效。
           -2 表示超时，-3 表示收到 NACK，-4 表示数据尚未提交完就出现 STOP。
           底层写入失败时返回原错误码。
*/
int CS42L51_WriteReg(const cs42l51_t *dev, uint8_t reg,
                    uint8_t value, uint32_t timeout_ticks)
{
    if((dev == NULL)  || (reg > 0x7FU))
    {
        return -1;
    }
    uint8_t val[2] = {reg, value};
    int result = I2C_MasterWrite(dev->i2c, dev->addr7, val, sizeof(val), timeout_ticks);
    if( result!= 0U)
    {
        return result;
    }
    return 0;
}

/*
    功能：先读取寄存器当前值，再按掩码更新指定几位，保留其他位并写回同一寄存器。
          复用 ReadReg 和 WriteReg，读取失败时不执行写入；调用前提与二者一致。
          仅适用于可正常读写的配置寄存器，不适用于只读、写 1 清除或读取有副作用
          的寄存器。读改写不是原子操作，调用期间不能由其他代码修改同一寄存器。
    参数：
        @dev：器件绑定信息，不能为 NULL；实例和地址限制与 ReadReg 一致。
        @reg：要更新的寄存器索引，范围为 0x00～0x7F，不携带 MAP.INCR 位。
              调用方负责确认目标寄存器及位字段适合读改写。
        @mask：需要修改的位取 1，需要保留的位取 0；可以同时选择多个位。
        @value：已对齐到寄存器实际位置的新位值，mask 以外的位被忽略。
                例如 mask=0x0C、value=0x08 表示将 bit3:2 设置为二进制 10。
        @timeout_ticks：每笔底层 I2C 事务分别使用的超时预算，单位为 TIM2 tick。
                        读取原值和写入新值的多笔事务不共用一次总预算。
    返回值：0 表示读取和写入均成功，不包含读回校验。
           -1 表示器件指针、实例、地址或寄存器索引无效。
           -2 表示超时，-3 表示收到 NACK，-4 表示意外 STOP 或数据不足。
           读写失败时返回原错误码；写入失败可能已改变器件内容，不自动回滚。
           当前 mask=0 或新旧值相同时仍会执行读取和写回。
*/
int CS42L51_UpdateBits(const cs42l51_t *dev, uint8_t reg,
                       uint8_t mask, uint8_t value,
                       uint32_t timeout_ticks)
{
    if(dev == NULL)
    {
        return -1;
    }
    uint8_t old_value;
    int result = CS42L51_ReadReg(dev, reg, &old_value, timeout_ticks);
    if(result != 0U)
    {
        return result;
    }
    uint8_t new_value = (old_value & ~mask) | (value & mask);
    result = CS42L51_WriteReg(dev, reg, new_value, timeout_ticks);
    if(result != 0U)
    {
        return result;
    }
    return 0;
}
