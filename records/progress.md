# 学习进度总表

> 最后更新：2026-11-10

UART波形验证与FIFO入门（2026-11-10，最新）：学习者报告已完成上一轮115200 8-N-1波形验证，不再要求重复该验证；未提供具体测量值，不补写实测数值。进入必学轮询FIFO：已依据本地ST CMSIS/HAL核对FIFOEN为CR1 bit29、TX/RX FIFO各8项、ISR bit7/5在FIFO模式表示TXFNF/RXFNE，TC仍为bit6。当前练习由学习者在初始化UE=0期间显式启用FIFO，完善相关状态位命名和说明，沿用既有逐字节轮询接口验证基本命令，不开启中断、不配置阈值中断；后续再练批量接口与溢出处理。尚未收到FIFO实现或实测。

接收返回值与命令恢复通过（2026-11-10，最新）：main现每轮用int result保存唯一一次HAL_UART4_GetString调用的结果，成功路径处理命令，返回2时提示超长，修复额外消费下一行的问题。源码复查通过，学习者明确报告已修改且验证，登记超长提示及后续合法命令恢复通过，不重复要求同一验证。README同步更新。下一步核对115200 8-N-1实际波形及轮询限制，再学习必学的轮询FIFO收发；教师未代改实现、构建或上板。

阶段4继续学习（2026-11-10）：复查261107/main.c，超长提示分支仍重复调用GetString。当前练习为每轮仅接收一次，用局部int保存返回值，再按0（处理命令）、2（提示超长）分别判断。重点理解接收函数调用会消费输入、可能阻塞，并非查询上次结果。修改与超长行后合法命令恢复验证待学习者完成，尚未进入FIFO实操。

超长命令提示复查（2026-11-09）：main函数说明已更新为LED_ON亮灯、LED_OFF灭灯。新增超长提示分支仍有问题：if和else if各调用一次阻塞GetString，第一行超长时会再次接收下一行，而非检查第一行结果；下一行若为合法命令会被消费但不执行。应在每轮循环只调用一次GetString，用int保存结果，再分别判断0/2。仅给局部修改指导，未改实现、未编译或上板。

阶段4 LED控制进展（2026-11-09）：学习者报告“使用按键控制灯亮灭并且经过了验证”。当前261107/main.c可见实现为串口LED_ON/OFF命令分别写固定高/低电平，执行后继续接收，未知命令返回parameter error，源码通过本轮相关逻辑复查。已询问“按键”指串口输入还是板载实体按键，尚未据此认定实体按键功能或具体边界场景已验证。待补超长行状态2提示，并更新main旧注释（仍描述永久闪烁）。README已按现有命令实现更新；FIFO必学安排保留。教师未修改代码、未代编译或上板。

阶段4 UART工程README更新（2026-11-08）：更新`project/stage_3/261107/README.md`，补充UART4引脚和J1401接口脚位、终端参数、已实测的轮询收发功能、当前LED_ON命令流程限制、构建产物与U-Boot运行方法，并列出后续学习安排。文档区分已报告上板验证的字节/字符串收发与尚未验证的命令控制应用。本次只改文档，未改代码、未构建或上板。

阶段4 UART后续学习安排（2026-11-08，按学习者补充更新）：FIFO收发列为必学内容。顺序为：完成`LED_ON/OFF`命令循环和边界测试；用逻辑分析仪核对115200 8-N-1波形并讨论轮询限制；以轮询方式学习TX/RX FIFO、状态标志和批量读写；再处理接收超时及错误/过载。若学习中断驱动FIFO或中断接收，先补A7异常入口与GIC基础；FIFO本身不会因使用而自动要求中断，因此先完成轮询FIFO练习。DMA、高级流控、RS-485/同步模式按以后项目需要选学。

Linux运行时按FS-MP1A系统复位键的安全性补充（2026-11-08）：已核对华清远见板卡资料，系统复位键用于MPU及外设复位；正常按键复位不会因次数本身损伤板卡。若Linux正在写eMMC/SD，硬复位可能打断存储操作，因此可用终端`reboot`并在U-Boot倒计时中断自动启动。若当前无Linux shell，偶尔按一次复位键重新进入U-Boot属于正常调试操作。看门狗复位仍未实测确认。

U-Boot运行裸机程序后看门狗复位（2026-11-08）：学习者判断裸机运行后自动复位可能由IWDG触发，这是合理但未确认的解释。周期性按复位键本身通常不会造成板卡物理损伤；若Linux已启动，优先在终端执行正常重启并于U-Boot倒计时打断自动启动，避免Linux写eMMC/SD期间强制硬复位或断电，以降低文件系统/存储数据不完整风险。记录每次go到复位的时间间隔，之后可查复位原因日志；本板具体复位源尚无证据。

261107工程函数注释补充（2026-11-08）：按学习者请求，仅为工程内GPIO、UART、RCC、main辅助函数及start.S入口补充/完善函数级注释，覆盖功能、参数、返回值、必要前置条件与行为说明；保留原有实现，不改业务逻辑。未编译或上板，本次只验证注释变更和diff空白检查。

命令字符串匹配与LED_ON逻辑复查（2026-11-08）：main已添加本地strings_equal辅助函数，完整字符串相等返回1，不相等返回0，能解决strcmp在-nostdlib链接下未定义的问题；但调用条件写成`strings_equal(...) == 0U`，导致不匹配命令反而进入LED处理分支，应改成非零/直接判断。该分支使用Toggle实现LED_ON，重复输入会在亮灭间切换，不符合ON命令幂等语义；应按LED有效电平调用HAL_GPIO_Write设为亮态，OFF命令再设为灭态。main仍包含已不再需要的string.h，可删但不是链接阻断。未编译/上板验证。

裸机链接错误诊断：strcmp undefined reference（2026-11-08）：在main.c中调用strcmp，头文件声明存在，但Makefile链接使用-nostdlib且仅显式加入-lgcc；因此链接器没有获得C库中strcmp的实现。推荐当前freestanding练习移除string.h并在main.c写小型字符串相等比较函数，且验证两串都在同一位置结束，避免前缀误匹配。-lgcc只用于GCC运行时辅助代码，不替代libc。尚未修改代码；学习者自行修复并重跑make。

UART GetString 行尾归一化与超长行处理已修改（2026-11-08）：按学习者请求只改`src/uart.c`中的GetString。现拒绝NULL/零容量；先读字节，再跨调用跳过CR后的可选LF；CR或LF均结束一行；最多保存buf_size-1字节并NUL结尾，超长后不再写缓冲区、继续丢弃到行结束，返回2；正常行返回0，参数/底层读取失败返回1。main仅在返回0时执行命令，因此超长输入不会按截断内容执行；目前没有超长提示。未编译或上板验证。

UART GetString 的行结束与超长输入处理指导（2026-11-08）：解释比较应使用`==`，而`=`是赋值表达式；建议把CR和LF都视作行结束，CR结束后用跨调用的skip_lf状态吞掉可能紧随的LF，以兼容CRLF且不误吞下一行首字符。缓冲区先要求容量非零；最多存buf_size-1个普通字符并始终保留NUL位置，超出后设置overflow但继续读取/丢弃直到CR/LF，终止字符串后返回独立的超长状态（如2），确保下一次读取从下一行开始。学习者自行实现，未审阅修订代码或测试。

UART字符串发送已验证；按行接收/命令处理代码复查（2026-11-08）：`HAL_UART4_PutString(const char *str)`对NULL返回1，逐字节发到NUL且不发送终止符，正常/空字符串返回0，源码符合约定；学习者报告上板验证通过。额外检查到新写的GetString暂不通过：buf_size==0时仍写buf[0]越界；行结束条件使用`data = '\\n'`赋值而非比较，仅CR能退出、LF不能退出；缓冲区装满时未清理本行剩余输入，残余字符会成为下一次命令。main检测到LED_ON时目前只发送"UART4 ready"，还没有操作LED GPIO；循环末尾的粗延时也降低命令响应。由学习者先修正/明确GetString契约并补上LED动作后再测命令流程；字符串发送既有验证不需重测。本轮只读审阅，未改实现、未编译。

UART4单字节接收回显上板通过（2026-11-08）：学习者明确报告已通过上板验证，结合此前源码审阅，当前基本轮询收发回显任务通过，不要求重复同一基础验证。下一步先实现`int HAL_UART4_PutString(const char *str)`：NULL返回1，空字符串成功返回0，逐字节调用PutChar，遇到'\0'结束但不发送终止符，完成返回0；调用时显式提供所需换行如"UART4 ready\r\n"。由学习者自行实现，后续接按行接收与简单命令，不提前引入中断/DMA，也不将其标记已完成。

UART4单字节接收回显源码审阅通过（2026-11-08）：GetChar已检查NULL、在ISR bit5为0时等待、收到后只读一次RDR并保存到uint8_t输出，成功0/错误1；声明定义一致。RDR赋给uint8_t会保留低8位，当前8-N-1范围正确，无须因未显式掩码而判错。main已改为接收后发送并移除实际延时调用；当前固定传入有效局部变量地址，未检查GetChar返回值不构成此路径的功能阻断，建议仍按契约仅成功后回显。UART_IST_RXE宏位号正确但名称拼写/含义不准确，建议同步改成UART_ISR_RXNE；两者为完善建议，不阻塞当前基本功能源码通过。未代编译、运行或修改实现；接收回显实测待学习者提供，验证时关闭SecureCRT本地回显。

UART4轮询发送实测完成、进入接收回显（2026-11-08）：学习者明确报告板端实测功能已实现，记录为当前单字节发送功能通过；未提交本轮完整构建日志或波形，不扩展为UART全部功能验收。下一步由学习者实现`int HAL_UART4_GetChar(uint8_t *data_out)`：NULL返回1，正常接收一个字节并写输出返回0，无数据时阻塞等待，当前只做8-N-1轮询接收。依据本地STM32MP157 CMSIS和ST官方HAL，ISR bit5为RXNE/RXFNE，有数据后读取RDR低8位；main接收成功后调用既有PutChar回显，移除周期'z'发送和人为延时。验收用终端关闭本地回显后逐字符收发验证。中断、DMA、超时和接收错误恢复留后续，不代写接收实现。

UART 当前完整流程复查（2026-11-08）：重新只读核对uart.c/h、rcc.c/h、main、GPIO与启动文件、Makefile。此前TXE等待条件、BaudRate配置、DeInit声明/定义、切换前HSI就绪和各步骤失败停留已修正，当前115200/8-N-1轮询发送'z'的主流程未发现新的明确阻断逻辑错误。仍需关注构建：uart.c的uint64_t除法在当前Cortex-A7/-O0下可能生成__aeabi_uldivmod等辅助调用，而Makefile使用-nostdlib且未显式链接libgcc；建议链接命令在对象文件后加入-lgcc，实际是否缺符号以学习者严格构建输出为准。源码审阅不等于构建/上板通过。PutChar无超时、错误路径只死循环及有限次数等待非毫秒计时为当前练习限制，后续再完善，不作为新增上板前功能任务。本轮未代改实现或编译运行。

UART HSI 切换前就绪流程（2026-11-08）：按学习者请求修正时钟切换项。在RCC模块新增RCC_OCENSETR映射及RCC_HSI_EnableAndWaitReady()：向HSION写1，并有限轮询OCRDYR.HSIRDY(bit0)和HSIDIVRDY(bit2)，超时返回失败。main现按顺序检查UART4 APB门控、UART4_DeInit、HSI开启/就绪、UART_HSI选源、频率查询、UART4_Init；任何失败均停留错误路径。寄存器地址与位号已和本地STM32MP157 CMSIS设备头文件核对。未构建或上板；轮询上限为CPU循环次数而非毫秒。

UART4_DeInit实现（2026-11-08）：按学习者明确请求直接修改DeInit实现及声明/调用点。接口统一为int；UART处于UE+TE启用状态时轮询ISR.TC，使用有限轮询次数，超时返回1并保留CR1原状态，避免切钟前截断发送或无条件永久阻塞；完成后清UE/TE/RE并返回0。main检查DeInit失败后停留错误路径。未编译或上板；有限轮询次数是迭代上限，不代表精确毫秒超时。

紧接DeInit修改的流程复核（2026-11-08）：当前main已将GetKernelClockHz放到SelectClock之前，但该查询函数只在当前选源已经是HSI时成功，因此这不能作为对任意原时钟状态都有效的HSI预就绪检查；若启动时已是HSI，则可检查状态并选择同一源。main仍忽略UART4_Init返回值，后续无条件进入发送循环，需在运行前补齐错误处理与时钟源状态前置条件。源码修改尚未编译或上板。

UART当前剩余项复查（2026-11-08）：UART4_DeInit声明/定义及TC等待已完成源码层修正。运行前仍需：先通过独立机制确认HSI已开启且HSIDIV已稳定，再选择HSI（GetKernelClockHz目前只有在当前UART24SRC已为HSI时才能成功，不能普遍充当前置检查）；将UART4_Init的返回值纳入错误路径，并在任何时钟查询、选源或初始化失败时停止后续发送。当前Makefile未由学习者重新构建验证，尚无板端输出证据。

UART 时钟切换前的关闭方式（2026-11-08）：建议由UART模块提供UART4_Disable或UART4_PrepareClockSwitch接口，main只调用接口，不直接操作CR1。关闭前保持UART4 APB接口时钟开启；若发送器已启用，先轮询ISR.TC确认最后一个停止位已发出，再清CR1的TE/RE/UE，最后才能切换共享UART24内核时钟。不要关闭RCC门控来代替UE关闭，也不要让RCC选源函数隐式修改UART寄存器。当前阶段可先实现无超时版本，后续再增加超时返回以避免硬件异常时永久阻塞；切换前还需协调共享该选源的USART2。

UART/RCC职责边界澄清（2026-11-08）：UART模块的关闭函数只访问UART寄存器，不在uart.c直接调用RCC接口或读写RCC寄存器。APB门控需由上层先使能并保持开启；main等上层流程依次协调UART发送完成/禁用、RCC切换和查询时钟、UART重新初始化。此前提及UART关闭函数需APB时钟可用，是调用前置条件，不代表UART驱动负责操作RCC。

UART4 上板前复查（2026-11-08）：TXE轮询条件和115200波特率已修正，main已加入UART4_DeInit与UART4_Init调用；但当前不建议上板运行。头文件将UART4_DeInit声明为void而uart.c定义为int，属于冲突类型，严格构建会失败；DeInit目前直接清UE，没有等待已启动发送完成；RCC的选源接口要求目标源已就绪，但main先切到HSI、再调用检查HSIRDY的GetKernelClockHz，顺序不能证明切换前HSI已ready；main忽略UART4_Init返回值，失败仍会进入PutChar轮询，可能永久卡住。由学习者先统一函数声明/定义、完善关闭收尾和切换前时钟就绪保证，并在任何初始化失败时停留错误路径，再自行严格编译和上板验证。仅源码审阅，未编译/运行。

UART 轮询发送与主流程复查（2026-11-08，覆盖下方首轮待办）：已确认uart_t寄存器顺序及volatile成员、HAL_UART4_PutChar(uint8_t data)声明/定义、局部配置验证后再写硬件的初始化结构，以及Makefile纳入rcc.c/uart.c等修改。当前仍未通过：发送函数等待TXE的条件写成!=0，应在该标志为0时等待；main的uart4_config未指定BaudRate，成员默认为0，UART4_Init会返回失败；main忽略选源/频率查询/初始化的返回值，查询失败时还会读取未初始化clock_hz；切换共享内核时钟前尚未在上层保证UART已停止并禁用、目标源已就绪，Init内部后续清UE不能补救先前切源顺序。由学习者修正上述事项，RCC不负责清UART的UE。本轮仅源码审阅并更新记录，未改驱动、未编译或上板。

UART 单字节发送首轮审阅（2026-11-08）：最新UART_SendByte(char *data)只检查NULL后写TDR，缺少轮询ISR的TXE/TXFNF bit7，尚不能作为可靠阻塞发送。建议当前8-N-1范围使用uint8_t按值传参并统一UART4_SendByte命名，避免char有符号扩展及不必要指针；如保留指针应使用const且将数据明确转为uint8_t。发送前等待TXE，写TDR后返回仅表示提交数据，未必线上发送完毕；需要发送收尾时另查TC bit6。无限轮询无超时须说明前提，暂不强制增加超时功能。main目前只使能UART4时钟后循环写TDR，尚未调用UART4_Init或查询频率，不可据U-Boot继承状态认定自写初始化通过。仅审阅、未改实现或编译上板。

UART4 波特率误差检查讲解（2026-11-08）：在已验证BRR范围后、任何硬件写入前，用expected_clock=(uint64_t)BaudRate*brr及diff=abs(uart_clock_hz-expected_clock)，通过diff*100 > expected_clock*2检查分频造成的相对波特率误差是否超过示例阈值2%。公式等价于|clock/BRR-baud|/baud，不先做整数除法，避免实际波特率截断掩盖边界误差。2%是本练习示例策略，不是UART通用保证；实际容差还需考虑双方时钟误差和数据格式。此前声称设置TE/RE前必须先清两位的建议不准确，原来的OR操作已经能够将两位置1，不必额外清除。未改驱动、未编译或上板。

UART4_Init 新版源码审阅（2026-11-08）：已加入零值检查、uint64_t四舍五入BRR计算及0x10至0xFFFF范围检查；OVER8清位、PRESC不分频、CR3的RTSE/CTSE清位和CR1的TE/RE/UE设置正确。但尚未通过：停止位switch内、case标签前的`uint8_t MASK=3U`初始化会被switch跳转绕过，使用未初始化MASK；uart.c使用UART_STOP_1_5BIT而uart.h仍定义UART_STOP_2_5BIT，标识符不匹配；枚举仍先收窄到uint8_t；参数枚举验证仍在禁用UART并写BRR之后，错误会留下已修改/部分配置的硬件。建议先用三个switch验证并生成局部配置，再按禁用UART、明确OVER8/PRESC、BRR及其他配置、TE/RE、UE的顺序应用。未编译、未代改驱动或上板。后续构建需注意uint64_t除法在A7裸机-nostdlib环境可能引入__aeabi_uldivmod等libgcc辅助符号，不等于链接已验证。

UART4_Init 参数检查讲解（2026-11-08）：针对学习者提问，明确入口检查顺序：config非NULL、uart_clock_hz和BaudRate非0；直接对枚举做范围/switch验证，不能先收窄成uint8_t；以16倍过采样、PRESC不分频为当前固定前提，用uint64_t计算四舍五入后的BRR，检查BRR非0且不超过寄存器可表示范围，必要时拒绝分频过小的不可实现波特率。所有检查先在局部配置值中完成，成功后才清UE并写寄存器。UART函数只验证传入时钟参数，时钟源是否就绪由RCC查询函数和调用方保证。未修改、未编译或上板。

UART4_Init 组织方式建议（2026-11-08）：只读检查最新uart.h/c，当前实现先清UE再检查WordLength，且把枚举收窄到uint8_t，可能让无效输入修改硬件或被截断。建议先检查指针/频率/波特率，再用三个独立switch验证字长、校验、停止位并生成局部CR1/CR2位配置，提前计算并校验BRR，所有参数有效后才禁用UART并应用配置。避免组合条件嵌套，不需立即引入配置查表或拆大量函数；枚举直接参与switch，不先收窄。提醒Parity枚举ODD=0并不等于PS=0，需显式映射；字长M1/M0分离，且启用校验会占用配置字长中的一位。当前STOP_2_5BIT名称仍需修正；当前先支持手册允许的目标停止位。未改驱动、未编译或上板。

RCC 三函数注释补齐（2026-11-08）：按学习者明确请求，仅在261107/src/rcc.c补充SelectClock、EnableClock、GetKernelClockHz的功能、参数、返回值和前置条件说明。明确USART2/UART4共享选源、由调用方管理UART停用与源就绪、使能函数返回0仅表示完成寄存器写入、查询只支持HSI且输出为PRESC之前的标称频率，失败保持输出不变。未修改函数实现，未编译或上板。

UART4 RCC 当前范围源码审阅通过（2026-11-08）：再次只读复核最新rcc.h/c，SelectClock已用无符号范围检查并一次读改写UART24SRC；EnableClock直接写APB1ENSETR bit16；GetKernelClockHz已检查NULL、HSI选源及OCRDYR的HSIRDY/HSIDIVRDY（0x05），正确提取HSIDIV并通过输出参数返回64MHz右移后的标称频率，成功0/失败1。当前三函数可按“选源、使能、仅HSI频率查询”范围收尾，不要求立即实现PLL/PCLK/CSI/HSE频率查询。选择前源已就绪、受影响UART已停用等前置条件由调用方保证，建议写入注释；RCC不修改UART_CR1.UE，也不负责开启振荡器。未编译、未上板，不代表完整时钟树或串口通信验收通过。下一步由学习者编写UART4_Init，构建集成时加入rcc.c/uart.c。

RCC/UART职责划分澄清（2026-11-08）：学习者指出RCC函数中清CR1.UE不利于模块边界，判断正确。时钟选择函数只处理RCC寄存器，UART禁用属于调用前置条件，由UART模块执行，main或上层流程协调；访问UART寄存器前需先使能APB接口时钟。切换共享UART24时钟还需协调USART2，不由RCC函数隐式停止通信。已补入当日学习记录，仅设计讲解，未改驱动或实测。

UART4 RCC 三函数修正版审阅（2026-11-08，覆盖上一轮错误清单）：最新 rcc.h/c 已修正 PLL4_Q 拼写、所有已检查寄存器地址、UART4EN 直接写bit16、UART24SRC低3位提取、HSIDIV按位与提取及成功0/失败1/频率输出参数契约。uint8_t可保存0至3的分频编码，无需强制改成uint32_t。SelectClock当前分两次清零/置位会保留其他位，但中间会短暂选择PCLK1；建议先局部读出、合并位域后一次写回。参数检查建议将source先转uint32_t再与UART_HSE比较，可靠拒绝负枚举值。当前Get仅支持HSI，其他源返回不支持，作为阶段实现可接受；仍需明确源已开启并就绪、分频已稳定的条件，检查RCC_OCRDYR.HSIRDY/HSIDIVRDY后再报告频率成功。切换前应停止受影响UART通信并清CR1.UE；UART24SRC同时影响USART2，不应把关闭UART误解成必须关闭APB门控。基础位操作修正通过，硬件时钟准备尚未验收；未代改驱动、编译或上板。

UART4 RCC 两个函数复查（2026-11-08）：学习者已实现 SelectClock 与 GetKernelClockHz。SelectClock 的清零后直接赋值会覆盖 RCC_UART24CKSELR 其他位，应使用读改写 `(reg & ~0x7U) | source`；枚举名 `UART4_PPL4_Q` 拼写应为 PLL4_Q。UART4EN bit16 和 RCC 地址已改正确，但 APB1ENSETR 是 write-one-to-set 寄存器，建议直接写 `(1U << 16U)`，不要先读后或写。GetKernelClockHz 当前把完整 UART24CKSELR 与2比较，应先掩码低3位；HSIDIV 当前使用 OR 0x3，应使用 AND 0x3；函数错误返回0、成功返回频率，和建议的状态返回契约冲突，应统一为成功0、错误非0、频率通过输出参数返回。还需考虑 HSI 就绪检查，且 UART24CKSELR 同时影响 USART2/UART4。Makefile 当前仍未加入 rcc.c/uart.c。未代改、未编译或上板。

外设选源范围补讲（2026-11-08）：不是每个外设都具有独立时钟源选择。以GPIOB/G固定使用AHB4接口时钟、UART4具有可选内核时钟且与USART2共用选择字段作对照；所有外设初始化应核实所需时钟条件，但不强制新增独立选源函数或重新配置已有正确选源。总线父级时钟树配置与每外设独立选源不同。详见当日UART4时钟辨析记录；无新代码或实测验收。

UART4 总线时钟与内核时钟辨析（2026-11-08）：学习者询问已使能APB1后为何还需RCC_UART24CKSELR。解释UART4的APB接口使用PCLK1访问寄存器，串口收发/波特率逻辑使用可选择来源的内核时钟；UART4EN是时钟使能控制，UART24SRC是时钟来源选择，不能把UART4EN理解为只打开寄存器访问或必然选择PCLK1。PCLK1本身也是内核候选源之一，因此不必须切换HSI；当前先读取继承的选源与父时钟状态，再决定保留或重新配置。此前HSI单路径建议是学习简化方案，不是硬件要求。频率用于后续PRESC/BRR计算，不能凭APB1归属假定。仅概念讲解与记录，未修改或验证硬件。

UART4 时钟使能与 HSI 路径补讲（2026-11-07，覆盖此前未核实时钟选择寄存器名称）：只读复核最新 rcc.h/rcc.c，使能位 UART4EN bit16 正确；宏将 RCC 地址误写为 0x40010A00，应为 RCC base 0x50000000 加 offset 0xA00，即 0x50000A00；名称 ENSTER 应改为 ENSETR，置位寄存器可直接写掩码，int 函数成功路径需 return 0。uart4_clock_source_e 尚未定义。通过本地 STM32 CMSIS 头文件及 HAL RCC 源码核实：USART2/UART4 共用 RCC_UART24CKSELR，offset=0x8E8、UART24SRC=[2:0]，值0=PCLK1、1=PLL4_Q、2=HSI、3=CSI、4=HSE。此前教师提及 RCC_UART4CKSELR 名称错误，已纠正。为降低学习跨度，当前先只支持 HSI 路径：选择前保证 UART 禁用且源已就绪；读 RCC_HSICFGR.HSIDIV[1:0]，按 64000000U >> div_code 取得标称内核频率（1/2/4/8 分频），不直接假定64MHz，也不为串口随意修改全局HSI分频。Get 函数检查输出指针、当前选源和就绪状态，不支持的选源返回错误。未代改工程、未编译或上板；以本地 ST CMSIS/HAL 为位域与计算交叉依据。

UART4 RCC 接口设计讲解（2026-11-07）：建议避免用含义过大的 `RCC_Init()` 只处理 UART4。把 UART4 内核时钟源选择、APB 外设时钟使能、实际内核时钟频率查询拆分为独立接口；候选原型为 `int RCC_UART4_SelectClock(uart4_clock_source_e source)`、`int RCC_UART4_EnableClock(void)`、`int RCC_UART4_GetKernelClockHz(uint32_t *hz)`。查询函数通过输出参数返回频率，错误用状态返回值表达。具体时钟源与父时钟频率须按 RM0436 以及当前启动链时钟状态核实；如后续统一配置整棵 RCC 时钟树，再另设全局时钟初始化接口。当前仅讨论接口设计，未写代码或上板。

UART 配置结构体审阅（2026-11-07）：只读检查 261107/inc/uart.h 与 src/uart.c。当前 uart_init_t 是软件初始化参数，已有波特率、字长、奇偶选择、停止位；奇偶校验枚举只有 ODD/EVEN，缺少表达 8-N-1 所需的关闭校验选项，建议明确 NONE/EVEN/ODD 并在初始化中分别配置 PCE/PS。UART_STOP_2_5BIT 名称不正确，通用 STOP 字段的对应选项是 1.5 位，具体 UART4 支持范围还需按手册限制选择；当前先使用 1 位。字长枚举是软件编号，需映射到 CR1 的 M1/M0，不能整体当作连续位域左移；使用校验时需核对有效数据位数量。发送/接收模式可增加成员，也可在当前固定收发方案中明确约定；计算 BRR 所需的 UART 实际输入时钟需由参数或时钟模块提供，不凭空假定频率。uart.c 中 UART_Init() 尚无返回类型和参数，应在实现前明确接口并在头文件声明。仅审阅，不代改实现、不编译或上板。

UART4 寄存器宏初步审阅（2026-11-07）：学习者新增 UART4_CR1/CR2/BRR 地址宏。当前强制转换写法 `(*(volatile uint32_t *))address` 语法不正确，应改成 `(*(volatile uint32_t *)address)`；UART4 基地址 0x40010000 及 CR1/CR2/BRR 偏移需以 RM0436 的 UART4 register boundary 表再次核对。后续发送还需要 ISR 与 TDR，接收需要 ISR 与 RDR，建议先用寄存器结构体或完整宏映射。uart.c 的 `UART_Init()` 仍缺少返回类型、参数和头文件声明；Makefile 的 C_SRCS 也尚未加入 src/uart.c、src/rcc.c。当前先完成地址/接口/时钟资料核对，再实现最小 UART4 初始化，不宣称已编译或上板。
> 当前能力基线：阶段 1 已通过；学习者自述学过数模电，会使用示波器、万用表和逻辑分析仪，模拟电路分析较弱；阶段 2 能力待任务验证。
> 当前阶段：阶段 4——ARM 与 Cortex-A7 裸机基础（教师指导学习）
> 当前任务：阶段 4.2 UART4 轮询通信起步；工程为 project/stage_3/261107，先补齐 GPIO 复用配置，再逐步学习时钟、UART 初始化和发送

串口 GPIO 初始化源码审阅（2026-11-07）：检查 261107 最新 gpio.c、gpio.h、main.c。AF 分支现已按 index 选择 AFRL/H、使用 (index%8)*4 位移、配置 Otype/Speed/Pupd，并写 GPIO_AF_MODE；TX PG11=AF6、RX PB2=AF8 的配置字段及 AHB4 GPIOB bit1/GPIOG bit6 选择正确。但发现 GPIOG 宏仍映射 0x50002000（GPIOA），应按 RM0436 改为 0x50008000；这是阻止 TX 正常工作的关键错误。AFRL/H 的 AlternateFunc 左移仍缺少 uint32_t 强转，高位有效 AF 下会有有符号溢出风险。建议所有 MMIO 成员改 uint32_t，保持寄存器宽度明确。GPIO 时钟门控与 UART4 外设时钟是两件事，目前代码仅开启 GPIOB/G 时钟、配置引脚复用，不代表 UART4 已初始化。源码未被修改、未编译或上板；等待学习者修正后再检查完整初始化。

GPIO AF 分支第二轮审阅（2026-11-07）：学习者已正确按 index 选择 AFR，并补上 OTYPER、OSPEEDR、PUPDR 及 GPIO_AF_MODE。只读复核当前源文件仍计算 af_shift=index*4；高半区清位减去 0x10（16）不正确，若保留该全局位移应减去 8*4=32。建议统一改为 af_shift=(index%8)*4，两路清位/设位均直接使用 af_shift；AFRH 设位仍使用 _pin 掩码计算，必须移除。两路 AlternateFunc 左移前均应转 uint32_t。以 PG11 从44换算到12说明。当前局部分支仍未通过，未代改源码、编译或上板。

GPIO AF 分支首轮审阅（2026-11-07）：学习者提交 AF 分支，已在函数入口检查 AlternateFunc 不大于 15，但分支实现尚未通过。主要问题：Pin 和 _pin 是位掩码，寄存器选择与位移应使用循环中的引脚编号 index；清除位域应为 ~(0xFU << af_shift)，而不是先对位移量取反；AFRH 的字段掩码同样应为 0xF，不能用 16；MODER 应写 GPIO_AF_MODE 而非 GPIO_OUTPUT_MODE。AF 编号左移前应转为 uint32_t，避免整数提升后高位左移的有符号溢出；AF 分支还需配置 Otype、Speed、Pupd。以 PB2 的 index=2、_pin=4 和 PG11 的 index=11、_pin=2048 对照说明。仅审阅与讲解，未代改工程、编译或上板；等待学习者修正局部分支。

UART4 新工程与接口确认（2026-11-07）：学习者新建 project/stage_3/261107，复制 GPIO 闪灯工程用于 UART 学习，并确认 UART4_TX=PG11、UART4_RX=PB2 是当前 SecureCRT 使用的通道。只读检查确认 main.c 仍为 LED 循环，已加入 TX/RX 端口和引脚宏，uart.h 只有头文件保护，uart.c/rcc.c/rcc.h 为空；GPIO AF 分支仍为空，寄存器结构体截至 BSRR。Makefile 仍以 gpio_led 命名且仅编译 main.c、gpio.c；后续实现模块时再同步维护。首个教学目标是理解并补齐 GPIO 复用：PG11 为 AF6、PB2 为 AF8；对照寄存器表扩展 GPIO 映射至 AFRL/AFRH，保留中间 LCKR 的位置，并在配置结构体加入 AF 编号。已核对 ST 官方 UART 调试资料与 CMSIS 设备头文件。当前不代写驱动、不编译或认定 UART 程序运行通过，UART 时钟来源与波特率配置待后续核对。此前编辑器问题未收到恢复反馈，新目录 .vscode 仍为 MinGW，保留待办。见当日 UART4 起步记录。

GPIO 复用寄存器结构体审阅（2026-11-07）：学习者提交的 `gpio_t` 已按顺序添加 LCKR、AFRL、AFRH；`gpioinit_t` 加入 `AlternateFunc`，结构顺序和字段用途符合当前目标。指出 AFR 每 pin 占 4 位：pin 0–7 使用 AFRL，pin 8–15 使用 AFRH；PG11 计算 AFRH shift=12 并放 AF6，PB2 计算 AFRL shift=8 并放 AF8；同时 AF 值需验证不大于 15，建议 MMIO 字段用 uint32_t 明确寄存器宽度。下一步由学习者实现 HAL_GPIO_Init 的 AF 分支（包括 MODER 复用模式和 AFR 字段），提交局部源码审阅；当前未编译或上板。寄存器布局参考 ST RM0436 GPIO register map。

阶段 2 接口电路复盘补记（2026-11-07）：按学习者要求，将 J1401 UART 的 100 Ω 串阻、上拉及 TVS 作用补回《阶段2-接口保护基础》，并在任务 2.2 原理图记录中加入摘要与引用。强调空闲高电平与高阻状态上拉的区别、串阻的辅助限流/信号边沿作用，以及 TVS 的瞬态钳位和布局/参数限制。沿用此前原理图记录，不新增 D50/D51 型号核查、保护裕量或硬件实测结论；当前阶段与待办不变。

VS Code 生效配置定位（2026-11-07，最新）：学习者反馈“编辑配置”打开的是 stage_3/.vscode/c_cpp_properties.json。只读检查确认父层配置仍为 MinGW，261102/.vscode 已由学习者改成 ARM，先前修改未作用于当前编辑上下文。此前仅凭资源管理器显示 261102 就排除工作区问题的判断过早。指导新建 VS Code 窗口，在新窗口通过“打开文件夹”选择完整的 261102 路径，再打开 user/main.c 并运行编辑配置，核对打开的是 261102/.vscode/c_cpp_properties.json；之后验证 gpio.h 跳转和类型诊断。无需再重复修改已正确的子目录 JSON，未删除父层配置，未代开 GUI，尚待学习者验证。

VS Code 同名头文件排查更新（2026-11-07）：学习者报告对 gpio.h 跳转后进入带有 mingw-w64 runtime package 声明的文件，而非自写头文件。只读确认本机 MinGW 确实存在 x86_64-w64-mingw32/include/gpio.h，与项目 inc/gpio.h 同名；项目头文件包含 stdint.h 并定义 gpioinit_t。当前证据指向编辑器命中错误同名头文件，尚未核对实际生效配置或证明搜索路径被忽略的具体原因。指导保留工程 inc 搜索路径，将编辑器编译器和目标配置对齐 arm-none-eabi-gcc/Cortex-A7/C11/windows-gcc-arm，选择对应配置并重建 IntelliSense 数据库；再 F12 验证是否进入项目 inc/gpio.h。未代改配置、未编译，编辑器恢复待学习者反馈。详见当日 VS Code 配置记录。

Makefile 本轮收尾（2026-11-07）：学习者执行并提交 `make -nB` 完整输出，展示 user/main.c、src/gpio.c、start/start.S 到对应 .o、链接 ELF/map、提取 BIN 和输出反汇编的命令。经对照确认已找到 user/main.c 的编译命令，理解其与 CC/CFLAGS/$</$@ 的对应；这是命令预览结果，不是本轮实际编译或板上验证。当前工程 Makefile 的基本阅读与使用讲解可以告一段落，不新增验收测试；后续新增源文件时巩固独立维护能力。栈保护、展开表、自动头文件依赖等留待需要时学习；此前 GPIO 头文件依赖的补充尚未由源码复核确认，保留待办。UART 未在本轮开始。

增量构建与现有遗漏（2026-11-07）：学习者已口头确认理解链接时由 Map 选项生成布局报告。本轮读取当前 Makefile（学习者已加部分复盘注释），继续解释普通文件目标通过存在性、依赖修改时间决定是否重建；结合 main.c 修改说明只重编 main.o，再重新链接并更新 BIN/反汇编。发现此前生成的 C 模式规则仅依赖 `%.c`，缺少 `inc/gpio.h`，因此只改 GPIO 头文件可能不会触发重编。当前两个 C 文件都使用该头文件，可先在 C 模式规则的依赖列表中增加它作为最小修正；未擅自改动工程，留待学习者补充。没有执行构建或重复基础测试；自动头文件依赖可后续学习。

编译参数与后续重点（2026-11-07）：已讲 CPU_FLAGS、C11/freestanding、调试告警和头文件目录；学习者逐项表示理解。栈保护与调用栈展开表参数介绍后，学习者明确要求暂时略过、以后慢慢理解，故不视为已掌握，不继续深挖。当前继续 LDFLAGS 的用途，重点说明 `-nostdlib`、通过 `-Wl,` 传递链接选项、使用 linker.ld 与生成 map 文件，build-id 只作简单说明。本轮未改动工程或执行构建，后续应结合实际操作巩固规则而非继续堆砌参数。

Makefile 连续复盘（2026-11-07）：学习者先混淆 ELF 目标与规则右侧依赖，后经 main.o/main.c 局部示例理解 `$<`；随后对 `$@`、ELF 链接规则、CROSS_COMPILE 前缀与 `?=`/`:=`、OBJS 后缀替换/addprefix、百分号模式规则、dir/mkdir、默认 all 与依赖查找过程分别表示“明白”。这些为口头理解反馈，尚未新增独立实现验收。当前继续讲 clean、inspect 和 `.PHONY`，解释操作目标不按同名文件判断是否需要执行；声明 all 为 phony 不等于强制重新编译全部文件。工程未修改、未运行构建或清理，下一步仍需解释 CPU_FLAGS/CFLAGS/ASFLAGS/LDFLAGS 中的参数。

Makefile 复盘恢复（2026-11-07）：按学习者要求继续学习当前 `261102/Makefile`，未改动或执行工程。第一步从源码到对象文件、ELF、BIN 的整体构建关系入手，用现有 BIN 生成规则对照展开后的具体文件名和命令，解释 TARGET/BUILD_DIR/OBJCOPY 变量引用、目标/依赖/命令，以及自动变量 `$<`、`$@`。本轮只作教学，不重复基础 Make 验收、不追加运行测试，不据讲解认定掌握；后续继续解释 CROSS_COMPILE、赋值形式、对象列表变换和模式规则。

学习收尾与下次安排（2026-11-05）：学习者明确暂不开始 UART，认为当前生成的 Makefile 写法较复杂，希望明天先学习该文件；今日到此结束。下次结合现有 `project/stage_3/261102/Makefile`，先串起“C/汇编源文件 → 目标文件 .o → 链接 ELF → objcopy 生成 BIN”的构建流程，再逐步解释目标/依赖/命令、变量、文件列表变换、模式规则及自动变量；从容易理解的显式规则对照现有简写，不一次堆砌全部参数，不追加今日练习。启动与链接知识继续保留为待巩固项，UART 只作为之后的方向，不提前推进。

后续路线与阅读建议（2026-11-05）：学习者表示链接脚本尚未理解透彻，希望边实践边慢慢学习，并询问相关书籍和点灯后的学习安排。建议按需阅读《程序员的自我修养——链接、装载与库》的目标文件、静态链接和装载内容，以 GNU ld 手册的 SECTIONS/Input Section Description/PHDRS 对照当前脚本；ARM Cortex-A Series Programmer's Guide 可作为后续 ARMv7-A 栈、寄存器和异常知识参考，不要求现在通读。下一步进入任务 4.2，先确认板载调试串口对应的 UART 与 TX/RX 引脚，再逐步实现轮询发送字符、字符串、接收及回显，之后学习定时器替代粗略忙等。当前仅确定教学方向，未新增 UART 代码或宣称完成 UART 验收；启动、链接脚本仍为待巩固项，不以阅读/运行成功视为掌握。

链接脚本复盘（2026-11-05）：学习者表示能分别理解通配符和局部语句，但尚不能理解完整脚本如何工作，要求从头连贯讲解当前 `261102/linker.ld`。本轮按“目标文件中的输入段 → 链接器合并并分配地址 → 启动代码引用链接符号 → BIN 加载和执行”的顺序讲解，覆盖 ENTRY、PHDRS/权限、SECTIONS/位置计数器、text/rodata/data/bss、对齐、NOLOAD、COMMON、栈顶符号和 DISCARD。强调链接脚本在构建时生效，不能自行初始化硬件或内存；PHDRS 权限不等于配置 MMU，栈顶赋值不等于预留栈空间。仅记录本轮讲解，不标记为独立掌握。

复盘安排与教学偏好（2026-11-05）：学习者已通读 main.c、start/start.S、linker.ld、Makefile 及说明，但表示仍有许多内容不理解，希望在后续学习中逐步讲清。后续结合现有闪灯例程，每次集中解释少量相关概念，先说明作用和必要性，再结合对应语句及可观察现象理解；避免一次铺开全部汇编、链接语法和编译参数，也不把通读或板上运行成功视为已掌握启动知识。待逐步覆盖：GPIO 配置与 volatile、从 U-Boot 到 _start/main 的执行路径、栈与 bl、BSS 清零、链接地址和链接符号、编译/汇编/链接与 Make 规则、ELF/BIN/map/反汇编的区别，以及 loadb/md.b/go 的作用。当前不追加练习或立即考核。

板上运行验证通过（2026-11-04）：首次通过 SecureCRT/U-Boot `loadb 0xc0008000` 加载时误发 ELF，执行 `go 0xc0008000` 后重启；改为加载 raw `gpio_led.bin` 后能够运行并观察到 LED 现象。原因是 `loadb` 只把文件字节原样写入指定内存，`go` 直接从该地址执行，不会解析 ELF 头并按 program header 装载段；ELF 开头是 `0x7f 'E' 'L' 'F'` 元数据，不能当成入口指令执行。BIN 是 objcopy 从 ELF 提取的裸镜像，首字节就是链接到 `0xC0008000` 的 `_start` 指令，适合当前 `loadb` + `go` 流程。硬件上已确认启动路径和 GPIO 闪烁现象可工作；尚未记录闪烁频率数值，也未以定时器替代粗略忙等。详见 `records/sessions/2026-11-04-阶段4-LED1闪烁裸机工程生成.md`。

GPIO LED 闪烁裸机工程已生成（2026-11-04）：在 `project/stage_3/261102` 新增 Cortex-A7 `start.S`（设栈、清 BSS、调用 main）、`linker.ld`、`main.c`、Makefile 和运行说明。主程序按 LED1=PZ5 的假设开启 A7 AHB5 GPIOZ 时钟，配置推挽输出后无限循环 Toggle，延时暂用粗略 volatile 忙等。为让 MMIO 指针类型匹配，GPIO 函数端口参数改为 `volatile gpio_t *`；`gpio.c` 补 `<stddef.h>` 定义 NULL。当前 Windows 工作区交叉编译 `make clean all inspect` 通过；ELF 为 ARM ELF32 EABI soft-float，入口 `0xC0008000`，使用 `0xC1000000` 栈顶，RAW BIN 1492 字节，readelf 确认 RX 代码段。尚未将文件传至 Ubuntu VM/板卡，也未通过 U-Boot 执行或观察 LED；PZ5 与 LED1 的板级连接仍须按原理图确认，延时不保证准确。见 `records/sessions/2026-11-04-阶段4-LED1闪烁裸机工程生成.md`。

GPIO LED 闪烁例程准备（2026-11-04）：学习者已更新 `project/stage_3/261102/src/gpio.c`。当前 Init 的输出分支会设置 OSPEEDR、OTYPER 对应 pin 位、用 BSRR 预置输出电平，并把 MODER 切为输出；GPIO 时钟按约定由调用者开启，AF 暂缓。严格语法检查 `gcc -std=c11 -Wall -Wextra -Werror -fsyntax-only -I project/stage_3/261102/inc project/stage_3/261102/src/gpio.c` 通过，退出码0；尚无行为测试或硬件验证。实现闪烁例程前还需核对板级 LED1 网络/有效极性，准备 main/启动与构建入口，在 main 中置位 RCC AHB5 GPIOZ 时钟后配置 GPIOZ 的 LED 引脚为推挽输出，并提供周期延时（先用可解释的临时忙等验证流程，后续学习用定时器获得稳定节拍），循环中调用 Toggle；目前该目录只有 GPIO 源码/头文件，缺 main、延时、启动/链接及构建配置，不能单独生成可上板运行程序。详见 `records/sessions/2026-11-04-阶段4-LED1闪烁例程准备.md`。

GPIO 初始化函数首轮审阅（2026-11-04）：检查 `project/stage_3/261102/src/gpio.c` 与 `inc/gpio.h` 最新版本。严格语法检查 `gcc -std=c11 -Wall -Wextra -Werror -fsyntax-only -I project/stage_3/261102/inc project/stage_3/261102/src/gpio.c` 通过，退出码0、无输出；只证明当前源文件语法/所启用告警通过，不代表行为正确或硬件运行。Init 主要问题：输出分支没设置 MODER 为 GPIO_OUPUT_MODE，因此即使写了速度、类型、初始电平，引脚仍可能保持输入/模拟模式；OTYPER 先清目标位再 `|= GPIO_Init->Otype`，开漏枚举值1未左移到 `index`，会总是置 bit0 而非当前引脚位；GPIO_OUTPUT_PP 值0可借清位得到，但应按对应寄存器位域语义显式处理。Init 未开启 GPIOZ 时钟，尽管头文件定义 RCC_MP_AHB5ENSETR；若时钟由调用方负责须写为前置条件，否则初始化无效。未校验 Mode/Otype/Speed/pupd/State enum 范围，错误配置可能静默忽略或污染相邻字段；Output/Input 没统一设置/清除 PUPDR，Analog 也未定义 pull 状态；AF 分支为空，会静默返回成功般无操作。GPIO_Init->Pin 零值静默 return，函数返回 void，无失败反馈；建议定义清晰错误策略，并校验多 pin mask/允许端口范围。当前初始化代码没有对 Pin mask 超出 16 位问题（uint16_t 本身限制范围），但组合 pin 通过逐位循环是有意支持的。此前 Write 对负 enum 的校验仍不足，Read 错误/低电平歧义按用户明确要求暂不列为本次阻塞；Toggle/Write 并发所有权前置条件仍需在 API 中说明。源码只存在 gpio.c/h，未做行为测试、寄存器模拟或上板验证。详细意见见 2026-11-04 会话记录。

GPIO 初始化函数复查（2026-11-04，按用户说明时钟由调用者开启、AF 功能暂缓）：用户明确 Init 不负责开时钟，故将时钟排除为本轮缺陷；复用功能暂缓，本轮不要求 AFSEL 实现。最新源码严格语法检查仍通过，退出码0。新发现/仍存在：switch 前的 MODER 两条赋值在 C switch 跳转时被 case 标签绕过，所有 case 都不会执行这两条语句，故 Init 实际不更新 MODER；应把公共 MODER 更新移到 switch 前（switch 外），或放入每个已支持 case。配置范围上界用 >= 写错：State >=1 会拒绝合法 SET；Otype >=1 会拒绝合法 OD；pupd >=2 会拒绝合法 PULLDOWN；Speed >=3 会拒绝合法 VERY_HIGH_SPEED；Mode >=3 会拒绝 ANALOG（若本轮支持 analog）。边界应按各 enum 最大合法值比较，或逐值白名单验证。OTYPER 仍 `|= GPIO_Init->Otype`，没有把开漏 1 移到当前 pin 的 bit index；需按目标 pin 位写入。GPIO_Init 的 State 被要求用于 output，但现值校验会拒绝 SET，且非法负值也要防止。AF 暂缓应确保接口/注释明确“不支持”，不要让调用者误以为已完成 AF；当前 Mode>=3 拒绝 ANALOG，但允许 AF=2，AF case 仅 break，若暂缓就应作为明确 unsupported 处理。输出模式下 PUPDR 字段未配置，而 config struct 带 pupd；需决定该参数适用范围。Init 返回 void，错误配置静默退出，建议定义错误状态返回（可后续做）。无行为测试/硬件实测。详细见本会话记录。

GPIO 再次复查（2026-11-03，按要求暂不讨论 Read 错误值歧义）：Read 的 one-hot 与 IDR 掩码逻辑正确；Write 的 one-hot 表达式本轮已改正确；Toggle 的 BSRR set/reset 方向也已改正确。RCC 地址方面，学习者的判断正确：本地 FS-MP1A Linux 5.4.31 源码 `drivers/clk/clk-stm32mp1.c` 第 56 行定义 `RCC_AHB5ENSETR 0x210`，第 1691 行将 G_GPIOZ 映射到该寄存器 bit 0；RCC base 0x50000000，因此 `RCC_MP_AHB5ENSETR` 地址为 `0x50000210`，当前 gpio.h 地址正确。本人此前声称偏移 0xA30/绝对地址 0x50000A30 是错误的，现明确撤回。仍有与初始化函数无关的问题：Write 校验 `_val > GPIO_PIN_SET` 不能拒绝负枚举值，需仅接受 RESET/SET；Write 以 ODR 读改写更新 pin，并发修改同端口时可能丢更新，单上下文独占时可接受，否则用 BSRR 或同步；Read/Write/Toggle 均假设 MODER 已配置为各自用途，可将其写成 API 前置条件。严格语法检查退出 1 的唯一诊断是本轮排除的 `HAL_GPIO_Init(gpio_t *GPIOx, )` 非法定义；忽略 Init 后，当前三个函数没有其他 -Wall/-Wextra/-Werror 语法诊断。无行为测试或上板运行。本轮不讨论 Read 错误/低电平不可区分。

RCC AHB5 offset 更正（2026-11-03）：此前一轮错误将 `RCC_MP_AHB5ENSETR` offset 记为 `0xA30`。检查本地随板 Linux 内核源码 `01-FS-MP1A基础资料/02-出厂系统源码/linux/fsmp1x-linux-5.4.31.tar/fsmp1x-linux-5.4.31/drivers/clk/clk-stm32mp1.c`：第 55 行 `RCC_AHB4ENSETR 0xA28`，第 56 行 `RCC_AHB5ENSETR 0x210`，第 1691 行 `G_GPIOZ` 使用 `RCC_AHB5ENSETR` bit 0。结合 RCC base `0x50000000`，MP AHB5 enable-set 地址为 `0x50000210`。学习者说“就是 210”正确；撤回本人 0xA30/0x50000A30 的错误结论。本地驱动是直接交叉核验依据；RM0436 中也应以 `RCC_MP_AHB5ENSETR` 自身寄存器表的 `Address offset` 项查证，而不要用总线名称推断 offset。

GPIO 三函数再次复查（2026-11-03，按学习者要求暂不评 Read 错误/低电平歧义）：本次文件中 RCC/GPIOZ 地址正确，三函数头文件原型完整，Read 的 one-hot 检查及 IDR 掩码表达式正确；Toggle 已使用 BSRR，避免直接改 ODR。仍有阻塞问题：`HAL_GPIO_Init(gpio_t *GPIOx, )` 是非法函数定义，当前 `-Wall -Wextra -Werror -fsyntax-only` 编译失败；Write pin 校验 `(_pin|(_pin-1u) != 0U)` 因 `!=` 优先级高于 `|`，不等于期望的 one-hot 检查，且 `-Wparentheses` 触发 Werror；按目前条件，非零引脚还会被拒绝。one-hot 判断应按 `(_pin & (_pin - 1U)) != 0U` 的性质重新推导，并保留零值检查。Toggle 的 BSRR 方向反了：若 ODR 当前为高，应写 `pin << 16` reset；当前写低半部 set；若当前为低，应写 pin set，当前却写高半部 reset，因此会把电平保持原状而非翻转。Write 只检查 `_val > GPIO_PIN_SET`，负枚举值不会被拒绝；应显式接受 SET 或 RESET。Write 仍用 ODR 读改写，若要支持并发/中断/双核共享，宜用 BSRR 或明确独占约定。Read/Write/Toggle 均未检查 MODER 模式，可将初始化正确作为调用前置条件并写入契约，或实现验证。寄存器结构新增 BSRR 后按 7 个连续 32-bit 字段映射 offset 0x00..0x18 与目标寄存器布局一致（目标 ABI 下 unsigned int 为 32 bit）；尚未动态验证。语法检查失败、无行为测试/上板结果。下一步优先修三个明确阻塞项，再跑严格检查。

GPIO 三函数二次复查（2026-11-03）：再次检查 `project/stage_3/261102/src/gpio.c` 和 `inc/gpio.h`。两个地址已更正为 RCC_MP_AHB5ENSETR `0x50000210`、GPIOZ `0x54004000`；Write/Read/Toggle 头文件声明已补参数。Read 已修正掩码比较括号并拒绝多位 mask；Toggle 已改用 one-hot 校验和 `ODR ^= _pin`；首轮 Read/Toggle 运算符错误已解决。`gcc -std=c11 -Wall -Wextra -Werror -fsyntax-only -I project/stage_3/261102/inc project/stage_3/261102/src/gpio.c` 报告通过（退出码0、无输出）。额外 `-Wstrict-prototypes` 检查失败：`HAL_GPIO_Init()` 在头文件和定义仍使用空参数列表；应写 `(void)` 或定义明确参数。API 尚需设计：Read 无效输入返回 0 与有效低电平相同，调用方无法辨别错误；建议返回状态并用输出参数给电平，或设计明确的错误哨兵。Write 接受任意非零 16-bit mask（支持多 pin），Read/Toggle 要求 one-hot；需在契约中说明是否有意如此，或统一为单引脚。Write 将所有非 SET 值都当 RESET，须拒绝非法枚举。Toggle 为 ODR 读改写异或，在单所有者情况下可用；若中断/另一个核同时更新该 GPIO 端口，需明确共享所有权/同步策略。`HAL_GPIO_Init` 当前为空实现；本轮只验三个函数语法，未测试行为、构建完整程序或上板。源码逻辑核心已比前轮改善，但 API/输入错误语义尚待修订，不宣布函数验收完成。

GPIO 读写反转复查（2026-11-03）：再次只读检查 `project/stage_3/261102/src/gpio.c` 与 `inc/gpio.h`。相较首轮，MMIO 指针转换语法已修正，重复枚举名称已清理，输出类型枚举已拆出；仍未通过。RM0436 Rev 7 核实 GPIOZ base 为 0x54004000，头文件写为 0x540004000；RCC base 为 0x50000000，RCC_MP_AHB5ENSETR offset 为 0x210，绝对地址应为 0x50000210，当前宏写为 0x54000210。Read 的 `(IDR & _pin != 0U)` 因 `!=` 优先级高于 `&`，实际按 `IDR & (_pin != 0U)` 运算；需改为 `((IDR & _pin) != 0U)`。Read 的 NULL/零 pin 分支裸 `return` 与 uint8_t 返回类型不符。Toggle 仍写 `ODR | _pin == GPIO_PIN_SET`，仍有优先级错误且不是按掩码判断；需用 `(ODR & _pin) != 0U`，并传播 HAL_GPIO_Write 返回值。Write 仅拒绝 NULL/零 pin，还需定义单 pin 或多 pin mask 契约，校验 16 位合法 mask、状态枚举，并明确是否检查 MODER 输出模式。函数原型仍为空参数列表；HAL_GPIO_Init 只有声明。无构建或测试文件，未编译/运行。下一步学习者先修两个绝对地址与 prototypes，再修函数逻辑并提交源码；不代改。

AHB4/AHB5 地址区分（2026-11-03）：学习者询问 AHB4 是否为 0x5000210。澄清：AHB4 和 AHB5 是不同寄存器组；RCC_MP_AHB4ENSETR offset=0xA28，absolute address=0x50000A28；RCC_MP_AHB5ENSETR offset=0x210，absolute address=0x50000210。用户写的 0x5000210 少一个 0，且不是 AHB4 地址。GPIOZ 位于 AHB5，所以当前 GPIOZ/LED1 PZ5 应使用 AHB5 MP set 寄存器 GPIOZEN bit0，即 0x50000210。此前复查指出的 AHB5 正确地址为 0x50000210 保持不变；当前工程里错误地址是 0x54000210。依据 RM0436 GPIOZ memory-map table、AHB4 register offset，以及 Linux STM32MP1 clock driver 的 AHB4/AHB5 offsets。

RM0436 地址查找位置补充（2026-11-03）：回答学习者“在哪里看到 AHB5 地址 0x50000210”。手册分两步：RM0436 Rev 7 Table 9 Register boundary addresses（印刷页 164）列 RCC 地址范围 `0x50000000–0x50000FFF`，所以 RCC base 为 `0x50000000`；第 10 章 10.7.138 `RCC_MP_AHB5ENSETR`（印刷页 840）给 `Address offset: 0x210`。相加得到 `0x50000000 + 0x210 = 0x50000210`。直接搜索 `RCC_MP_AHB5ENSETR` 可到章节标题/目录，再到 10.7.138 的寄存器表确认 offset；搜索 `Register boundary addresses` 可确认 RCC base。来源为 ST 官方 RM0436 Rev 7。

RCC 所在总线澄清（2026-11-03）：学习者问“那说明 RCC 是挂在 AHB4 总线上的吗”。确认是：RM0436 Rev 7 Table 9 的 AHB4 地址表把 RCC `0x50000000–0x50000FFF` 列在 AHB4 项下，故 MPU 侧 RCC 寄存器接口映射在 AHB4 地址空间。与此同时，RCC 内部 `RCC_MP_AHB5ENSETR` 是控制 AHB5 外设时钟使能的寄存器；GPIOZ 位于 AHB5，所以经 AHB4 映射访问 RCC，再由它使能 AHB5 GPIOZ 时钟。区分“RCC 寄存器接口所在总线”与“RCC 控制的外设/时钟总线”。依据 ST RM0436 Rev 7 Table 9 与第 10 章 RCC 寄存器章节。

附加严格语法检查（2026-11-03）：对 gpio.c 执行 `gcc -std=c11 -Wall -Wextra -Werror -fsyntax-only -I project/stage_3/261102/inc project/stage_3/261102/src/gpio.c`，退出码 1。报告的硬错误包括：Write/Read/Toggle 空参数声明与带参数定义类型冲突；Read 的非值 return；Read `&`/`!=` 优先级告警升级为错误；Toggle `|`/`==` 优先级告警升级为错误。该命令仅语法检查，不生成可执行文件、不运行目标硬件。宏未展开，故不验证 MMIO 地址。当前修订仍不能严格编译。

阶段 4 GPIO 源码首轮审阅（2026-11-03）：学习者明确授权本轮检查 `project/stage_3/261102`，实际工程位于 `project/stage_3/261102`，仅只读查看 `src/gpio.c` 和 `inc/gpio.h`。发现头文件有编译阻塞：MMIO 宏指针转换语法错误，GPIOZ 地址疑似多一个 0（需对照 RM0436 核准，不能直接套写），两个 enum 重复定义普通标识符，导致编译错误；函数声明使用空参数列表且与实现接口未声明具体参数。函数问题：Write/Read 没做 GPIOx NULL 和 pin mask 校验；MODER 是每引脚 2 位字段，不能将整个寄存器与模式 enum 比较；Read 应以 IDR & pin 掩码判断，现有 `|` 会把无关位混入，且非输入路径缺返回；Toggle 条件受运算符优先级影响（`|` 与 `==`），且应测试掩码位，else 分支有 `AL_GPIO_Write` 拼写错误并仍传 RESET；未传播 Write 失败状态。当前没有 main/Makefile/test 文件，未执行编译或运行。本轮仅源码审阅未通过；下一步学习者先修正头文件接口与 MMIO 定义，再逐函数修复并贴片段；不代改项目。

GPIO 配置理解反馈（2026-11-02）：学习者指出 13.3.10 不够清楚，并自行总结需配置 MODER 输出模式、OTYPER 推挽、OSPEEDR、PUPDR、输出数据寄存器及开启时钟。该寄存器功能清单和时钟前提方向正确，当前不再重复要求只读输出配置小节；教师补充须将 GPIO 章与 RCC 章结合，时钟使能在 RCC 中查对应端口。建议流程：核对 LED 连接/有效电平，开启相应端口时钟，预设输出数据，配置输出类型/速度/上下拉，最后将目标 MODER 字段切为输出，以减小不期望的瞬态；用 BSRR 置位/复位可避免修改输出数据时影响其他引脚。OSPEEDR 是输出边沿速度配置，不是闪烁频率；实际 RCC 位、地址与配置值待核对目标端口，不预设或代写。仅概念反馈，无上板/代码验收。

PZ5 位域回答（2026-11-02）：学习者给出 `RCC_MP_AHB5ENSETR` bit0、MODER[11:10]、OTYPER[5]、OSPEEDR[11:10]、PUPDR[11:10]、ODR[5]。按 RM0436 Rev 7 核对，端口时钟 GPIOZEN bit0 及所有字段位置正确。继续澄清字段值：MODER[11:10]=01 选择输出；OTYPER[5]=0 推挽；OSPEEDR 可从 00 低速开始（是边沿速度档，不代表输出频率）；PUPDR[11:10]=00 不使能内部上下拉，是否需外部/内部偏置按电路决定；ODR[5] 写 1/0 控制高/低。RCC_MP_AHB5ENSETR 是 A7/MP 侧 set 寄存器，GPIOZ 的 MCU 侧对应 RCC_MC_AHB5ENSETR，不要跨核混用。GPIO 设置应保留同寄存器其他引脚位；BSRR bit5 置位、bit21 复位只改 PZ5。依据 ST RM0436 Rev 7。该次是手册位域推导，没有原理图页码或代码/实测，板级 LED1=PZ5 和有效极性仍待原理图确认。

PZ5/LED1 位域定位（2026-11-02）：学习者按 FS-MP1A LED1=PZ5 推导出 GPIOZ 的 MODER[11:10]、OTYPER[5]、OSPEEDR[11:10]、PUPDR[11:10]、ODR[5]，字段编号正确；STM32MP157 RM0436 中 GPIOZ 位于 AHB5，RCC_MP_AHB5ENSETR 的 bit 0 名为 GPIOZEN，写 1 置位使能，故 A7/MP 侧这一位正确。MODER[11:10] 设 01 为输出，OTYPER[5] 清零为推挽；OSPEEDR 与 PUPDR 的具体取值按边沿/板级需求决定。配置 GPIO 字段需先清目标字段再写新值，保留其他引脚配置；RCC *_ENSETR 是 write-one-to-set 语义，写入掩码即可置位，不应把“低 0 位 1”理解为配置整组 GPIO 寄存器。ODR[5] 控制输出数据；BSRR bit5 置位、bit21 复位可单独更新 PZ5。板卡 LED1=PZ5 和有效电平以 FS-MP1A 原理图为准；学习者尚未提交对应页码，本轮不验收板级连接。仅手册推导，无代码/上板实测。依据 ST RM0436 Rev 7、FS-MP1A 原理图主题资料。

GPIO 配置章节定位（2026-11-02）：学习者询问手册哪里说明配置 GPIO。已核对 ST 官方 RM0436 Rev 7 目录：第 13 章 General-purpose I/Os (GPIO) 从正文页 1067 开始；13.3.9 Input configuration 在 1072，13.3.10 Output configuration 在 1073，13.4 GPIO registers 从 1078 开始。13.4.1 MODER/13.4.2 OTYPER 在 1078，13.4.3 OSPEEDR/13.4.4 PUPDR 在 1079，13.4.6 ODR/13.4.7 BSRR 在 1080。先建议看输出配置与模式寄存器，理解功能说明与位域说明的分工；不同修订版页码可能变，优先按章节标题查找。此次仅资料定位，不记录为已理解寄存器或已点灯。官方来源为 ST RM0436 PDF。

阶段切换意图纠正及资料说明（2026-11-02）：此前教师将“先学习阶段 4，自己阅读相关内容后再回来阶段 3”误记为独立通读阶段 4。当前安排修正为教师指导阶段 4，同时学习者自行阅读阶段 3 系统编程书籍，之后恢复阶段 3.2。学习者询问是否阅读 MP157 数据手册；说明阶段 4 按主题配合不同资料，数据手册用于型号/引脚/复用/电气参数，STM32MP157 参考手册 RM0436 用于外设地址、寄存器及配置规则，板卡原理图用于实际连接。按当前任务查相关章节，不要求先通读整本手册。资料用途已讲解，尚无新概念答题、代码或实测，阶段 4 未验收。

阶段切换记录（2026-11-02）：学习者决定先自行阅读阶段 4 相关内容，暂缓阶段 3.2 的 `sigsuspend` 与管道信号整合，阅读后再返回阶段 3。阶段 3.2 已完成文件 I/O、fork/waitpid、僵尸进程、SIGTERM、sigaction 基础片段、匿名管道单字节通信和 EOF 观察；未完成可靠信号等待与 READY 握手，保留为恢复入口。阶段 4 的任务范围为 ARM 寄存器/工作模式/异常向量/中断、存储器映射与 MMU/Cache 基础、启动汇编/栈/链接脚本/map/反汇编，以及 GPIO/UART/定时器/中断寄存器级工作；任务 4.1 从原理图和参考手册完成寄存器级 GPIO 点灯，4.2 完成 UART 收发和定时器节拍，4.3 完成按键中断并分析 ELF、map 与启动到 main 的路径。当前不把阅读视为阶段 4 通过，不新增实测或代码验收；恢复阶段 3 时从现有记录继续。

匿名管道 EOF 运行反馈（2026-11-01）：学习者报告严格编译命令成功，运行依次输出 Hello world 的 11 个字符，随后父进程报告 `child exited normally, code = 0`。相较首轮 code=1，符合子进程将 read 返回 0 作为正常循环结束并正常退出的预期；EOF 未单独打印，但本轮按用户此前要求仅检查修正后的构建和一次运行，不追加测试。未提交修正版源码，因此只记录运行行为，循环 break/读端 close 的最终代码位置未复核。该运行反馈通过。

下一步进入信号与管道的可靠整合：解释 `while (!stop_requested) pause()` 存在检查标志与进入 pause 之间的竞态，接着学习 `sigprocmask` 与 `sigsuspend` 的原子解阻塞等待；再用管道 READY 字节通知父进程“子进程已经屏蔽 SIGTERM 并准备好等待”，父进程收到 READY 后发送 SIGTERM，子进程处理后在正常流程清理/退出，父进程 waitpid 验证。尚未布置或实现该整合实验。

学习者提出暂停新练习并阅读系统编程书籍巩固进程、信号、管道。推荐 Michael Kerrisk《Linux/UNIX系统编程手册》作为主读物，以及《UNIX环境高级编程（第3版）》作为备选/对照；今日学习暂停，阅读后从当前任务 3.2 的信号等待竞态继续。

匿名管道循环读取首轮反馈（2026-11-01）：学习者报告严格编译通过；运行中依次读出 `Hello world` 的 11 个字符，父进程关闭写端后子进程输出 `EOF, no data` 并以退出码 1 结束。实际显示 read 到 EOF 的时序，但源码把 n == 0 与错误/无数据失败处理合并为 return 1；EOF 是缓冲数据耗尽且所有写端关闭后的正常读完标记，并非本例数据丢失。子进程在每轮读到数据时正确用 `%c`，但循环后关闭读端的代码不可达，因为循环从不 break。要求 read 返回 0 时跳出循环，随后关闭读端、报告正常 EOF 并返回 0；n < 0 仍为 read 错误并返回失败。当前运行确实证明 11 字节可分次读出和写端关闭触发 EOF，但 EOF 正常处理尚未通过。修正后仅需再严格编译运行一次。

匿名管道单字节练习通过（2026-11-01）：学习者提交修正版完整源码，使用 `%c` 输出单个字节，分别处理 read 的错误/EOF/读到 1 字节，检查 write 是否恰好写入 1 字节，正常路径关闭各自不用及用完的管道端。父进程 waitpid 后检查 WIFEXITED 与退出码，非零退出或信号终止返回失败；此前漏报子进程失败的问题已修复。学习者报告 gcc -std=c11 -Wall -Wextra -Werror pipe_demo.c -o pipe_demo 编译通过，运行输出 `child received: A`、`child exited normally, code = 0`。本轮单字节正常路径源码审阅和运行反馈通过；未单独提供 Shell 退出码，fork/系统调用罕见错误分支未实测。首轮 `%s` 越界问题仅留作历史记录，不是当前状态。下一步让子进程循环 read 到 EOF，父进程写完后关闭写端，观察 EOF 发生条件；不重复单字节测试。

匿名管道修正版源码审阅（2026-11-01）：学习者修正为 `%c` 打印单字节，检查 read 的 -1/0/1 结果，关闭子读端/父写端，并检查父 write 的 -1 和非 1 返回，解决了首轮越界读取和忽略 I/O 返回值问题。正常数据路径逻辑基本正确。剩余问题：父进程 `WIFEXITED(status)` 后无论 `WEXITSTATUS(status)` 是否为 0，main 最终均返回 0；因此子进程读取失败并 return 1 时，工具整体仍报告成功。请父分支在子进程非零退出或异常终止时返回非零，并只在子进程正常退出码 0 时视作成功。fork 失败和少数早退路径未显式关闭仍持有的管道描述符，但进程退出时内核会回收；建议用统一清理路径完成，不要求故意触发这些罕见失败。当前本次源码未附编译/运行输出，不能将修正版运行验收通过。提交状态传播改动及修正版严格编译/运行反馈即可，不重复增加测试矩阵。

匿名管道单字节实验首轮反馈（2026-11-01）：学习者报告严格命令 `gcc -std=c11 -Wall -Wextra -Werror pipe_demo.c -o pipe_demo` 编译成功，运行输出 `child received: A` 和 `child exited normally, code = 0`，并提交完整程序。pipe 在 fork 前创建、父子选择不同读写方向、父写后 waitpid 的主链路清楚。但源码不能验收通过：子进程 `char buf[1]` 后用 `%s`，没有 NUL 终止符，printf 会越过数组边界读取；输出 A 只是偶然结果，不能证明安全。应检查 read 恰好返回 1，使用 `%c` 打印 `buf[0]`；read=0 是意外 EOF，read=-1 是错误。父进程忽略 write 返回值，应确认恰写 1 字节；两进程均未关闭自己使用的剩余管道端，fork 失败也泄漏两个端点，错误路径存在资源泄漏。需在所有路径合理关闭已持有端点，再检查子进程 wait 状态和退出码。程序已严格编译/报告运行，但该行为有未定义行为，故运行结果不作为正确性验收。只要求修改相关片段并提交，不重写完整程序；短写罕见分支不要求人为制造。参考 Linux man-pages read(2)、write(2)、pipe(7)。

sigaction 与正常退出运行反馈（2026-11-01）：学习者运行 ./pid_demo，输出父 PID 19437/PPID 6230、子 PID 19438/PPID 19437、`stop_requested = 1`、`child exited normally, code = 0`。符合本轮注册处理函数、子进程自发 SIGTERM、正常流程检查标志并返回 0 的预期，当前运行环节通过。未提交最终子分支、完整源码或重新编译命令，前轮注册写法及返回值检查的源码修正尚未直接核验，不据运行输出推断所有错误路径通过。下一步在 Ubuntu 新建 pipe_demo.c，先做父进程写一个字节 A、子进程读一个字节的匿名管道实验：fork 前 pipe，pipefd[0] 读、pipefd[1] 写；父子关闭各自不使用的端点，检查 read/write 与 close，父进程发送后关闭写端再 waitpid，子进程读到恰好一字节后打印、关闭读端并正常退出。仅给接口和契约，不提供完整程序，不代编译运行。管道掌握后用于子进程向父进程发注册就绪通知；可靠异步等待和受控退出整合仍待完成，任务 3.2 尚未整体结项。参考 Linux man-pages pipe(2)、pipe(7)。

sigaction 注册首轮片段审阅（2026-11-01）：学习者提交子分支，写为 `sa_handler = handle_sigterm`、`sa.sa_mask = 0`，且 sigaction/raise 未检查返回值，raise 后保留无限 pause 循环。当前片段未通过：handler 应赋给 `sa.sa_handler`；sa_mask 为 sigset_t，用 sigemptyset(&sa.sa_mask) 初始化为空集合，不直接赋整数 0；sigaction 成功返回 0、失败 -1，raise 失败为非零。正常未屏蔽 SIGTERM 的本次单线程练习中，raise 调用 handler 并在 handler 返回后才返回，因此后续无限 pause 不检查停止标志，会继续等待。讲解局部注册示例与三个参数，要求在 raise 成功后检查 stop_requested，在正常流程打印并从子分支所在 main 返回 0；父分支此轮移除 kill，仅 waitpid，预期正常退出码 0。仅审阅片段，未编译或运行。参考 Linux man-pages sigaction(2)、sigsetops(3)、raise(3)、pause(2)、wait(2)。

下一步实验边界（2026-11-01）：先在 fork 后的子分支配置并检查 sigaction，之后由子进程自行 raise(SIGTERM)，处理函数设置停止标志，正常流程检查标志后返回退出码 0；父进程用 waitpid 验证 WIFEXITED/退出码。该顺序隔离 handler 注册与父进程 kill 的调度竞态。父进程发送信号的反馈已收到，但其 kill 源码未提交，另行记录为用户报告运行符合预期。

父进程发送信号运行反馈（2026-11-01）：学习者运行 ./pid_demo，输出父 PID 18978/PPID 6230，以及 `child killed by signal 15`，没有子分支打印。这与父进程按实验要求发送 SIGTERM、子进程可能在打印前被终止的预期一致；本轮未贴修改源码、kill 返回值及编译命令，不将输出扩大为具体 kill 调用与错误分支源码核验。不重复正常路径实验。下一步先学习信号处理函数：处理函数只设置文件作用域的 static volatile sig_atomic_t 停止标志，正常执行流程再检查标志、清理并正常退出；先独立写 handle_sigterm(int signo)，收到 SIGTERM 时标志置 1，其他输入不改，不在处理函数中 printf/free。sigaction 注册与可靠等待尚未讲解，不直接拼接 while(!flag) pause()，避免检查与挂起之间的竞态。资料参考 GNU C Library 信号处理/原子类型和 Linux man-pages signal(7)、signal-safety(7)。

信号处理函数片段审阅（2026-11-01）：学习者提交 `handle_sigterm(int signo)`，仅在 signo == SIGTERM 时把 `stop_requested` 设为 1；逻辑符合处理函数契约。该片段依赖 `stop_requested` 已在文件作用域声明为 `static volatile sig_atomic_t`，且包含 `<signal.h>`；本次未提供声明上下文，也未编译或运行，不记录为构建验证通过。下一步学习 `sigaction()` 注册处理函数，并设计无丢失唤醒的信号等待/受控退出流程。

处理函数单独源码片段通过；接下来先教 `sigaction` 安装，利用子进程自身 `raise(SIGTERM)` 验证 handler 标记与正常控制流退出，避免此步引入父子信号竞态。预期与默认动作的 wait 状态不同：handler 返回后子进程正常退出，父进程应观察 `WIFEXITED` 和指定退出码。父进程经 kill 触发 handler 将放在学习同步/管道握手之后，避免用 sleep 猜测子进程已完成 handler 注册。

信号终止状态运行反馈（2026-11-01）：学习者运行 ./pid_demo，输出父 PID 18919、子 PID 18920/PPID 18919，父进程报告 `child killed by signal 15`。说明已有状态判断程序运行到信号终止分支并报告编号 15，与本轮 SIGTERM 默认终止实验预期一致。未提交修改版源码、编译命令或退出码，不据输出声称 raise/fflush 的错误处理已核验。下一步学习 kill(pid, SIGTERM)：与 raise 给自身发送信号区别，正 PID 指定目标子进程，返回 0 表示发送成功而不等于已完成退出；仍须 waitpid 确认终止并回收。布置现有程序的最小改动：子进程移除主动 raise，在 for(;;) pause() 中等待；父进程检查 kill 返回值后 waitpid。子进程可能在执行打印前就被终止，不要求固定输出顺序，不以 sleep 保证同步。该新分支尚待实现及实测。

僵尸进程观察实测通过（2026-11-01）：学习者提交两次 `ps -o pid,ppid,state,cmd -p 18723` 输出。第一次显示子 PID 18723、父 PID 18722、状态 Z、命令 `[pid_demo] <defunct>`；第二次仅有表头，查询时已无该 PID 的记录。符合本轮先延迟回收、随后 waitpid 的实验预期；第二次 ps 本身只证明记录消失，本轮未提交延迟版源码或 waitpid 返回值，不扩大为对具体回收调用的直接验证。该观察环节通过，无需重复。下一步在现有单线程 pid_demo 中学习信号及 raise(SIGTERM)，让父进程的 WIFSIGNALED/WTERMSIG 分支读取终止信号；当前信号分支尚未实测。

ps 命令占位符澄清（2026-11-01）：学习者直接输入 `ps -o pid,ppid,state,cmd -p <子进程PID>`，Bash 报 newline 附近语法错误。已说明尖括号内文本仅为文档占位符，执行时须用本次运行实际打印的数字 PID 替换整个 `<子进程PID>`，不保留尖括号；Shell 将 `<` 和 `>` 解释为重定向符号，因此此次错误发生在 Shell 解析阶段，尚未执行 ps。示例数字仅用于说明，不能沿用前一次实验的旧 PID；应在父进程 sleep、尚未 waitpid 的窗口查询，回收后再查询。僵尸观察实验仍待实际输出，不记为实测通过。

waitpid 正常退出实验反馈（2026-11-01）：学习者提交加入 sys/wait.h 的 fork/waitpid 程序；fork 和 waitpid 错误均检查，子进程 printf 后 _exit(7)，父进程用 WIFEXITED/WEXITSTATUS 与 WIFSIGNALED/WTERMSIG 解析，结构合理。运行输出父进程 PID 17318、子进程 PID 17319/PPID 17318，父进程报告正常退出码 7，waitpid 回收路径正常。此轮未贴重新编译命令，故只记录运行输出和源码审阅，不声称该修改版刚刚严格编译；之前首版 fork 程序的严格编译已通过。提示 _exit 不刷新 stdio 缓冲，输出重定向时 printf 文本可能丢失；要保留它需在 _exit 前显式 fflush(stdout)，或在此单线程示例中使用正常退出方式。下一步学习 waitpid 的回收效果和僵尸进程概念，然后再进入信号终止分支。

僵尸进程概念检查通过（2026-11-01）：学习者正确回答子进程退出后不会继续运行。补充：若父进程尚未 wait/waitpid，子进程会以 zombie 状态暂留进程表，保留退出状态供父进程读取；waitpid 完成回收后，该僵尸记录消失。下一步可用短暂 sleep 加 ps 观察 zombie，再进入信号终止状态。

僵尸进程观察实验已布置（2026-11-01）：子进程退出后，父进程先 sleep(10) 暂不 waitpid；用户需在另一终端用 `ps -o pid,ppid,state,cmd -p <子进程PID>` 观察 `Z`/`defunct`，父进程随后 waitpid 回收，再次查询确认记录消失。打印 PID 后应适当 fflush(stdout)。完成后进入信号终止与 WIFSIGNALED/WTERMSIG。

waitpid 入口讲解（2026-11-01）：已解释 waitpid(child_pid, &status, 0) 的三个参数、阻塞等待语义、返回值，以及用 WIFEXITED/WEXITSTATUS/WIFSIGNALED/WTERMSIG 读取状态。布置学习者把 waitpid 加入 pid_demo：父进程等待刚创建的子进程，子进程返回 7，父进程区分正常退出与信号终止并处理 waitpid 错误。尚无修改源码或运行证据。资料依据 Linux man-pages wait(2)/waitpid(2)。

PPID 关系澄清（2026-11-01）：学习者询问 6230 是否为父进程的父进程。依据已提交输出，16595 的 PPID 为 6230，16596 的 PPID 为 16595，因此 6230 是 16595 的父进程、16596 的祖父进程。尚无进程名称信息，不断定 6230 一定是 Shell；未增加重复运行要求。

fork 最小实验审阅与运行通过（2026-11-01）：学习者主动提交 pid_demo.c，在 fork 失败时 perror 并返回 1，子进程按返回 0 分支打印 PID/PPID，父进程按正数分支打印。报告 gcc -std=c11 -Wall -Wextra -Werror 编译成功；运行显示父 PID=16595、PPID=6230，子 PID=16596、PPID=16595，父子关系正确。输出行有交错，不以此推断固定调度顺序。讲解同一次成功 fork 在父进程返回子 PID、子进程返回 0，两者从调用返回处继续而不是重新执行 main；本次父进程变量 pid 应为 16596，子进程变量 pid 为 0（按接口推导，未实际打印该变量）。当前代码未调用 wait/waitpid；下一步学习等待子进程结束和回收退出状态，不把创建成功等同于任务 3.2 整体通过。参考 Linux man-pages 的 fork(2)、wait(2)。

任务 3.2 开始（2026-11-01）：学习者同意继续。先讲程序是可执行文件、进程是程序的一次运行实例，PID 用于区分同时运行的进程。通过同一程序同时运行两份、各自普通 static 变量是否共享的场景连接阶段 1 的存储期知识；当前仅讲解并布置概念题，尚未收到回答，也没有进程实验源码/输出。随后再介绍 getpid/getppid 与最小进程创建，保持一步一步、独立编码的节奏。详细记录见 sessions/2026-11-01-阶段3.2-进程信号与管道.md。

进程地址空间概念检查通过（2026-11-01）：学习者正确说明两个独立启动的进程各有自己的用户空间，一个进程中的普通 static 变量不会自动影响另一个。教师补充 fork 后逻辑地址空间初始内容复制、写时复制实现细节；这属于讲解，尚未通过 fork 实验验证。下一步独立编写 getpid/getppid 的最小观察程序，再学习进程创建。

任务 3.1 本轮验收结论（2026-11-01）：通过，86/100。用户报告错误清理修改后的最终严格编译无问题；结合此前完整 CLI 的 copy/cmp 一致、scan 类型输出、同文件拒绝和参数不足退出码 1 的运行证据，以及分块读取/短写循环、DT_UNKNOWN 回退和资源清理片段审阅，达到本轮基础目标。评分：原理理解17/20、功能实现24/25、调试与测量17/20、异常与边界12/15、工程质量8/10、记录与复现8/10。扣分及后续完善项集中在格式参数和资源清理曾需多轮纠正、部分罕见运行分支未覆盖及完整版本/commit 未归档。不追加重复的明显测试，不要求为关闭失败制造环境。证据限定：最后编译为用户报告，最终完整源码与 status 初始化声明未再次提交；初始化沿用此前各函数局部 status=0、扫描循环外声明的要求，不记录为已单独核验。真实短写重试、DT_UNKNOWN 回退、关闭失败未实测；不等同于阶段 3 全部完成。下一步按小步教学进入 3.2，先讲进程/PID，再做最小创建进程实验，不直接提供完整实现。

错误清理修正版审阅（2026-11-01）：学习者将 copy_file 的两个 close 失败分支改为设置 status=1，两个关闭调用均会执行，最后返回 status；scan_directory 的 fstatat 失败分支记录 status=1 并继续，循环后执行 closedir，关闭失败返回 1，否则返回 status。提交片段的控制流和错误保留逻辑审阅通过。片段未展示 status 声明，提醒两个函数分别将其初始化为 0，扫描函数需放在循环外。当前待确认初始化与最终完整文件严格编译，不重复既有正常路径测试，不声称实际触发关闭失败或 DT_UNKNOWN。

错误清理片段审阅（2026-11-01）：学习者分别检查两个 close，但第一个失败后立即 return 1 会跳过第二个；scan_directory 的 fstatat 失败分支新增 return 1，会跳过原有循环后 closedir，泄漏目录流。当前片段未通过。继续练习累计状态：copy_file 在关闭阶段以初始为 0 的局部状态记录任一失败，两个 close 都尝试后返回状态；scan_directory 在循环外保存状态，条目查询失败设为 1，完成扫描并关闭目录后返回状态。只审阅相关片段，不代写核心实现、不要求人为制造关闭失败。

最新实测（2026-11-01）：学习者报告 filetool.c 使用 gcc -std=c11 -Wall -Wextra -Werror 编译成功；./filetool copy filetool.c copy.txt 正常结束，cmp filetool.c copy.txt 无差异输出；./filetool scan . 正确显示 file/directory 与条目名称。整合 CLI 的复制与扫描正常路径联调通过。当前未提供最终完整源码或退出码，不把构建成功等同于已确认格式参数及 close/fstatat 失败状态处理。剩余源码收尾：copy_file 两个 close 独立检查且任一失败最终返回 1；scan_directory 若条目查询失败，可继续扫描，但应累计失败状态并最终返回 1。只需提交变更片段，不重复明显错误路径运行；任务 3.1 尚未整体结项。

最新审阅（2026-11-01）：学习者提交 main 片段；先检查 argc < 2，再通过 strcmp 识别 copy/scan，分别要求 argc=4/3，向功能函数传入正确路径，并直接返回其状态。未知命令和参数错误返回 1，main 源码审阅通过，无需重写。整合程序尚无编译运行证据；前轮 copy_file 的 %s 参数须使用 source/destination，成功路径还应检查两个 close 的结果且都尝试关闭。整合时包含 string.h；main 之前须能看到两个功能函数的声明或定义。下一步在 Linux 编译完整 filetool.c，通过 copy 后 cmp 一致及 scan 输出验证分派，不重复要求明显的参数错误测试。

CLI 参数映射问答通过（2026-10-31）：学习者正确回答 copy 命令 argc=4、argv[1]=copy；scan 命令 argc=3、argv[1]=scan，并理解 argv[1] 为子命令。下一步独立提取 copy_file(source, destination) 与 scan_directory(path)，由 main 校验参数、用 strcmp 分派并传递功能函数返回值；暂在单个 filetool.c 内整合，不加入新功能或递归。

copy_file 函数首轮审阅（2026-11-01）：学习者提取了复制逻辑，打开顺序和“检查同一文件后再截断”也保留正确。待修：同文件提示把整数文件描述符传给 %s，这是格式不匹配；严格 -Werror 编译会失败，应打印 source/destination 路径或使用匹配格式。成功路径未关闭两个文件描述符，导致资源泄漏；错误路径也应确保已打开 fd 关闭。当前只贴 copy_file，尚未提交 scan_directory/main 或整合文件，未编译验收。

函数提取第二轮审阅（2026-11-01）：学习者增加了 copy_file 成功路径的 close，并提交 scan_directory；扫描主体的目录读取、类型分类和 DT_UNKNOWN 回退总体保持正确。same-file fprintf 改为传 &fd_source/&fd_dst，仍然错误：两个参数类型是 int*，%s 需要 char* 字符串；应传入原路径 source/destination。copy_file 末尾 close 返回值尚未检查，属于资源错误报告改进；scan_directory 中 fstatat 失败会 perror 并标 stat-error，但仍继续扫描且最终可能返回 0，需明确是尽力扫描并标注还是将此视为整体失败，与函数注释契约一致。尚未提交 main/完整 filetool.c，未编译运行。

CLI 错误状态问答（2026-10-31）：学习者判断未知子命令或参数数量错误应返回 1，判断正确；已澄清由 C 程序 return 1 设置退出状态，Shell 的 echo $? 只读取上一条命令的退出状态。复制/扫描命令格式及 argc 映射尚待学习者设计。
> 阶段状态：阶段 1 已结项；阶段 2 纸面部分完成，实测待办保留；阶段 3 进行中
> 最近得分：任务 3.1 86/100（通过）；阶段 1 闭卷测验 90/100、阶段作品 86/100

目录项类型实现首轮通过（2026-10-31）：学习者修正 DT_REG 分支并提交独立 switch，所有分支均设置类型字符串后统一输出。严格编译 gcc -std=c11 -Wall -Wextra -Werror dir_demo.c -o dir_demo 成功，运行 ./dir_demo . 显示 file、directory 分类及名称。证明当前 Linux 文件系统提供的 d_type 在这些条目上可用；DT_UNKNOWN 后备查询尚未实现，且 d_type 可移植性有限，后续再决定是否补 fstatat。

未知目录项类型回退学习（2026-10-31）：开始学习 fstatat(dirfd(dir), name, &st, AT_SYMLINK_NOFOLLOW)，用于 d_type == DT_UNKNOWN 时查询实际类型；通过 S_ISREG/S_ISDIR/S_ISLNK 检查 st_mode。AT_SYMLINK_NOFOLLOW 用于把符号链接本身分类为 symlink，而非跟随到目标。当前仅讲解接口及布置独立修改，尚无源码或实测证据。

复制工具同文件保护入口（2026-10-31）：目录项生命周期已解释；开始补查复制工具的重要数据安全边界。源与目标即使路径字符串不同，也可能通过硬链接指向同一 inode；若以 O_TRUNC 打开目标，会在复制前清空源。下一步用 fstat 获取两个已打开文件的 st_dev/st_ino，在截断目标前拒绝相同文件；当前先让学习者说明正确打开/检查/截断顺序，再独立修改。

同文件保护源码审阅（2026-10-31）：学习者实现了不带 O_TRUNC 打开目标、分别 fstat、比较 st_dev/st_ino、相同则拒绝、否则 ftruncate 后复制。顺序和核心判断正确，能够覆盖相同路径、符号链接/硬链接指向同一对象的常见情况。源码缺少 <sys/stat.h>，且严格 C11 下 POSIX 声明可能需要在所有头文件前定义 _POSIX_C_SOURCE 200809L；补齐后做严格编译。建议实测同路径拒绝并用 cmp/文件大小确认源未被清空，再做一次正常复制。退出路径目前未检查 close 返回值，列为后续工程完善，不阻塞本轮核心安全行为审阅。

同文件保护运行反馈（2026-10-31）：学习者报告严格 C11/Wextra/Werror 编译成功。第一次输入 fd_demo 未加 ./，Shell 未找到命令；随后 ./fd_demo fd_demo.c fd_demo.c 输出 same-file 拒绝提示，确认相同路径安全分支在运行时触发。下一步用不同目标路径运行复制并 cmp，回归检查非相同文件分支。未单独记录退出码，不声称已检查 shell 状态。

复制保护回归实测（2026-10-31）：学习者运行 ./fd_demo fd_demo.c copy.txt，随后 cmp fd_demo.c copy.txt 无差异输出，确认同文件保护加入后不同文件复制路径仍正确。结合上一轮相同路径拒绝提示，核心复制行为的正常与保护路径均有实测证据。接下来整合独立的复制与目录扫描程序为一个命令行工具；先由学习者设计命令语法、参数数量及 main 的分派逻辑，再开始实现，不直接提供完整代码。

fstatat 回退首轮源码审阅（2026-10-31）：学习者在 switch 的 default 分支调用 fstatat 并按 st_mode 分类，查询成功路径方向正确。但 C11 中 case/default 标签后不能直接声明变量，需用复合语句块包围局部声明；fstatat 失败路径未设置 type，公共 printf 会读取未初始化指针。另按原要求只对 DT_UNKNOWN 调查，其余未识别 d_type 应明确标为 other/unknown。待修正后严格编译；无真实 DT_UNKNOWN 环境时注明未覆盖该运行时分支。

fstatat 回退第二轮审阅（2026-10-31）：学习者已把 struct stat 声明移到循环外，DT_UNKNOWN 单独调用 fstatat，成功时按 S_IS* 分类，失败时也为 type 赋值，其他 d_type 归 unknown。未初始化读取和调用范围问题已修复。剩余语义改进：fstatat 失败与实际无法进一步归类的其他文件类型都显示 unknown，建议区分为 stat-error 并报告 errno，使输出保留诊断信息。源码基本结构通过，待最后错误标签修正及 Linux 严格编译/常规扫描运行；本环境尚未实际触发 DT_UNKNOWN。

目录扫描未知类型回退源码审阅完成（2026-10-31）：学习者把 fstatat 失败分支修正为先 perror("fstatat")、再设置 type = "stat-error"；default 为 unknown。分支语义审阅通过，诊断发生在下一轮 errno 清零之前。此前严格编译和常见目录类型扫描通过；本次仅提交变更片段，未提供加上 perror 后的重新编译输出，且未触发 DT_UNKNOWN/查询失败，不据此声称这些运行时路径实测通过。

目录扫描分类编译运行反馈（2026-10-31）：学习者报告修改后以 -std=c11 -Wall -Wextra -Werror 编译成功，运行当前目录正确显示普通文件与目录。未提供修改后的 fstatat 失败分支源码，且本次输出未触发 DT_UNKNOWN；因此确认常见类型路径和构建通过，但 stat-error 标签区分及未知类型回退分支仍无源码/运行证据。

fstatat 错误标签定位（2026-10-31）：学习者随后提交完整源码，发现其把 stat-error 放入 switch default；default 表示其他 d_type 值，不代表 fstatat 失败。DT_UNKNOWN 分支的 fstatat 失败仍赋值 unknown。要求把 stat-error 和 perror("fstatat") 放入失败 else，default 恢复为 other/unknown。当前待修正并重新严格编译；此问题与常规目录运行成功不冲突，但错误标签语义未通过。

fstatat 错误标签二次修正（2026-10-31）：学习者已将 stat-error 放入 DT_UNKNOWN 的 fstatat 失败 else，并将 default 设回 unknown，分支语义现正确。当前失败分支尚未调用 perror("fstatat")，无法输出 errno 对应原因；建议补上后严格编译。常规路径此前已编译运行，DT_UNKNOWN/查询失败仍无实际触发证据。

## 当前唯一目标

最新进展（2026-10-31）：阶段二综合复盘中，学习者指出“远端 TX 解码正常、MCU RX 帧错误”的原题若不同时测量 TX 与 RX，证据链不完整；并正确提出同步测量两端、分别使用对应参考地，比较电平、位宽、边沿、振铃和毛刺，以区分信号线耦合、地回流影响和测量伪影。该批判和修正版诊断方案通过，阶段 2 纸面学习完成。原路线 2.1、2.3 的真实搭建/仪器实测仍延期，不把纸面通过等同于阶段全部实测验收。现在进入阶段 3 任务 3.1：学习 Linux 文件描述符、open/read/write/close、短读写和错误返回，随后实现文件复制与目录扫描工具。

阶段 3.1 文件描述符入门（2026-10-31）：学习者通过 `open(O_RDONLY)` 打开源文件，观察 `fd=3`；单字节/连续读取得到 `#`、`i`，完整读取到 EOF 并统计字节数。通过 `write(STDOUT_FILENO, ...)` 写出 17 字节；短写索引纸面判断通过：10 字节请求返回 6 后，从 `buffer+6` 续写 4 字节。随后打开 `message.txt` 并用可靠写循环写入固定字符串，学习者报告 `cat message.txt` 内容正确，目标文件写入基本路径通过；这次没有触发真实短写。下一步组合分块 `read` 与写完当前数据块的循环，实现实际复制。

最新实测反馈（2026-10-31）：学习者再次运行 `fd_demo`，报告 `write returned 17`、读到 EOF，并用 `cat message.txt` 确认文件内容为 `Hello from write`。这进一步确认固定消息写入路径；仍没有实际触发短写。下一步按分块读写实现 `fd_demo.c` 到 `copy.txt` 的复制，并用 `cmp` 比较。

文件复制基本功能实测通过（2026-10-31）：首轮复制因保留固定消息写入并提前关闭 `out_fd`，报告 `write: Bad file descriptor`。学习者随后报告使用 `gcc -std=c11 -Wall -Wextra -Werror fd_demo.c -o fd_demo` 严格编译成功，运行程序后 `cmp fd_demo.c copy.txt` 无差异输出，表明副本与源文件一致。未看到最终源码或 `$?`，因此验收限于报告的正常路径结果，不扩展为所有错误清理路径已审阅。下一步将源/目标路径改为命令行参数，检查参数数量并处理打开错误；再继续目录扫描。

命令行路径正常路径反馈（2026-10-31）：学习者报告严格编译成功，执行 `./fd_demo fd_demo.c copy.txt` 输出 `fd = 3`，随后 `cmp fd_demo.c copy.txt` 无输出。确认通过参数指定两个路径并复制一致的正常路径。尚未看到参数数量错误路径的输出/退出码；下一步运行 `./fd_demo` 并执行 `echo $?`，预期显示用法并返回非零状态。该错误路径确认后开始目录扫描部分。

参数错误路径反馈（2026-10-31）：学习者运行 `./fd_demo`，程序显示 `Usage: ./fd_demo <source><destination>`，紧接着 `echo $?` 得到 1。确认参数数量错误时拒绝执行且返回非零状态。用法字符串缺少两个占位参数间的空格，属于易读性小修正。文件复制的正常参数路径和参数不足路径均有报告的实测证据。当前进入任务 3.1 的目录扫描部分，从 `opendir/readdir/closedir` 基本接口开始。

目录扫描入口讲解（2026-10-31）：学习者表示此前不了解 `opendir/readdir/closedir` 和 `_POSIX_C_SOURCE`。已先解释目录流 `DIR *`、逐项读取、关闭资源，以及 `readdir` 返回 `NULL` 时用调用前清零的 `errno` 区分正常结束与读取错误。当前只练习打开一个目录并打印名称，不加入类型判断或递归。

目录扫描正常路径实测（2026-10-31）：学习者使用 `gcc -std=c11 -Wall -Wextra -Werror dir_demo.c -o dir_demo` 编译成功，运行 `./dir_demo .` 输出当前目录条目（包括 `.`、`..`），`echo $?` 为 0。确认基本 `opendir/readdir/closedir` 正常路径通过；未假设目录项输出顺序。参数不足和不存在目录的错误路径尚待运行验证。

错误路径验证策略调整（2026-10-31）：学习者指出参数缺失和明显的不存在目录错误路径不必每次贴出完整输出。后续对由已审阅控制流直接覆盖、风险低且行为明确的路径，记录为“已理解，未单独实测”，把实测重点放在目录项类型、`DT_UNKNOWN` 和后续组合行为；不把未运行的路径写成实测通过。

教学方式修正（2026-10-31）：目录项类型练习中教师直接给出可粘贴的 `switch` 实现，学习者指出这会削弱独立实现价值。后续对核心代码只给接口契约、分支要求和边界提示，先由学习者独立编写，再进行源码审阅和测试设计；已给出的示例不作为学习者独立完成证据。

阶段路线讨论（2026-10-31）：学习者询问能否先学阶段 5。已说明阶段 5 包含 Cortex-M4 裸机、外设、中断、DMA、FreeRTOS 并发和看门狗，依赖阶段 4 的 ARM/Cortex-A7 裸机、启动、寄存器、链接脚本和反汇编基础；阶段 3 Linux 系统调用、进程/线程和诊断工具也会支撑后续工程调试。当前不跳过阶段 3，先完成任务 3.1；可在阶段 3 中提前穿插必要的实时系统概念，但不把预习计为阶段 5 验收。

按学习者要求继续阶段 2 纸面学习，不安排实物实验。任务 2.2 原理图追踪及 UART/I2C 复用查表、任务 2.3 已布置的 UART 位时间和采样时间轴练习通过。去耦部分已通过负载突变时的充放电判断、ΔV=ΔQ/C 估算、同频理想容抗比较、PCB 回路电感影响和自谐振前后容性/感性判断。

2026-10-28：学习者正确回答，在 1 MHz 下，0.1 μF + 1 nH 呈容性，10 μF + 5 nH 呈感性；题给简化模型的自谐振频率分别约为 15.9 MHz 和 0.712 MHz。本轮分类检查通过。学习者此前关于小电容的表述考虑了具体器件实际参数；在所选器件确有较低 ESL 时，其高频实际阻抗可能更低，这一条件推理成立。容量值本身不能保证低 ESL，器件封装/结构与 PCB 安装回路均需考虑。此澄清覆盖历史记录中对其理由的初步判错。

教师补充的性能限定：呈感性不自动等于阻抗更大。在上述 1 MHz 简化模型中，忽略 ESR，前者 |Z| 约 1.585 Ω，后者约 0.0155 Ω，因此不能用容性/感性分类单独排序去耦效果；该数值比较为教师讲解，不是学习者测量或独立作答证据。去耦核心概念检查至此通过，不追加重复算题。

当前进行阶段 2 接口保护基础，区分串联限流、分压和瞬态钳位。学习者回答仅串 100 Ω 可以把 5 V 限制到 3.6 V，理由为电阻分压；该判断需纠正。教师说明压降由 V_R=IR 决定，高阻输入简化模型中电流很小、串阻压降很小，不能凭单个阻值保证固定降压；若要在 100 Ω 上降去 1.4 V，需要流过 14 mA。实际内部钳位可能因过压导通，是否允许须按具体输入范围及注入电流规格判断，不能把其当通用电平转换。尚无纠正后的独立回答，不标为通过。此前时钟忙等待、晶体负载等效关系与计时全跨度判断有通过证据，不扩大为完整设计验收。详见 [晶振与时钟记录](sessions/2026-10-28-阶段2-晶振与时钟基础.md) 和 [接口保护记录](sessions/2026-10-28-阶段2-接口保护基础.md)。继续纸面学习，不安排实验。任务 2.1/2.3 实测、阶段 2 其余内容与整体验收、阶段 0 遗留项仍保留。

2026-10-29：学习者确认已理解串联电阻不能单独完成固定电平转换。下一检查题转为完整分压网络的连接方式，尚待作答。

2026-10-29：学习者描述“5 V→R1→R2→GND，取 R1/R2 中点”，连接方式正确；术语“并联”不准确，R1 与 R2 在该支路中是串联，输入节点取中点。下一步按 `Vout=Vin×R2/(R1+R2)` 选择比例，尚待作答。

2026-10-29：学习者判断分压输出端存在输入漏电流时，输出电压会变小；在题设采用“漏电流从节点流走”的常见方向下正确。教师补充一般规律：漏电流会使输出偏离理想分压值，方向取决于电流流入还是流出输入端；电阻值越大，同样漏电流造成的误差通常越明显。下一步检查分压电阻取值与漏电流的关系。

2026-10-29：学习者比较两组分压电阻时正确判断 B 组（100 kΩ/200 kΩ）受 1 μA 漏电流影响更大，并尝试用 KCL 分析；教师修正数值口径。将分压网络化为戴维南等效，A 组 Rth≈667 Ω、输出下降约 0.67 mV；B 组 Rth≈66.7 kΩ、输出下降约 66.7 mV，B 组输出约 3.27 V。学习者的 0.2 V 是直接把漏电流乘 R2 得到的近似，未采用 R1||R2，方向和比较结论正确。

2026-10-29：阻值选择题中，学习者给出 `R1+R2=33 kΩ`、比例“1/2”。教师纠正总电流条件：5 V/100 μA=50 kΩ，因此总电阻应约 50 kΩ；`R1:R2≈1:2` 的比例判断正确，对应理想输出约 3.33 V。可取约 R1=16.7 kΩ、R2=33.3 kΩ，实际选标准值时还需检查输出误差、漏电、功耗和允许电阻范围。

2026-10-29：学习者确认继续学习，接口保护进入 TVS 与串联电阻的分工判断题。

2026-10-29：学习者正确回答 TVS 题：正常 3.3 V 时不导通，20 V 短时尖峰时导通，串联电阻主要限流。该分工检查通过；下一步说明 TVS 的实际钳位电压仍需与 MCU 允许范围、串阻和脉冲电流一起校核。

2026-10-29：学习者回答 TVS 钳位 8 V、MCU 绝对最大 3.6 V 时“不能保证，要看 MCU 内部保护电路”。方向正确；教师补充不能只看内部保护，8 V 本身已超过 3.6 V，必须联合核对 TVS 的工作反向电压、击穿/钳位曲线与测试电流、峰值脉冲能力、串联电阻和源阻抗、MCU 输入绝对最大值及允许注入/钳位电流、布局寄生等。该题基本判断通过，未据此标记具体保护网络选型完成。

2026-10-29：学习者正确判断 `VRWM=3.6 V`、正常信号最高 3.3 V 时 TVS 不应明显导通。教师补充实际仍有漏电流，且 VRWM 只是规定的连续工作反向电压范围，不是零电流或最终钳位电压。该题通过，接着区分 VRWM、VBR 与 VC。

2026-10-29：学习者对 `VRWM`、`VBR`、`VC` 三个 TVS 参数暂时不了解。教师补讲：VRWM 是连续工作反向电压，VBR 是雪崩击穿开始的规定测试区间，VC 是指定脉冲电流下的最大钳位电压；三者不是同一个电压。下一步用参数判断器件是否适合 MCU 输入保护。

2026-10-29：学习者正确判断 `VC=6.5 V`、MCU 绝对最大输入 3.6 V 时，TVS 不能单独直接作为 MCU 输入脚的完整保护。该题通过，接口保护部分继续转入“TVS 放在连接器侧、MCU 侧限流/钳位以及布局”的系统分工。

2026-10-29：布局题中，学习者回答“距离 MCU 引脚越近越能处理尖峰”。该原则对 MCU 侧低电感保护回路成立，但本题针对连接器引入的外部尖峰，教师补充 TVS 通常优先靠近连接器入口，在尖峰进入板内前分流；只放 MCU 旁可能让尖峰先经过较长走线并叠加寄生电感。具体系统可能同时需要入口 TVS、串联阻抗和 MCU 侧低电感钳位，不能用单一位置规则替代拓扑分析。

2026-10-29：高速通信线并联大电容题中，学习者回答“信号直接从电容流走了”。方向正确；教师精确化为电容对快速变化分量提供旁路，和源阻抗形成低通，导致边沿变慢、幅度衰减，可能造成采样失败。该题通过，后续转入保护器件对信号完整性的影响。

2026-10-29：接口保护纸面重点完成。学习者对高速线保护器件应关注寄生电容的判断正确。随后进入示波器探头负载：学习者暂时不知道 10× 探头相对 1× 探头为何更适合约 10 ns 边沿；教师补讲 10× 通常输入电容更小、对被测节点负载更轻，但信号衰减 10 倍，需要示波器相应设置。该测量概念尚待后续题确认。

2026-10-29：探头对高阻高速节点的影响题中，学习者正确选出 1× 探头更可能测慢上升沿，但把原因中的输入电容关系说反。教师纠正：通常 1× 输入电容更大，因而负载更重；10× 输入电容通常更小。学习者的影响方向正确，电容负载原因尚待确认。

教师解释：探头地线自身电感会与探头输入电容形成谐振回路，快速边沿可能激发过冲/振铃，长地线会增加电感。下一纸面题检查为何高速边沿测量要缩短探头地线。依据 Keysight 与 Tektronix 探头测量资料；未进行实际测量。

当前阶段 2 继续按纸面方式学习示波器测量。接口保护概念练习本轮已完成；探头倍率/电容因果关系仍待确认，当前题目检查长地线引入的振铃。此状态覆盖前文较早的接口保护待办描述。

2026-10-29：学习者正确判断长鳄鱼夹地线比短地弹簧更容易引入振铃，并指出探头地线电感与输入电容形成谐振回路。该纸面题通过。下一题为示波器带宽不足时测量 10 ns 边沿的影响。

2026-10-29：学习者估算 20 MHz 示波器自身上升时间约 17 ns，并正确判断相对 10 ns 待测边沿不够快，测得边沿会比真实值更慢。按 `tr≈0.35/BW` 得约 17.5 ns，题目通过。随后正确判断 `100 MS/s` 对应 `10 ns` 采样间隔，无法可靠重建约 `10 ns` 的边沿；并正确指出提高采样率不能补偿 `20 MHz` 模拟带宽不足，因为 ADC 只能更密地记录模拟前端已经变慢的波形。带宽估算题中学习者最初答 `35 MHz`，随后修正为 `105 MHz`，正确；目标示波器自身上升时间约 `3.3 ns` 时，按 `tr≈0.35/BW` 需约 `105 MHz`（可粗略取 `100 MHz`），该题通过。随后按上升时间合成关系估算实测边沿约 `10.5 ns`，相对真实 `10 ns` 约慢 `5%`，判断通过。学习者正确判断 `100 MHz` 示波器与 `50 MHz` 探头的系统带宽无法达到 `100 MHz`；补充近似 Gaussian 级联公式 `1/BW_total²≈1/BW_scope²+1/BW_probe²`，该组合约 `44.7 MHz`。学习者再判断探头与示波器均为 `100 MHz` 时系统带宽为 `70 多 MHz`，正确，估算值约 `70.7 MHz`；综合计算 `10 ns` 边沿经该系统后的读数约 `11.2 ns`，相对误差约 `12%`，通过。时间设置题中学习者正确指出应在保证记录完整时间范围的前提下尽可能提高采样率；补充单次边沿需看触发前后窗口，周期信号通常保留多个周期。学习者正确计算 `1 Mpoints/1 GS/s=1 ms`，并确认记录 `1 ms` 所需采样率为 `1 GS/s`；提高到 `2 GS/s` 时记录窗口缩短到 `0.5 ms`。该题通过。学习者要求后续避免单纯代公式题，改用综合测量场景，并选择分别观察整体波形和局部细节；该方案通过。下一题转入触发与混叠：解释整体波形稳定而局部放大可能不稳定的原因。

2026-10-30：学习者指出长时间窗的整体波形会隐藏细节，触发抖动可能使局部边沿位置不稳定。补充信号自身周期抖动、噪声以及屏幕抽取/压缩也会造成整体与局部观感差异。本题通过。下一步用 `10 MHz` 信号、`8 MS/s` 采样的综合情景学习混叠诊断与验证方法，避免单纯代公式题。

2026-10-30：混叠题中学习者表示暂时不知道。已讲解 `10 MHz` 信号以 `8 MS/s` 采样可能呈现为 `2 MHz` 假象，以及提高采样率并检查模拟带宽来验证的方法；学习者尚未复述确认，混叠仍待检查。

2026-10-30：学习者正确判断改变采样率后显示频率随之变化，更可能是混叠假象，而真实 `2 MHz` 信号应基本保持频率不变。混叠基本判断通过。下一题为测量方案设计：测 `2 MHz` 纹波但输入可能含 `10 MHz` 干扰时，如何用采样率和模拟滤波避免混叠。

2026-10-30：测量方案题中学习者建议将示波器带宽设为 `25 MHz` 并提高采样率。提高采样率方向正确，但 `25 MHz` 带宽仍会通过 `10 MHz` 干扰；若只保留 `2 MHz` 纹波，应使用 ADC 前模拟低通滤波器，截止频率设在两者之间。带宽限制与抗混叠滤波的区别待确认。

2026-10-30：学习者在比较方案时选择 `5 MHz` 低通加 `8 MS/s`，抑制 `10 MHz` 干扰的意图正确；但 `8 MS/s` 的奈奎斯特频率为 `4 MHz`，截止 `5 MHz` 的滤波器不能保证在奈奎斯特频率前充分衰减。需修正为保留 `2 MHz`、在 `4 MHz` 前充分衰减的模拟低通，或提高采样率并留出过渡带余量。此设计题待完善。

2026-10-30：学习者将滤波要求修正为 `0–2 MHz` 通带、其余频率阻带，方向正确。补充实际滤波器的过渡带：`0–2 MHz` 通带，`2–4 MHz` 过渡，`4 MHz`（`8 MS/s` 的奈奎斯特频率）及以上应有足够衰减。抗混叠滤波方案通过。

2026-10-30：学习者指出通带外并非理想地全部滤除，滤波器存在过渡带，理解正确。继续补充阻带衰减要结合干扰幅度确定：`1 V` 干扰是 `2 mV` 目标纹波的 `500` 倍，需把 `10 MHz` 处残余压到目标信号以下，避免残余混叠掩盖纹波。该设计指标尚待检查。

2026-10-30：按 `1 V` 的 `10 MHz` 干扰、`2 mV` 目标纹波估算阻带衰减：将干扰压至 `2 mV` 需约 `54 dB`；若将残余限制在纹波的 10%（`0.2 mV`），需约 `74 dB`。这说明截止频率和阻带衰减是不同规格，需依据允许误差共同确定。阻带指标检查尚待学习者判断具体滤波器规格。

2026-10-30：学习者正确判断，若 `10 MHz` 干扰已在 `8 MS/s` 采样时混叠到 `2 MHz`，采样后的软件低通不能把它与真实 `2 MHz` 纹波区分开；抗混叠滤波必须放在 ADC 前，或先提高采样率再数字滤波。本题通过。下一步综合检查示波器带宽限制/模拟低通对噪声和尖峰测量的双重影响。

2026-10-29：学习者正确指出高速信号保护器件应优先关注寄生电容，因为它为高频变化提供低阻抗旁路。教师补充 `X_C=1/(2πfC)`，寄生电容越大，对边沿的负载越重；该题通过。接口保护纸面重点至此完成，下一步转入示波器探头负载与测量误差。

下一纸面题：比较两组都能理想分到约 3.3 V 的电阻：A 组为 R1=1 kΩ、R2=2 kΩ；B 组为 R1=100 kΩ、R2=200 kΩ。假设输入漏电流大小为 1 μA 且从分压节点流走，哪一组的输出电压受影响更大？原因是什么？

2026-10-28：晶振首题曾混淆“候选外部时钟源未就绪”和“当前系统无时钟”，教师按题设纠正内部 RC 仍在驱动 CPU。补充题中学习者回答“CPU 有时钟，一直在执行等待”，正确，无时钟与程序忙等待的区别检查通过；不据此宣告全部时钟切换机制或晶振设计掌握。超时后保留当前时钟并处理错误为教师建议。负载电容题最初答约 10 pF，教师给出每只 18 pF 的代数结果；学习者随后说明为口算，并明确知道两只电容的乘积除以和应为 9 pF，等效关系理解通过，不再追加此题。误差预算题中，学习者回答“30”；在明确分列初始频差 ±20 ppm 与温度额外频偏 ±30 ppm、只计这两项最坏同向叠加的题设下，应估算为 ±50 ppm。教师用 +20+30=+50 与 −20−30=−50 解释单项和总误差的区别；没有新的独立复述，不标记误差预算题通过，不要求重复精算。

2026-10-28：学习者正确判断简化 RC 复位电路上电瞬间 NRST 约为 0 V、充分充电后约为 3.3 V，并判断低有效外部复位在上电瞬间有效。教师补充充电后外部复位请求解除、τ=RC、1τ/5τ 的充电比例、C 增大十倍时到同一假定阈值的时间增大十倍，以及假定 2.0 V 解除阈值时 t≈0.93RC；这些时间/阈值计算均为教师示例，不能记为学习者独立验收。学习者本轮回答“可以对毛刺起到滤波作用”，概念方向正确。教师进一步限定 RC 平滑的是 NRST 节点，不能因此认定 VDD 已稳定；若真实欠压危及运行，需适当配置的 BOR 或外部复位监控器。学习者随后正确判断：VDD=2.6 V 高于假定 BOR 阈值 2.4 V，因此不会触发 BOR；但 2.6 V 低于最低可靠工作电压 2.7 V，不能据此认为芯片可靠运行。该比较题通过。下一题转向欠压恢复过程，POR/BOR 其余细节仍待作答，不标记为完整掌握。

2026-10-28 恢复题纠错：学习者回答“不能，要测量芯片内部降压后的电压是否满足要求”。认可其不能直接认定可靠的谨慎判断；教师澄清，首先依据外部 VDD 工作范围与复位/供电时序规格，内部稳压电压是否需要或能够测量取决于具体芯片，不能替代外部供电合规性。原题 2.6 V 上升至 3.3 V 不经过 2.5 V，且前例没有触发 BOR；若沿用下降门限 2.4 V，上升门限 2.5 V 的差应为 100 mV，不能同时称 50 mV。该题作废，不记为学员错误或通过证据。新讲解例子统一为：最低工作电压 2.7 V、下降触发 2.8 V、上升释放条件 2.85 V、滞回 50 mV，电源 3.3→2.6→3.3 V；假设监测有效并忽略门限误差。达到 2.85 V 仅满足电压条件，仍按器件要求等待释放延时等条件。该例为教师讲解，尚无独立作答或实测。

2026-10-28 复位恢复判断：学习者回复“不能接触复位，不能”，按上下文将“接触”理解为“解除”，两问判断正确。教师补全依据：2.81 V 未达假设的 2.85 V 释放电压条件；达到该门限只满足电压条件，仍需规定延时及其他启动条件。复位已布置的基本概念题到此结束，不追加重复数值题；这不等于完成实测、器件级供电设计或阶段 2 验收。开始晶振与时钟基础，尚待新的独立作答。

2026-10-28 计时误差题：给定时钟总误差 ±50 ppm、时间 10 分钟，学习者答 0.06 s，随后明确说明“是的，我计算的是两个时间差”，确认报告的是最快与最慢两种结果之间的总跨度。按此口径 0.06 s 正确，本题通过，不记为计算错误。相对于理想时间的偏差约为 ±0.03 s，两端总跨度为 0.06 s；原题未明确报告形式，现已澄清，不追加重复口算题，不扩大为其他误差预算或硬件实测验收。

## 历史进展（以下保留过程，以顶部当前目标为准）

阶段 1 结项结论（2026-10-25）：通过。闭卷测验 90/100，变量内存区域与生命周期说明通过，任务 1.21 阶段作品 86/100。作品由学习者使用 `gcc -std=c11 -Wall -Wextra -Werror` 编译，90 项集成检查 PASS、0 FAIL，退出码 0；GDB 普通断点、条件断点、单步、参数/局部变量、调用栈、内存查看、watchpoint 和反汇编均有实际操作记录。未达满分的改进项：集成测试由手工生成帧，尚未将 `frame_encode` 接入 ring→parser 做完整端到端往返；task21.mk 未将头文件列入依赖；条件断点曾因条件过宽停在 CMD，尚无在真正非法 LEN 分支停住的证据。以上不影响既定 80 分结项门槛。任务 1.7 动态运行库兼容构建仍为延期项，不阻塞当前学习路径。下一学习阶段计划于 2026-10-26 开始，具体首任务届时按阶段路线确定。

内存区域与生命周期复习（2026-10-25，已通过）：普通全局变量初始化为非零通常在 .data；未显式初始化或显式初始化为 0 的全局/静态对象通常在 .bss，静态存储期持续至程序结束。自动局部对象通常由栈实现，离开作用域生命周期结束。const 不决定存储期或段；`const char *name` 指针和字符串字面量是不同对象，局部指针随调用结束，字面量具有静态存储期，通常在只读区。学习者已能区分五类对象并说明 malloc 对象由 free 结束、指针变量自身生命周期独立，口头验收通过。

内存对象练习（2026-10-25）：学习者正确判断 `g_count=5` 通常在 .data、`limit` 为自动局部对象、字符串字面量通常在只读区且具有静态存储期、malloc 对象在堆上并由 `free` 结束生命周期；也正确区分 `free(p)` 不会立即结束局部指针变量 `p` 的生命周期，p 到 `demo` 返回才结束。唯一错误是把 `static int calls = 0` 判为 .data；在常见 ELF 布局中显式零初始化对象通常在 .bss。待学习者补充该修正后，再标记五类对象说明通过。

内存对象说明结项（2026-10-25）：学习者补充确认“零初始化的全局和静态变量通常位于 `.bss`”。五类对象的存储区域、存储期和 `free` 与局部指针生命周期区别已通过口头验收。该结论限定为常见 ELF 实现的布局；C 标准本身不规定 `.data/.bss/.rodata` 段名。

阶段 1 闭卷测验（2026-10-25，已完成）：共 10 题、每题 10 分，闭卷作答；覆盖指针/生命周期、ELF 常见段和程序段、volatile/const/原子性、模块/API、FIFO/链表、协议帧/CRC、增量解析、超时、Make/GDB。成绩及错题说明见下条。

阶段 1 闭卷测验成绩（2026-10-25）：学习者首次作答按题意修正为 90/100，通过 80 分线。题 2 未明确 `a`、`b` 是全局还是函数内局部，因此不扣分；若是全局，通常 `a=5` 在 `.data`、`b` 在 `.bss`，若是函数内自动对象，通常在栈。`static c=0` 通常在 `.bss` 且程序全程存在。其余掌握内容包括 malloc/free 与局部指针生命周期、volatile 与原子性区别、const 两种写法、链表尾插基本关系、帧长度/CRC 覆盖、CRC 向量与碰撞、解析器基本流程和 GDB 常用命令。需要复习的边界：①题 8 中 `used` 是当前候选帧已收到的字节数，`expected` 是根据 LEN 得到的当前帧总字节数，不是泛称的空间。②题 9 的超时条件是 `elapsed >= timeout_ms`，119 ms 保留半帧，120 ms 清除半帧；`0xFFFFFFF0` 到 `0x00000004` 的无符号差为 20 ms。③题 10 中 GDB 停在某行时，通常该行尚未执行；`next` 执行当前行但不进入被调函数，`step` 可进入，`backtrace` 查看调用栈。阶段知识测验通过，作品验收仍需 GDB 证据整理和最终自评。

超时题意澄清（2026-10-25）：学习者原先把 119 ms 和 120 ms 理解为分别有新数据到达的时刻；题目含义是最后一个字节在 100 ms 处理，之后没有新字节，分别在 119 ms/120 ms 检查半帧是否超时。于是 elapsed 分别为 19 ms 和 20 ms；阈值为 20 ms 时前者保留，后者清除。若 119 ms 时确有新字节并被处理，`last_byte_ms` 会更新为 119，之后应从 119 重新计算等待时间。

任务 1.21 与阶段 1 已结项：作品 86/100，严格编译与七组 90 项集成测试通过；阶段闭卷 90/100，变量内存区域与生命周期口头验收通过。GDB 主要命令有实际操作证据。评分理由及非阻塞改进项见任务 session。下一学习目标为阶段 2，计划 2026-10-26 开始。

任务 1.21 GDB 入口记录（2026-10-25）：在 GDB 中执行 `break protocol_rx_process` 会按函数名设置断点；输出 `Breakpoint 1 at ...` 只表示断点已建立，程序尚未运行。`run` 才启动被调试程序；只有程序运行到断点并停下后，GDB 才有当前栈帧。`info args` 查看当前函数的参数，`print now_ms` 查看当前上下文中的变量；在尚未命中断点时执行它们会得到 `No frame selected` 或 `No symbol ... in current context`，这是正常现象。后续命令：`info locals` 查看局部变量，`backtrace` 查看调用栈，`next` 单步但不进入函数，`step` 单步进入函数，`continue` 继续运行，`quit` 退出 GDB。下一步命中 `protocol_rx_process` 后记录参数、`now_ms`、`rx->parser.used` 和 `rx->ring.count`。

GDB 调试符号诊断（2026-10-25）：学习者执行 `run` 后确实命中 `protocol_rx_process`，但 `info args` 输出 `No symbol table info available`，`print now_ms` 输出 `No symbol ...`。这说明当前加载的 `task21_test.exe` 没有可用的 DWARF 调试信息；断点地址能命中不等于调试符号存在。应退出 GDB，执行 `make -f task21.mk clean` 后 `make -f task21.mk`，确认构建规则含 `-g -O0`，再用新生成的 `task21_test.exe` 启动 GDB。重新命中后再执行 `info args`、`info locals` 和 `print`。若仍无符号，检查是否启动了旧目录/旧文件，或执行了 strip。

GDB 首次符号调试通过（2026-10-25）：学习者使用带 `-g -O0` 的程序，在 `protocol_rx_process` 设置断点并运行成功；断点显示源码文件和第 45 行。`info args` 显示 `rx=0x61fdb0`、`now_ms=100`、`frame_out=0x61fda6`；`print now_ms` 得到 100；`rx->parser.used` 为 0，表示还没有从 ring 取出当前字节；`rx->ring.count` 为 6，正好是测试帧 `AA 02 20 FF 01 BF` 的长度。`backtrace` 显示当前在 `protocol_rx_process`，由 `test_1_complete_frame` 调用，再由 `main` 调用。停在函数第一行时，局部 `byte` 和 `result` 尚未执行赋值语句，GDB 显示的 `99`、`32765` 是未初始化值，不能当作业务结果。下一步用 `next` 越过参数检查和 pop 语句，在赋值后重新查看 `byte`、`result`、`rx->parser.used`、`rx->ring.count`。

GDB 行停点补充（2026-10-25）：第一次 `next` 后停在第 51 行，超时检查调用尚未执行；第二次 `next` 后停在第 52 行，声明初始化 `int result = rx_ring_pop(...)` 也尚未执行。GDB 通常停在“将要执行的当前源代码行”上，因此此时 `byte` 和 `result` 仍可能显示旧的未初始化值，而 `parser.used`、`ring.count` 尚未改变。再执行一次 `next` 越过第 52 行后，才查看 `byte`、`result` 和 ring 状态。

GDB 单步验证（2026-10-25）：越过 ring pop 后停在第 53 行，再越过错误判断并停在 parser 调用前；执行 parser 调用后，`byte=170`（十六进制 `0xAA`）、`result=0`、`rx->ring.count=5`、`rx->parser.used=1`。这证明第一个帧头字节已从 FIFO 取出并被解析器接收，尚未形成完整帧。下一步使用条件断点观察非法 LEN 路径，并用 `x/12bx` 查看 parser 候选帧内存。

GDB 条件断点第一次观察（2026-10-25）：设置 `break frame_parser_push if byte > 8` 成功建立断点 2，但普通断点 1 仍启用；执行 `continue` 后先在下一个 `protocol_rx_process` 入口命中断点 1。此时当前栈帧仍是 `protocol_rx_process`，`parser` 是该函数的局部指针名而不是当前作用域变量，因此 `print parser->used` 和 `x/12bx parser->bytes` 报 `No symbol` 属于正常现象；`byte` 也是 process 中尚未赋值的局部变量，不能据此判断条件断点。可用 `disable 1` 暂停普通断点，再 `continue` 直到命中断点 2；命中后才执行 `info args`、`print byte`、`print parser->used` 和 `x/12bx parser->bytes`。

GDB 条件表达式修正（2026-10-25）：条件 `byte > 8` 会捕获命令字节 `0x20` 等普通数据，因为它们也大于 8；学习者命中时 `parser->used=2`、`byte=32`，缓存为 `AA 02 ...`，表示正在处理 CMD 而非 LEN。学习者没有新建断点，而是在该次停点直接执行 `continue`；continue 会沿用已启用的断点 2，因此若条件仍是 `byte > 8`，后续 CMD/payload 字节也可能再次触发。真正的非法长度分支只在 parser 已收到 SOF、即 `parser->used == 1` 时检查。无需新增断点，可直接修改现有断点条件：`condition 2 parser->used == 1 && byte > 8`。命中后预期 `parser->used=1`，`byte` 为 9~255，缓存首字节为 `0xAA`。

GDB 重启程序记录（2026-10-25）：GDB 调试期间可以重启被调试程序以重置其运行时状态。`kill` 结束当前 inferior；随后 `run` 从程序入口重新启动，栈变量、全局运行状态和测试进度重新开始。断点和 GDB 设置仍保留，因此重启前可用 `info breakpoints` 检查断点，使用 `condition 2 parser->used == 1 && byte > 8` 修改断点 2 条件，然后 `kill`、`run`。若直接在已运行/暂停的程序中输入 `run`，GDB 可能提示先确认终止当前程序；显式 `kill` 的流程更清楚。`continue` 只是从当前停点继续，不会重置状态。

GDB 学习计划（2026-10-25）：学习者决定重新开始一次 GDB 调试，目标是理解并练习常用命令后结束本轮 GDB 学习。按实际进展逐项练习：断点和断点列表、条件断点、run/kill/continue、next/step、info args/info locals、print、backtrace、x 查看内存、disable/enable/delete、quit。每条命令说明作用和当前上下文；结束后记录已掌握项及未完成项，不把单次命令成功等同于全部 GDB 验收。

GDB 源码与断点管理练习（2026-10-25）：`list protocol_rx_process`、`list frame_parser_push` 成功显示源码行号和附近代码；Windows 控制台中的注释字符出现乱码属于终端编码显示问题，符号和行号仍可用。`info breakpoints` 输出 `No breakpoints or watchpoints`，因为之前用 `delete` 清除了断点，这是正常状态。下一步设置两个函数断点并练习 `info breakpoints`、`disable`、`enable`、`delete`。

GDB step/finish 练习（2026-10-25）：学习者从 `protocol_rx_process` 连续使用 `next` 到 parser 调用行，`step` 成功进入 `frame_parser_push`；`info args` 显示 parser 地址、byte=0xAA 和 frame_out 地址，说明 step 会进入被调用函数。之后执行 `finish`，终端直接回到 PowerShell，没有通常的 `Value returned ...` 和 `(gdb)` 提示，因此只确认 finish 命令已发出，不声称已观察到返回值；可能是调试会话/终端提前结束或输出截断。下一步新开 GDB 会话练 `x` 内存查看和 `watch` 变量监视，再明确退出并总结命令。

GDB 内存与 watchpoint 练习（2026-10-25）：在 `frame_parser_push` 入口执行 `x/12bx parser->bytes`，看到 12 个 0 字节，符合首个 SOF 尚未写入 parser 缓存的时点；`watch parser->used` 建立硬件监视点，`info breakpoints` 显示函数断点和 watchpoint 均启用。第一次 `next` 只执行空指针判断并停在 `if (parser->used == 0U)`，该判断行尚未执行，因此 used 尚未变化、watchpoint 未触发。下一步再 `next` 执行判断分支并停在写入 SOF/更新 used 之前；继续单步后应观察 watchpoint 在 `used` 从 0 变为 1 时报告旧值和新值。

watchpoint 行步进进度（2026-10-25）：学习者从第 42 行继续 `next`，依次停在第 44 行（帧头比较）、第 46 行（写 bytes[0]）和第 47 行（即将写 used=1）。这些停点都在对应当前语句执行前；此时 `used` 尚未变化。下一次 `next` 执行第 47 行赋值后应由硬件 watchpoint 停住。

watchpoint 命中确认（2026-10-25）：再次 `next` 后硬件 watchpoint 命中，报告 `Old value = 0`、`New value = 1`，当前停在赋值后的第 48 行。`watch parser->used` 已完成实际验证。继续练习缓存更新后的内存检查及 `disassemble`，随后整理 GDB 命令复习表并结束本轮 GDB 学习。

GDB 内存及反汇编观察（2026-10-25）：watchpoint 之后 `x/12bx parser->bytes` 显示 `AA 00 ...`，确认帧头已写入缓存。`disassemble /m frame_parser_push` 成功输出带源代码行的 x86-64 汇编；例如序言中的 `push %rbp`、建立栈帧和 `sub $0x30,%rsp`，以及空指针检查对应的比较/条件跳转，直观展示 C 代码如何映射到机器指令。输出进入 GDB 分页器并提示 `Type <return> to continue, or q <return> to quit`；按 `q` 只退出当前分页显示，不退出 GDB。下一步练 `finish`/`continue` 收束运行，再整理命令复习并退出。

任务范围说明（2026-10-25）：任务 1.21 当前先冻结协议功能，不在 GDB 和阶段验收前继续扩展代码。剩余工作是常用 GDB 命令、至少 5 类变量的内存区域/生命周期说明、阶段闭卷测验和作品验收。若完成本任务后继续练习，再另设有独立目标、接口和测试的扩展任务，不把扩展内容混入当前作品结项证据。

任务 1.16 已结项：五个函数源码审阅通过，学习者自行使用 `gcc -std=c11 -Wall -Wextra -Werror` 编译并运行 `task16_verify.c`，全部 25 项显示 PASS，`ALL TESTS PASSED`，PowerShell `$LASTEXITCODE` 为 0；概念问答通过。学习者理解数组按下标访问与链表逐节点访问的复杂度差异、环形缓冲区 FIFO 特性、clear 先保存 next 再 free 的原因，以及多节点删除头节点时 head 前移而 tail 不变。评分 95/100，扣分为首轮删除头节点表述和术语精度。教师未代编译或运行。

任务 1.17 当前进展：`counter` 是保存一个 `uint32_t` 数值的简单计数器模块，用于练习模块边界、API 契约和对象所有权。`counter.h`、`counter.c` 和 `counter_test.c` 源码审阅全部通过（3/3）；测试正常路径、NULL 错误路径和销毁行为，且未访问私有成员。概念问答通过；尚待学习者提交严格编译和运行结果后结项。每个函数均有“功能、参数、返回值”注释。
任务 1.17 已结项：`counter.h`、`counter.c` 和 `counter_test.c` 源码审阅通过，测试未访问私有成员，概念问答通过；学习者使用 `gcc -std=c11 -Wall -Wextra -Werror task17_test.c task17_counter.c -o task17` 编译成功，运行后 `$LASTEXITCODE` 为 0。评分 100/100。

任务 1.18 当前进展：使用主机内存模拟 GPIO 寄存器，练习 `volatile`、`const volatile`、读改写和位字段保留。`gpio_sim.h` 与 `gpio_sim.c` 源码审阅通过；教师已将 `task18_test.c` 重写为统一 `PASS/FAIL` 断言式测试，覆盖初始化、绑定、ENABLE/MODE 位操作、其他位保留、READY 0/1、NULL/未绑定/非法参数。尚待学习者自行编译运行。

任务 1.18 已结项：学习者使用 `gcc -std=c11 -Wall -Wextra -Werror task18_test.c task18_gpio_sim.c -o task18.exe` 编译成功，完整验证程序 20 项全部 `PASS`，输出 `ALL TESTS PASSED`，PowerShell `$LASTEXITCODE` 为 0；源码、测试和概念验收通过，评分 100/100。

任务 1.19 当前进展：串口帧格式和校验和概念已讲解。`task19_frame.h`、`frame_checksum` 与 `frame_encode` 源码审阅通过；教师已编写 `learning/practice/c_study/task19_test.c`，覆盖校验和向量/回绕/空输入/NULL、示例帧、空 payload、最大长度、长度超限、容量不足无写入和 NULL 参数。下一步学习者自行严格编译运行并反馈结果。payload 长度 0 时允许 payload 为 NULL，非零时必须非 NULL。

任务 1.19 测试运行通过：学习者报告 `task19.exe` 的 13 项测试全部 PASS，输出 `ALL TESTS PASSED`，`$LASTEXITCODE` 为 0。已解释将 `uint8_t` 长度与 `size_t` 容量比较时先检查上限，再计算 `size_t required_length`，避免有符号性诊断。严格编译命令的执行结果尚未明确提供；确认后结项。

任务 1.19 已结项：学习者确认严格编译无问题，13 项测试全部 PASS，`ALL TESTS PASSED`，`$LASTEXITCODE` 为 0；源码、编码和校验测试通过。

任务 1.20 当前进展：`task20_parser.h`、`frame_parser_init`、`frame_parser_push` 源码审阅通过。教师已创建 `task20_test.c`，覆盖 NULL 参数、初始化 NULL、帧前垃圾、拆包状态保留、完整帧输出、成功后复位、两帧粘连、零/最大 payload、非法 LEN 后恢复、坏校验后恢复。下一步学习者自行严格编译运行并反馈。

任务 1.20 编译与运行测试通过：学习者使用 `gcc -std=c11 -Wall -Wextra -Werror task20_test.c task20_parser.c task19_frame.c -o task20.exe` 成功编译；17 项全部 PASS，`ALL TESTS PASSED`，PowerShell `$LASTEXITCODE` 为 0。尚需概念验收：说明 used/expected 如何跨调用保留半帧、坏长度/坏校验后如何恢复同步，以及逐字节 API 如何处理粘连帧。拼错的 `$LASTEXTICODE` 未定义，不作为证据。

任务 1.20 已结项：学习者理解坏长度/坏校验后重置解析器并等待帧头；能说明 used 统计当前候选帧已收到的字节数、expected 是期望总字节数；成功重置后，剩余粘连字节继续输入可解析下一帧。

任务 1.21 当前进展：CRC、帧编码和增量解析单模块已有审阅及学习者测试证据；RX 环形缓冲区与 protocol 模块已编写，frame_t 已归入 frame 头文件。整体架构、目标接口、处理时间超时规则和测试预期已整理到 [任务1.21-架构与预期结果](labs/任务1.21-架构与预期结果.md)。超时首版尚待修正：used>0 即检查，使用 timeout_ms，补齐所有返回路径；process 需增加 now_ms 并记录成功处理时间，头文件和测试调用同步。目标统一初始化接口尚待实现，整个 RX 链路集成验收、调试及阶段测验未完成。下一步先修正超时及时间接口，不把设计文档或单模块历史结果算作整体通过。

阶段 1 剩余闸门：无，阶段已于 2026-10-25 结项。任务 1.7 动态运行库兼容构建仍延期并如实保留，不阻塞阶段 1 结项；可在后续相关目标板学习中安排补做。阶段 2 计划于 2026-10-26 开始。

函数注释格式偏好：统一使用 `功能：`、`参数：`（逐项 `@name`）、`返回值：` 的分段格式；当前两个解析器 API 注释按该格式提供。

任务 1.21 RX 链路最新补充（2026-10-25，覆盖此前待修正状态）：protocol_rx_init 和 check_timeout 已通过源码审阅，尚未集成验收。当前须移除 protocol 头文件中的全局 rx 对象定义，随后给 process 接入 now_ms、消费前超时检查和消费后时间记录，同步声明/定义/测试调用。ring 的 FIFO/满空/回绕与超时边界统一放入 RX 集成验证，不单独重测三个 ring 函数。

`task20_parser.h` 已通过审阅，正确包含 `<stdint.h>`、`<stddef.h>` 并声明帧、固定缓存解析器和逐字节 API。当前实现 `.c`：`used` 记录候选帧字节数，合法 LEN 后 `expected = LEN + 4`；每次 push 一字节，不完整返回 0，完整有效帧填充输出并返回 1，非法长度/校验失败返回 -2 并重置，空指针参数返回 -1。

任务 1.19 概念首轮：学习者指出 UART 数据寄存器通常按 1 字节接收；已补充硬件接收粒度不等于 `read()` 返回粒度，驱动缓冲会导致一帧拆分或多帧合并，解析器必须维护跨读取的状态。下一步继续确认帧格式和校验和。

任务 1.19 帧计算：学习者正确判断示例帧的 LEN、payload 数量和总长度；校验和已从首答 `0x2A` 纠正为 `0x29`，理解 8 位累加只保留低 8 位。下一步创建协议头文件。

任务 1.18 概念验收通过：学习者理解 `volatile` 约束编译器保留真实内存访问，理解原子性与不可分割访问有关；已纠正 `const int *p` 与 `int const *p` 等价、都限制通过指针修改目标对象，`int * const p` 才限制指针本身。`gpio_sim.h` 和 `gpio_sim.c` 源码审阅通过（2/3）；实现检查寄存器地址、正确处理 ENABLE/MODE 字段并保留其他位，READY 读取归一化为 0/1。测试初步输出 `F8 -> F9 -> F8 -> FE`、READY 为 1，正常路径方向正确；尚缺完整断言和错误路径覆盖。

`gpio_sim.h` 首版：寄存器结构、上下文结构、`volatile` 类型和四个函数声明正确。按学习者约定，函数注释放在 `.c` 定义前而不在 `.h` 重复；头文件源码审阅通过。下一步实现 `gpio_sim.c`，四个定义前必须有完整注释。


任务 1.15 Git/GitHub 首次管理验收通过：学习者确认网页 main 上源码和学习记录可见、产品资料未上传；SSH push 成功并建立 origin/main；指定仓库的 safe.directory 持久设置经不带前缀的 git status 验证。HTTPS 握手错误根因未查明，但 SSH 已完成上传目标。当前进入任务 1.16「单向链表」，实现初始化、尾插、按值查找、删除首个匹配节点和清空，维护 head/tail/size 与动态节点生命周期。完整契约见 records/sessions/2026-10-23-任务1.16-单向链表.md。阶段 1 仍需完成纯 C 协议库综合作品及闭卷测验。本轮记录晚于远端 6da68c5，留待下一次常规提交同步。

当前补讲：首次推送成功不解除本地仓库所有权检查。已向学习者说明可自行运行 `git config --global --add safe.directory C:/Users/zgd/Desktop/study`，让本人 Git 配置长期信任此目录，随后在 study 用不带前缀的 `git status` 验证。尚未收到执行结果，未代改全局配置；若验证成功，日常命令不再要求 `-c safe.directory`。任务仍待网页核对。

验证更新（2026-10-23）：学习者已在 study 目录直接运行 `git status`，成功显示 `main...origin/main`，持久 safe.directory 配置生效；工作区有 3 个记录文件修改，属于推送后新增的学习记录，尚未提交/推送。任务 1.15 仅待 GitHub 网页确认 main 分支文件范围，随后再提交本轮记录并结项。

最新状态：学习者经 SSH 首次推送成功，GitHub 接收 main 并建立 origin/main 跟踪关系；只读复核为 `main 6da68c5 [origin/main] docs: record first local commit`。本地提交、SSH 远端关联、分支重命名和 push 操作通过，不再把 HTTPS 握手问题作为当前阻塞。任务 1.15 当前只待学习者在 GitHub main 分支网页核对源码和记录存在、未上传产品资料。尚未声称教师已直接读取网页。本轮更新的学习记录留待下次常规提交，不为每次回复循环 add/commit。

本轮反馈：学习者暂存更新记录时收到 LF→CRLF 工作区行尾转换警告。已解释这类 warning 不阻止暂存，无需为此改配置；继续计划中的文档提交、远端关联与推送，尚无新增提交或上传成功证据。本轮说明是暂存后的工作区更新，可留待下一次常规记录提交，不要求为每条补讲重复 add/commit。

当前进展（2026-10-23）：学习者回到 study 目录执行暂存与检查，已提供部分清单。教师只读核对选定范围，显示 72 个新增文件、5696 行新增（本轮记录更新前），包括源码、Agent.md、records，不含 .exe；`.gitignore` 已加入 `**/*.exe`。学习者随后确认首次本地提交 `b2f0415 (HEAD -> master) Initial commit: learning code and records`，本地 commit 验收通过。由于本轮记录是在该提交之后更新的，当前记录还有待再次暂存提交；远端关联和推送尚未完成。add 仅保存当时内容，记录后续编辑需重新暂存。下方旧状态为历史过程，以本段为准。

2026-10-23：学习者提供 GitHub 地址 `https://github.com/zgd7788/embebded_study`，希望亲自上传并管理。本地只读检查仍为 master 尚无提交，选定范围 `.gitignore`、`input/Agent.md`、`learning/`、`records/` 均未跟踪，`remote -v` 无输出；没有暂存、提交或上传完成证据，未验证远端内容。learning 内存在 main.exe、a.exe，当前规则不忽略 .exe。当前指导学习者亲自添加 `**/*.exe` 忽略规则，再对上述四个明确路径暂存并检查暂存清单和统计，之后学习本地提交、关联远端、首次推送。教师不代执行核心操作，不读取 Project。详见 `records/sessions/2026-10-23-任务1.15-GitHub远端管理.md`。任务 1.15 保持进行中。下方 2026-10-22 内容为历史快照。

2026-10-22 当前目标：任务 1.15 Git 版本控制基础。学习者针对已确认的学习仓库使用一次性 `safe.directory` 命令级配置，未修改全局 Git 配置。只读输出显示仓库位于尚无提交的 `master` 分支；`status` 报告根目录 `.gitignore`、`input/`、`records/` 和 `learning/` 为未跟踪，暂存区无已暂存条目；`git diff --stat` 为空，直接原因是普通 `git diff` 不显示未跟踪路径；`git log` 因当前尚无任何提交而报错。学习者已正确说明分支无提交、暂存区为空、`-C` 改变工作目录、`-- .` 限定路径，并在纠正后准确说明 `git diff` 忽略未跟踪内容；只读检查理解通过。此前教师错误地声称 `git -C .\learning` 会自动把 `status`/`diff` 范围限制到子目录；实际 `-C` 仅改变工作目录，status 默认仍显示全仓库状态。学习者尝试 `git add \input` 时再次遇到 dubious ownership，Git 在执行暂存前中止，未有内容被暂存；对单条命令使用 `-c safe.directory=C:/Users/zgd/Desktop/study` 即可，不必写入全局配置或改所有者。学习者随后修改 `.gitignore` 并说明 records 需要纳入版本控制。复查确认当前规则忽略项目资料目录、常见生成物及 `*.log`，但未忽略 `learning/` 或 `records/`；限定状态检查显示 `?? .gitignore`、`?? learning/`、`?? records/`，均尚未暂存或提交。Markdown 记录可纳入；`.log` 文件仍受忽略规则影响。详见 `records/sessions/2026-10-22-任务1.15-Git版本控制基础.md`。任务 1.14 已通过 100/100，未代编译/运行。阶段 1 仍未整体完成。以下旧任务仅作历史回顾。

当前处于阶段 1——C 语言与计算机基础。任务 1.1、1.2、1.3 已完成；阶段 0 仍为进行中，硬件相关待办不阻塞阶段 1 的主机 C 学习，但在上板运行、烧录和阶段 0 结项前必须补齐。

任务 1.4 已完成：交叉编译命令退出状态为 0，file 和 readelf 均确认 reg_sim-arm 为 ARM 32 位、EABI5 hard-float、动态加载器为 /lib/ld-linux-armhf.so.3。该结果不等于板端运行库兼容。

任务 1.5 已完成，仍属于阶段 1“C 语言与计算机基础”：板端 /lib/ld-linux-armhf.so.3 指向 ld-2.32.so，/lib/libc.so.6 指向 libc-2.32.so，strings 最高显示 GLIBC_2.32，而 reg_sim-arm 需要 GLIBC_2.34。板端未安装 file/readelf，但兼容性诊断证据充分，未运行程序。

任务 1.6 已完成：静态 ARM 产物确认无 PT_INTERP 和 DT_NEEDED，静态编译退出码为 0；学习者能区分静态链接与 sysroot，并理解静态程序体积和维护代价。不得修改板端系统 libc。

任务 1.7 当前结论已同步：最终目标是把 Windows 编写的 C 源码经 ARM 交叉编译，传输到 FS-MP1A 并在板端运行。静态 ARM 版本已传输到 /mnt，板端输出 0、0x2，退出码为 0，且与 Windows 运行结果一致，静态端到端链路验收通过。动态兼容版本仍需匹配 Buildroot SDK/staging sysroot，已延期到后续工具链专题；/mnt 目录不得删除。

任务 1.8 已完成：reg_write、reg_read、reg_update_bits 源码审阅通过，学习者理解 volatile 不保证读改写原子性。

当前进入阶段 1 任务 1.9：ELF 节区、程序段与内存占用。

任务 1.9 已开始：先分析静态 ARM 程序的 size、ELF 节区和程序段。

任务 1.9 已完成：学习者能解释静态/动态 text、.bss、节区与程序段，以及加载器依据程序段建立映射。

任务 1.10、1.11 均已通过；当前进入任务 1.12：C 内存、变量生命周期与 static。

任务 1.8 的 reg_write 已通过源码审阅；下一步实现 reg_read。

任务 1.8 的 reg_read 已通过源码审阅；根据学习者要求，剩余函数和整体验收标准一次性布置。

任务 1.8 三个函数源码审阅和概念验收均已通过。

任务 1.7 动态兼容路径已延期到后续工具链专题；静态端到端路径已通过，不阻塞阶段 1 当前学习。

## 历史过程记录（以下内容保留，不代表当前状态）

任务 1.2 首版源码审阅未通过：`bounded_copy` 当前未保证截断时的 `\\0`、返回值恒为 0，且未处理 `src == NULL`。下一步修正函数并提交边界测试；不能仅凭“测试通过”标记完成，必须核对测试是否覆盖这些契约。

任务 1.2 函数终版复审：核心边界逻辑通过。当前契约为空指针返回 `SIZE_MAX`、容量 0 返回 0、正常复制最多保留一个字节写入 `\\0`，返回实际复制长度。待确认完整测试和头文件；printf 耦合列为工程质量改进，不阻塞核心功能。

任务 1.1 已完成运行验收：Windows PowerShell 实测代表性正常、边界和错误用例，功能闸门通过，评分 88/100。下一步进入任务 1.2；任务 1.1 的 errno 清零、long 到 int 收窄和错误消息改进作为复盘项保留。

任务 1.2 当前目标：在 `learning/practice/c_study/` 编写受限长度字符串工具，练习数组、指针、字符串结束符和边界条件；一次性提交源码、构建结果、正常/边界/异常测试和简短解释。具体要求见 `records/sessions/2026-10-16-任务1.2-受限字符串工具.md`。

当前终端调整：C 语言练习改在 Windows。先提交实际终端类型和 `gcc --version`；PowerShell 中使用 `$LASTEXITCODE` 检查程序退出状态，Git Bash/WSL 才使用 `echo $?`。嵌入式主线仍在 Ubuntu 26.04。

Windows 工具已确认：PowerShell 中 MinGW-w64 GCC 8.1.0 可执行，`$LASTEXITCODE` 为 0。下一步在同一目录用 `-std=c11 -Wall -Wextra -Werror` 编译 calc.c，提交编译输出和 `$LASTEXITCODE`；不把编译成功等同于测试通过。

构建已通过：学习者报告编译无输出、`$LASTEXITCODE` 为 0，说明当前 Windows GCC 构建和告警检查通过。下一步只运行正常用例 `12 + 5` 并检查 `$LASTEXITCODE`，随后再做边界和错误用例。

学习者已提交 `calc.c` 修正版。当前源码审阅确认：已使用 `strtol`、检查尾随字符、a/b 范围、操作符长度和非法操作符返回值，保留函数拆分与除零处理；仍需检查 `end == argv[i]`、`errno == ERANGE`，并明确 long 到 int 的收窄安全性。提交 GCC 构建命令、告警结果、正常/边界/错误输入及退出状态实测后再验收，不进入任务 1.2。

已只读查看 `learning/practice/c_study/tsak01.c`：两个 int 变量求和并用 `%d` 输出，源码逻辑正确。目录还有 `task01.exe`，没有执行该文件，未据其存在判断构建或运行成功。学习者代码未修改，嵌入式其他能力不因自述 C 基础而直接通过。

当前补讲：学习者询问 strtol。教师解释 endptr、errno/ERANGE、完整输入和 -1000～1000 业务范围，示范局部调用，不代写解析函数。本轮先用“输入 0 与 abc 都返回 0，如何区分”检查理解；计算器修正仍为当前实验，无新通过结论。

新增补讲：`errno.h` 是 C 标准库头文件，提供 `errno` 和 `ERANGE` 等错误状态接口；`echo $?` 是 Ubuntu Shell 查询上一条命令退出状态的命令。已说明 C 的 `return 0` 通常对应 Shell 状态 0，`return -1` 通常显示 255；当前实验仍待学习者实际验证。

学习环境调整：学习者明确 C 语言练习使用 Windows。若使用 PowerShell，上一条原生命令退出码用 `$LASTEXITCODE` 查看；`echo $?` 是 Bash 语法，不能直接照搬。嵌入式主线仍使用 Ubuntu 26.04，工具链和板端操作不因 C 练习换到 Windows 而改变。

教学节奏调整：学习者指出代码已在目标目录且逐条确认预测会浪费时间。后续任务改为一次性提交完整测试矩阵和关键解释，不再将明显的预测拆成独立往返；当前任务仍需实际运行证据。

学习者询问何时开始任务 1：依据主机 C 不依赖阶段 0 所有硬件验收的实际前置关系，现在开始基础学习。阶段 0 保留未完成，不虚标通过，详见 Agent.md 的前置任务规则。后续按完整小实验推进，减少单条查询的任务碎分。

## 阶段状态

已讲解进程退出和配置持久化相互独立；本轮收到进程查询无输出的实际反馈，未据此认定配置已保存。历史锁定、快捷键全选及概念澄清过程保留在任务 0.27 会话记录。

| 阶段 | 名称 | 状态 | 得分 | 证据 |
| ---: | --- | --- | ---: | --- |
| 0 | 入门准备与基线 | 进行中 | — | `records/environment.md`、`records/sessions/2026-10-13-环境档案与概念检查.md`、`records/sessions/2026-10-14-任务0.3-交叉编译目标三元组.md`、`records/sessions/2026-10-14-任务0.4-sysroot检查.md`、`records/sessions/2026-10-14-任务0.5-交叉编译搜索目录.md`、`records/sessions/2026-10-14-任务0.6-主机与ARM二进制比较.md`、`records/sessions/2026-10-14-任务0.7-ARM动态库依赖.md`、`records/sessions/2026-10-14-任务0.8-GLIBC版本需求.md`、`records/sessions/2026-10-14-任务0.9-交叉工具链libc路径.md` |
| 1 | C 语言与计算机基础 | 已结项 | 闭卷 90/100；作品 86/100 | `records/sessions/2026-10-24-任务1.21-纯C串口协议库综合作品.md`；任务 1.7 动态 sysroot 延期、不阻塞结项 |
| 2 | 电子基础、原理图与仪器 | 进行中：任务 2.1 理论学习；实测暂缓 | — | `records/sessions/2026-10-26-任务2.1-LED限流与按键电路.md` |
| 3 | Linux 基础与应用开发 | 未开始 | — | — |
| 4 | ARM 与 Cortex-A7 裸机基础 | 未开始 | — | — |
| 5 | Cortex-M4 裸机与 FreeRTOS | 未开始 | — | — |
| 6 | Linux 设备树与驱动 | 未开始 | — | — |
| 7 | A7/M4 多核通信 | 未开始 | — | — |
| 8 | 初级综合项目 | 未开始 | — | — |
| 9 | Linux 系统移植与启动链 | 未开始 | — | — |
| 10 | 高级驱动、工业协议与性能 | 未开始 | — | — |
| 11 | 工程化、可靠性、安全与升级 | 未开始 | — | — |
| 12 | 中级综合项目 | 未开始 | — | — |

## 已完成事项

- 任务 1.15：暂存清单检查、本地提交、SSH origin 配置、main 分支首次推送及上游跟踪关系均已完成；学习者确认网页内容和上传范围合规，验收通过。
- 任务 1.16：单向链表已完成源码审阅、严格编译、边界自测和概念验收，95/100。
- 阶段 1 当前进度已同步：任务 1.1～1.16 已完成；下一步进入后续 C/嵌入式基础任务。
- 阶段 1 当前进度已同步：任务 1.1～1.15 已完成；当前进行任务 1.16 单向链表。
- 任务 1.1 命令行整数计算器已完成功能验收，评分 88/100；代码质量改进项保留为复盘内容。
- 任务 1.2 受限字符串工具已完成终版源码审阅，核心边界逻辑通过。
- 任务 1.3 寄存器字段模拟器已完成源码审阅，六个函数符合契约，严格编译通过，评分 95/100。
- 任务 1.14 状态机与回调已完成概念推演及函数源码审阅，评分 100/100；未代编译或运行。

- 已阅读并采用 `input/Agent.md` 作为唯一学习规范。
- 已确认后续不读取、不分析、不引用、不修改 `project/` 目录。
- 已建立 `learning/`、`records/` 和本次任务记录结构。
- 已确认按任务和验收推进，不以学习时长作为进度依据。
- 已确认 FS-MP1A V3.0、Ubuntu 26.04、USB 转串口、万用表和逻辑分析仪。
- 已完成“开发主机、目标板与交叉编译”讲解。
- 任务 0.1a 已通过：能指出 `Exec format error` 应先检查目标架构和二进制格式。
- 任务 0.1b 已通过：能指出文件存在却报告 `No such file or directory` 时，可能缺少 ELF 动态加载器。
- 任务 0.1c 已通过：能指出动态加载器存在后，还要检查共享库；同时知道执行权限也属于后续检查项。
- 任务 0.1d 已通过：知道当前用户需有执行权限，并知道权限字符串中的 `x` 表示执行。
- 任务 0.1e 已通过：能按架构、动态加载器、共享库、执行权限顺序说明程序启动排查链。
- 任务 0.1f 已通过：能指出板卡启动失败或反复复位时应转向板级复位/时钟诊断。
- 任务 0.2 已通过：确认 Ubuntu 26.04 VMware/x86_64 环境、Git/GCC/Make/GDB 版本、ARM hard-float 编译器存在、AArch64 编译器缺失及磁盘空间。
- 任务 0.3 已通过：学习者报告目标三元组为 `arm-linux-gnueabihf`，并能区分它面向 ARM Linux、主机原生 GCC 面向 x86-64 Linux。
- 任务 0.4 已通过：学习者报告 sysroot 为 `/`，并能说明 sysroot 只是目标头文件/库查找环境的一部分，GCC 还会使用其他搜索路径和工具链配置；没有据此断定库架构或板端兼容性。
- 任务 0.5 已通过：提交了交叉编译器的库搜索目录，并能解释目录路径不等于目录内库文件架构，也不能证明链接器最终选中了其中某个库。
- 任务 0.6 已通过：学习者提供同一份 Hello World 源码、原生与 ARM 交叉编译命令，以及 `file` 输出；能区分 x86-64 ELF 与 ARM 32-bit EABI5 ELF。尚未运行 ARM 产物。
- 任务 0.7 已通过：学习者能区分动态加载器 `/lib/ld-linux-armhf.so.3` 与 `NEEDED libc.so.6` 共享库依赖，并知道这些 ELF 信息不能证明板端存在 ABI/版本兼容的 libc。
- 任务 0.8 已通过：输出显示 `hello-arm` 对 `libc.so.6` 有 `GLIBC_2.4` 和 `GLIBC_2.34` 版本需求；学习者能说明这不单独证明 FS-MP1A 板端 libc 兼容。
- 任务 0.9 已通过：学习者能解释 `-print-file-name` 返回 Ubuntu 工具链侧的 `arm-linux-gnueabihf` 目标库路径，不把它当作板端文件或兼容性证明。
- 任务 0.10 已通过：学习者根据 `file -L` 识别工具链 libc 为 ARM 32-bit EABI5 共享对象，动态解释器为 `/lib/ld-linux-armhf.so.3`；清楚这不能证明 FS-MP1A 板端兼容。
- 任务 0.11 已通过（准备信息确认，非实际串口通信）：学习者转述产品手册的 TTL/115200 信息，并确认配套串口设备与板卡兼容；实际连接和通信尚未验证。

- 任务 0.12 已通过：Ubuntu 枚举板载 ST-LINK/V2.1，稳定设备链接指向 `/dev/ttyACM0`，学习者确认是 FS-MP1A 板载接口；尚未验证通信。
- 任务 0.13 基础判断通过，90/100：提交权限与用户组输出，正确判断当前用户无直接读写权限；完整的属主、属组排除依据由教师补充。

- 任务 0.14 已通过，100/100：当前会话包含 `dialout`，与原输出比较，原有用户组全部保留；实际打开串口和通信待验证。

- 任务 0.15 环境检查操作完成，操作项 100/100：学习者报告查询没有输出，当前 PATH 未找到 `minicom`；概念解释未单独验收，不据此断言整个系统完全未安装。

- 任务 0.16 版本验证通过，操作项 100/100：学习者提交 `minicom 版本 2.10`；完整安装日志未提交，串口通信待验证。

- 任务 0.17 最终配置通过，指导下操作项 100/100：A 已修正为 `/dev/ttyACM0`，结合前次 E=`115200 8N1`、F/G=否，符合要求；编辑失败及修正记录保留，设置尚未持久化、通信尚未验证。

- 任务 0.18 首次串口接收验证通过，操作项 100/100：收到内核/用户空间启动日志片段，服务管理器报告到达 Multi-User System，并显示 Buildroot 欢迎标识；登录、双向交互及完整启动日志尚未验收。

- 任务 0.19 登录提示确认通过，操作项 100/100：已收到 `buildroot login:`，当前等待用户名，尚未进入 Shell。

- 任务 0.20 首次登录通过，操作项 100/100：已收到 `[root@buildroot]:~#`；同时出现 resize2fs “Nothing to do”信息，未报告调整错误。提示符身份待下一项命令核对。

- 任务 0.21 通过，100/100：学习者正确预测板端为 32 位 ARM，并提交 `armv7l`、`root`；已区别于主机 x86_64，板端命令往返与当前用户名已核对。

- 任务 0.22 通过，100/100：板端 `uname -r` 为 `5.4.31`，系统标识为 Buildroot `2021.02.1`，并保留 `-g003e4fc-dirty` 原始版本后缀。

- 任务 0.23 通过，100/100：`/dev/mmcblk2p4` 挂载为根目录 `/`，类型 `ext4`、读写；`/dev/mmcblk2p5` 挂载为 `/root`。该证据证明根文件系统在 MMC 块设备分区上，但单凭 `mmcblk2` 未区分 eMMC/SD。

- 任务 0.24 通过，100/100：`type=MMC`、`name=SCA08G`；与根分区位于 `mmcblk2` 的证据一致，支持板载 eMMC 判断，但不单独证明启动链。

- 任务 0.25 通过，100/100：`removable=0`；结合 `MMC/SCA08G`、根分区 `/dev/mmcblk2p4`，支持板载 eMMC 运行时存储判断；不单独证明启动顺序。

- 任务 0.26 通过，100/100：收到 `reboot: Power down`；PSCI 后续信息已原样保存，未据此判断物理断电状态。

- 任务 0.27 尝试启动捕获会话时被占用阻塞：学习者提交 `minicom -s` 的 PID 6472 正在占用 `/dev/ttyACM0`，并存在对应锁文件；已判断应先正常退出配置会话，再启动带 `-C` 的捕获会话。尚未触发下一次启动，暂不评分。

## 当前未通过项

- 动态 sysroot/Buildroot SDK：延期到后续 Linux 应用与工具链专题，不阻塞阶段 1 当前学习。
- 任务 1.1 的首版问题记录保留在历史记录中，不再作为当前未通过项；功能闸门已通过。
- 任务 0.27 文件写入和关键阶段标记检查通过；原始完整日志归档待补，不声称已直接读取全部原始内容。`grep` 小写版本成功执行，无需安装。
- 阶段 0 待办：原始日志归档与连续三次冷启动验收、ARM 程序兼容性核对及板端运行、恢复流程核对，以及环境/版本记录的收尾；在相关上板任务前补齐。
- 启动片段中 `dhcpd.service` 启动失败、`proc: Bad value for 'hidepid'`，已保留原文，根因待查；不据此认定整板启动失败。
- 已完成登录、板端命令交互和一次受控关机；完整启动日志、实际供电规格仍待验证，阶段 0 的完整日志及冷启动闸门未通过。
- 任务 0.13 巩固点：能正确判断其他用户无权限；需进一步完整表述为何当前用户既非属主、也非属组成员。
- 后续实操仍需补充目标 ABI、sysroot、动态加载器和运行时库兼容性之间的关系。

## 当前环境待确认项

- ARM 交叉编译器最终选中的库文件及其与 FS-MP1A 当前系统的兼容性。
- 板载 ST-LINK VCP 已枚举并指向 `/dev/ttyACM0`，Ubuntu 会话含 `dialout`，minicom 2.10 可执行，板端命令交互已验证；板端架构为 `armv7l`，用户名 `root`，内核 `5.4.31`，Buildroot `2021.02.1`。根目录 `/dev/mmcblk2p4`、设备 type/name `MMC/SCA08G`、removable=0，支持板载 eMMC 运行时证据；正常关机、实际供电规格和完整启动日志待验证。
- 出厂镜像恢复方法与验证状态。

## 当前学习状态（2026-10-18 更新）

- 当前阶段：阶段 1——C 语言与计算机基础。
- 已完成：任务 1.1、1.2、1.3、1.4、1.5、1.6、1.7 静态端到端路径、1.8、1.9。
- 任务 1.7 的动态 sysroot/Buildroot SDK 路径延期到后续 Linux 应用与工具链专题，不阻塞阶段 1 当前学习。
- 任务 1.9 已通过，100/100；学习者已理解节区服务于链接和分析，程序段是运行时加载器建立映射的主要依据。
- 截至 2026-10-18 的进度快照：当时唯一学习目标为任务 1.14；该任务现已通过，当前目标为任务 1.15 Git 版本控制基础。

## 阶段 1 规模与剩余量（2026-10-18）

阶段 1 的任务数量不是按时间固定的。按当前主线估计总量约 12～15 项；任务 1.1～1.9 已完成，任务 1.10 正在进行，预计还剩约 3～5 项。剩余内容预计覆盖 GDB、构建自动化/Make、Git 与调试复盘、主机到 ARM 的小型综合验收。时间仅作计划参考，不作为完成标准。

## 任务 1.10 当前进展（2026-10-18）

学习者已提交 GDB 调试证据：成功读取调试符号、命中 `reg_set_mode` 断点、查看参数和枚举值、使用 `next`、查看调用栈，并正常结束程序。`next` 停在 `main.c:9`，`step` 进入 `reg_sim.c:60`；已纠正“`backtrace` 显示变量值”的表述，变量查看由 `info args`/`info locals`/`print` 完成。随后将 `mode` 临时改为非法值，使用 `finish` 定位 `mode error` 分支并确认返回 `-1`。任务 1.10 通过，100/100。

任务 1.11：Make 构建与依赖管理，已完成，100/100。学习者从最小规则逐步完成对象文件、链接、增量构建、头文件依赖、clean、默认目标，并已提交完整 Makefile 通过最终审阅；此前进行中和待审阅状态已关闭。

2026-10-20 阶段进展：最小规则步骤通过。学习者确认 `main.o` 存在，`file` 显示其为带调试信息的 x86-64 ELF relocatable；修正 `-o0` 大小写笔误为 `-O0` 后，`make -B main.o` 成功执行 GCC recipe。当前进入对象文件与链接规则的分步练习；完整 Makefile 验收尚未进行。

## 下一项任务（2026-10-22 历史安排，现以顶部目标为准）

当前继续任务 1.15：只读检查理解已通过。仓库因所有权不同触发 Git 安全保护；命令级 `-c safe.directory=C:/Users/zgd/Desktop/study` 是已验证的临时处理方式，不必写全局配置或改所有者。学习者最近的 `git add \input` 在安全检查时中止，没有暂存文件。学习者更新 `.gitignore` 并确认学习记录需要纳入版本控制；复查确认 `learning/`、`records/` 均未被忽略，当前状态为未跟踪。常见 `*.log` 文件仍被忽略。尚未暂存任何内容；下一步需先教学并选择学习记录路径，再由学习者执行单路径 `git add`。不得读取 Project。教师已更正：`-C .\learning` 只改变命令工作目录，status 默认仍报告全仓库；如需严格限于 learning，pathspec 应写在 `--` 后，例如 `status --short --branch -- .`。任务 1.14 终审记录见 `records/sessions/2026-10-22-任务1.14-状态机实现终审与结项.md`。

## 任务 1.3 当前状态

- 结构体/枚举/位运算寄存器模拟器已完成源码审阅。
- `reg_set_field`、`reg_get_field`、`reg_set_mode`、`reg_set_enable`、`reg_set_irq_enable`、`reg_set_prescaler` 均符合接口契约。
- 使用 `-std=c11 -Wall -Wextra -Werror` 整体编译通过。
- 根据规则，函数任务通过后不保留逐项测试输出作为进度证据。

## 下一项任务（已更新）

任务 1.15：Git 版本控制基础。阶段 0 剩余硬件待办继续保留。
## 2026-10-18 规则更新

- 已约定：函数实现通过后，不再保留测试证据；以源码审阅和验收结论记录完成状态。
- 任务 1.2（受限字符串工具）核心实现已通过，进入任务 1.3。

## 2026-10-18 接口说明规则

后续每个函数练习开始前，先说明功能、参数、返回值、错误情况、边界条件和验收标准，再进入编码和审阅。

2026-10-29：学习者正确说明分压电阻过小会增加功耗并要求电阻承受相应功率，同时指出输入漏电流通常较小，因此不必机械地把电阻取到很小。针对其疑问，教师解释分压输出端的戴维南等效：将理想 5 V 电源置零后等效为短路，R1 上端和 R2 下端都接地，所以从输出节点看 R1、R2 都是到地的支路，等效为 R1||R2。该解释尚待学习者复述确认。

2026-10-29：学习者回复“明白了”，确认区分了原分压电路中的串联关系和求输出端戴维南等效时将理想电压源置零后得到的并联关系。下一步布置带目标分压电流的电阻取值题。
