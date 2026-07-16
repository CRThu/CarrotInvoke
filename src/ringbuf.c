/****************************
* RINGBUF - 通用环形缓冲区
* CRTHu
* 2025.07.16
*****************************/
#include "ringbuf.h"
#include <string.h>

void ringbuf_init(ringbuf_t* ring, uint8_t* buf, uint16_t size)
{
    if (ring == NULL || buf == NULL || size == 0) return;

    ring->buf = buf;
    ring->size = size;
    ring->head = 0;
    ring->tail = 0;
#ifdef RINGBUF_DMA
    ring->head_reader = NULL;
    ring->tail_reader = NULL;
#endif
}

#ifdef RINGBUF_DMA
void ringbuf_set_head_reader(ringbuf_t* ring, uint16_t (*func)(void))
{
    if (ring == NULL) return;
    ring->head_reader = func;
}

void ringbuf_set_tail_reader(ringbuf_t* ring, uint16_t (*func)(void))
{
    if (ring == NULL) return;
    ring->tail_reader = func;
}

void ringbuf_sync_head(ringbuf_t* ring)
{
    if (ring == NULL || ring->head_reader == NULL) return;
    ring->head = ring->head_reader();
}

void ringbuf_sync_tail(ringbuf_t* ring)
{
    if (ring == NULL || ring->tail_reader == NULL) return;
    ring->tail = ring->tail_reader();
}
#endif

void ringbuf_set_head(ringbuf_t* ring, uint16_t head)
{
    if (ring == NULL) return;
    ring->head = head % ring->size;
}

void ringbuf_set_tail(ringbuf_t* ring, uint16_t tail)
{
    if (ring == NULL) return;
    ring->tail = tail % ring->size;
}

uint16_t ringbuf_readable(ringbuf_t* ring)
{
    if (ring == NULL) return 0;

    if (ring->head >= ring->tail)
        return ring->head - ring->tail;
    else
        return ring->size - ring->tail + ring->head;
}

uint16_t ringbuf_writable(ringbuf_t* ring)
{
    if (ring == NULL) return 0;
    return ring->size - 1 - ringbuf_readable(ring);
}

uint16_t ringbuf_peek(ringbuf_t* ring, uint8_t* dst, uint16_t len)
{
    if (ring == NULL || dst == NULL) return 0;

    uint16_t avail = ringbuf_readable(ring);
    if (len > avail)
        len = avail;

    uint16_t first = ring->size - ring->tail;

    if (first >= len)
    {
        memcpy(dst, &ring->buf[ring->tail], len);
    }
    else
    {
        memcpy(dst, &ring->buf[ring->tail], first);
        memcpy(dst + first, &ring->buf[0], len - first);
    }

    return len;
}

void ringbuf_skip(ringbuf_t* ring, uint16_t len)
{
    if (ring == NULL) return;

    uint16_t avail = ringbuf_readable(ring);
    if (len > avail)
        len = avail;

    ring->tail = (ring->tail + len) % ring->size;
}

uint16_t ringbuf_write(ringbuf_t* ring, const uint8_t* src, uint16_t len)
{
    if (ring == NULL || src == NULL) return 0;

    uint16_t space = ringbuf_writable(ring);
    if (len > space)
        len = space;

    uint16_t first = ring->size - ring->head;

    if (first >= len)
    {
        memcpy(&ring->buf[ring->head], src, len);
    }
    else
    {
        memcpy(&ring->buf[ring->head], src, first);
        memcpy(&ring->buf[0], src + first, len - first);
    }

    ring->head = (ring->head + len) % ring->size;
    return len;
}

void ringbuf_flush(ringbuf_t* ring)
{
    if (ring == NULL) return;
    ring->tail = ring->head;
}
