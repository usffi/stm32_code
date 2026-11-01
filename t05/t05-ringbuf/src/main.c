#include "stm32f1xx_hal.h"
#include "ringbuf.h"


void SysTick_Handler(void)
{
    HAL_IncTick();
}

int main(void)
{
    // 库初始化
    HAL_Init();
    RCC->APB2ENR |= (0x1U << 4);
    (void)RCC->APB2ENR;

    GPIOC->CRH &= ~(0xFU << 20);
    GPIOC->CRH |= (0x2U << 20);


    static uint8_t rx_raw[128];
    static ringbuf_t rx_rb;
    ringbuf_init(&rx_rb, rx_raw, 128);


    /*
        测试一：写满 128 个，再读 128 个，值要一一对应
    */
    bool flag_1 =1;
    bool flag_3=0;
    static uint8_t arr_d[200],arr_r[200];
    int test1_w=0;
    for(int i=0; i<=127; i++)
    {
        if(ringbuf_write(&rx_rb,test1_w))
        {
            arr_d[test1_w]=test1_w;
            test1_w++;
        }
    }
    /*
        测试 3：满的时候写，要返回 false，不能覆盖旧数据
    */
    if(ringbuf_freesize(&rx_rb)==0)
    {
        if(ringbuf_write(&rx_rb,127)==false)
        {
            flag_3=1;
        }
    }

    uint8_t test1_r;
    int n=0;
    for(int j=0; j<=127;j++)
    {
        if(ringbuf_read(&rx_rb,&test1_r))
        {
            arr_r[n++]=test1_r;
        }
    }
    for(int z=0;z<127;z++)
    {
        if(arr_d[z]!=arr_r[z])
        {
            flag_1=0;
        }
    }

    /*
        测试 2：空的时候读，要返回 false，不能返回垃圾
    */
    bool flag_2=0;
    uint8_t test2_r;
    if(rx_rb.head == rx_rb.tail)
    {
        if(ringbuf_read(&rx_rb,&test2_r)==false)
        {
            flag_2=1;
        }
    }

    /* 测试 4：读写指针跨过 size 边界回绕 */
    bool flag_4=1;
    uint8_t arr4w[5],arr4r[5];
    uint8_t test4_w=0,test4_r;
    // 此时head在127，先写数据回饶到4
    for(int i = 0;i<5;i++)
    {
        if(ringbuf_write(&rx_rb,test4_w))
        {
            arr4w[i]=test4_w;
            test4_w++;
        }
    }
    for(int j=0;j<5;j++)
    {
        if(ringbuf_read(&rx_rb,&test4_r))
        {
            arr4r[j]=test4_r;
        }
    }
    for(int z=0;z<5;z++)
    {
        if(arr4w[z]!=arr4r[z])
        {
            flag_4=0;
        }
    }
    if(rx_rb.head!=4 || (rx_rb.head!=rx_rb.tail))
    {
        flag_4=0;
    }


    



    // // rx_rb.head = (rx_rb.head + 1) % rx_rb.size; 
    // // rx_rb.head = (rx_rb.head + 1) & (rx_rb.size-1); 
    // // rx_rb.head = (rx_rb.head + 1 == rx_rb.size) ? 0 : rx_rb.head + 1;

    // /* 测试 1：连写 128 次，数成功次数 */
    // // int te=0;
    // // for(int i=0; i<=127; i++)
    // // {
    // //     if(ringbuf_write(&rx_rb,te))
    // //         te++;
    // // }

    // // /* 测试 2：连读 128 次，数成功次数 */
    // // int got = 0; 
    // // uint8_t v;
    // // while (ringbuf_read(&rx_rb, &v)) 
    // // {
    // //     got++;
    // // }
        
    // // if( got==127 && te==127)
    // // {
    // //     while(1)
    // //     {
    // //         GPIOC->BSRR = (0x1U << 13);
    // //         HAL_Delay(500);
    // //         GPIOC->BSRR = (0x1U << (13+16));
    // //         HAL_Delay(500);
    // //     }
        
    // // }
    // // else
    // // {
    // //     GPIOC->BSRR = (0x1U << (13+16));
    // // }

    if (flag_1 && flag_2 && flag_3 && flag_4) 
    {
        while (1) 
        {                     /* 全过：慢闪 */
            GPIOC->BSRR = (1U << 13);
            HAL_Delay(500);
            GPIOC->BSRR = (1U << (13 + 16));
            HAL_Delay(500);
        }
    }
    while (1) 
    {                          /* 有失败：常亮 */
        GPIOC->BSRR = (1U << (13 + 16));
    }

   

}

