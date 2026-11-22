.syntax unified
.cpu cortex-a7
.arm

.section .text.start, "ax", %progbits
.global _start
.type _start, %function
.extern main
.extern __bss_start__
.extern __bss_end__
.extern __stack_top__

/*
 * 功能：裸机程序入口
 * 前置条件：由 Bootloader 转入，DDR 等运行内存已可用
 * 说明：屏蔽中断、设置栈、清零 BSS 并调用 main；main 返回后停留
 */
_start:
    cpsid if
    ldr sp, =__stack_top__

    ldr r0, =__bss_start__
    ldr r1, =__bss_end__
    mov r2, #0
1:
    cmp r0, r1
    strlo r2, [r0], #4
    blo 1b

    bl main
2:
    b 2b

.size _start, . - _start
