#include "stm32f1xx_hal.h"

#define REG_SET_BITS(reg, mask)          ((reg) |=  (mask))
#define REG_CLR_BITS(reg, mask)          ((reg) &= ~(mask))
#define REG_WRITE_FIELD(reg, mask, val)  ((reg) = ((reg) & ~(mask)) | ((val) & (mask)))
// static uint8_t  g_buf[16];      /* 全局，初值 0 0x2000 0084  0x2000 0088  RAM .bss中*/
// static uint8_t g_test[16];      /* 全局，初值   0x2000 0098  0x2000 0098  RAM .bss中*/
// static uint32_t g_val = 0x1234; /* 全局，有初值 0x2000 0000   RAM .data中*/
// static uint32_t test = 0x0012;  /* 全局  有初值 0x2000 0004   RAM .data中*/
// static const char msg[] = "hi"; /* 常量        0x0800 10f0   FLASH中*/


// 计时中断处理函数，使全局计时+1ms
void SysTick_Handler(void)
{
    HAL_IncTick();
}

// typedef struct {
//     volatile uint32_t CRL, CRH, IDR,  BSRR, ODR,BRR;
// } FakeGPIO;

// static uint32_t fake_regs[6];                        /* 一片假寄存器 */
// static FakeGPIO *GPIOX = (FakeGPIO *)fake_regs;      /* 把数组当地址用 */

typedef void (*mode_fn)(void);
static void mode_on(void);
static void mode_slow(void);
static void mode_fast(void);
static mode_fn modes[] = { mode_on, mode_slow, mode_fast };

static void mode_on(void)
{
    REG_SET_BITS(GPIOC->BSRR,(0x1U<<13+16));
    //HAL_Delay(3000);
}
static void mode_slow(void)
{
    REG_SET_BITS(GPIOC->BSRR,(0x1U<<(13+16)));
    HAL_Delay(100);
    REG_SET_BITS(GPIOC->BSRR,(0x1U<<13));
    HAL_Delay(300);
}
static void mode_fast(void)
{
    REG_SET_BITS(GPIOC->BSRR,(0x1U<<(13+16)));
    HAL_Delay(100);
    REG_SET_BITS(GPIOC->BSRR,(0x1U<<13));
    HAL_Delay(50); 
}


int main(void)
{
    HAL_Init();
    // int  a = 1;                     // 0x2000 4ff4 RAM栈中
    // char c = 'x';                   // 0x2000 4ff3 RAM栈中
    // uint32_t arr[4] = {1,2,3,4};    // 0x2000 4fe0 RAM栈中

    // g_buf[16]=0xaa;     // 0x20000098 
    // printf("%p  %p %p %p %p %p %p %p\r\n", &a, &c, &arr[0], &g_buf, &g_val, msg,&test,&g_test);
    // printf("%d %d %d %d\r\n", sizeof(int), sizeof(char), sizeof(arr), sizeof(&a));
    // while (1);


    //GPIO_TypeDef
    // GPIOX->CRH &= ~(0xFU << 20);
    // GPIOX->CRH |= (0x2U << 20);
    // GPIOX->BSRR = (1U << 13);
    // printf(" \n");

    // 0x2000008c <fake_regs+12>  0x20000090 <fake_regs+16>
    // 0x2000008c <fake_regs+12>  0x20000090 <fake_regs+16>

    // 打开时钟GPIOC并读回
    REG_SET_BITS(RCC->APB2ENR, (0x1U<<4));
    //RCC->APB2ENR |=(0x1U<<4);
    (void)RCC->APB2ENR;

    // 配置寄存器
    REG_CLR_BITS(GPIOC->CRH, (0xFU<<20)) ;
    // GPIOC->CRH &= ~(0xFU<<20);
    REG_SET_BITS(GPIOC->CRH,(0x2U<<20));
    // GPIOC->CRH |=(0x2U<<20);
    // 电平置位
    uint16_t i=0;
    uint32_t wts=0,wait=3000; // 等待3s
    while(1)
    {
        i=i>2?0:i;
        wts=HAL_GetTick(); // 获取当前时间
        while(HAL_GetTick()-wts<wait)
        {
            modes[i]();
        }
        i=i+1;
    }
    


    //while(1);

}

