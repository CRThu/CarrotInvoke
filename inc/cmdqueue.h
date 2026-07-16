/****************************
* CMD QUEUE - 命令队列
* CRTHu
* 2025.07.15
*
* 设计目标：
* 1. 环形缓冲区，FIFO 顺序执行
* 2. 存储原始命令 + 紧凑元数据
* 3. 支持协作式中断检查
* 4. 兼容环形/非环形 DMA 缓冲区
*****************************/
#pragma once
#ifndef _CMD_QUEUE_H_
#define _CMD_QUEUE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <inttypes.h>
#include "cmdscan.h"
#include "dyncall.h"
#include "ringbuf.h"

#define CMD_QUEUE_SIZE              128
#define CMD_QUEUE_BUF_SIZE          2048

/* 命令队列 */
typedef struct
{
    cmd_prefetch_t items[CMD_QUEUE_SIZE];
    ringbuf_t ring;             /* 环形缓冲区（管理命令数据） */
    uint8_t head;               /* 队头索引 */
    uint8_t tail;               /* 队尾索引 */
    uint8_t count;              /* 当前队列长度 */
} cmd_queue_t;

/**
 * @brief 初始化命令队列
 */
void cmd_queue_init(cmd_queue_t* queue);

/**
 * @brief 命令入队（复制原始命令到 buf）
 * @param queue 队列指针
 * @param prefetch 预解析结果（来自 cmdscan_prefetch）
 * @return DYNCALL_NO_ERROR 成功，其他错误码
 */
dyncall_status_t cmd_queue_push(cmd_queue_t* queue, cmd_prefetch_t* prefetch);

/**
 * @brief 命令出队
 * @param queue 队列指针
 * @param prefetch 输出：预解析结果（cmd 指向 queue 内部 buf）
 * @return DYNCALL_NO_ERROR 成功，DYNCALL_ERR_POOL 队列空
 */
dyncall_status_t cmd_queue_pop(cmd_queue_t* queue, cmd_prefetch_t* prefetch);

/**
 * @brief 检查队列是否为空
 */
uint8_t cmd_queue_is_empty(cmd_queue_t* queue);

/**
 * @brief 检查队列是否已满
 */
uint8_t cmd_queue_is_full(cmd_queue_t* queue);

/**
 * @brief 获取队列中命令数量
 */
uint8_t cmd_queue_count(cmd_queue_t* queue);

/**
 * @brief 清空队列
 */
void cmd_queue_flush(cmd_queue_t* queue);

/**
 * @brief 扫描队列是否有指定函数名的命令
 * @param queue 队列指针
 * @param func_name 要查找的函数名
 * @return 1 找到，0 未找到
 */
uint8_t cmd_queue_check(cmd_queue_t* queue, const char* func_name);

#ifdef __cplusplus
}
#endif

#endif /* _CMD_QUEUE_H_ */
