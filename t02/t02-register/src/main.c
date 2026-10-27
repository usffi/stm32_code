#include "stm32f1xx_hal.h"

// 计时中断处理函数，使全局计时+1ms
void SysTick_Handler(void)
{
    HAL_IncTick();
}

int main(void)
{
    // 库初始化
    HAL_Init();

    // 手动开启外设GPIOC时钟,GPIOC的掩码
    RCC->APB2ENR |= (1U<<4);
    (void)RCC->APB2ENR;      // 手动读回，开启缓冲，确保启动成功

    // 配置pc13的模式，高位，用CHR来配置，四位掩码，偏移是(13-8)*4=20
    // 先清零后再配置模式，推挽输出，mode是10，2MHz，cnf是00，0010即0x2U
    GPIOC->CRH &= ~(0xFU<<20);
    GPIOC->CRH |=(0x2U<<20);

    while(1)
    {
        // 拉低电平，BSRR高位置1，pc13要偏移，13+16，一位掩码进行操作
        GPIOC->BSRR = (1U<<(13+16));
        HAL_Delay(500);

        // 拉高电平，BSRR低位置1，pc13无需偏移，13，一位掩码进行操作
        GPIOC->BSRR=(1U<<13);
        HAL_Delay(500);

    }


   

}

