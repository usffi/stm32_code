#include "usart.h"

// 初始化串口1函数
void uart_init_my(void)
{
    // 使能 GPIOA 和 USART1 时钟
    RCC->APB2ENR |= (1U << 2);    /* GPIOA 时钟 */
    RCC->APB2ENR |= (1U << 14);   /* USART1 时钟 */
    (void)RCC->APB2ENR;           /* 读回屏障，等时钟真正生效 */

    // 配置引脚
   /* PA9: TX 复用推挽输出 50MHz —— CNF=10, MODE=11 → 0xB，偏移 (9-8)*4 = 4 */
    GPIOA->CRH &= ~(0xFU << 4);
    GPIOA->CRH |=  (0xBU << 4); 

    /* PA10:RX 上拉输入 —— CNF=10, MODE=00 → 0x8，偏移 (10-8)*4 = 8 */
    GPIOA->CRH &= ~(0xFU << 8);
    GPIOA->CRH |=  (0x8U << 8);
    GPIOA->ODR |=  (1U << 10);    /* CNF=10 时，ODR=1 才是上拉，=0 是下拉 */

    /*
        配置波特率
        USARTDIV = 72MHz / (16 × 115200) = 39.0625
        整数部分 39 = 0x27，小数部分 0.0625 × 16 = 1
        BRR = (39 << 4) | 1 = 0x271
    */ 
    USART1->BRR = 0x271;
    USART1->CR1 |= (1U << 13) | (1U << 3) | (1U << 2);

    // 使能RXNE接收非空中断，当缓冲区有1个字节时，产生中断。
    USART1->CR1 |= (1U << 5);   
    // 使能USART1_IRQn,允许 CPU 响应 USART1 的中断请求
    NVIC->ISER[1] |= (1U << 5);

}


