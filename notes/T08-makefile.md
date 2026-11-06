# T08 · Makefile：看清构建过程

> 日期：2026-11-06 ｜ 对应任务：T08 ｜ 不碰 HAL，纯构建
> 来源：`Desktop/t08.docx`（本人草稿）+ 终端实测输出。**带 ⚠️ 的是原文有误、已修正的地方**；带〔实测〕的是我在你工程上跑出来的，原文没有。
> 复习用法：先做「一」的自测，答不上再往下翻。

---

## 一、自测（遮住答案先答一遍）

1. 编译 / 链接 / objcopy 三步，各自的输入、输出、关键动作是什么？**地址在哪个阶段才定下来**？
2. `-c` 是什么意思？去掉它会怎样？（别答"不编译了"，要答出"它照编不误"）
3. `-T xxx.ld` 除了"摆位置"，还提供了什么？去掉它报的 `undefined reference to '_sdata'` 是从哪来的？
4. `-nostartfiles` 去掉会报什么错？为什么工程里必须有 `SystemInit` 和 `_init` 两个空函数？
5. `--specs=nano.specs` 和 `--specs=nosys.specs` 各管什么？**什么情况下加了等于没加**？
6. 改头文件为什么 `make` 不重编？责任在编译器还是 make？
7. `-MMD -MP` 生成什么？`.d` 文件里第二行 `src/delay.h:`（空的）是干什么的？
8. `$@` / `$<` / `$^` 各是什么？为什么 `firmware.bin` 用 `$<` 而不是 `$^`？

<details>
<summary>答案（点开对照）</summary>

1. **编译** `gcc -c`：`.c` + `.h` → `.o`，预处理 → 编译 → 汇编，**地址全从 0 起算，未定**。
   **链接** `gcc -T`：`.o` + `.a` + `.ld` → `.elf`，合并段、**按脚本分配绝对地址**、把每条 `U` 填上、`--gc-sections` 删无用段。
   **转换** `objcopy -O binary`：`.elf` → `.bin`，抠出要烧进 Flash 的字节，丢掉段表/符号表/调试信息。
   **地址只有链接阶段才定**，编译阶段永远不知道自己的代码会落在哪。
2. `-c` = **只到 `.o` 为止，不进入链接**。去掉它，**编译照做**，只是编完继续去链接，因为没有 `main`、没有 `_start`、没有链接脚本而失败。⚠️ 原文答"不会执行编译"是反的——报错里出现 `crt0.o` 和 `libc.a` 恰恰证明编译早就成功了。
3. `-T` 给的不只是"摆位置"，**它还定义符号**。`startup_stm32f103xb.s` 里 `ldr r0, =_sdata` 用的 `_sdata` / `_sidata` / `_sbss` / `_ebss` 全部由 `.ld` 的段定义提供。去掉 `-T`，链接器用默认脚本，这四个符号找不到。
4. 去掉 `-nostartfiles`，工具链自己的 `crti.o` 会加进来，它**也定义了 `_init`** → `multiple definition of '_init'`。
   `SystemInit`：启动文件跳 `main` 前会调它（原本配时钟）。`_init`：newlib 的 `__libc_init_array` 会调它（C++ 全局构造用）。两个空函数撑住两条依赖链。
5. `nano.specs` = **换一套库**（`libc_nano.a`），体积小但 `printf` 默认不支持 `%f`。
   `nosys.specs` = **提供系统调用桩**（`_write` / `_sbrk` / `_close` … 的空实现），不是"精简库"。
   ⚠️ **本工程一个 libc 函数都没调，所以两个 spec 都是空转**：加不加、去不去，都是 564 B。只有调了 `printf` / `malloc` 才现形。
6. 责任在 **make**，不在编译器。编译器每次被调用都从头编，没有"记不记得"的问题。make 只看**依赖图里的时间戳**，而 `delay.h` 根本不在图里，于是判断"不用调编译器"。
7. `-MMD` 让 GCC 编译时顺手写一个 `.d`，内容是 `目标: 源文件 头文件`；`-MP` 再为每个头文件补一条**空目标**，防止头文件被删后报 `No rule to make target`。Makefile 里 `-include` 进来，依赖图就补全了。
8. `$@` = 目标名；`$<` = **第一个**依赖；`$^` = 全部依赖（去重，`$+` 才保留重复）。
   `firmware.elf` 要全部 `.o` 当输入 → `$^`；`objcopy` 只吃"一个输入 + 一个输出" → `$<`。
   现在 `firmware.bin: firmware.elf` 只有一个依赖，两者等价，用 `$<` 是**防御性写法**：以后加依赖也不会悄悄坏。

</details>

---

## 二、一句话结论

> **编译把每个 `.c` 变成"带清单（T）和待解析引用（U）、地址从 0 起算"的机器码；链接按脚本图纸把它们摆进同一地址空间、填完每条 `U`、删掉没人用的段；objcopy 拆掉包装只留字节。Makefile 就是把这三步写成"谁变了就重做谁"的依赖图。**

---

## 三、骨架

### 3.1 三步流水线

| 步骤 | 命令 | 输入 | 输出 | 关键动作 | 地址状态 |
| --- | --- | --- | --- | --- | --- |
| 编译 | `gcc -c` | `.c` / `.h` | `.o` | 预处理 → 机器码 | 相对，未定 |
| 链接 | `gcc -T` | `.o` + `.a` + `.ld` | `.elf` | 合并段、分配地址、解析符号 | 绝对，已定 |
| 转换 | `objcopy` | `.elf` | `.bin` | 抠字节、丢元数据 | 只保留字节 |

### 3.2 编译参数（PlatformIO 那条长命令，只抄骨架）

```
arm-none-eabi-gcc  -o main.o  -c  src/main.c
    架构    -mcpu=cortex-m3  -mthumb
    优化    -Os
    分段    -ffunction-sections  -fdata-sections
    宏      -DSTM32F103xB  -DUSE_HAL_DRIVER  -DF_CPU=72000000L
    包含    -Iinclude -Isrc -I(HAL 的 4 个目录)  ← 共 8 个
```

| 参数 | 作用 | 去掉后 |
| --- | --- | --- |
| `-c` | 只编译，不链接 | **照编不误**，编完继续链接 → `undefined reference to 'main'`、`'_exit'` |
| `-o xxx.o` | 指定输出名 | 默认 `a.out` |
| `-mcpu=cortex-m3 -mthumb` | 目标 CPU + Thumb 指令集 | 生成 ARM 指令，M3 跑不了 |
| `-Os` | 优化体积 | 固件变大 |
| `-ffunction-sections -fdata-sections` | 每个函数/变量单独成段 | `--gc-sections` 无从下手 |
| `-Wall` | 打开全部警告 | 漏掉隐患（T04 那个括号 bug 就靠它） |
| `-DSTM32F103xB` | 选芯片型号 | 头文件的条件编译分支走错 |
| `-I 路径` | 头文件搜索路径 | 找不到 `#include` |
| `-nostdlib` | **既不要系统启动文件，也不要标准库**（不是"只不要启动文件"） | ⚠️ 且它出现在带 `-c` 的行里，**整个是空转**——`-c` 根本不链接 |

### 3.3 链接参数：四个参数 = 四个决定〔实测〕

| 参数 | 它回答的问题 | 去掉后（真实报错） |
| --- | --- | --- |
| `-T stm32f103x8.ld` | **摆到哪**（Flash/RAM 起点、栈顶） | `undefined reference to '_sidata'` `'_sdata'` `'_sbss'` `'_ebss'` + `cannot find entry symbol _start` |
| `-nostartfiles` | **启动代码谁提供** | `multiple definition of '_init'`（`crti.o` 也定义了一份） |
| `--specs=nano.specs` | **缺实现时去哪找** | 本工程无影响；有 `printf` 时 text **3348 → 7728**（data 100 → 2112） |
| `--specs=nosys.specs` | **库要、系统没给的函数怎么办** | 本工程无影响；有 `printf` 时 7 条 `undefined reference`（`_sbrk` `_write` `_close` `_fstat` `_isatty` `_lseek` `_read`） |

### 3.4 Makefile：三条规则对应三次形态转换

```makefile
firmware.elf: $(OBJS)                 # 链接：要全部 .o  → $^
	$(CC) ... -T stm32f103x8.ld $^ -o $@

firmware.bin: firmware.elf            # 转换：只要一个输入 → $<
	$(OBJCOPY) -O binary $< $@
	$(SIZE) $<

%.o: %.c                              # 编译：任何 .o 由同名 .c 生成
	$(CC) $(CFLAGS) -c $< -o $@
```

| 自动变量 | 含义 |
| --- | --- |
| `$@` | 规则的目标文件名 |
| `$<` | **第一个**依赖 |
| `$^` | 全部依赖，**去重**（`$+` 保留重复） |

### 3.5 头文件依赖〔实测〕

```makefile
CFLAGS  += -MMD -MP
-include $(SRCS:.c=.d)
```

生成的 `src/delay.d`：

```
src/delay.o: src/delay.c src/delay.h

src/delay.h:
```

- 第一行：真正的依赖，make 靠它知道"改 `delay.h` 要重编 `delay.o`"
- 第二行：`-MP` 加的**空目标**，防止头文件被删后报 `No rule to make target`

**打开前后对比**：改 `delay.h` → 之前 `make: Nothing to be done for 'all'.`；之后 `main.o` 和 `delay.o` **都重编**（两个都 include 了它）。

---

## 四、我原来错在哪

| # | 原文 | 问题 | 改成 |
| --- | --- | --- | --- |
| 1 | "去掉 `-c` 的话 gcc 不会执行编译" | ⚠️ 反了。实测报错里有 `crt0.o` / `libc.a`，**说明编译成功了，是链接挂的** | `-c` = 只到 `.o` 为止；去掉它 → 编译 + 链接一条龙 |
| 2 | "`-nostdlib` = 不链接标准库启动文件" | 那是 `-nostartfiles`。`-nostdlib` 更狠：启动文件和 `-lc -lgcc` **全不要** | 见 3.2 表；且它在 `-c` 行里是空转 |
| 3 | "改了头文件，编译器不会重编" | 责任主体错。编译器没有记忆，**是 make 不去调它** | "make 判断不用调编译器，因为 `.h` 不在依赖图里" |
| 4 | `.d` 示例写成 `main.o: main.c defs.h config.h` | 通用示意，和自己工程对不上 | 实际是 `src/delay.o: src/delay.c src/delay.h` + 一行空目标 |
| 5 | "CMakeLists.txt 文件是我写的" | 本工程**没有** CMakeLists.txt，也没用 CMake | 那段只是"CMake → Makefile → make"的概念示意 |
| 6 | `&<` / `&^` | 打字错误 | `$<` / `$^` |
| 7 | "bin 用 `$^` 会报错，那语法不符" | 不是 make 语法错，是 **objcopy 拒绝**：`Usage: objcopy [option(s)] in-file [out-file]` | 工具的用法错误，不是 make 的语法错误 |

---

## 五、关键数字

| 项 | 值 |
| --- | --- |
| 固件占用 | text **564 B** / data **0** / bss **0**（`firmware.elf` 文件本身 135 KB，因为带调试信息） |
| 三步产物 | `main.o` 1148 B、`delay.o` 872 B、`startup.o` 4152 B、`firmware.elf` 135 KB、`firmware.bin` 564 B |
| RAM 顶 | `_estack = 0x20005000` = `0x20000000 + 0x5000`（20 KB） |
| 宏改动的机器码证据 | `cmp.w r3, #720` ↔ `cmp.w r3, #7200` |
| 有 `printf` 时 nano vs 完整库 | text 3348 vs 7728（**nano 省 4380 B**） |
| 工具链 | w64devkit（GNU Make 4.4.1）+ arm-none-eabi-gcc 7.2.1 |

---

## 六、待补

- [ ] T08 的 `make clean` 已执行后，仓库里应只剩 `Makefile` / `src/` / `.s` / `.ld`（`.o` `.elf` `.bin` 都在 `.gitignore`）
- [ ] AI 对照三问（步骤 6）已在对话里做过，答案要点：`-c` 去掉会链接失败 / `_estack` 是 RAM 顶端 / 隐患是 `-MMD -MP`
- [ ] README 四处已更新（进度表、目录结构、固件占用、踩坑）
