#include "delay.h"
void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++)
        for (volatile uint32_t j = 0; j < LOOPS_PER_MS; j++) { }
}
