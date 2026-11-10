/* align_demo.c —— 只用来"看"结构体布局，不参与固件编译
 *
 * 用法（Git Bash）：
 *   TC=/c/Users/a3331/.platformio/packages/toolchain-gccarmnoneeabi/bin
 *   $TC/arm-none-eabi-gcc.exe -mcpu=cortex-m3 -mthumb -c align_demo.c -o /tmp/a.o -Wpadded
 *
 * -Wpadded 会逐条告诉你：哪个成员前面塞了填充、末尾补齐了几字节
 */
#include <stdint.h>
#include <stddef.h>

struct A { char a; int b; char c; };   /* 小-大-小 */
struct B { int  b; char a; char c; };  /* 大-小-小 */
struct __attribute__((packed)) C { char a; int b; char c; };

_Static_assert(sizeof(struct A) == 12, "A should be 12");
_Static_assert(sizeof(struct B) == 8,  "B should be 8");
_Static_assert(sizeof(struct C) == 6,  "C should be 6");
_Static_assert(offsetof(struct A, b) == 4, "A.b should be at 4");
_Static_assert(offsetof(struct A, c) == 8, "A.c should be at 8");
_Static_assert(offsetof(struct C, b) == 1, "C.b should be at 1");

/* 让编译器真的去访问这些成员，反汇编里才看得到偏移 */
int get_a(struct A *p) { return p->b + p->c; }
int get_b(struct B *p) { return p->b + p->a; }
int get_c(struct C *p) { return p->b + p->c; }
