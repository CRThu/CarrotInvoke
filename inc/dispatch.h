/****************************
 * DISPATCH - 函数注册与查找
 * CarrotRPC
 *
 * 设计目标：
 * 1. 拥有独立的函数注册表 (static groups[8] + group_count)
 * 2. 无依赖：不依赖 typeconv 或 invoke
 * 3. 提供长度限定查找 (适配 cmdparse_args 非 null-terminate 输出)
 *****************************/
#pragma once
#ifndef _DISPATCH_H_
#define _DISPATCH_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "dyncall.h"

/**
 * @brief 重置所有注册信息
 */
void dispatch_init(void);

/**
 * @brief 注册函数组
 *
 * @param group 函数组指针
 * @return int 0=成功, -1=已满
 */
int dispatch_register(function_group_t* group);

/**
 * @brief 查找函数 (null-terminated name)
 *
 * @param name 函数名
 * @return function_info_t* 找到的函数信息, NULL=未找到
 */
function_info_t* dispatch_find(const char* name);

/**
 * @brief 查找函数 (长度限定, 非 null-terminate)
 *
 * cmdparse_args 输出的 func_name 不保证 null 终止，
 * 此函数通过栈上临时 null-terminate 实现匹配
 *
 * @param name 函数名指针
 * @param len  函数名长度
 * @return function_info_t* 找到的函数信息, NULL=未找到
 */
function_info_t* dispatch_find_len(const char* name, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* _DISPATCH_H_ */
