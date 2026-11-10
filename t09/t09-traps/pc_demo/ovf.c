/* 越界：往 buf[4..7] 写 4 个字节，刚好盖住 guard */
#include <stdio.h>

struct box { char buf[4]; unsigned guard; };

int main(void)
{
    struct box b;
    b.guard = 0x11223344u;

    printf("写之前 guard = %08X\n", b.guard);

    for (int i = 0; i < 8; i++)
        b.buf[i] = 0xAA;          /* 后 4 个越界 */

    printf("写之后 guard = %08X\n", b.guard);
    return 0;
}
