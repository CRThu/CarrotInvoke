/****************************
* CMD QUEUE - 命令队列
* CRTHu
* 2025.07.15
*****************************/
#include "cmdqueue.h"
#include <string.h>

void cmd_queue_init(cmd_queue_t* queue)
{
    if (queue == NULL) return;

    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    queue->buf_head = 0;
}

dyncall_status_t cmd_queue_push(cmd_queue_t* queue, cmd_prefetch_t* prefetch)
{
    if (queue == NULL || prefetch == NULL)
        return DYNCALL_ERR_NULL_OBJECT;

    if (prefetch->cmd_len == 0)
        return DYNCALL_ERR_NULL_OBJECT;

    if (queue->count >= CMD_QUEUE_SIZE)
        return DYNCALL_ERR_POOL;

    if (queue->buf_head + prefetch->cmd_len >= CMD_QUEUE_BUF_SIZE)
        return DYNCALL_ERR_POOL;

    cmd_prefetch_t* item = &queue->items[queue->tail];
    item->buf = queue->buf;
    item->buf_len = CMD_QUEUE_BUF_SIZE;
    item->cmd_start = queue->buf_head;
    item->cmd_len = prefetch->cmd_len;
    item->func_len = prefetch->func_len;

    memcpy(&queue->buf[queue->buf_head],
           &prefetch->buf[prefetch->cmd_start],
           prefetch->cmd_len);
    queue->buf_head += prefetch->cmd_len;

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

    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    queue->buf_head = 0;
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
            const char* name = &item->buf[item->cmd_start];
            uint8_t fn_len = 0;

            while (func_name[fn_len] != '\0' && fn_len < 255)
                fn_len++;

            if (item->func_len == fn_len)
            {
                uint8_t match = 1;
                for (uint8_t j = 0; j < fn_len; j++)
                {
                    if (name[j] != func_name[j])
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
