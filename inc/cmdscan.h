/****************************
* CMD SCAN - 零拷贝状态机解析器
* CRTHu
* 2025.07.15
*
* 设计目标：
* 1. 无内存复制 - 直接返回缓冲区指针
* 2. 状态机按 byte 扫描 - 适配 DMA 增量接收
* 3. 无 std 库依赖 - 纯嵌入式友好
* 4. 预解析函数名长度 - 供 cmd_queue 直接使用
*****************************/
#pragma once
#ifndef _CMD_SCAN_H_
#define _CMD_SCAN_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <inttypes.h>

#define CMD_SCAN_VERSION                "1.0.0"

#define CMD_SCAN_MAX_ARGS               10
#define CMD_SCAN_MAX_CMD_LEN            256

/* 参数指针结构 - 零拷贝，直接指向原始缓冲区 */
typedef struct
{
    const char* ptr;        /* 参数起始地址 */
    uint16_t len;           /* 参数长度（不含终止符） */
} cmd_arg_t;

/* 扫描器状态 */
typedef enum
{
    SCAN_IDLE = 0,          /* 空闲状态，等待新命令 */
    SCAN_func_name,         /* 正在扫描函数名 */
    SCAN_args,              /* 正在扫描参数 */
    SCAN_error              /* 格式错误 */
} cmd_scan_state_t;

/* 扫描结果 */
typedef enum
{
    SCAN_INCOMPLETE = 0,    /* 扫描未完成，等待更多数据 */
    SCAN_COMPLETE = 1,      /* 扫描到完整命令（遇到 \n 或 \0） */
    SCAN_OVERFLOW = -1,     /* 缓冲区溢出 */
    SCAN_ERROR = -2         /* 格式错误 */
} scan_status_t;

/* 扫描器上下文 - 适配 DMA 增量接收 */
typedef struct
{
    /* 缓冲区信息 */
    const uint8_t* buf;     /* 缓冲区指针 */
    uint16_t buf_size;      /* 缓冲区总大小 */

    /* 扫描位置 */
    uint16_t scan_pos;      /* 当前扫描位置 */

    /* 当前命令信息 */
    uint16_t cmd_start;     /* 当前命令起始位置 */
    uint16_t cmd_len;       /* 命令长度（不含 \n） */
    uint8_t  func_len;      /* 函数名长度（预解析） */

    /* 状态机状态 */
    cmd_scan_state_t state;

    /* 是否在参数区域内 */
    uint8_t in_args;

    /* 错误码 */
    int8_t error;
} cmd_scanner_t;

/* 预解析结果 - 存储命令在 DMA 缓冲区中的位置信息 */
typedef struct
{
    const uint8_t* buf;     /* DMA 缓冲区指针 */
    uint16_t buf_len;       /* DMA 缓冲区总长度（环形缓冲区需要） */
    uint16_t cmd_start;     /* 命令在 buf 中的起始位置 */
    uint16_t cmd_len;       /* 命令总长度 */
    uint8_t  func_len;      /* 函数名长度 */
} cmd_prefetch_t;

/* 解析结果 */
typedef struct
{
    const char* func_name;          /* 函数名指针 */
    uint16_t func_name_len;         /* 函数名长度 */

    cmd_arg_t args[CMD_SCAN_MAX_ARGS]; /* 参数指针数组 */
    uint8_t args_count;             /* 实际参数个数 */
} cmd_parse_result_t;

/*=============================================================
 * API 函数
 *=============================================================*/

/**
 * @brief 初始化扫描器
 * @param scanner 扫描器上下文
 * @param buf 缓冲区指针
 * @param buf_size 缓冲区大小
 */
void cmdscan_init(cmd_scanner_t* scanner, const uint8_t* buf, uint16_t buf_size);

/**
 * @brief 重置扫描器到初始状态
 * @param scanner 扫描器上下文
 */
void cmdscan_reset(cmd_scanner_t* scanner);

/**
 * @brief 预解析命令 - 使用 scanner 记录扫描位置
 *
 * 从 scanner 当前扫描位置预解析单条命令，返回位置信息
 * 可连续预解析多条命令，scanner 自动记录位置
 *
 * 使用方法：
 * 1. 调用 cmdscan_init() 初始化 scanner
 * 2. 循环调用 cmdscan_prefetch() 获取每条命令
 * 3. 返回 SCAN_COMPLETE 表示找到完整命令
 * 4. 返回 SCAN_INCOMPLETE 表示缓冲区扫描完毕
 *
 * @param scanner 扫描器上下文（需要先调用 cmdscan_init）
 * @param prefetch 输出：预解析结果（含 buf 指针和位置信息）
 * @return scan_status_t 扫描结果
 *
 * 示例：
 * @code
 * cmd_scanner_t scanner;
 * cmdscan_init(&scanner, dma_buf, dma_len);
 *
 * cmd_prefetch_t prefetch;
 * while (cmdscan_prefetch(&scanner, &prefetch) == SCAN_COMPLETE) {
 *     cmd_queue_push(&queue, &prefetch);
 * }
 * @endcode
 */
scan_status_t cmdscan_prefetch(cmd_scanner_t* scanner, cmd_prefetch_t* prefetch);

/**
 * @brief 从指定位置开始扫描，直到遇到 \n 或 \0
 * 
 * 使用方法：
 * 1. 调用 cmdscan_init() 初始化
 * 2. 每次 DMA 接收数据后，调用 cmdscan_scan()
 * 3. 如果返回 SCAN_COMPLETE，说明找到完整命令
 * 4. 调用 cmdparse_args() 解析参数
 * 
 * @param scanner 扫描器上下文
 * @return scan_status_t 扫描结果
 * 
 * 示例：
 * @code
 * cmd_scanner_t scanner;
 * cmdscan_init(&scanner, dma_buf, dma_len);
 * 
 * while (1) {
 *     // 等待 DMA 数据
 *     wait_dma_data();
 *     
 *     scan_status_t status = cmdscan_scan(&scanner);
 *     if (status == SCAN_COMPLETE) {
 *         // 找到完整命令，可以解析
 *         cmd_parse_result_t result;
 *         cmdparse_args(scanner.buf + scanner.cmd_start, 
 *                       scanner.cmd_len, &result);
 *     }
 * }
 * @endcode
 */
scan_status_t cmdscan_scan(cmd_scanner_t* scanner);

/**
 * @brief 获取下一个命令在缓冲区中的位置
 *
 * @param scanner 扫描器上下文
 * @param next_start 下一个命令的起始位置
 * @return uint16_t 下一个命令的长度，0 表示没有下一个命令
 */
uint16_t cmdscan_next(const cmd_scanner_t* scanner, uint16_t* next_start);

/**
 * @brief 将完整命令解析为参数指针数组
 *
 * 无内存复制 - 所有指针直接指向原始 cmd 缓冲区
 *
 * @param cmd 命令字符串指针
 * @param len 命令长度
 * @param result 解析结果
 * @return uint8_t 解析出的参数个数，0 表示无参数，错误时返回 0xFF
 */
uint8_t cmdparse_args(const char* cmd, uint16_t len, cmd_parse_result_t* result);

#ifdef __cplusplus
}
#endif

#endif /* _CMD_SCAN_H_ */
