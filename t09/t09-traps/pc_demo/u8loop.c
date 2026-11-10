/* T09 · 顺手搞坏 X26：uint8_t 自增到 255 之后不会停下来
 * 纯 PC 侧演示，不进固件。编译方式见任务卡步骤 5「顺手搞坏」。
 *
 * 为什么加 volatile sink：不加的话 -O2 会把整个空循环删掉，
 * 警告照样有，但反汇编里什么都看不到。
 */
#include <stdint.h>

volatile uint8_t sink;

void u8loop(void)
{
    for (uint8_t i = 0; i <= 255; i++) {
        sink = i;
    }
}
