/****************************
 * DISPATCH Demo - 函数注册与查找演示
 *
 * 演示 dispatch 模块功能：
 * 1. 函数注册
 * 2. 函数查找
 * 3. 类型签名解析
 *****************************/
#include "dispatch.h"
#include <stdio.h>
#include <string.h>

/*=============================================================
 * 示例 RPC 处理函数
 *=============================================================*/
static void hello(void)
{
    printf("[RPC] hello()\n");
}

static int64_t add(void* a, void* b)
{
    return *(int64_t*)a + *(int64_t*)b;
}

static void led_on(void* channel)
{
    printf("[RPC] LED_On(channel=%d)\n", *(int64_t*)channel);
}

static void motor_set(void* id, void* speed)
{
    printf("[RPC] motor_set(id=%d, speed=%d)\n", *(int64_t*)id, *(int64_t*)speed);
}

/*=============================================================
 * 主函数
 *=============================================================*/
int main(void)
{
    static dispatch_registry_t dispatcher;

    printf("=== Dispatch Demo ===\n\n");

    /* 1. 初始化注册表 */
    dispatch_init(&dispatcher);
    printf("Registry initialized\n\n");

    /* 2. 注册函数 */
    printf("--- Register functions ---\n");
    dispatch_reg(&dispatcher, hello, "hello()");
    dispatch_reg(&dispatcher, add, "add(i, i) -> i");
    dispatch_reg(&dispatcher, led_on, "LED_On(i)");
    dispatch_reg(&dispatcher, motor_set, "motor_set(i, i)");
    printf("Registered 4 functions\n\n");

    /* 3. 查找函数 */
    printf("--- Find functions ---\n");
    dispatch_func_t* f;

    f = dispatch_find(&dispatcher, "hello", 5);
    printf("find('hello') = %s -> %s\n", f ? "found" : "NOT found", f ? f->name : "");

    f = dispatch_find(&dispatcher, "add", 3);
    printf("find('add') = %s -> %s, ret_type=%d, args_count=%d\n",
           f ? "found" : "NOT found",
           f ? f->name : "",
           f ? f->ret_type : -1,
           f ? f->args_count : -1);

    f = dispatch_find(&dispatcher, "LED_On", 6);
    printf("find('LED_On') = %s -> %s\n", f ? "found" : "NOT found", f ? f->name : "");

    f = dispatch_find(&dispatcher, "motor_set", 9);
    printf("find('motor_set') = %s -> %s, args_count=%d\n",
           f ? "found" : "NOT found",
           f ? f->name : "",
           f ? f->args_count : -1);

    /* 4. 查找不存在的函数 */
    printf("\n--- Find non-existent function ---\n");
    f = dispatch_find(&dispatcher, "nonexistent", 11);
    printf("find('nonexistent') = %s\n", f ? "found" : "NOT found");

    /* 5. 重置注册表 */
    printf("\n--- Reset registry ---\n");
    dispatch_init(&dispatcher);
    f = dispatch_find(&dispatcher, "hello", 5);
    printf("After reset: find('hello') = %s\n", f ? "found" : "NOT found");

    printf("\n=== Demo Complete ===\n");
    return 0;
}
