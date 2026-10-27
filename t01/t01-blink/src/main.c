#include "stm32f1xx_hal.h"

void SysTick_Handler(void)
{
    HAL_IncTick();
}

int main(void)
{
    // 库初始化

    uint16_t laytime=0;
    HAL_Init();
 // SysTick_Config(SystemCoreClock / 1000);
    
    // 要用到pc13引脚，所以要在寄存器中使能gpioc的时钟
    __HAL_RCC_GPIOC_CLK_ENABLE();
    // 配置初始化引脚的结构体
    GPIO_InitTypeDef gp_13=
    {
        .Pin=GPIO_PIN_13,
        .Mode=GPIO_MODE_OUTPUT_PP,
        .Pull=GPIO_NOPULL,
        .Speed=GPIO_SPEED_FREQ_LOW
    };
    // 配置端口和引脚
    HAL_GPIO_Init(GPIOC,&gp_13);

    while(1)
    {
        // 设置端口和引脚电平
        for(laytime=10;laytime<=1000;laytime+=100)
        {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
            HAL_Delay(laytime);
            HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_SET);
            HAL_Delay(laytime);
        }

    }
    //HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_SET);

}

