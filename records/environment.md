# 开发环境基线

> 建档日期：2026-10-12  
> 状态：已通过 minicom 登录并执行板端命令，架构 armv7l、当前用户 root，内核 5.4.31、Buildroot 2021.02.1；根分区 `/dev/mmcblk2p4` ext4，MMC type/name `MMC/SCA08G`、removable=0，支持板载 eMMC 运行时存储判断；一次受控关机已收到 Power down，完整启动日志及恢复流程待验证

## 学习者主机（学习者提供）

- 操作系统：Ubuntu 26.04
- 架构：`x86_64`
- 可用磁盘空间：根文件系统 96G，总可用 78G（检查时）
- 环境类型：VMware 虚拟机
- 学习者另在 Ubuntu 虚拟机桌面创建了 `study` 练习目录；该虚拟机目录不在当前助手可见的 Windows 工作区中。源码与命令由学习者粘贴提交，未声称助手直接访问或验证该目录。

## 工具（学习者 Ubuntu 环境状态待查）

## C 语言 Windows 环境（学习者提供）

- 终端：Windows PowerShell
- GCC：MinGW-w64 `gcc.exe` 8.1.0，目标描述 `x86_64-win32-seh-rev0`
- `gcc --version` 成功后，PowerShell `$LASTEXITCODE` 为 0
- 该 GCC 仅作为 Windows C 主机练习工具；不替代 Ubuntu GCC 15.2.0、`arm-linux-gnueabihf-gcc` 或 FS-MP1A 板端环境

- Git：2.53.0
- GCC：15.2.0，原生目标 `x86_64-pc-linux-gnu`
- Make：4.4.1
- GDB：17.1
- minicom：版本 2.10，配置页面曾提交 `/dev/ttyACM0`、115200 8N1、两类流控均关闭；已收到启动日志并完成板端登录与命令交互，配置持久化待验证
- ARM 交叉编译器：`arm-linux-gnueabihf-gcc` 存在；目标三元组 `arm-linux-gnueabihf`；`-print-sysroot` 输出 `/`；`-print-search-dirs` 包含 `/usr/lib/arm-linux-gnueabihf/` 等目标路径以及 `/usr/lib/` 等通用路径。任务 0.6 已提交源码和构建命令；构建检查显示原生产物为 x86-64 ELF，交叉产物为 ARM 32-bit EABI5 ELF，ARM 动态解释器为 `/lib/ld-linux-armhf.so.3`。任务 0.7 的 `readelf -d` 输出含 `NEEDED libc.so.6`；任务 0.8 的版本需求输出含 `GLIBC_2.4` 和 `GLIBC_2.34`。板端 ABI/库版本兼容性待进一步验证。
- AArch64 交叉编译器：未发现 `aarch64-linux-gnu-gcc`
- 串口终端：Ubuntu VM 已枚举板载 ST-LINK/V2.1（USB ID `0483:374b`），稳定链接指向 `/dev/ttyACM0`；设备为 `root:dialout`，Ubuntu 会话含 `dialout`。已通过 minicom 收到板端日志并完成登录及命令往返；完整启动日志待归档
- STM32CubeProgrammer：待确认
- STM32CubeIDE/其他 IDE：待确认

## 曾检查的执行环境（不得与学习者主机混淆）

- 本助手工具执行环境为 Windows，曾发现 Git 2.54.0、MinGW GCC 8.1.0 和 GDB 8.1.0。
- 该结果只描述助手执行环境，不代表学习者 Ubuntu 主机已安装这些工具，也不作为课程环境验收证据。

## 资料中发现的环境资源

- Ubuntu 开发环境资料目录：`09-开发环境-Ubuntu18.04/`
- FS-MP1A Buildroot SDK 压缩包：`01-FS-MP1A基础资料/01-工具软件/SDK/arm-fsmp1x-linux-gnueabihf_sdk-buildroot.tar.gz`
- SDK 尚未解压、安装或加入 PATH；不得把这一项记为已完成。

## 开发板

- 型号与硬件版本：FS-MP1A V3.0（学习者提供）
- 供电方式：实物待确认；V3 产品手册 PDF 第 16 页推荐通过 J301 使用 5V/2A 直流电源适配器，识别到 ST-LINK 不代表主处理器已上电
- 关机实物观察：学习者明确确认执行 `poweroff` 后电源指示灯全部熄灭、板卡已关机；结合 `reboot: Power down`，已收到正常关机的软件日志和实物观察，不用 NRST 字段否定该结果，不将灯灭扩展为全部电源轨电压均已测量为零
- 启动介质：任务 0.27 的 BL2 日志报告 `Using EMMC`、`Instance 2`、`Boot used partition fsbl1`，支持早期启动使用 eMMC；结合此前根分区 `/dev/mmcblk2p4`，分别保留启动阶段与运行时证据，完整后续镜像加载来源待查
- 早期固件与硬件报告：STM32MP157AAA Rev.Z；Model 为 HQYJ FS-MP1A Discovery Board；BL2 `v2.2-r1.0(debug):a70053f`，构建时间原文 `14:56:55, Aug 26 2023`；DDR 报告 `DDR3-DDR3L 16bits 533000Khz`、`0x20000000 (512 MB)`；均为启动日志报告值
- 本次复位原因：`Reset reason (0x814)`、`Pad Reset from NRST`；该字段不足以单独确定冷启动或具体触发原因，主电源操作过程尚未单独描述
- 调试串口：TTL，学习者已确认配套设备兼容；使用板载 ST-LINK VCP，可读日志接收及登录后命令往返已验证
- 串口参数：手册值为 115200 8N1、无流控，前次配置页面符合要求；已收到可读日志，重新运行后的具体参数页面未再次提交，配置持久化待验证
- 板端系统证据：已收到 Multi-User System、Buildroot 欢迎标识与 Shell 提示符；学习者报告 `uname -m` 为 `armv7l`、`id -un` 为 `root`，`uname -r` 为 `5.4.31`，Buildroot 用户空间为 `2021.02.1`。`/dev/mmcblk2p4` 挂载为 `/`、`/dev/mmcblk2p5` 挂载为 `/root`，均为 ext4；`mmcblk2` sysfs type/name/removable 为 `MMC`/`SCA08G`/`0`，支持板载 eMMC 运行时存储判断，完整启动链待验证
- 登录依据及结果：按产品使用指导的 root 用户名登录后，学习者已提交 Shell 提示符；片段未显示密码提示，不泛化到其他认证方式。登录后还显示 resize2fs 1.45.6 及 “Nothing to do”，未报告调整错误，作用设备及脚本未核对
- 启动异常待诊断：`dhcpd.service` 启动失败，`proc: Bad value for 'hidepid'`；根因未定，未修改配置
- 网络连接方式：待确认
- 可用仪器：USB 转串口、万用表、逻辑分析仪等（学习者提供；具体型号待记录）
- 出厂镜像备份位置：待确认
- 恢复流程验证日期：待确认

## 环境变更记录

| 日期 | 变更 | 原因 | 结果 |
| --- | --- | --- | --- |
| 2026-10-15 | 捕获会话内启动到登录提示 | 任务 0.27 | 收到 BL2 与 Buildroot 登录片段；早期固件报告 eMMC/fsbl1；DHCP 服务失败复现，捕获文件及中间启动链待验证 |
| 2026-10-15 | 板端正常关机 | 任务 0.26 | 串口收到 `reboot: Power down` 及 PSCI 信息；物理断电状态未单独验证，完整启动日志待采集 |
| 2026-10-15 | MMC 可移除属性核对 | 任务 0.25 | `mmcblk2/removable` 为 `0`；结合 MMC/SCA08G 和根分区挂载，支持板载 eMMC 判断 |
| 2026-10-15 | MMC 设备类型识别 | 任务 0.24 | `mmcblk2` 的 type 为 `MMC`，name 为 `SCA08G`；与板载 eMMC 判断一致，可移除属性待查 |
| 2026-10-15 | 根文件系统挂载核对 | 任务 0.23 | `/dev/mmcblk2p4` 为根 ext4 分区，`/dev/mmcblk2p5` 挂载 `/root`；MMC 设备具体介质待识别 |
| 2026-10-15 | 板端版本基线 | 任务 0.22 | `uname -r` 为 5.4.31；`/etc/os-release` 为 Buildroot 2021.02.1，保留 `-g003e4fc-dirty` 后缀 |
| 2026-10-15 | 首次接收板端启动日志 | 任务 0.18 | 内核/用户空间片段可读，显示 Multi-User System 与 Buildroot；DHCP 服务及 hidepid 异常待诊断，完整日志与登录待验证 |
| 2026-10-15 | minicom 版本验证 | 任务 0.16 | 学习者提交 `minicom 版本 2.10`，工具可执行；串口参数及通信待验证 |
| 2026-10-15 | 串口终端工具查询 | 任务 0.15 | 学习者报告 `command -v minicom` 没有输出；当前 PATH 未找到，安装与版本验证待完成 |
| 2026-10-15 | 当前会话串口用户组验证 | 任务 0.14 | 组列表新增 `dialout`，原有组全部保留；实际串口通信待验证 |
| 2026-10-15 | 串口设备权限检查 | 任务 0.13 | `/dev/ttyACM0` 为 `root:dialout`、`crw-rw----`；当前会话不含 `dialout`，尚未配置访问或打开串口 |
| 2026-10-12 | 创建基线文件 | 阶段 0 初始化 | 已完成 |
| 2026-10-12 | 检查主机工具 | 阶段 0 环境检查 | Git/GCC/GDB 已发现；Make/ARM 交叉编译器待配置 |
| 2026-10-13 | 学习者环境确认 | 阶段 0 | Ubuntu 26.04；FS-MP1A V3.0；USB 转串口、万用表、逻辑分析仪在手。具体工具安装和板卡连接待验证 |
| 2026-10-14 | Ubuntu 只读环境检查 | 任务 0.2 | VMware、x86_64、Ubuntu 26.04；Git 2.53.0、GCC 15.2.0、Make 4.4.1、GDB 17.1；ARM hard-float 编译器存在，AArch64 未发现；根文件系统可用 78G |
| 2026-10-14 | 交叉编译器目标确认 | 任务 0.3 | 学习者报告 `arm-linux-gnueabihf`；sysroot 和板端运行库兼容性待验证 |
| 2026-10-14 | 交叉编译器 sysroot 查询 | 任务 0.4 | 学习者报告 `arm-linux-gnueabihf-gcc -print-sysroot` 输出 `/`；不据此判定目标库或板端兼容性 |
| 2026-10-14 | ARM ELF 运行库版本检查 | 任务 0.8 | `hello-arm` 声明需要 `libc.so.6` 的 `GLIBC_2.4` 和 `GLIBC_2.34` 符号版本；板端兼容性未验证 |
| 2026-10-14 | 交叉工具链 libc 定位 | 任务 0.9 | 学习者报告 GCC 输出 `/usr/lib/gcc-cross/arm-linux-gnueabihf/15/../../../../arm-linux-gnueabihf/lib/libc.so.6`；实际文件和板端兼容性待验证 |
| 2026-10-14 | 工具链 libc 路径解释 | 任务 0.9 | 学习者能解释该路径属于 Ubuntu 交叉工具链侧，目录名提示 32 位 ARM Linux hard-float 目标；不视为板端兼容证据 |
| 2026-10-14 | 串口准备确认 | 任务 0.11 | 学习者转述手册标明 TTL、115200，并确认配套串口设备兼容；未进行接线或通信测试 |
| 2026-10-14 | 工具链 libc ELF 检查 | 任务 0.10 | `file -L` 输出显示工具链库为 ARM 32-bit EABI5 shared object，解释器为 `/lib/ld-linux-armhf.so.3`；板端兼容性待确认 |
| 2026-10-14 | 工具链库与板端边界说明 | 任务 0.10 | 学习者正确区分 Ubuntu 交叉工具链文件和板端运行环境；仅凭主机 Linux 信息不能证明目标板的库状态 |
