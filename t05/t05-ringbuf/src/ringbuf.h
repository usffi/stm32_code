#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdbool.h>
#include <stdint.h>


/*
    在设计环形缓冲区时，我选择的是空出一个容量来，
    当head=tail-1的时候，说明容量是满的，当tail=head的时候，说明容量是空的
    其次，这个实现是最简单的
*/

typedef struct {
    uint8_t  *buf;                 /* 指向缓冲区数组 */
    uint16_t  size;                /* 容量，必须是 2 的幂 */
    volatile uint16_t head;        /* 写指针，中断里改 */
    volatile uint16_t tail;        /* 读指针，主循环里改 */
} ringbuf_t;

void     ringbuf_init(ringbuf_t *rb, uint8_t *buf, uint16_t size);
bool     ringbuf_write(ringbuf_t *rb, uint8_t byte);   /* 返回是否成功 */
bool     ringbuf_read (ringbuf_t *rb, uint8_t *byte);  /* 返回是否成功 */
uint16_t ringbuf_freesize (const ringbuf_t *rb);

#endif