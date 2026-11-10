/* 整型提升：小类型参与运算前先变 int */
#include <stdio.h>
#include <stdint.h>

int main(void)
{
    uint8_t a = 200, b = 100;
    printf("a+b          = %d\n", a + b);              /* 提升成 int */
    printf("(uint8_t)    = %d\n", (uint8_t)(a + b));   /* 赋值才截断 */

    int i = -1; unsigned j = 1;
    printf("-1 < 1u      = %d\n", i < j);              /* i 被转成 unsigned */

    printf("sizeof(a+b)  = %zu\n", sizeof(a + b));
    return 0;
}
