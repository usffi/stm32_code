#include <stdint.h>
#include "delay.h"

#define GPIOC_CRH   (*(volatile uint32_t *)0x40011004)
#define GPIOC_BSRR  (*(volatile uint32_t *)0x40011010)
#define RCC_APB2ENR (*(volatile uint32_t *)0x40021018)

/* 启动文件会调用它，这里留空（时钟保持 HSI 8MHz） */
void SystemInit(void) { }

/* newlib 的 __libc_init_array 会调用它（C++ 全局构造用），C 程序给个空桩 */
void _init(void) { }

int main(void)
{
    RCC_APB2ENR |= (1U << 4);
    (void)RCC_APB2ENR;
    GPIOC_CRH &= ~(0xFU << 20);
    GPIOC_CRH |=  (0x2U << 20);

    while (1) {
        GPIOC_BSRR = (1U << 13);
        delay_ms(500);
        GPIOC_BSRR = (1U << (13 + 16));
        delay_ms(500);
    }
}
