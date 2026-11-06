#ifndef DELAY_H
#define DELAY_H
#include <stdint.h>

/* 每毫秒空转多少圈。故意放在头文件里——步骤 5 要验证"改 .h 会不会重编 .c" */
#define LOOPS_PER_MS  720

void delay_ms(uint32_t ms);
#endif
