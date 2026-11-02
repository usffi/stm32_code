#include "stm32f1xx_hal.h"
#include "ringbuf.h"
#include "usart.h"

static uint8_t rx_raw[128];
static ringbuf_t rx_rb;
static volatile uint8_t g_ore_count = 0;   /* 溢出错误计数 */
static volatile uint8_t g_drop_count = 0;  /* 丢弃计数 */

void clock_72mhz(void)
{
    RCC->CR |= (1U << 16);                        /* HSEON：开外部 8MHz 晶振 */
    while (!(RCC->CR & (1U << 17))) { }           /* 等 HSERDY */

    FLASH->ACR = (FLASH->ACR & ~0x7U) | 0x2U;     /* 72MHz 需要 2 个 Flash 等待周期 */

    RCC->CFGR |=  (0x7U << 18);                   /* PLLMUL = ×9 → 8×9 = 72MHz */
    RCC->CFGR |=  (1U << 16);                     /* PLLSRC = 1 → 用 HSE（F1 里 1 才是 HSE，0 是 HSI/2） */
    RCC->CFGR &= ~(1U << 17);                     /* PLLXTPRE = 0 → HSE 不分频 */
    RCC->CFGR |=  (0x4U << 8);                    /* PPRE1 = 100 → APB1 = 36MHz（上限就是 36） */

    RCC->CR |= (1U << 24);                        /* PLLON */
    while (!(RCC->CR & (1U << 25))) { }           /* 等 PLLRDY */

    RCC->CFGR = (RCC->CFGR & ~0x3U) | 0x2U;       /* SW = 10 → 切到 PLL */
    while (((RCC->CFGR >> 2) & 0x3U) != 0x2U) { } /* 等 SWS 确认切换完成 */

    SystemCoreClockUpdate();                      /* 让 HAL 的 SystemCoreClock 也更新 */
}


// 计时中断处理函数，使全局计时+1ms
void SysTick_Handler(void)
{
    HAL_IncTick();
}

// USART1 中断服务函数，当NVIC接收中断请求后，CPU会自动跳转到该函数执行
void USART1_IRQHandler(void)
{
    uint32_t sr = USART1->SR;   /* 先读 SR 寄存器，清除中断标志位,SR是快照，保存当时的状态 */
    uint8_t  dr = USART1->DR;   /* 再读 DR 寄存器，清 RXNE/ORE/NE/FE 的完整序列  */

    if (sr & (1U << 3))          /* ORE 溢出错误 */
    {
        g_ore_count++;
    }
    if(sr & (1U << 5))          /* RXNE 接收非空中断 */
    {
        if(!ringbuf_write(&rx_rb, dr))   /* 写入环形缓冲区 */
        {
            g_drop_count++;
        }
    }
}



int main(void)
{
    clock_72mhz();
    // 库初始化
    HAL_Init();
    ringbuf_init(&rx_rb, rx_raw, 128);
    uart_init_my(); 
    

    while(1)
    {
        uint8_t byte;
        while (ringbuf_read(&rx_rb, &byte))
        {
            while (!(USART1->SR & (1U << 7)))
            {

            }
            // CPU写数据到DR寄存器，硬件会发送回外部设备中
            USART1->DR = byte;
            
        }
    }

}

