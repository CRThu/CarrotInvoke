# CarrotInvoke RPC 使用指南

## 1. 即时执行（单条命令）

```c
#include "dyncall.h"
#include "cmdscan.h"
#include "cmdqueue.h"

/* RPC 函数定义 */
void LED_On(int32_t *channel) {
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
}

void LED_Off(int32_t *channel) {
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
}

/* 函数组注册 */
function_group_t led_group = {
    FUNCTION_GROUP("LED",
        FUNCTION_INFO(LED_On, T_NULL, T_DEC64),
        FUNCTION_INFO(LED_Off, T_NULL, T_DEC64)
    )
};

/* DMA 回调：预解析后推入队列 */
cmd_queue_t cmd_queue;
cmd_scanner_t cmd_scanner;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    cmd_prefetch_t prefetch;
    cmdscan_init(&cmd_scanner, rx_buffer, rx_len);
    if (cmdscan_prefetch(&cmd_scanner, &prefetch) == SCAN_COMPLETE) {
        cmd_queue_push(&cmd_queue, &prefetch);
    }
    HAL_UART_Receive_DMA(&huart1, rx_buffer, RX_SIZE);
}

/* 主程序 */
int main(void) {
    register_rpc_group(&led_group);
    cmd_queue_init(&cmd_queue);
    
    while (1) {
        cmd_queue_item_t *item;
        if (cmd_queue_pop(&cmd_queue, &item) == DYNCALL_NO_ERROR) {
            /* 执行时用 cmdparse_args 解析参数 */
            cmd_parse_result_t result;
            cmdparse_args(cmd_queue_get_cmd(&cmd_queue, item),
                          item->cmd_len, &result);
            
            function_info_t *func = find_func(result.func_name);
            if (func) {
                invoke_by_pool(func, result.args);
            }
        }
    }
}
```

**命令格式：**
```
LED_On(1)
LED_Off(1)
```

---

## 2. 队列执行（多条命令）

```c
/* 命令队列 */
cmd_queue_t cmd_queue;
cmd_scanner_t cmd_scanner;

/* DMA 回调：预解析后推入队列 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    cmd_prefetch_t prefetch;
    cmdscan_init(&cmd_scanner, rx_buffer, rx_len);
    if (cmdscan_prefetch(&cmd_scanner, &prefetch) == SCAN_COMPLETE) {
        cmd_queue_push(&cmd_queue, &prefetch);
    }
    HAL_UART_Receive_DMA(&huart1, rx_buffer, RX_SIZE);
}

/* 主循环：逐条执行 */
int main(void) {
    register_rpc_group(&motor_group);
    cmd_queue_init(&cmd_queue);
    
    while (1) {
        cmd_prefetch_t pf;
        if (cmd_queue_pop(&cmd_queue, &pf) == DYNCALL_NO_ERROR) {
            cmd_parse_result_t result;
            cmdparse_args(pf.buf + pf.cmd_start, pf.cmd_len, &result);
            
            function_info_t *func = find_func(result.func_name);
            if (func) {
                invoke_by_pool(func, result.args);
            }
        }
    }
}
```

**命令格式（多条）：**
```
Motor_SetSpeed(1, 100)
Motor_SetSpeed(2, 200)
Motor_Run(1)
```

---

## 3. 中断任务

使用 `cmd_queue_check()` 在长任务中检查是否有中断命令：

```c
/* 电机运行（可被中断） */
void Motor_Run(int32_t *speed) {
    for (int i = 0; i < 1000; i++) {
        /* 检查是否有 "Motor_Stop" 命令 */
        if (cmd_queue_check(&cmd_queue, "Motor_Stop")) {
            return;  /* 被中断，提前退出 */
        }
        
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, *speed);
        HAL_Delay(10);
    }
}

/* 命令队列 */
cmd_queue_t cmd_queue;

/* DMA 回调 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    cmd_prefetch_t prefetch;
    if (cmdscan_prefetch(rx_buffer, rx_len, &prefetch) == SCAN_COMPLETE) {
        cmd_queue_push(&cmd_queue, &prefetch);
    }
    HAL_UART_Receive_DMA(&huart1, rx_buffer, RX_SIZE);
}

/* 主程序 */
int main(void) {
    register_rpc_group(&motor_group);
    cmd_queue_init(&cmd_queue);
    
    while (1) {
        cmd_prefetch_t pf;
        if (cmd_queue_pop(&cmd_queue, &pf) == DYNCALL_NO_ERROR) {
            cmd_parse_result_t result;
            cmdparse_args(pf.buf + pf.cmd_start, pf.cmd_len, &result);
            
            function_info_t *func = find_func(result.func_name);
            if (func) {
                invoke_by_pool(func, result.args);
            }
        }
    }
}
```

**使用流程：**
1. 发送 `Motor_Run(100)` 启动电机
2. 发送 `Motor_Stop` 中断电机运行

---

## 注意事项

1. **回调中不要执行命令**：DMA 回调只预解析并推入队列，在主循环中执行
2. **参数必须是指针**：`void Func(int32_t *value)` 而不是 `void Func(int32_t value)`
3. **函数名区分大小写**
4. **DMA 缓冲区**：`cmdscan_prefetch` 支持环形/非环形缓冲区，传入实际数据长度即可
