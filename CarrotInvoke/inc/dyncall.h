/****************************
* DYNCALL
* CRTHu
* 2025.03.25
*****************************/
#pragma once
#ifndef _DYNCALL_H_
#define _DYNCALL_H_

#ifdef __cplusplus
extern "C"
{
    #endif

    #ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
    #endif // !_CRT_SECURE_NO_WARNINGS


    #include <inttypes.h>
    #include <stdio.h>
    #include <stdarg.h>
    #include "dynpool.h"
    #include "cmdparse.h"

    #define DYNCALL_VERSION		        "1.1.0"

    #define DYNCALL_DEBUG               1
    #define DYNCALL_FUNC_SIG            0


    #define DYNCALL_FUNC_MAX_CNT        256
    #define DYNCALL_ARGS_MAX_CNT	    9
    #define DYNCALL_ARGS_MAX_SIZE       64

    #define NAME_ISEQUAL(a,b)			(strcmp(a, b) == 0)
    #define FN_ARGS_CNT(args)			(strlen(args))


    #define PVAL(p)						(**(p))
    #define PREF(p)						(*(p))

    typedef int64_t ret_dec64_t;
    typedef int64_t** dyn_dec64p_t;
    typedef uint64_t** dyn_hex64p_t;
    typedef uint64_t** dyn_enump_t;
    typedef char** dyn_string_t;
    typedef uint8_t** dyn_bytes_t;


    typedef void* delegate;
    typedef void (*delegate_a0r0)(void);
    typedef void (*delegate_a1r0)(void* arg1);
    typedef void (*delegate_a2r0)(void* arg1, void* arg2);
    typedef void (*delegate_a3r0)(void* arg1, void* arg2, void* arg3);
    typedef void (*delegate_a4r0)(void* arg1, void* arg2, void* arg3, void* arg4);
    typedef void (*delegate_a5r0)(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5);
    typedef void (*delegate_a6r0)(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
    typedef void (*delegate_a7r0)(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6, void* arg7);
    typedef void (*delegate_a8r0)(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6, void* arg7, void* arg8);
    typedef void (*delegate_a9r0)(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6, void* arg7, void* arg8, void* arg9);

    typedef struct
    {
        char* name;
        delegate handler;
        dtypes_t ret_type;
        dtypes_t args_type[DYNCALL_ARGS_MAX_CNT];
        uint8_t args_count;
    } function_info_t;

    typedef struct
    {
        char* name;
        function_info_t* func_table;
        uint16_t func_count;
    } function_group_t;

    #define FUNCTION_GROUP(NAME, ...)                                                   \
    .name = NAME,                                                                       \
    .func_table = (function_info_t[]) { __VA_ARGS__ },                                  \
    .func_count = sizeof((function_info_t[]) { __VA_ARGS__ }) / sizeof(function_info_t) \

    #define FUNCTION_INFO(HANDLER, RET, ...) {                                          \
    .name = #HANDLER,                                                                   \
    .handler = HANDLER,                                                                 \
    .ret_type = RET,                                                                    \
    .args_type = { __VA_ARGS__ },                                                       \
    .args_count = sizeof((dtypes_t[]) { __VA_ARGS__ }) / sizeof(dtypes_t)               \
    }

    // check if func args is only one and type is void for msvc
    #define GET_FUNC_ARGS_COUNT(f)     (((f)->args_count == 1 && (f)->args_type[0] == T_VOID) ? 0 : (f)->args_count)

    #if defined(__GNUC__) || defined(__clang__)
    #define FUNC_SIGNATURE __PRETTY_FUNCTION__
    #elif defined(_MSC_VER)
    #define FUNC_SIGNATURE __FUNCSIG__
    #else
    #define FUNC_SIGNATURE __func__
    #endif

    #if(DYNCALL_FUNC_SIG)
    #define PRINT_FUNC_SIGNATURE()  printf("[funcsig]: %s\n", FUNC_SIGNATURE)
    #else
    #define PRINT_FUNC_SIGNATURE()  void
    #endif

    function_info_t* get_func_by_name(function_group_t* group, char* name);
    void invoke(function_group_t* group, char* cmd, ...);
    void invoke_by_cmd(function_group_t* group, dynpool_t* pool);
    void invoke_by_pool(dynpool_t* pool, function_info_t* f);


    #ifdef __cplusplus
}
#endif

#endif /* _DYNCALL_H_ */