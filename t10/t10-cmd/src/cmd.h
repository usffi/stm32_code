#ifndef CMD_H
#define CMD_H

#include <stdbool.h>
#include <stddef.h>

/* 静态池容量：最多注册这么多命令，不用 malloc */
#define CMD_POOL_SIZE   8
/* 命令名最大长度（含结尾 '\0'） */
#define CMD_NAME_MAX    12
/* 一次最多切出几个词 */
#define CMD_ARGV_MAX    6

typedef void (*cmd_handler_t)(int argc, char **argv);

/* 一个节点 = 一条命令 = 池里的一格 */
typedef struct cmd_node {
    char                name[CMD_NAME_MAX];
    cmd_handler_t       handler;
    struct cmd_node    *next;
} cmd_node_t;

/* 建池：把所有节点串成一条空闲链表 */
void        cmd_init(void);



/* 从池里取一个节点挂到命令链表（头插）。池满 / 重名 / 参数非法 → false */
bool        cmd_register(const char *name, cmd_handler_t handler);

/* 摘掉一条命令并把节点还给池 */
bool        cmd_unregister(const char *name);

/* 按名字找节点，找不到返回 NULL */
cmd_node_t *cmd_find(const char *name);

/* 遍历命令链表，逐个打印名字（help 用） */
void        cmd_list(void);

/* 把一行文本切词并执行。返回 false 表示没找到这条命令 */
bool        cmd_exec(char *line);

/* 池里还剩几个空闲节点（白盒测试用） */
int         cmd_free_count(void);

#endif
