#include "stm32f1xx_hal.h"
#include "ringbuf.h"
#include "cmd.h"
#include <string.h>

UART_HandleTypeDef huart1;

static uint8_t   g_rx_byte;
static uint8_t   g_rb_buf[128];
static ringbuf_t g_rb;

/* ===== 时钟 / LED / 串口：和前面几包一样，不用改 ===== */

void clock_72mhz(void)
{
    RCC->CR |= (1U << 16);
    while (!(RCC->CR & (1U << 17))) { }
    FLASH->ACR = (FLASH->ACR & ~0x7U) | 0x2U;
    RCC->CFGR |=  (0x7U << 18);
    RCC->CFGR |=  (1U << 16);
    RCC->CFGR &= ~(1U << 17);
    RCC->CFGR |=  (0x4U << 8);
    RCC->CR |= (1U << 24);
    while (!(RCC->CR & (1U << 25))) { }
    RCC->CFGR = (RCC->CFGR & ~0x3U) | 0x2U;
    while (((RCC->CFGR >> 2) & 0x3U) != 0x2U) { }
    SystemCoreClockUpdate();
}

static void led_init(void)
{
    RCC->APB2ENR |= (1U << 4);
    (void)RCC->APB2ENR;
    GPIOC->CRH &= ~(0xFU << 20);
    GPIOC->CRH |=  (0x2U << 20);
    GPIOC->BSRR =  (1U << 13);
}
static void led_on(void)  { GPIOC->BSRR = (1U << (13 + 16)); }
static void led_off(void) { GPIOC->BSRR = (1U << 13); }

/* 往串口打一串字符（阻塞式，只在主循环里用） */
void con_puts(const char *s)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)s, (uint16_t)strlen(s), 100);
}

static void usart1_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    // 使能 USART1 和 GPIOA 时钟
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // 配置 GPIOA9 为复用推挽输出（TX），GPIOA10 为输入上拉（RX）
    gpio.Pin   = GPIO_PIN_9;            /* TX */
    gpio.Mode  = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin  = GPIO_PIN_10;            /* RX */
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &gpio);

    // 配置 USART1 参数
    huart1.Instance          = USART1;                  // 串口
    huart1.Init.BaudRate     = 115200;                  // 波特率
    huart1.Init.WordLength   = UART_WORDLENGTH_8B;      // 数据位
    huart1.Init.StopBits     = UART_STOPBITS_1;         // 结束位
    huart1.Init.Parity       = UART_PARITY_NONE;        // 校验位
    huart1.Init.Mode         = UART_MODE_TX_RX;         // 收发模式
    huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;     // 硬件流控
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;    // 过采样
    HAL_UART_Init(&huart1);

    // 配置 USART1 中断优先级并使能中断
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
}

/* ★ 这一行不能漏：没有它，SysTick 中断会掉进 startup 里的 Default_Handler 死循环，
   表现为"串口发到一半就不动了"。T01 踩过一次，T10 又踩了一次。 */
// void SysTick_Handler(void)
// {
//     HAL_IncTick();
// }

void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}

/* 收到一个字节：丢进 ringbuf，然后重新武装中断 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *h)
{
    if (h->Instance == USART1) {
        ringbuf_write(&g_rb, g_rx_byte);
        HAL_UART_Receive_IT(&huart1, &g_rx_byte, 1);
    }
}

/* 溢出错误：不清 ORE 就再也收不到数据了（T06 踩过） */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *h)
{
    if (h->Instance == USART1) {
        __HAL_UART_CLEAR_OREFLAG(h);
        HAL_UART_Receive_IT(&huart1, &g_rx_byte, 1);
    }
}

/* ===== 三个命令 ===== */

static void cmd_help(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    con_puts("commands:\r\n");
    cmd_list();
}

static void cmd_led(int argc, char **argv)
{
    if (argc < 2) {
        con_puts("usage: led on|off\r\n");
        return;
    }
    if (strcmp(argv[1], "on") == 0) {
        led_on();
        con_puts("led on\r\n");
    } else if (strcmp(argv[1], "off") == 0) {
        led_off();
        con_puts("led off\r\n");
    } else {
        con_puts("usage: led on|off\r\n");
    }
}

static void cmd_echo(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        con_puts(argv[i]);
        if (i + 1 < argc) {
            con_puts(" ");
        }
    }
    con_puts("\r\n");
}

typedef struct demo_node
{
    const char *text;           // 结点内容
    struct demo_node *next;     // 指向下一个结点
}demo_node_t;

demo_node_t n1 = { "first", NULL };
demo_node_t n2 = { "second", NULL };


/* ===== 主循环：从 ringbuf 捡字符，攒够一行就交给 cmd_exec ===== */

int main(void)
{
    // 把时钟拉到72MHz，并且进行相应的初始化
    clock_72mhz();
    HAL_Init();
    led_init();
    // 出事还缓冲区
    ringbuf_init(&g_rb, g_rb_buf, (uint16_t)sizeof(g_rb_buf));

    // 初始化命名静态区
    cmd_init();
    // 注册三条命名和对应的调用函数
    cmd_register("help", cmd_help);
    cmd_register("led",  cmd_led);
    cmd_register("echo", cmd_echo);
    


    // 串口初始化   
    usart1_init();
    HAL_UART_Receive_IT(&huart1, &g_rx_byte, 1);

    con_puts("\r\nT10 cmd shell ready\r\n"
             "type 'help' to list commands\r\n> ");

    char line[64];
    int  len = 0;



    while (1) {
        uint8_t b;
        // 缓冲区没有数据，一直循环
        if (!ringbuf_read(&g_rb, &b)) {
            continue;
        }

        if (b == '\r' || b == '\n') 
        {
            if (len > 0) {
                line[len] = '\0';
                con_puts("\r\n");
                if (!cmd_exec(line)) {
                    con_puts("unknown command: ");
                    con_puts(line);
                    con_puts("\r\n");
                }
                len = 0;
            }
            con_puts("> ");
        } else if (b == 0x08 || b == 0x7F) {     /* 退格 */
            if (len > 0) {
                len--;
                con_puts("\b \b");
            }
        } else if (len < (int)sizeof(line) - 1) {
            line[len++] = (char)b;
            char t[2] = { (char)b, '\0' };
            con_puts(t);                          /* 回显 */
        }
    }
}
