#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/*
 * 单生产者单消费者（SPSC）环形缓冲区。
 *
 * - head 只由写者（通常是中断）修改
 * - tail 只由读者（通常是主循环）修改
 * - 容量 size 必须是 2 的幂，最大 32768
 * - head/tail 单调递增，不做回绕；索引时用 & mask 回绕
 * - 缓冲区内存由调用方提供，不使用 malloc
 */
typedef struct {
    uint8_t *buf;                 /* 调用方提供的缓冲区 */
    uint16_t size;                /* 容量，必须是 2 的幂 */
    uint16_t mask;                /* size - 1，用于索引回绕 */
    volatile uint16_t head;       /* 写索引，写者修改 */
    volatile uint16_t tail;       /* 读索引，读者修改 */
} ringbuf_t;

/* 初始化。size 必须是 2 的幂且 <= 32768，否则初始化失败（返回 false） */
bool     ringbuf_init(ringbuf_t *rb, uint8_t *buf, uint16_t size);

/* 写入一个字节。成功返回 true，缓冲区满返回 false */
bool     ringbuf_write(ringbuf_t *rb, uint8_t byte);

/* 读取一个字节。成功返回 true，缓冲区空返回 false */
bool     ringbuf_read(ringbuf_t *rb, uint8_t *byte);

/* 查询剩余可写空间（字节数） */
uint16_t ringbuf_space(const ringbuf_t *rb);

/* 查询已存数据量（字节数） */
uint16_t ringbuf_count(const ringbuf_t *rb);

#endif /* RINGBUF_H */