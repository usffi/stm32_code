#include "stm32f1xx_hal.h"

// 一位掩码操作，设定某位为1
#define BIT_SET(reg, n)      ((reg) |=  (1U << (n)))  
//  清除某位为0  
#define BIT_CLR(reg, n)      ((reg) &= ~(1U << (n)))  
//  反转某位  
#define BIT_TOGGLE(reg, n)   ((reg) ^=  (1U << (n)))   
//  读取某位的状态 
#define BIT_GET(reg, n)      (((reg) >> (n)) & 1U)      
// 错误验证版本，不带括号，优先级存在问题
#define BIT_GET_BAD(reg, n)   reg & (1U << (n))

#define SQUARE(x)  ((x) * (x))

#define MAX(a,b)  ((a) > (b) ? (a) : (b))

#define BLINK()  do { GPIOC->BSRR = (1U << (13 + 16)); } while (0)   /* 拉低 PC13，点亮 */

#define BLINK_BAD()  { GPIOC->BSRR = (1U << (13 + 16)); }

#define CHECK()  if (x) x = 9
// 计时中断处理函数，使全局计时+1ms
void SysTick_Handler(void)
{
    HAL_IncTick();
}

int main(void)
{
    // 库初始化
    HAL_Init();

    // uint32_t x = 0;
    // if (BIT_GET_BAD(x, 3) == 0) 
    // {
    //      /* 本意：bit3 为 0 时进来 */
    //     while(1);
    // }
    // int i = 3,j=1;
    // int r = SQUARE(i++);
    // int r1 = MAX(i++, j++);

    // 手动开启外设GPIOC时钟,GPIOC的掩码
 //   RCC->APB2ENR |= (1U<<4);
  //  (void)RCC->APB2ENR;      // 手动读回，开启缓冲，确保启动成功
    // uint32_t a = 1  << 31;   /* 有符号 int 左移 31 位 */
    // uint32_t b = 1U << 31;   /* 无符号 */
    // uint32_t c = 1U << 32;   /* 移了 32 位 */   

    uint8_t flag=0U,a=1U,b=1U, x =1U; 
    if (a)
        CHECK();
    else
        other();

    BIT_SET(RCC->APB2ENR,4);
    BIT_GET(RCC->APB2ENR,4);

    // 配置pc13的模式，高位，用CHR来配置，四位掩码，偏移是(13-8)*4=20
    // 先清零后再配置模式，推挽输出，mode是10，2MHz，cnf是00，0010即0x2U
    GPIOC->CRH &= ~(0xFU<<20);
    GPIOC->CRH |=(0x2U<<20);

   // BIT_CLR(GPIOC->CRH,20|21|22|23);
   // BIT_SET(GPIOC->CRH,21);




    while(1)
    {
        // 拉低电平，BSRR高位置1，pc13要偏移，13+16，一位掩码进行操作
        //GPIOC->BSRR = (1U<<(13+16));
        //BIT_SET(GPIOC->BSRR,(13+16));
        //HAL_Delay(500);


        // 拉高电平，BSRR低位置1，pc13无需偏移，13，一位掩码进行操作
        //GPIOC->BSRR=(1U<<13);
        //HAL_Delay(500);
        //BIT_TOGGLE(GPIOC->ODR,13);
        HAL_Delay(500);
        

    }


   

}

