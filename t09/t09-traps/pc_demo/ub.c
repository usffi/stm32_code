/* UB：有符号溢出。硬件绕回，编译器却假设不会发生 */
#include <stdio.h>
#include <limits.h>

int main(void)
{
    int x = INT_MAX;
    int y = x + 1;                     /* 硬件上绕回成 INT_MIN */
    printf("INT_MAX + 1  = %d\n", y);
    printf("x + 1 > x    = %d\n", (x + 1) > x);   /* 编译器：恒为真 */
    return 0;
}
