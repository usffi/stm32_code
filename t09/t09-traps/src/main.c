#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <limits.h>

/* ===== 改这个数字，跑不同实验 ===== */
#define TEST  5       /* 0=正常 1=对齐 2=越界 3=栈爆(RAM外) 4=野指针 5=u8死循环 6=volatile 7=栈2KB对照 */

void clock_72mhz(void)
{
    RCC->CR |= (1U << 16);
    while (!(RCC->CR & (1U << 17))) { }
    FLASH->ACR = (FLASH->ACR & ~0x7U) | 0x2U;
    RCC->CFGR |=  (0x7U << 18);
    RCC->CFGR |=  (1U << 16);
    RCC->CFGR &= ~(1U << 17);
    RCC->CFGR |=  (0x4U << 8);
    RCC->CR |= (1U << 24);
    while (!(RCC->CR & (1U << 25))) { }
    RCC->CFGR = (RCC->CFGR & ~0x3U) | 0x2U;
    while (((RCC->CFGR >> 2) & 0x3U) != 0x2U) { }
    SystemCoreClockUpdate();
}

void SysTick_Handler(void) { HAL_IncTick(); }

static void led_init(void)
{
    RCC->APB2ENR |= (1U << 4);
    (void)RCC->APB2ENR;
    GPIOC->CRH &= ~(0xFU << 20);
    GPIOC->CRH |=  (0x2U << 20);
    GPIOC->BSRR = (1U << 13);
}
static void led_on(void)  { GPIOC->BSRR = (1U << (13 + 16)); }
static void led_off(void) { GPIOC->BSRR = (1U << 13); }

/* 进 HardFault 就快闪（不用 HAL_Delay，避免依赖 SysTick） */
void HardFault_Handler(void)
{
    for (;;) {
        led_on();
        for (volatile uint32_t i = 0; i < 720000; i++) { }
        led_off();
        for (volatile uint32_t i = 0; i < 720000; i++) { }
    }
}

/* ① 结构体对齐：结果存全局，调试器里看 */
struct A { char a; int b; char c; };
struct B { int b; char a; char c; };
struct __attribute__((packed)) C { char a; int b; char c; };

_Static_assert(sizeof(struct A) == 12, "A 应该是 12");
_Static_assert(sizeof(struct B) == 8,  "B 应该是 8");
_Static_assert(sizeof(struct C) == 6,  "C 应该是 6");
_Static_assert(offsetof(struct A, b) == 4, "A.b 在 4");
_Static_assert(offsetof(struct A, c) == 8, "A.c 在 8");
_Static_assert(offsetof(struct C, b) == 1, "C.b 在 1");




volatile uint32_t g_size_a, g_size_b, g_off_b, g_off_c;
volatile int32_t  g_packed, g_aligned;   /* 让 get_* 的结果有地方去，不然整段被删 */

/* ② 越界：结构体内部布局是有保证的，buf[4..7] 必然踩到 guard */
struct box { uint8_t buf[4]; uint32_t guard; };
__attribute__((used)) static struct box g_box = { {0}, 0x11223344U };

/* ③ 栈：一个函数吃掉 21000 字节（比整片 RAM 还大，必然走出 SRAM） */
__attribute__((noinline, used)) static void stack_hog(void)
{
    volatile uint8_t big[21000];
    big[0] = 1;
    big[20999] = 2;
    (void)big;
}

/* ③b 对照：2000 字节看起来很多，其实完全装得下 */
__attribute__((noinline, used)) static void stack_frame_demo(void)
{
    volatile uint8_t big[2000];
    big[0] = 1;
    big[1999] = 2;
    (void)big;
}

__attribute__((noinline)) int get_packed(struct C *p)  { return p->b; } 
__attribute__((noinline)) int get_aligned(struct B *p) { return p->b; }

void loop_plain(void) { for (int i = 0; i < 1000; i++) { } }
void loop_vol(void)   { for (volatile int i = 0; i < 1000; i++) { } }

//int *make(void) { int x = 42; return &x; }

int main(void)
{
    clock_72mhz();
    HAL_Init();
    led_init();
    // SCnSCB->ACTLR |= SCnSCB_ACTLR_DISDEFWBUF_Msk; 
    // struct B b1 = { 0x11, 0x11, 0x66 };
    // struct C c1 = {  0x11, 0x22334455, 0x66 };
    // g_packed  = get_packed(&c1);    /* packed：b 在偏移 1，非对齐读 */
    // g_aligned = get_aligned(&b1);   /* 对齐：b 在偏移 0 */
    // loop_plain();
    // loop_vol();
    int x = INT_MAX;
    printf("%d\n", x + 1);          /* 硬件绕回 */
    printf("%d\n", (x + 1) > x);    /* 编译器说"恒为真" */

#if TEST == 1
    g_size_a = sizeof(struct A);
    g_size_b = sizeof(struct B);
    g_off_b  = offsetof(struct A, b);
    g_off_c  = offsetof(struct A, c);
#elif TEST == 2
    for (int i = 0; i < 8; i++) g_box.buf[i] = 0xAA;
#elif TEST == 3
    stack_hog();
#elif TEST == 4
    volatile uint32_t *p = (volatile uint32_t *)0x00000001U;
    *p = 5;
#elif TEST == 5
    for (uint8_t i = 0; i <= 255; i++) { led_on(); }
#elif TEST == 6
    for (int i = 0; i < 1000; i++) { }      /* 会被优化掉 */
#elif TEST == 7
    stack_frame_demo();
#endif

    while (1) {
        led_on();  HAL_Delay(500);
        led_off(); HAL_Delay(500);
    }
}
