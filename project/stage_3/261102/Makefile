TARGET := gpio_led
BUILD_DIR := build

# ?=表示如果这个变量还没有定义，就给他这个默认值
CROSS_COMPILE ?= arm-none-eabi-
# :=表示立即展开右侧的变量引用，在保存结果
CC := $(CROSS_COMPILE)gcc
# | 变量 | 默认内容 | 用途 |
# |---|---|---|
# | CC | arm-none-eabi-gcc | 编译 C，也可调用链接过程 |
# | OBJCOPY | arm-none-eabi-objcopy | 从 ELF 生成 BIN |
# | OBJDUMP | arm-none-eabi-objdump | 查看反汇编 |
# | READELF | arm-none-eabi-readelf | 查看 ELF 信息 |
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump
READELF := $(CROSS_COMPILE)readelf

CPU_FLAGS := -mcpu=cortex-a7 -marm -mfloat-abi=soft
CFLAGS := $(CPU_FLAGS) -std=c11 -ffreestanding -fno-builtin \
	-fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables \
	-O0 -g -Wall -Wextra -Werror -Iinc
ASFLAGS := $(CPU_FLAGS) -g
LDFLAGS := $(CPU_FLAGS) -nostdlib -Wl,-T,linker.ld \
	-Wl,-Map,$(BUILD_DIR)/$(TARGET).map -Wl,--build-id=none

C_SRCS := user/main.c src/gpio.c
S_SRCS := start/start.S
OBJS := $(addprefix $(BUILD_DIR)/,$(C_SRCS:.c=.o) $(S_SRCS:.S=.o))
# .PHONY 表示把右边这些名称声明为伪目标。
.PHONY: all clean inspect

all: $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).dis

$(BUILD_DIR)/$(TARGET).elf: $(OBJS) linker.ld
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/$(TARGET).dis: $(BUILD_DIR)/$(TARGET).elf
	$(OBJDUMP) -d -S $< > $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

inspect: $(BUILD_DIR)/$(TARGET).elf
	$(READELF) -h -S -s $<

clean:
	rm -rf $(BUILD_DIR)
