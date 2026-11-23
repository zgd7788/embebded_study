# STM32MP157 I²C5 与 CS42L51 通信学习工程

本工程面向 FS-MP1A / STM32MP157 的 Cortex-A7，学习通过 I²C5 访问板载音频芯片 CS42L51。当前主程序先读取芯片 ID，再使用通用多字节发送接口写入 ADCA 衰减寄存器 `0x0C`，读回比较并恢复原值，通过 UART4 显示结果。TIM2 为等待复位释放、I²C 超时和 UART 发送超时提供时间基准，采用轮询方式。

> 更新日期：2026-11-23 晚。单字节读取与重复 START 读取已有学习者上板通过反馈。按学习者要求新增多字节发送、main 中的写入/读回/恢复验证，并补齐新增函数注释。当前版本完整交叉构建和主机检查通过，新增写入例程尚待上板；明天先复习代码。

## 1. 当前完成情况

| 内容 | 状态 |
|---|---|
| GPIO 端口时钟通用接口 | 已实现，当前支持 GPIOA、GPIOB、GPIOC、GPIOG、GPIOZ |
| I²C5 寄存器映射、总线时钟使能及初始化 | 已实现并接入应用 |
| PA11/PA12 复用与开漏配置 | 已接入，使用板载外部上拉 |
| PC0 控制音频芯片复位 | 已接入，释放后通过 TIM2 等待 |
| I²C 主机单字节发送 | 已实现，学习者反馈上板通过 |
| UART4 显示 ID 与错误结果 | 已接入，区分写、读阶段的 NACK、超时等返回值 |
| I²C 主机单字节接收、读取芯片 ID | 已实现，学习者报告读到 `0xD9` |
| 重复 START 寄存器单字节读取 | 已实现并接入 main，学习者报告上板验证通过 |
| 通用多字节发送 I2C_MasterWrite | 教师按授权实现，首版支持 1～255 字节，交叉构建和主机检查通过 |
| ADCA 衰减寄存器写入、读回与原值恢复 | 已接入 main，应用恢复逻辑通过主机检查，尚待学习者上板 |

STOP → START 和重复 START 的 ID 读取例程均已有学习者板端通过反馈。主机模拟不代表板端时序测量，SCL 频率和时序尚无独立测量数据；当前单个器件的验证结果不推广为所有从机均支持相同寄存器读法。

多字节参考实现供后续阅读学习，不将教师代写、编译或主机检查等同于学习者已独立掌握，也不将下面的预期串口输出记作板端实测。

本目录沿用定时器工程的驱动，构建目标已改为 `i2c5_cs42l51`。蜂鸣器源码仍参与构建，当前主程序没有启动蜂鸣器或字符回显。此前定时器时钟推导与 PWM 学习见[定时器工程说明](../261114/README.md)。

## 2. 硬件连接

依据已核对的 FS-MP1A 主板 V3.1 原理图：

| 功能 | MPU 引脚及复用 | 板级连接 |
|---|---|---|
| I2C5_SCL | PA11 / AF4 | CS42L51 控制接口时钟，R136 为 4.7 kΩ 上拉 |
| I2C5_SDA | PA12 / AF4 | CS42L51 控制接口数据，R137 为 4.7 kΩ 上拉 |
| 音频芯片复位 | PC0 / 普通推挽输出 | AUDIO_RST 接 CS42L51 的低有效 RESET，R145 为 10 kΩ 下拉 |
| 音频芯片地址选择 | CS42L51 的 AD0 | R147 为 10 kΩ 下拉，选择 7 位地址 `0x4A` |
| UART4_TX | PG11 / AF6 | J1401 的 11 脚 |
| UART4_RX | PB2 / AF8 | J1401 的 12 脚 |
| 串口地 | GND | J1401 的 13 脚 |

SCL 和 SDA 均配置为开漏、无内部上下拉，板上两个外部电阻接到 Audio_3V3。器件释放线路时由电阻拉高，输出低电平时由对应器件下拉。

PA11 和 PA12 共用 GPIOA 时钟，开启一次即可。PC0 使用 GPIOC 时钟；仅配置 I²C 两根信号线并不会自动释放音频芯片复位。

## 3. I²C5 时钟与时序

### 3.1 总线时钟和内核时钟

两种时钟承担不同用途：

- I²C5 挂在 APB1，`RCC_MP_APB1ENSETR.I2C5EN`（bit 24）控制其时钟门控，用于开启外设访问。
- `RCC_I2C35CKSELR.I2C35SRC[2:0]` 选择 I2C3/I2C5 共用的内核时钟源，I²C 时序计算依赖该内核时钟。

此前在 U-Boot 读取：

| 寄存器 | 地址 | 实读值 | 结论 |
|---|---|---|---|
| RCC_I2C35CKSELR | `0x500008C4` | `0x00000002` | I2C5 选择 HSI 内核时钟 |
| RCC_HSICFGR | `0x50000018` | `0x01130000` | HSIDIV[1:0] 为 0，HSI 不分频 |

因此当前采用标称 **64 MHz** 计算 I²C5 时序。应用沿用已核对的 I2C5 时钟选源，没有实现 I2C5 选源或频率自动查询；启动环境改变后需要重新核对。

UART4 在应用初始化时显式选择 HSI，并通过 RCC 接口查询频率来计算波特率。TIM2 则沿用此前定时器时钟链推导，二者不能直接共用 I²C 的时序参数。

### 3.2 当前 TIMINGR 参数

`main()` 向 `I2C_Init()` 传入 `0xF0421515U`：

| 字段 | 数值 | 按 64 MHz 计算的时间 |
|---|---:|---|
| PRESC | 15 | t_PRESC = (15 + 1) / 64 MHz = 250 ns |
| SCLDEL | 4 | (4 + 1) × 250 ns = 1.25 µs |
| SDADEL | 2 | 2 × 250 ns = 0.5 µs |
| SCLH | 0x15，即 21 | (21 + 1) × 250 ns = 5.5 µs |
| SCLL | 0x15，即 21 | (21 + 1) × 250 ns = 5.5 µs |

高低段计时合计 11 µs，实际周期还包含 SCL 检测、滤波和同步延迟。因此当前值为 CS42L51 的 100 kHz 上限留出余量，不能把它写成“实测 100 kHz”。完整时序仍需结合实际边沿测量确认。

### 3.3 TIM2 时间基准

TIM2 配置为 `PSC=20887`、`ARR=UINT32_MAX`，启动后通过 `Timebase_GetTicks(TIM2)` 读取计数。按此前约 208.878 MHz 的定时器内核时钟，计数频率约为 10 kHz，即一个 tick 约 100 µs。

| 应用参数 | tick 数 | 用途 |
|---|---:|---|
| AUDIO_RESET_WAIT_TICKS | 2 | PC0 拉高后等待两个计数增量，满足首次 START 前至少 500 ns 的要求 |
| I2C_WRITE_TIMEOUT_TICKS | 1000 | 整笔 I²C 写事务的等待预算，约 100 ms |
| I2C_READ_TIMEOUT_TICKS | 1000 | 整笔 I²C 读事务的等待预算，约 100 ms |
| UART_TX_TIMEOUT_TICKS | 1000 | UART 单字节提交及最终发送完成的各次等待上限 |

超时以无符号差值 `now - start` 判断，计时期间不能停止或重置 TIM2，计时跨度须小于一次完整回绕周期。两个计数增量的等待会受起始采样相位影响，不表示精确延时 200 µs。

## 4. 当前程序流程

1. 开启 TIM2 时钟，初始化并启动自由运行计数器。
2. 配置 UART4 的 GPIO、时钟源、115200 8N1 和 FIFO。
3. 开启 GPIOA/GPIOC 时钟，将 PC0 配为低电平推挽输出，使音频芯片保持复位。
4. 配置 PA11、PA12 为 AF4 开漏输出，使用外部上拉。
5. 开启 I²C5 总线时钟，在 `CR1.PE=0` 时写 `TIMINGR`，随后使能外设。
6. 将 PC0 拉高释放芯片复位，通过 TIM2 等待两个计数增量。
7. 调用 `I2C_ReadRegisterByte(I2C5, 0x4A, 0x01, &id, 1000)`，在同一事务中发送索引并读取 ID。
8. 输出 ID 并核对高五位是否为 `11011`，身份不匹配时结束，不进行写入验证。
9. 读取 `0x0C` 的原值并保存；读取失败时结束。
10. 测试值选为 `0xFF`；如果原值已经是 `0xFF`，则改为 `0xFE`，保证实际发生变化。
11. 把 `{0x0C, 测试值}` 作为两个字节交给 `I2C_MasterWrite()`，再读取 `0x0C` 比较。
12. 尝试写测试值以后，即使写入或读回失败，也尝试写回原值并再次读取核对。恢复失败会报告错误。
13. 测试值读回、原值恢复全部成功才输出 `I2C multi-byte write PASS`。`main()` 返回后由启动代码停留在循环中。

此处 `0x01` 是选择待访问寄存器的索引。芯片 ID 寄存器是只读寄存器，写阶段只设置器件内部寄存器指针。

本次写入选择 DS679F1 第 60 页第 6.11 节的 **ADCA Attenuator（0x0C）**，八位全部是可写衰减量，没有需要改动的保留位；`0xFF` 表示 -1 dB，`0xFE` 表示 -2 dB。程序保存并恢复原值，不修改电源、音频接口或时钟控制寄存器。本例只验证控制寄存器读写，没有配置音频流。

线上正常事务为：

```text
START → 0x94 → ACK → 0x01 → ACK
      → 重复 START → 0x95 → ACK → ID → NACK → STOP
```

多字节写入测试值的事务为：

```text
START → 0x94 → ACK → 0x0C → ACK → 测试值 → ACK → STOP
```

缓冲区的两个字节是寄存器索引和寄存器数据，因此 `NBYTES=2`，从机地址字节不计入其中。MAP 的 INCR 位保持 0，本次只访问一个寄存器。

函数参数传入原始 7 位地址 `0x4A`。驱动把它左移一位填入 `CR2.SADD[7:1]`，硬件按写、读方向分别发送地址字节 `0x94`、`0x95`。不能向这些接口传入线上地址字节。

## 5. 驱动接口与返回值

### 5.1 RCC 与 GPIO

| 接口 | 职责 |
|---|---|
| `RCC_GPIO_EnableClock(GPIOx)` | 根据端口选择 MPU 侧 AHB4/AHB5 时钟使能位，成功返回 0，未知端口返回 -1 |
| `RCC_I2C5_EnableClock()` | 向 APB1 使能寄存器的 I2C5EN（bit 24）写 1，不选择内核时钟源 |
| `HAL_GPIO_Init(GPIOx, config)` | 配置端口中的目标引脚，调用方先开启端口时钟 |
| `HAL_GPIO_Write(GPIOx, pin, state)` | 写引脚输出状态，输出模式需提前配置 |

本工程 GPIO 实例宏表示寄存器组对象，所以传给指针参数时使用 `&GPIOA`、`&GPIOC` 等。I2C5 实例宏本身是指针，使用 `I2C5`。

### 5.2 I²C 初始化

```c
int I2C_Init(i2c_t *I2Cx, uint32_t timing);
```

当前仅支持 I2C5。调用前须开启外设时钟，传入依据实际内核时钟计算的 `TIMINGR` 值。

| 返回值 | 含义 |
|---|---|
| 0 | 写入 TIMINGR 并置位 PE |
| -1 | 不是当前支持的 I2C5 实例 |
| -2 | PE 已为 1，本次未改写 TIMINGR |

该函数不主动关闭已经使能的外设。如果初始化前 PE 已为 1，应由调用方先按手册要求完成禁用与等待，再调用初始化。

### 5.3 主机单字节发送

```c
int I2C_MasterWriteByte(i2c_t *I2Cx, uint8_t addr7,
                       uint8_t data, uint32_t timeout_ticks);
```

函数先等待 `ISR.BUSY=0`，通过 `ICR` 写 1 清除旧 NACK/STOP 标志。在发出 START 前设置 7 位地址、写方向、`NBYTES=1`、`RELOAD=0` 和 `AUTOEND=1`。

轮询期间只在 `TXIS=1`、未收到 NACK 且数据尚未提交时写一次 `TXDR`。随后等待 STOP，再结合 NACK 标志和数据提交状态返回结果。不能通过直接改写 `ISR` 来清除 NACK/STOP。

| 返回值 | 含义 |
|---|---|
| 0 | 已提交数据、检测到 STOP，且没有收到 NACK |
| -1 | 实例不支持或地址超过 7 位范围 |
| -2 | 等待总线空闲或传输结束超时 |
| -3 | 收到 NACK 并检测到 STOP |
| -4 | 写入 TXDR 前意外检测到 STOP |

等待总线空闲和发送结束共用同一起始 tick。当前 NACK 返回值不区分地址阶段和数据阶段，超时返回也不会自动执行总线恢复。

### 5.4 主机单字节接收

```c
int I2C_MasterReadByte(i2c_t *I2Cx, uint8_t addr7,
                      uint8_t *data_out, uint32_t timeout_ticks);
```

该接口发起独立的读事务。配置读方向、`NBYTES=1`、`AUTOEND=1`，看到 `RXNE` 后读取 `RXDR` 并暂存；等正常 STOP 后才写入输出指针。RXNE 与 STOPF 同时出现时先处理数据，主机末字节主动发出的 NACK 不会被当作收到从机 NACK。

返回 0 表示成功，-1 表示参数无效，-2 表示超时，-3 表示收到 NACK 并检测到 STOP，-4 表示 STOP 时尚未取得数据。失败时不修改输出值。

### 5.5 重复 START 寄存器读取

```c
int I2C_ReadRegisterByte(i2c_t *I2Cx, uint8_t addr7, uint8_t reg,
                         uint8_t *data_out, uint32_t timeout_ticks);
```

这是一个学习用的组合访问接口，支持 7 位从机地址、一个字节的寄存器地址和一个字节的数据，当前仅支持 I2C5，按单主机轮询方式使用。调用前须完成初始化并启动 TIM2，从机须支持这种寄存器访问流程。

```text
START → 写地址 → ACK → 寄存器地址 → ACK
      → 重复 START → 读地址 → ACK → 数据 → NACK → STOP
```

| 阶段 | 配置及等待条件 |
|---|---|
| 事务开始 | 等待 BUSY=0，清旧 STOP/NACK 标志 |
| 发送寄存器地址 | RD_WRN=0、NBYTES=1、RELOAD=0、AUTOEND=0；TXIS 后写 TXDR |
| 切换到读取 | 等待 TC，再配置读方向、NBYTES=1、AUTOEND=1 并设置 START |
| 接收并结束 | RXNE 后暂存 RXDR，等正常 STOP 后写入输出指针 |

`TC` 是 ISR 的 bit 6，表示软件结束模式下这一段数据已经传输完成。设置 START 会清除 TC 并发出重复 START。中间不能等待 BUSY 清零，因为总线还由本次访问占用。

整个访问共用一个起始 tick，0 tick 预算直接超时。返回码沿用 0/-1/-2/-3/-4；-3 表示任一阶段收到 NACK 后检测到 STOP，-4 表示写阶段意外 STOP 或读阶段未取得数据就结束。失败时不修改输出值。发起传输后的超时路径会请求 STOP，但不等待 STOP 完成，也没有实现总线解锁、仲裁丢失或总线错误的专用处理；再次访问前应确认空闲并按需恢复。

依据 RM0436 Rev 2 第 52.4.9 节（第 2556 页）、CR2 与 ISR 描述实现。**CS42L51 DS679F1 第 44 页图 25 给出的寄存器读法是 STOP → START**。学习者随后将 main 改为调用此重复 START 接口并报告上板通过，记录当前开发板例程的实际反馈；手册示例与实验结果分别保留，不据本例推断其他器件的寄存器访问要求。

### 5.6 通用多字节发送

```c
int I2C_MasterWrite(i2c_t *I2Cx, uint8_t addr7, const uint8_t *data,
                    size_t length, uint32_t timeout_ticks);
```

`data` 指向调用期间有效且内容不变的发送缓冲区，`length` 为 1～255，不包含从机地址字节。函数不解释寄存器协议，调用方组织缓冲区；首版不使用 RELOAD，长度 0、超过 255 或空缓冲区均返回 -1。

等待 BUSY 清零后，配置写方向、NBYTES=length、RELOAD=0、AUTOEND=1 并发起 START。用 `size_t sent` 记录已提交数量，每次 TXIS 到来后写入 `data[sent]` 并递增。全部提交之后仍须等待 STOP，只有未收到 NACK 且 `sent == length` 才成功，最后一个字节被 NACK 也会失败。

| 返回值 | 含义 |
|---|---|
| 0 | 全部数据已提交，正常 STOP，未收到 NACK |
| -1 | 实例、地址、缓冲区或长度无效 |
| -2 | 等待空闲或发送结束超时 |
| -3 | 收到 NACK 并检测到 STOP |
| -4 | 数据尚未全部提交就检测到 STOP |

整笔事务共用起始 tick，0 tick 预算不发起传输。传输中超时会请求 STOP，但不等待恢复完成；失败前可能已有数据到达从机，因此 main 即使收到写失败也尝试恢复保存的寄存器原值。该函数尚未实现总线解锁或专门的总线错误/仲裁丢失处理。

## 6. 串口观察结果

SecureCRT 配置为 **115200、8 数据位、无校验、1 停止位、无硬件流控**。

| 串口输出 | 含义 |
|---|---|
| `ID=0xD9` | 既有板端 ID 结果，芯片标识为 11011，修订号字段值为 1；其他有效修订号也允许继续 |
| `ID read: I2C NACK (-3)` | 读取芯片 ID 的组合访问收到 NACK |
| `test value write: I2C NACK (-3)` | 写入测试值时收到 NACK，程序仍会尝试恢复原值 |
| `test value readback: I2C timeout (-2)` | 测试值读回超时，程序仍会尝试恢复原值 |
| `write/readback FAIL` | 读回值与测试值不一致 |
| `original value restore: I2C timeout (-2)` | 恢复原值失败，不能认为寄存器已经恢复 |
| `restore FAIL` | 恢复写入后的读回值与原值不同 |
| `I2C multi-byte write PASS` | 本次测试值读回和原值恢复全部成功 |

正常复位后原值为 `0x00`、ID 为 `0xD9` 时，以下是**预期输出示例，尚未作为这版程序的板端实测记录**：

```text
ID=0xD9
REG0x0C original=0x00
REG0x0C test=0xFF
REG0x0C readback=0xFF
write/readback PASS
REG0x0C restored=0x00
restore PASS
I2C multi-byte write PASS
```

若原值为 `0xFF`，测试值会改为 `0xFE`，最终应恢复为 `0xFF`。某次测试失败但成功恢复时，会显示 `restore PASS` 和最终的 `I2C multi-byte write FAIL`，恢复成功不会掩盖前面的失败。

`console_report_i2c_error()` 输出操作名称及 -1/-2/-3/-4 错误原因，不再把组合访问的全部错误写成 `write`。应用还会报告 GPIO 时钟配置失败或 I²C5 初始化失败。诊断通过已学习的 UART4 超时发送与 TC 等待接口输出，UART4 初始化前的失败无法通过该串口报告。

## 7. 文件分工与阅读顺序

| 文件 | 内容 |
|---|---|
| [inc/i2c.h](inc/i2c.h) | I2C5 基地址、寄存器结构体、位掩码和接口声明 |
| [src/i2c.c](src/i2c.c) | 初始化、单字节收发、通用多字节发送及重复 START 寄存器读取 |
| [src/rcc.c](src/rcc.c) | GPIO、I2C5、TIM2 与 UART4 时钟相关接口 |
| [src/gpio.c](src/gpio.c) | GPIO 复用、开漏、普通输出和读写 |
| [src/timebase.c](src/timebase.c) | 读取定时器计数作为 tick |
| [src/uart.c](src/uart.c) | UART4 初始化及诊断输出所需的发送接口 |
| [user/main.c](user/main.c) | 初始化、ID 核对、0x0C 写入/读回/恢复验证，以及串口显示与报错 |
| [start/start.S](start/start.S) | 设置栈、清 BSS、调用 main |
| [linker.ld](linker.ld) | DDR 中的代码、数据和栈地址 |
| [Makefile](Makefile) | Cortex-A7 构建、链接、BIN 和反汇编生成 |

明天复习顺序：先看 `user/main.c` 中 ID 核对之后的验证代码，理解 `{寄存器索引, 数据}` 两个字节的分工；再看 `I2C_MasterWrite()` 的 length/NBYTES、sent/TXIS 和 STOP 判断；随后回看 `I2C_ReadRegisterByte()` 的 TC/重复 START；最后查看两个串口显示辅助函数。今晚涉及的 I²C 和 main 函数均按“功能、参数、返回值”补齐注释。

## 8. 构建与运行

在本目录、已配置 Arm GNU 工具链与 make 的终端中执行：

```text
make -B
```

`-B` 强制重新构建，适合当前 Makefile 尚未自动跟踪头文件依赖的情况。当前 `TARGET=i2c5_cs42l51`，产物为：

- `build/i2c5_cs42l51.elf`：包含调试信息的链接镜像。
- `build/i2c5_cs42l51.bin`：当前 U-Boot 加载流程使用的裸二进制镜像。
- `build/i2c5_cs42l51.map`：链接映射。
- `build/i2c5_cs42l51.dis`：反汇编与源代码混合列表。

镜像链接地址为 `0xC0008000`，初始栈顶为 `0xC1000000`。U-Boot 初始化 DDR 后，沿用已验证的传输流程：

```text
loadb 0xC0008000
```

通过 SecureCRT 的 Kermit 发送方式传输 `build/i2c5_cs42l51.bin`，完成后执行：

```text
go 0xC0008000
```

此流程直接跳转执行 BIN 的入口，不接受将 ELF 文件原样当作 BIN 发送。启动代码屏蔽 IRQ/FIQ，初始化栈和 BSS，调用 main，main 返回后留在循环中。当前程序没有喂狗处理。

## 9. 下一步学习

前期已完成并读到 `0xD9` 的分离事务流程为：

```text
第一笔：START → 0x94 → ACK → 0x01 → ACK → STOP
第二笔：START → 0x95 → ACK → ID 字节 → NACK → STOP
```

接收阶段重点学习 `RXNE`、`RXDR`、读方向、末字节硬件 NACK 与 STOP。如果 RXNE 和 STOPF 同时置位，必须先处理尚未取出的接收数据。

CS42L51 的 `0x01` 寄存器中，`[7:3]` 为固定芯片标识 `11011`，`[2:0]` 为修订号。当前读到 `0xD9 = 11011 001`，与芯片标识一致。

重复 START 的 ID 读取已获得学习者上板通过反馈。今晚按要求完成多字节发送参考实现、寄存器写入/读回/恢复例程及注释，明天先阅读理解，再由学习者上板验证当前版本并提交串口结果。之后继续多字节接收、器件地址自增和错误恢复，中断与 DMA 仍按阶段安排后学。

学习过程与板端反馈见[I²C 起步记录](../../../records/sessions/2026-11-22-阶段4-I2C通信起步.md)，当前状态见[学习进度](../../../records/progress.md)。
