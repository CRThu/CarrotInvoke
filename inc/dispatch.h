/****************************
 * DISPATCH v2 - 函数注册与查找
 * CarrotRPC
 *
 * 完全自包含，不依赖旧代码
 * 运行时字符串签名注册
 *
 * 用法：
 *   dispatch_reg(add, "(i, i) -> i");
 *   dispatch_reg(hello, "()");
 *****************************/
#pragma once
#ifndef _DISPATCH_H_
#define _DISPATCH_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <inttypes.h>

/*=============================================================
 * 常量
 *=============================================================*/
#define DISPATCH_MAX_FUNC_CNT   256
#define DISPATCH_ARGS_MAX_CNT   9
#define DISPATCH_FUNC_NAME_MAX  64
#define DISPATCH_SIG_MAX        128

/*=============================================================
 * 状态码
 *=============================================================*/
typedef enum {
    DISPATCH_OK = 0,
    DISPATCH_ERR_NULL,          // 空指针
    DISPATCH_ERR_FULL,          // 注册表已满
    DISPATCH_ERR_SIG,           // 签名解析失败
    DISPATCH_ERR_NOT_FOUND,     // 函数未找到
} dispatch_status_t;

/*=============================================================
 * 类型系统 (独立于 dynpool)
 *=============================================================*/
typedef enum {
    DV = 0,     // void
    DI,         // int64
    DU,         // uint64
    DS,         // string
    DF,         // float64
} dispatch_type_t;

/*=============================================================
 * 函数信息
 *=============================================================*/
typedef struct {
    const char*  name;
    void*        handler;
    uint8_t      ret_type;                   // dispatch_type_t
    uint8_t      args_type[DISPATCH_ARGS_MAX_CNT];
    uint8_t      args_count;
} dispatch_func_t;

/*=============================================================
 * 注册宏
 *=============================================================*/
#define dispatch_reg(HANDLER, SIG) \
    _dispatch_add(#HANDLER, (void*)(HANDLER), SIG)

/*=============================================================
 * 公开 API
 *=============================================================*/

/**
 * @brief 重置所有注册信息
 */
void dispatch_init(void);

/**
 * @brief 注册函数 (内部由宏调用)
 *
 * @param name    函数名 (字符串)
 * @param handler 函数指针
 * @param sig     签名字符串, 如 "(i, i) -> i" 或 "hello()"
 * @return dispatch_status_t
 */
dispatch_status_t _dispatch_add(const char* name, void* handler, const char* sig);

/**
 * @brief 查找函数 (null-terminated name)
 *
 * @param name 函数名
 * @return dispatch_func_t* 找到的函数信息, NULL=未找到
 */
dispatch_func_t* dispatch_find(const char* name);

/**
 * @brief 查找函数 (长度限定, 非 null-terminate)
 *
 * @param name 函数名指针
 * @param len  函数名长度
 * @return dispatch_func_t* 找到的函数信息, NULL=未找到
 */
dispatch_func_t* dispatch_find_len(const char* name, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* _DISPATCH_H_ */
