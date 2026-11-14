/* T10 白盒测试：在 PC 上跑，不用板子。
 *
 * 用法（Git Bash）：
 *     cd /d/stm32_code/t10/t10-cmd
 *     ./pc_test/run_test.sh
 *
 * 每写完一个 TODO，重跑一次，看 PASS 数量有没有涨。
 */
#include <stdio.h>
#include <string.h>
#include "cmd.h"

/* cmd.c 里声明了这个，由 main.c 提供；PC 上就用 printf 顶上 */
void con_puts(const char *s) { fputs(s, stdout); }

static int g_pass = 0;
static int g_fail = 0;

static void check(int cond, const char *msg)
{
    if (cond) {
        printf("  [PASS] %s\n", msg);
        g_pass++;
    } else {
        printf("  [FAIL] %s\n", msg);
        g_fail++;
    }
}

/* 两个假的命令处理函数，用 g_hit 区分是谁被调用了 */
static int g_hit = 0;
static void h_a(int argc, char **argv) { (void)argc; (void)argv; g_hit += 1; }
static void h_b(int argc, char **argv) { (void)argc; (void)argv; g_hit += 10; }

int main(void)
{
    printf("\n=== T10 白盒测试 ===\n");

    printf("\n[1] init：池建好了吗（TODO 2）\n");
    cmd_init();
    check(cmd_free_count() == CMD_POOL_SIZE, "初始应该 8 个空闲节点");

    printf("\n[2] register：能注册吗（TODO 6）\n");
    check(cmd_register("help", h_a) == true,  "注册 help 成功");
    check(cmd_register("led",  h_b) == true,  "注册 led 成功");
    check(cmd_free_count() == CMD_POOL_SIZE - 2, "空闲应该剩 6");

    printf("\n[3] 非法输入要拦住（TODO 6）\n");
    if (cmd_find("help") != NULL) {          /* 先确认正常注册能用，否则下面全是假通过 */
        check(cmd_register("help", h_b) == false,         "重名要拒绝");
        check(cmd_register(NULL, h_a) == false,           "空名字要拒绝");
        check(cmd_register("x", NULL) == false,           "空回调要拒绝");
        check(cmd_register("0123456789ab", h_a) == false, "12 字符的名字要拒绝");
    } else {
        check(0, "前置条件不满足：正常命令还没注册上，先做 TODO 6");
    }

    printf("\n[4] find：找得到 / 找不到（TODO 5）\n");
    if (cmd_find("help") != NULL) {
        check(cmd_find("nope") == NULL, "找不到 nope 时返回 NULL");
    } else {
        check(0, "前置条件不满足：find 找不到已注册的 help，先做 TODO 5");
    }

    printf("\n[5] exec：切词 + 派发（TODO 8）\n");
    char l1[] = "help";
    g_hit = 0;
    int exec_ok = (cmd_exec(l1) == true && g_hit == 1);
    check(exec_ok, "help 被派发到 h_a");

    char l2[] = "led on";
    g_hit = 0;
    check(cmd_exec(l2) == true && g_hit == 10, "led 被派发到 h_b");

    char l3[] = "  led   on  ";
    g_hit = 0;
    check(cmd_exec(l3) == true && g_hit == 10, "多余空格也能派发");

    if (exec_ok) {
        char l4[] = "nope";
        check(cmd_exec(l4) == false, "未知命令返回 false");
        char l5[] = "   ";
        g_hit = 0;
        check(cmd_exec(l5) == false && g_hit == 0, "全空格返回 false 且不调 handler");
    }

    printf("\n[6] unregister：删头 / 删中间（TODO 7）\n");
    if (cmd_find("help") != NULL && cmd_find("led") != NULL) {
        check(cmd_unregister("led") == true,   "删中间节点 led");
        check(cmd_find("led") == NULL,         "led 真的没了");
        check(cmd_find("help") != NULL,        "help 还在（链没断）");
        check(cmd_free_count() == CMD_POOL_SIZE - 1, "空闲回到 7");
        check(cmd_unregister("help") == true,  "删头节点 help");
        check(cmd_unregister("help") == false, "重复删返回 false");
    } else {
        check(0, "前置条件不满足：help / led 没注册上，先做完 TODO 6");
    }

    printf("\n[7] 池满：第 9 个要失败（TODO 3 + 6）\n");
    cmd_init();
    int ok8 = 0;
    for (int i = 0; i < CMD_POOL_SIZE; i++) {
        char nm[8];
        nm[0] = 'c';
        nm[1] = (char)('0' + i);
        nm[2] = '\0';
        if (cmd_register(nm, h_a)) {
            ok8++;
        } else {
            printf("  注册 c%d 失败\n", i);
        }
    }
    check(ok8 == CMD_POOL_SIZE, "8 个都注册成功");
    if (ok8 == CMD_POOL_SIZE) {
        check(cmd_free_count() == 0, "空闲归零");
        check(cmd_register("overflow", h_a) == false, "第 9 个被拒绝");
    }

    printf("\n=== 结果：PASS %d / FAIL %d ===\n", g_pass, g_fail);
    if (g_fail == 0) {
        printf("全过了，可以烧板子了。\n");
        return 0;
    }
    printf("还没写完，继续对着 TODO 补。\n");
    return 1;
}
