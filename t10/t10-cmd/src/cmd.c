#include "cmd.h"
#include <string.h>

/* 由 main.c 提供：往串口打一段字符串 */
void con_puts(const char *s);

/* ===== 静态池：8 个节点，编译期就占好 RAM，运行期只借不还 ===== */
static cmd_node_t g_pool[CMD_POOL_SIZE];

/* ─────────────────────────────────────────────────────────
 * TODO 1｜声明两条链表的头（这一条我直接给你）
 *
 *   g_free —— 空闲链的头，指向「第一个还没用的节点」
 *   g_head —— 命令链的头，指向「第一条注册的命令」
 *
 *   注意：静态变量默认就是 0（也就是 NULL），不用显式初始化
 * ───────────────────────────────────────────────────────── */
static cmd_node_t *g_free;
static cmd_node_t *g_head;


void cmd_init(void)
{
    /* ─────────────────────────────────────────────────────
     * TODO 2｜建池：把 8 个节点串成一条空闲链
     *
     *   做完之后应该长这样：
     *     g_free -> pool[0] -> pool[1] -> ... -> pool[7] -> NULL
     *     g_head -> NULL
     *
     *   步骤：
     *     ① g_head 置 NULL（一条命令都还没有）
     *     ② for (int i = CMD_POOL_SIZE - 1; i >= 0; i--)   ← 倒着走
     *          把 pool[i].next 指向当前的 g_free
     *          再把 g_free 指向 &pool[i]
     *
     *   为什么倒着走：正着走也能串出来，但第一次 alloc 会拿到 pool[7]。
     *   倒着走让 pool[0] 先被用掉，调试时地址看着顺眼。
     * ───────────────────────────────────────────────────── */

    g_head=NULL;
    g_free=NULL;
    for(int i=CMD_POOL_SIZE-1;i>=0;i--)
    {
        g_pool[i].next=g_free;  // g_pool指向free所在的位置
        g_free=&g_pool[i];      // 更新free的位置
    }

    
}


static cmd_node_t *node_alloc(void)
{
    /* ─────────────────────────────────────────────────────
     * TODO 3｜从空闲链上摘一个节点下来
     *
     *   ① 如果 g_free == NULL → 池空了，直接 return NULL
     *   ② 把 g_free 现在指的那个节点记到 n
     *   ③ g_free 挪到 n->next（这一步就是「摘链」）
     *   ④ return n
     * ───────────────────────────────────────────────────── */
    if(g_free==NULL)
        return NULL;
    cmd_node_t *n = g_free;
    g_free = n->next;
    return n;

}


static void node_free(cmd_node_t *n)
{
    /* ─────────────────────────────────────────────────────
     * TODO 4｜把节点挂回空闲链（头插）
     *
     *   ① n->next 指向当前的 g_free
     *   ② g_free 指向 n
     *   就两行。
     * ───────────────────────────────────────────────────── */
    n->next = g_free;
    g_free = n;
}


/* ═══════════════════════════════════════════════════════
 * 下面两个函数是「遍历」的范例，已经写好了。
 * 后面 cmd_find 也是这个结构：
 *     for (p = 头; p != NULL; p = p->next)
 * ═══════════════════════════════════════════════════════ */

int cmd_free_count(void)
{
    int c = 0;
    for (cmd_node_t *p = g_free; p != NULL; p = p->next) {
        c++;
    }
    return c;
}

void cmd_list(void)
{
    for (cmd_node_t *p = g_head; p != NULL; p = p->next) {
        con_puts("  ");
        con_puts(p->name);
        con_puts("\r\n");
    }
}


cmd_node_t *cmd_find(const char *name)
{
    /* ─────────────────────────────────────────────────────
     * TODO 5｜按名字找节点
     *
     *   照 cmd_list 的遍历写，只是把「打印」换成「比较」：
     *     如果 strcmp(p->name, name) == 0 → return p
     *   走到底还没找到 → return NULL
     * ───────────────────────────────────────────────────── */
    for(cmd_node_t *p=g_head; p !=NULL; p = p->next)
    {
        if(strcmp(p->name,name) == 0)
        {
            return p;
        }
    }
    return NULL;
}


bool cmd_register(const char *name, cmd_handler_t handler)
{
    /* ─────────────────────────────────────────────────────
     * TODO 6｜注册一条命令
     *
     *   先做四个检查，任何一个不过就 return false：
     *     ⓐ name 和 handler 都不能是 NULL
     *     ⓑ strlen(name) 必须 < CMD_NAME_MAX
     *        （name[12] 最多放 11 个字符 + 结尾 '\0'）
     *     ⓒ cmd_find(name) 必须返回 NULL（否则就是重名）
     *     ⓓ node_alloc() 必须拿到非 NULL（否则池满）
     *
     *   然后填节点 + 头插：
     *     · strcpy 把名字拷进 n->name
     *     · 把 handler 存进去
     *     · ★ 这两句的顺序不能换：
     *           n->next = g_head;
     *           g_head  = n;
     *       反过来写就先断了原来的链，再让自己指向自己 → 成环 → 遍历死循环
     * ───────────────────────────────────────────────────── */
    if(name == NULL || handler == NULL )
        return false;
    if(strlen(name) >= CMD_NAME_MAX)
        return false;
    if(cmd_find(name)!=NULL)
        return false;
    cmd_node_t *n = node_alloc();
    if(n == NULL)
        return false;

    strcpy(n->name,name);
    n->handler = handler;

    
    n->next = g_head;
    g_head = n;

    return true;
   
}


bool cmd_unregister(const char *name)
{
    /* ─────────────────────────────────────────────────────
     * TODO 7｜注销一条命令（这题用二级指针，起点我给你）
     *
     *       cmd_node_t **pp = &g_head;
     *
     *   pp 指向的不是节点，而是「指向节点的那个指针」：
     *     删头节点时   pp == &g_head
     *     删中间节点时 pp 指向「前一个节点的 next 字段」
     *
     *   循环 while (*pp != NULL)：
     *     如果 strcmp((*pp)->name, name) == 0：
     *         dead = *pp;              // 记住要删的
     *         *pp  = dead->next;       // ★ 一句同时覆盖「删头」和「删中间」
     *         node_free(dead);         // 还池
     *         return true;
     *     否则：pp 往前走一步，指向下一个节点的 next 字段
     *           ★ 是 pp = &(*pp)->next，不是 pp = (*pp)->next
     *             （前者走一步还停在「指针的地址」上，后者就跑到节点上了）
     *
     *   走到底没找到 → return false
     * ───────────────────────────────────────────────────── */
    cmd_node_t **pp = &g_head;
    while(*pp != NULL)
    {
       if(strcmp((*pp)->name,name) ==0 )
       {
            cmd_node_t *dead = *pp;
            *pp = dead->next;
            node_free(dead);
            return true;        
       } 
       else
       {
            pp= &((*pp)->next);
       }
    }
    return false;

    /*
    cmd_node_t *p = g_head;
    while(p != NULL)
    {
        if(strcmp(p->name,name)==0)
        {
            cmd_node_t *dead = p;
            p = dead -> next;
            node_free(dead);
            return true;        
        }
        else
        {
            p = p->next;    
        }
    }
    */


}


bool cmd_exec(char *line)
{
    /* ─────────────────────────────────────────────────────
     * TODO 8｜切词 + 派发（这题最长，分两半做）
     *
     *   【前半：原地切词】
     *   目标是把 "  led   on  " 变成：
     *      内存里  'l' 'e' 'd' '\0' 'o' 'n' '\0'
     *                ▲              ▲
     *       argv[0] ─┘              │
     *       argv[1] ────────────────┘
     *       argc = 2
     *
     *   做法：
     *       char *argv[CMD_ARGV_MAX];
     *       int   argc = 0;
     *       char *p = line;
     *
     *       循环（*p 不是结尾，且 argc 还没满）：
     *         ① 跳过所有空格（while (*p == ' ') p++;）
     *         ② 如果到结尾了就 break
     *         ③ argv[argc++] = p;          // 记下这一段的开头
     *         ④ 往后走，直到空格或结尾
     *         ⑤ 如果停在空格上：把它写成 '\0'，p 再往后挪一格
     *
     *   【后半：派发】
     *       argc == 0                 → return false（空行）
     *       cmd_find(argv[0]) == NULL → return false（没这条命令）
     *       否则调 n->handler(argc, argv); return true;
     *
     *   注意：argv[0] 是命令名，argv[1] 开始才是参数——
     *        所以 handler 里要用 argv[1]、argv[2]，不是从 argv[0] 取参数。
     * ───────────────────────────────────────────────────── */
    char *argv[CMD_ARGV_MAX];
    int argc = 0;
    char *p = line;
    while(*p !='\0' && argc < CMD_ARGV_MAX)
    {
        while(*p == ' ')
        {
            p++;
        }
        if(*p == '\0')
        {
            break;
        }
        argv[argc++] = p;
        while(*p != ' ' && *p != '\0')
        {
            p++;
        }
        if(*p == ' ')
        {
            *p = '\0';
            p++;
        }
    }

    if(argc == 0)
    {
        return false;
    }
    cmd_node_t *n = cmd_find(argv[0]);
    if(n == NULL)
    {
        return false;
    }
    n->handler(argc,argv);
    
    return true;
}
