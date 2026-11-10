# STM32 学习工程

从 GPIO 到电机闭环的嵌入式学习记录。目标：嵌入式软件 / 端侧部署岗位。

> **一块 STM32 + 一台电机 + 一个编码器**

## 环境

| 项 | 内容 |
| --- | --- |
| 开发板 | STM32F103C8T6（蓝板 / 江科大套件） |
| 核心 | Cortex-M3 / 72MHz / 64KB Flash / 20KB RAM |
| 晶振 | 8MHz（PLL ×9 = 72MHz，**需手动配置**，默认是 HSI 8MHz） |
| 编辑器 | VSCode + PlatformIO |
| 框架 | STM32Cube（HAL）；GPIO 和 USART 部分手写寄存器 |
| 调试器 | ST-Link V2 |
| 辅助工具 | 逻辑分析仪 Kingst LA1010（抓 UART 波形、量位宽） |

## 怎么跑起来

1. VSCode 安装 **PlatformIO IDE** 扩展
2. **打开单个工程目录**（不是仓库根目录）：
   `文件 → 打开文件夹 → t06/t06_UART`
3. 底部状态栏点 **→（Upload）** 编译并烧录；点 **✓（Build）** 只编译
4. 串口实验需要 USB-TTL：模块 RX → 板子 **PA9(TX)**，模块 TX → 板子 **PA10(RX)**，GND 共地

> ⚠️ 每个 `tXX-xxx` 是独立工程，必须单独打开。在仓库根目录打开会找不到 `platformio.ini`。

## 目录结构

```
stm32_code/
├── README.md          本文件
├── LOG.md             学习日志：做了什么 / 卡在哪 / 得到什么数字
├── notes/             每包一份复习卡（自测题 + 折叠答案 + 错点清单）
├── t01/t01-blink/     GPIO 点灯（HAL 版）
├── t02/t02-register/  纯寄存器点灯
├── t03/t03-neicun/    指针与内存布局（FakeGPIO + 函数指针状态机）
├── t04/t04-hong/      位操作与寄存器宏
├── t05/t05-ringbuf/   环形缓冲区 + 四个白盒测试
├── t06/t06_UART/      串口收发（寄存器版）+ ringbuf 中断接收回显
├── t08/t08-makefile/  手写 Makefile：编译 → 链接 → objcopy（不依赖 PlatformIO）
└── t09/t09-traps/     内存陷阱：对齐 / 越界 / 栈爆 / 野指针 / UB（改 `#define TEST` 切 8 个实验）
```

## 进度

| 编号 | 内容 | 核心产出 | 状态 |
| --- | --- | --- | --- |
| T01 | PlatformIO 环境 + 点亮 PC13 | HAL 版闪烁 | ✅ |
| T02 | 寄存器 vs HAL | 696 B 覆盖 1208 B（省 42%） | ✅ |
| T03 | 指针与内存布局 | `.data` / `.bss` / 栈 三段对账 | ✅ |
| T04 | 位操作与寄存器宏 | 四个宏 + 反汇编对照 | ✅ |
| T05 | 环形缓冲区（上） | `ringbuf.c` + 4 个白盒测试 | ✅ |
| T06 | 环形缓冲区（下）+ 串口 | 115200 回显 + 逻辑分析仪验证 | ✅ |
| T07 | Git 规范与仓库建设 | 两种合并形态 + 冲突处理 + 10 条提交体检 | ✅ |
| T08 | Makefile：看清构建过程 | 手写构建链 + `-MMD` 头文件依赖 | ✅ |
| T09 | 内存陷阱与未定义行为 | 真实栈 **20.4 KB** 实测 + HardFault 栈帧定位 | ✅ |
| T10 | 数据结构：手写链表 | — | — |
| T11 | 数据结构：队列与栈 | — | — |
| T12 | 阶段一小结 + 模拟面试 | — | — |

## 固件占用（2026-11-10 实测）

| 工程 | Flash（text + data） | RAM（含堆栈预留） |
| --- | --- | --- |
| t01-blink | 1484 B（2.3%） | 1592 B（7.8%） |
| t02-register | 972 B（1.5%） | 1592 B（7.8%） |
| t03-neicun | 1100 B（1.7%） | 1592 B（7.8%） |
| t04-hong | 952 B（1.5%） | 1592 B（7.8%） |
| t05-ringbuf | 1432 B（2.2%） | 2128 B（10.4%） |
| t06_UART | 1424 B（2.2%） | 1736 B（8.5%） |
| t08-makefile | 564 B（0.9%） | 0 B（0%）\* |
| t09-traps（TEST=4） | 5152 B（7.9%） | 1704 B（8.3%） |

> 口径：`arm-none-eabi-size`，Flash = text + data，RAM = data + bss。
> RAM 里含链接器预留的 **1540 B**（1KB 栈 + 512B 堆），所以比变量实际占用量大。
> 分母：64KB Flash / 20KB RAM。
> \* `t08-makefile` 用的是 platform 自带的 `stm32f103x8.ld`，栈段是 `.stack_dummy (COPY)`——**不预留空间**，所以 `size` 报 bss = 0；实际栈仍从 `0x20005000` 往下长，只是没有静态预留段。

## 踩过的坑

完整记录在 [LOG.md](LOG.md)，这里只列会反复遇到的：

| 现象 | 原因 |
| --- | --- |
| `HAL_Delay` 卡死不返回 | 手写工程缺 `SysTick_Handler()`，`HAL_IncTick()` 没被调用 |
| 配了一位，八个引脚全乱 | 单 bit 宏的参数是"第几位"，不是掩码；四位字段要 `&= ~(0xFU << n)` |
| 写 BSRR 毫无反应 | BSRR 只写、读回恒 0，用 `\|=` 等于写 0，必须直接赋值 |
| 灯亮但模式不对 | PC13 被配成开漏而非推挽——`20\|21\|22\|23` 按位或等于 **23** |
| 时钟不是 72MHz | PlatformIO 的 `SystemInit()` 是空函数，默认 HSI 8MHz |
| 串口全是乱码、字节数还翻倍 | `PLLSRC` 极性写反，**F1 里 bit16=1 才是 HSE**，实际跑 36MHz → 波特率 57600 |
| 延时看着正常，通信全错 | `HAL_Delay` 按 `SystemCoreClock` 算，时钟错了它也跟着错——**延时对时钟不敏感，通信绝对敏感** |
| 改了 `#define`，编译过了但行为没变 | Makefile 里 `.o` 只依赖 `.c`，**`.h` 不在依赖图里**，make 判定"不用重编"且**不报错**。修法：`-MMD -MP` + `-include $(SRCS:.c=.d)` |
| 算栈大小差 4 字节 | `end` 前面有 `. = ALIGN(8)`，`__bss_end__` 只满足 4 对齐；那 4 字节是链接器补的 `*fill*`，不属于任何变量 |
| `nm` 里搜不到 `end` | `PROVIDE(end = .)` 只在**有人引用**时才落地。没有 `malloc` 就没这个符号，但地址照样参与计算 |
| `size` 显示 bss 1.5 KB，实际变量只有几十字节 | 默认输出的 `bss` 列把 `._user_heap_stack` 的预留算进去了，要看真实分段得用 `size -A` |
| HardFault 停在 `while(1)` 而不是出错那行 | 写未映射地址是 **imprecise bus fault**，写缓冲让错误延后一条总线事务才报，压栈 PC 指向下一条指令 |
| 调试器里 `main` 的地址和 `nm` 对不上 | 板子里的固件 ≠ 调试器加载的符号（用了 skip Pre-Debug / 没重烧 / 会话开着又编译）。差几百字节先查这个 |
| 断点打不上、反汇编没这一行 | `_Static_assert`、`__attribute__` 这类是**编译期**的东西，不产生指令；死代码被 `-O2` 整段删掉也一样 |

## 约定

- 每包完成时：代码 + notes 复习卡 + LOG.md 条目**一起提交**
- commit 格式：`type(scope): 说明`，type 取 `feat` / `fix` / `docs` / `refactor` / `chore` / `test`
- 反汇编快照（`disasm*.txt`）是导出文件，改了代码就过期，已在 `.gitignore` 排除
- 构建产物 `.pio/` 不进仓库（体积大、机器相关、每次编译都变）
