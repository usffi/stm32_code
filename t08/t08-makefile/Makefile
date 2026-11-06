# ---- 工具链（改成你的实际路径）----
TC      = C:/Users/a3331/.platformio/packages/toolchain-gccarmnoneeabi/bin
CC      = $(TC)/arm-none-eabi-gcc
OBJCOPY = $(TC)/arm-none-eabi-objcopy
SIZE    = $(TC)/arm-none-eabi-size

# ---- 选项 ----
CFLAGS  = -mcpu=cortex-m3 -mthumb -O2 -Wall -Isrc
LDFLAGS = -T stm32f103x8.ld --specs=nano.specs --specs=nosys.specs -nostartfiles

# ---- 文件列表 ----
SRCS = src/main.c src/delay.c
OBJS = $(SRCS:.c=.o) startup.o

# ---- 默认目标（放在第一个）----
all: firmware.bin

# 链接：把所有 .o 拼成 elf
firmware.elf: $(OBJS)
	$(CC) -mcpu=cortex-m3 -mthumb $(LDFLAGS) $^ -o $@

# 转 bin 并打印大小
firmware.bin: firmware.elf
	$(OBJCOPY) -O binary $< $@
	$(SIZE) $<

# 模式规则：任何 .o 都由同名 .c 生成
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 启动文件是汇编
startup.o: startup_stm32f103xb.s
	$(CC) -mcpu=cortex-m3 -mthumb -c $< -o $@

# 清理
clean:
	rm -f $(OBJS) firmware.elf firmware.bin

.PHONY: all clean

# ---- 靠 -MMD -MP 自动生成头文件依赖，解决"改头文件不重编" ----
CFLAGS += -MMD -MP
-include $(SRCS:.c=.d)