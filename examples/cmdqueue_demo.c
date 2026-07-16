/****************************
* CMD QUEUE 示例 - 命令队列演示
* 演示排队执行和协作式中断
*****************************/
#include "cmdqueue.h"
#include "cmdscan.h"
#include <stdio.h>
#include <string.h>

/*=============================================================
 * 模拟耗时任务
 *=============================================================*/

void long_running_task(cmd_queue_t* queue)
{
    printf("[TASK] 开始耗时任务...\r\n");

    for (int i = 0; i < 10; i++)
    {
        printf("[TASK] 处理中... %d/10\r\n", i + 1);

        if (cmd_queue_check(queue, "stop"))
        {
            printf("[TASK] 检测到 stop 命令，主动退出！\r\n");
            return;
        }
    }

    printf("[TASK] 任务完成\r\n");
}

/*=============================================================
 * 主程序
 *=============================================================*/

int main(void)
{
    printf("CMD QUEUE 命令队列示例\r\n");
    printf("========================\r\n\r\n");

    cmd_queue_t queue;
    cmd_queue_init(&queue);

    /* --- 示例1: cmdscan_prefetch + cmd_queue 流程 --- */
    printf("=== 示例1: prefetch + queue 流程 ===\r\n");

    /* 模拟 DMA 接收的命令 */
    const char* dma_data = "led_on(1)\nled_off(2)\nstop\n";
    uint16_t dma_len = strlen(dma_data);

    /* 使用 scanner 逐条预解析并推入队列 */
    cmd_scanner_t scanner;
    cmdscan_init(&scanner, (const uint8_t*)dma_data, dma_len);

    cmd_prefetch_t prefetch;
    while (cmdscan_prefetch(&scanner, &prefetch) == SCAN_COMPLETE)
    {
        cmd_queue_push(&queue, &prefetch);
    }

    printf("[QUEUE] 队列中有 %d 条命令\r\n", cmd_queue_count(&queue));

    /* 出队执行 */
    cmd_prefetch_t pf_out;
    while (cmd_queue_pop(&queue, &pf_out) == DYNCALL_NO_ERROR)
    {
        const char* cmd = pf_out.buf + pf_out.cmd_start;

        /* 执行时用 cmdparse_args 解析 */
        char cmd_buf[64];
        memcpy(cmd_buf, cmd, pf_out.cmd_len);
        cmd_buf[pf_out.cmd_len] = '\0';

        cmd_parse_result_t result;
        cmdparse_args(cmd_buf, pf_out.cmd_len, &result);

        printf("[EXEC] %.*s (func: %.*s, args: %d)\r\n",
               pf_out.cmd_len, cmd,
               result.func_name_len, result.func_name,
               result.args_count);
    }
    printf("\r\n");

    cmd_queue_flush(&queue);

    /* --- 示例2: 协作式中断 --- */
    printf("=== 示例2: 协作式中断 ===\r\n");

    cmd_scanner_t scanner2;
    cmdscan_init(&scanner2, (const uint8_t*)"stop", 4);
    cmd_prefetch_t pf;
    cmdscan_prefetch(&scanner2, &pf);
    cmd_queue_push(&queue, &pf);

    printf("[CHECK] 检查队列: %s\r\n",
           cmd_queue_check(&queue, "stop") ? "找到 stop" : "未找到");

    long_running_task(&queue);
    printf("\r\n");

    cmd_queue_flush(&queue);

    /* --- 示例3: 非环形缓冲区（分号分隔） --- */
    printf("=== 示例3: 非环形缓冲区 ===\r\n");

    const char* linear_buf = "cmd_a;cmd_b;cmd_c";
    uint16_t linear_len = strlen(linear_buf);

    cmd_scanner_t scanner3;
    cmdscan_init(&scanner3, (const uint8_t*)linear_buf, linear_len);

    cmd_prefetch_t pf2;
    while (cmdscan_prefetch(&scanner3, &pf2) == SCAN_COMPLETE)
    {
        cmd_queue_push(&queue, &pf2);
    }

    printf("[QUEUE] 入队 %d 条命令\r\n", cmd_queue_count(&queue));

    while (cmd_queue_pop(&queue, &pf_out) == DYNCALL_NO_ERROR)
    {
        printf("[EXEC] %.*s\r\n", pf_out.cmd_len, pf_out.buf + pf_out.cmd_start);
    }

    return 0;
}
