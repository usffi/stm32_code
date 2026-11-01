#include "ringbuf.h"

void ringbuf_init(ringbuf_t *rb, uint8_t *buf, uint16_t size)
{
    /*
        1. 把 buf、size 存进结构体
        2. head = tail = 0
    */
   rb->buf = buf;
   rb->size = size;
   rb->head = 0U;
   rb->tail = 0U;

}

// 还有多少剩余空间可用
uint16_t ringbuf_freesize (const ringbuf_t *rb)
{
    /*
        1. 算已用：(head - tail) & (size - 1)
        2. 剩余 = size - 1 - 已用（浪费一格方案）
        3. 注意 head - tail 是无符号相减，绕回时自动借位，不用额外判断
    */
    // 缓冲区已经用的大小
    uint16_t used_size = ((rb->head-rb->tail) & (rb->size-1));
    return rb->size -1 - used_size;
}

bool ringbuf_write(ringbuf_t *rb, uint8_t byte)
{
    /*
        1. 先判满：free == 0 就返回 false，什么都不写
        2. 写：buf[head] = byte
        3. 移指针：head = (head + 1) & (size - 1)
        4. 返回 true
    */

    uint16_t free_size = ringbuf_freesize(rb);
    if (free_size == 0)
        return false;
    
    rb->buf[rb->head] = byte;
    rb->head = (rb->head + 1) & (rb->size -1);
    return true;
}

bool ringbuf_read (ringbuf_t *rb, uint8_t *byte)
{
    /*
        1. 先判空：head == tail 就返回 false，不许返回垃圾值
        2. 取值：*byte = buf[tail]
        3. 移指针：tail = (tail + 1) & (size - 1)
        4. 返回 true
    */
   if( rb->head == rb->tail)
   {
        return false;
   }
    

    *byte = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1) & (rb->size -1);

    return true;
}

