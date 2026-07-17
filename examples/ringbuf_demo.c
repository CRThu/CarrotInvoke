/****************************
 * RINGBUF Demo - 通用环形缓冲区演示
 *
 * 演示 ringbuf 模块功能：
 * 1. 基本读写
 * 2. wrap-around 处理
 * 3. peek / skip 操作
 * 4. flush 操作
 *****************************/
#include "ringbuf.h"
#include <stdio.h>
#include <string.h>

/*=============================================================
 * 辅助函数
 *=============================================================*/
static void print_ring_info(const ringbuf_t* ring, const char* label)
{
    printf("[%s] size=%d, head=%d, tail=%d, readable=%d, writable=%d\n",
           label,
           ring->size,
           ring->head,
           ring->tail,
           ringbuf_readable(ring),
           ringbuf_writable(ring));
}

/*=============================================================
 * 主函数
 *=============================================================*/
int main(void)
{
    uint8_t buf[16];
    ringbuf_t ring;
    uint8_t tmp[32];

    printf("=== RingBuf Demo ===\n\n");

    /* 1. 初始化 */
    ringbuf_init(&ring, buf, sizeof(buf));
    print_ring_info(&ring, "init");

    /* 2. 写入数据 */
    printf("\n--- Write 8 bytes ---\n");
    const char* data = "Hello!";
    ringbuf_write(&ring, (const uint8_t*)data, 6);
    print_ring_info(&ring, "after write");

    /* 3. peek 数据（不消费） */
    printf("\n--- Peek 6 bytes ---\n");
    memset(tmp, 0, sizeof(tmp));
    ringbuf_peek(&ring, tmp, 6);
    printf("peeked: '%s'\n", (char*)tmp);
    print_ring_info(&ring, "after peek");

    /* 4. skip 数据 */
    printf("\n--- Skip 3 bytes ---\n");
    ringbuf_skip(&ring, 3);
    print_ring_info(&ring, "after skip");

    /* 5. peek 剩余数据 */
    printf("\n--- Peek remaining 3 bytes ---\n");
    memset(tmp, 0, sizeof(tmp));
    ringbuf_peek(&ring, tmp, 3);
    printf("peeked: '%s'\n", (char*)tmp);
    print_ring_info(&ring, "after peek");

    /* 6. 再写入数据触发 wrap-around */
    printf("\n--- Write more data (trigger wrap-around) ---\n");
    const char* more = "World!!!";
    ringbuf_write(&ring, (const uint8_t*)more, 8);
    print_ring_info(&ring, "after wrap write");

    /* 7. 读取所有数据 */
    printf("\n--- Read all data ---\n");
    memset(tmp, 0, sizeof(tmp));
    uint16_t readable = ringbuf_readable(&ring);
    ringbuf_peek(&ring, tmp, readable);
    printf("readable=%d, data='%s'\n", readable, (char*)tmp);
    ringbuf_skip(&ring, readable);
    print_ring_info(&ring, "after read all");

    /* 8. flush */
    printf("\n--- Flush ---\n");
    ringbuf_write(&ring, (const uint8_t*)"test", 4);
    print_ring_info(&ring, "before flush");
    ringbuf_flush(&ring);
    print_ring_info(&ring, "after flush");

    /* 9. 写满测试 */
    printf("\n--- Write until full ---\n");
    ringbuf_init(&ring, buf, sizeof(buf));
    uint16_t writable = ringbuf_writable(&ring);
    printf("writable=%d\n", writable);
    ringbuf_write(&ring, (const uint8_t*)"12345678", writable);
    printf("After writing %d bytes:\n", writable);
    print_ring_info(&ring, "full");

    /* 10. 试图写入更多（应该失败） */
    printf("\n--- Try write when full ---\n");
    uint16_t written = ringbuf_write(&ring, (const uint8_t*)"X", 1);
    printf("written=%d (expected 0)\n", written);

    printf("\n=== Demo Complete ===\n");
    return 0;
}
