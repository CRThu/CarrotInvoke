/****************************
* CMD QUEUE - 命令队列
* CRTHu
* 2025.07.15
*****************************/
#include "cmdqueue.h"
#include <string.h>

static uint8_t queue_buf[CMD_QUEUE_BUF_SIZE];

void cmd_queue_init(cmd_queue_t* queue)
{
    if (queue == NULL) return;

    ringbuf_init(&queue->ring, queue_buf, CMD_QUEUE_BUF_SIZE);
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
}

dyncall_status_t cmd_queue_push(cmd_queue_t* queue, cmd_prefetch_t* prefetch)
{
    if (queue == NULL || prefetch == NULL)
        return DYNCALL_ERR_NULL_OBJECT;

    if (prefetch->cmd_len == 0)
        return DYNCALL_ERR_NULL_OBJECT;

    if (queue->count >= CMD_QUEUE_SIZE)
        return DYNCALL_ERR_POOL;

    if (ringbuf_writable(&queue->ring) < prefetch->cmd_len)
        return DYNCALL_ERR_POOL;

    const uint8_t* src = (const uint8_t*)&prefetch->buf[prefetch->cmd_start];
    uint16_t written = ringbuf_write(&queue->ring, src, prefetch->cmd_len);

    if (written != prefetch->cmd_len)
        return DYNCALL_ERR_POOL;

    cmd_prefetch_t* item = &queue->items[queue->tail];
    item->buf = queue->ring.buf;
    item->buf_len = CMD_QUEUE_BUF_SIZE;
    item->cmd_start = (queue->ring.head - prefetch->cmd_len + CMD_QUEUE_BUF_SIZE) % CMD_QUEUE_BUF_SIZE;
    item->cmd_len = prefetch->cmd_len;
    item->func_len = prefetch->func_len;

    queue->tail = (queue->tail + 1) % CMD_QUEUE_SIZE;
    queue->count++;

    return DYNCALL_NO_ERROR;
}

dyncall_status_t cmd_queue_pop(cmd_queue_t* queue, cmd_prefetch_t* prefetch)
{
    if (queue == NULL || prefetch == NULL)
        return DYNCALL_ERR_NULL_OBJECT;

    if (queue->count == 0)
        return DYNCALL_ERR_POOL;

    cmd_prefetch_t* item = &queue->items[queue->head];
    *prefetch = *item;

    ringbuf_skip(&queue->ring, item->cmd_len);

    queue->head = (queue->head + 1) % CMD_QUEUE_SIZE;
    queue->count--;

    return DYNCALL_NO_ERROR;
}

uint8_t cmd_queue_is_empty(cmd_queue_t* queue)
{
    if (queue == NULL) return 1;
    return queue->count == 0;
}

uint8_t cmd_queue_is_full(cmd_queue_t* queue)
{
    if (queue == NULL) return 1;
    return queue->count >= CMD_QUEUE_SIZE;
}

uint8_t cmd_queue_count(cmd_queue_t* queue)
{
    if (queue == NULL) return 0;
    return queue->count;
}

void cmd_queue_flush(cmd_queue_t* queue)
{
    if (queue == NULL) return;

    ringbuf_flush(&queue->ring);
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
}

uint8_t cmd_queue_check(cmd_queue_t* queue, const char* func_name)
{
    if (queue == NULL || func_name == NULL) return 0;

    uint8_t idx = queue->head;

    for (uint8_t i = 0; i < queue->count; i++)
    {
        cmd_prefetch_t* item = &queue->items[idx];

        if (item->func_len > 0)
        {
            uint8_t fn_len = 0;
            while (func_name[fn_len] != '\0' && fn_len < 255)
                fn_len++;

            if (item->func_len == fn_len)
            {
                uint8_t match = 1;
                for (uint8_t j = 0; j < fn_len; j++)
                {
                    uint16_t pos = (item->cmd_start + j) % CMD_QUEUE_BUF_SIZE;
                    if (item->buf[pos] != func_name[j])
                    {
                        match = 0;
                        break;
                    }
                }

                if (match)
                    return 1;
            }
        }

        idx = (idx + 1) % CMD_QUEUE_SIZE;
    }

    return 0;
}
