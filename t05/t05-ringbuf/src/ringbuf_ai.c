#include "ringbuf_ai.h"

/*
 * 内存屏障：
 * - "dmb" 是 Cortex-M3 的数据内存屏障指令，保证它之前的存储器访问
 *   在它之后的存储器访问之前完成。
 * - "memory" clobber 告诉编译器不要跨过这条指令重排内存访问。
 * 如果你的工程已经包含 CMSIS，也可以直接用 __DMB()。
 */
#define RINGBUF_DMB()   __asm volatile("dmb" ::: "memory")

bool ringbuf_init(ringbuf_t *rb, uint8_t *buf, uint16_t size)
{
    /* size 必须非 0，且是 2 的幂 */
    if (rb == NULL || buf == NULL) {
        return false;
    }
    if (size == 0U || (size & (uint16_t)(size - 1U)) != 0U) {
        return false;
    }
    /* uint16_t 的 size 最大只能表示 65535，2 的幂最大 32768 */
    if (size > 32768U) {
        return false;
    }

    rb->buf  = buf;
    rb->size = size;
    rb->mask = (uint16_t)(size - 1U);
    rb->head = 0U;
    rb->tail = 0U;
    return true;
}

bool ringbuf_write(ringbuf_t *rb, uint8_t byte)
{
    /* 先读 head 和 tail 的快照。
     * head 是自己（写者）维护的，tail 是读者维护的，各读一次。 */
    uint16_t head = rb->head;
    uint16_t tail = rb->tail;

    /* 满判断：已存数据量 = head - tail。
     * 因为 head/tail 单调递增、只在索引时掩码，
     * 所以 (head - tail) 就是当前数据量，范围 0 ~ size。
     * 数据量 == size 时满，写入失败。 */
    if ((uint16_t)(head - tail) >= rb->size) {
        return false;
    }

    /* 索引回绕：head & mask 把单调递增的 head 映射到 0 ~ size-1 */
    rb->buf[head & rb->mask] = byte;

    /* 保证“写 buf”在“更新 head”之前完成。
     * 否则读者可能看到新 head，却读到旧的 buf 数据。 */
    RINGBUF_DMB();

    /* 单调递增，不回绕。uint16_t 加法溢出回绕是定义良好的。 */
    rb->head = (uint16_t)(head + 1U);
    return true;
}

bool ringbuf_read(ringbuf_t *rb, uint8_t *byte)
{
    if (byte == NULL) {
        return false;
    }

    /* tail 是自己（读者）维护的，head 是写者维护的 */
    uint16_t tail = rb->tail;
    uint16_t head = rb->head;

    /* 空判断：head == tail，数据量为 0 */
    if (head == tail) {
        return false;
    }

    *byte = rb->buf[tail & rb->mask];

    /* 保证“读 buf”在“更新 tail”之前完成。
     * 否则写者可能看到新 tail，提前覆盖还没读走的数据。 */
    RINGBUF_DMB();

    rb->tail = (uint16_t)(tail + 1U);
    return true;
}

uint16_t ringbuf_space(const ringbuf_t *rb)
{
    uint16_t head = rb->head;
    uint16_t tail = rb->tail;

    /* 剩余空间 = size - 已存数据量 */
    return (uint16_t)(rb->size - (uint16_t)(head - tail));
}

uint16_t ringbuf_count(const ringbuf_t *rb)
{
    uint16_t head = rb->head;
    uint16_t tail = rb->tail;

    /* 已存数据量 = head - tail */
    return (uint16_t)(head - tail);
}