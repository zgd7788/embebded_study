# GPIOZ PZ5 LED Blink

This is a minimal Cortex-A7 bare-metal exercise for the FS-MP1A board. It links
at DDR address `0xC0008000` and uses `0xC1000000` as the initial stack top, in
line with the existing A7 exercises in this workspace.

## Build

Run from this directory in the Ubuntu development VM:

```sh
make
```

The build creates `build/gpio_led.elf`, `build/gpio_led.bin`, a link map, and
`build/gpio_led.dis`. It uses `arm-none-eabi-gcc` and does not link a C library.
If the Ubuntu VM only has the Linux-targeting cross compiler, try
`make clean all CROSS_COMPILE=arm-linux-gnueabihf-`; the image still uses
freestanding/no-library link flags.

## Run

The board must already have completed its normal bootloader setup, including
DDR initialization. Interrupt the normal boot at U-Boot, transfer
`build/gpio_led.bin` into RAM at `0xC0008000` using the transfer method
configured for the board, then enter the image with:

```text
go 0xC0008000
```

For example, a configured TFTP server may load it with
`tftpboot 0xC0008000 gpio_led.bin`. Do not run this image from a Linux shell:
it directly controls GPIOZ while Linux may own the same pin and clock.

The program enables GPIOZ's A7-side AHB5 clock, configures PZ5 as a low-speed
push-pull output, and toggles it in an infinite loop. The delay is a volatile
busy loop, so its period depends on the CPU clock and compiler settings; it is
only for confirming visible GPIO operation. A hardware timer should replace it
for a stable period. LED1's connection and active polarity still need
confirmation from the board schematic; toggling should blink with either
polarity if PZ5 is indeed connected to LED1.

This startup code sets the stack, clears `.bss`, and calls `main`. It does not
set up a vector table, exception handlers, caches, MMU, or interrupt handling.
