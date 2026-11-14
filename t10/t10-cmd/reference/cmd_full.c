#include "cmd.h"
#include <string.h>

/* 由 main.c 提供：往串口打一段字符串 */
void con_puts(const char *s);

/* ===== 静态池：8 个节点，编译期就占好 RAM，运行期只借不还 ===== */
static cmd_node_t g_pool[CMD_POOL_SIZE];

/* 空闲链表的头：所有没被用的节点串在这条链上 */
static cmd_node_t *g_free;

/* 命令链表的头：注册过的命令串在这条链上 */
static cmd_node_t *g_head;

void cmd_init(void)
{
    g_head = NULL;
    g_free = NULL;

    /* 倒着串，这样第一次 alloc 拿到的是 g_pool[0] */
    for (int i = CMD_POOL_SIZE - 1; i >= 0; i--) {
        g_pool[i].next = g_free;
        g_free = &g_pool[i];
    }
}

static cmd_node_t *node_alloc(void)
{
    cmd_node_t *n = g_free;
    if (n != NULL) {
        g_free = n->next;          /* 从空闲链上摘下来 */
    }
    return n;
}

static void node_free(cmd_node_t *n)
{
    n->next = g_free;              /* 挂回空闲链（头插） */
    g_free = n;
}

int cmd_free_count(void)
{
    int c = 0;
    for (cmd_node_t *p = g_free; p != NULL; p = p->next) {
        c++;
    }
    return c;
}

cmd_node_t *cmd_find(const char *name)
{
    for (cmd_node_t *p = g_head; p != NULL; p = p->next) {
        if (strcmp(p->name, name) == 0) {
            return p;
        }
    }
    return NULL;
}

bool cmd_register(const char *name, cmd_handler_t handler)
{
    if (name == NULL || handler == NULL) {
        return false;
    }
    if (strlen(name) >= CMD_NAME_MAX) {     /* 名字放不下 */
        return false;
    }
    if (cmd_find(name) != NULL) {           /* 重名 */
        return false;
    }

    cmd_node_t *n = node_alloc();
    if (n == NULL) {                        /* 池满 */
        return false;
    }

    strcpy(n->name, name);
    n->handler = handler;
    n->next = g_head;                       /* 头插：新命令排在最前 */
    g_head = n;
    return true;
}

bool cmd_unregister(const char *name)
{
    /*
        二级指针的用处：pp 指向"指向当前节点的那个指针"。
        删头节点时 *pp 就是 g_head，删中间节点时 *pp 是前一个节点的 next——
        两种情况同一句 `*pp = dead->next` 就解决了，不用特判头节点。
    */
    cmd_node_t **pp = &g_head;
    while (*pp != NULL) {
        if (strcmp((*pp)->name, name) == 0) {
            cmd_node_t *dead = *pp;
            *pp = dead->next;               /* 摘链 */
            node_free(dead);                /* 还池 */
            return true;
        }
        pp = &(*pp)->next;
    }
    return false;
}

void cmd_list(void)
{
    for (cmd_node_t *p = g_head; p != NULL; p = p->next) {
        con_puts("  ");
        con_puts(p->name);
        con_puts("\r\n");
    }
}

bool cmd_exec(char *line)
{
    char *argv[CMD_ARGV_MAX];
    int   argc = 0;
    char *p = line;

    /* 原地切词：把空格换成 '\0'，argv 指向每段的开头 */
    while (*p != '\0' && argc < CMD_ARGV_MAX) {
        while (*p == ' ') {
            p++;
        }
        if (*p == '\0') {
            break;
        }
        argv[argc++] = p;
        while (*p != '\0' && *p != ' ') {
            p++;
        }
        if (*p != '\0') {
            *p = '\0';
            p++;
        }
    }

    if (argc == 0) {
        return false;
    }

    cmd_node_t *n = cmd_find(argv[0]);
    if (n == NULL) {
        return false;
    }

    n->handler(argc, argv);
    return true;
}
