#include "dyncall.h"

/// <summary>
/// 获取函数
/// </summary>
/// <param name="group">函数组</param>
/// <param name="name">函数名称</param>
/// <returns>找到的函数,没有返回NULL</returns>
function_info_t* get_func_by_name(function_group_t* group, char* name)
{
    // 遍历注册方法
    for (uint16_t i = 0; i < group->func_count; i++)
    {
        // 寻找匹配方法名称
        if (NAME_ISEQUAL(group->func_table[i].name, name))
        {
            return &group->func_table[i];
        }
    }

    return NULL;
}


void invoke(function_group_t* group, char* funcname, ...)
{
    function_info_t* f = get_func_by_name(group, funcname);
    if (f == NULL)
    {
        printf("Function '%s' not found\n", funcname);
        return;
    }

    // 准备参数
    va_list args;
    va_start(args, funcname);

    dynpool_t pool;
    dynpool_init(&pool);

    // 根据函数参数类型添加参数到pool
    for (uint8_t i = 0; i < GET_FUNC_ARGS_COUNT(f); i++)
    {
        dtypes_t type = f->args_type[i];

        switch (type)
        {
            case T_DEC64:
            {
                int64_t val = va_arg(args, int64_t);
                dynpool_set(&pool, T_DEC64, &val, sizeof(val));
                break;
            }
            case T_HEX64:
            {
                uint64_t val = va_arg(args, uint64_t);
                dynpool_set(&pool, T_HEX64, &val, sizeof(val));
                break;
            }

            case T_STRING:
            {
                char* val = va_arg(args, char*);
                dynpool_set(&pool, T_STRING, val, strlen(val) + 1);
                break;
            }

            case T_VOID:
            {
                break;
            }
            default:
            {
                break;
            }
        }
    }

    va_end(args);

    #if(DYNCALL_DEBUG)
    dynpool_print(&pool);
    #endif

    // 调用函数
    invoke_by_pool(&pool, f);
}

/// <summary>
/// 调用函数
/// </summary>
/// <param name="pool">数据存储结构</param>
/// <param name="f">函数</param>
void invoke_by_pool(dynpool_t* pool, function_info_t* f)
{
    if (f == NULL || pool == NULL)
    {
        printf("Invalid function or pool\n");
        return;
    }

    // 创建新的dynpool来存储转换后的参数
    dynpool_t arg_pool;
    dynpool_init(&arg_pool);

    // 从原始pool中读取并转换参数到新pool
    for (uint8_t i = 0; i < GET_FUNC_ARGS_COUNT(f); i++)
    {
        uint16_t used_size = 0;
        uint8_t arg_buffer[DYNCALL_ARGS_MAX_SIZE];

        // 从原始pool读取参数
        dynpool_status_t status = dynpool_get(pool, f->args_type[i], arg_buffer, sizeof(arg_buffer), &used_size);

        if (status != DYNPOOL_NO_ERROR)
        {
            printf("Failed to get argument %d\n", i);
            return;
        }

        // 将参数存入新pool
        dynpool_set(&arg_pool, f->args_type[i], arg_buffer, used_size);
    }

    #if(DYNCALL_DEBUG)
    dynpool_print(&arg_pool);
    #endif

    // 根据参数数量调用函数
    dtypes_t* dtype;
    uint16_t used_size = 0;
    switch (GET_FUNC_ARGS_COUNT(f))
    {
        case 0:
        {
            ((delegate_a0r0)f->handler)();
            break;
        }
        case 1:
        {
            void* arg0;
            dynpool_peek(&arg_pool, 0, &dtype, &arg0, &used_size);
            ((delegate_a1r0)f->handler)(&arg0);
            break;
        }
        case 2:
        {
            void* arg0, * arg1;
            dynpool_peek(&arg_pool, 0, &dtype, &arg0, &used_size);
            dynpool_peek(&arg_pool, 1, &dtype, &arg1, &used_size);
            ((delegate_a2r0)f->handler)(&arg0, &arg1);
            break;
        }
        case 3:
        {
            void* arg0, * arg1, * arg2;
            dynpool_peek(&arg_pool, 0, &dtype, &arg0, &used_size);
            dynpool_peek(&arg_pool, 1, &dtype, &arg1, &used_size);
            dynpool_peek(&arg_pool, 2, &dtype, &arg2, &used_size);
            ((delegate_a3r0)f->handler)(&arg0, &arg1, &arg2);
            break;
        }
        case 4:
        {
            void* arg0, * arg1, * arg2, * arg3;
            dynpool_peek(&arg_pool, 0, &dtype, &arg0, &used_size);
            dynpool_peek(&arg_pool, 1, &dtype, &arg1, &used_size);
            dynpool_peek(&arg_pool, 2, &dtype, &arg2, &used_size);
            dynpool_peek(&arg_pool, 3, &dtype, &arg3, &used_size);
            ((delegate_a4r0)f->handler)(&arg0, &arg1, &arg2, &arg3);
            break;
        }
        case 5:
        {
            void* arg0, * arg1, * arg2, * arg3, * arg4;
            dynpool_peek(&arg_pool, 0, &dtype, &arg0, &used_size);
            dynpool_peek(&arg_pool, 1, &dtype, &arg1, &used_size);
            dynpool_peek(&arg_pool, 2, &dtype, &arg2, &used_size);
            dynpool_peek(&arg_pool, 3, &dtype, &arg3, &used_size);
            dynpool_peek(&arg_pool, 4, &dtype, &arg4, &used_size);
            ((delegate_a5r0)f->handler)(&arg0, &arg1, &arg2, &arg3, &arg4);
            break;
        }
        case 6:
        {
            void* arg0, * arg1, * arg2, * arg3, * arg4, * arg5;
            dynpool_peek(&arg_pool, 0, &dtype, &arg0, &used_size);
            dynpool_peek(&arg_pool, 1, &dtype, &arg1, &used_size);
            dynpool_peek(&arg_pool, 2, &dtype, &arg2, &used_size);
            dynpool_peek(&arg_pool, 3, &dtype, &arg3, &used_size);
            dynpool_peek(&arg_pool, 4, &dtype, &arg4, &used_size);
            dynpool_peek(&arg_pool, 5, &dtype, &arg5, &used_size);
            ((delegate_a6r0)f->handler)(&arg0, &arg1, &arg2, &arg3, &arg4, &arg5);
            break;
        }
        case 7:
        {
            void* arg0, * arg1, * arg2, * arg3, * arg4, * arg5, * arg6;
            dynpool_peek(&arg_pool, 0, &dtype, &arg0, &used_size);
            dynpool_peek(&arg_pool, 1, &dtype, &arg1, &used_size);
            dynpool_peek(&arg_pool, 2, &dtype, &arg2, &used_size);
            dynpool_peek(&arg_pool, 3, &dtype, &arg3, &used_size);
            dynpool_peek(&arg_pool, 4, &dtype, &arg4, &used_size);
            dynpool_peek(&arg_pool, 5, &dtype, &arg5, &used_size);
            dynpool_peek(&arg_pool, 6, &dtype, &arg6, &used_size);
            ((delegate_a7r0)f->handler)(&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6);
            break;
        }
        case 8:
        {
            void* arg0, * arg1, * arg2, * arg3, * arg4, * arg5, * arg6, * arg7;
            dynpool_peek(&arg_pool, 0, &dtype, &arg0, &used_size);
            dynpool_peek(&arg_pool, 1, &dtype, &arg1, &used_size);
            dynpool_peek(&arg_pool, 2, &dtype, &arg2, &used_size);
            dynpool_peek(&arg_pool, 3, &dtype, &arg3, &used_size);
            dynpool_peek(&arg_pool, 4, &dtype, &arg4, &used_size);
            dynpool_peek(&arg_pool, 5, &dtype, &arg5, &used_size);
            dynpool_peek(&arg_pool, 6, &dtype, &arg6, &used_size);
            dynpool_peek(&arg_pool, 7, &dtype, &arg7, &used_size);
            ((delegate_a8r0)f->handler)(&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7);
            break;
        }
        case 9:
        {
            void* arg0, * arg1, * arg2, * arg3, * arg4, * arg5, * arg6, * arg7, * arg8;
            dynpool_peek(&arg_pool, 0, &dtype, &arg0, &used_size);
            dynpool_peek(&arg_pool, 1, &dtype, &arg1, &used_size);
            dynpool_peek(&arg_pool, 2, &dtype, &arg2, &used_size);
            dynpool_peek(&arg_pool, 3, &dtype, &arg3, &used_size);
            dynpool_peek(&arg_pool, 4, &dtype, &arg4, &used_size);
            dynpool_peek(&arg_pool, 5, &dtype, &arg5, &used_size);
            dynpool_peek(&arg_pool, 6, &dtype, &arg6, &used_size);
            dynpool_peek(&arg_pool, 7, &dtype, &arg7, &used_size);
            dynpool_peek(&arg_pool, 8, &dtype, &arg8, &used_size);
            ((delegate_a9r0)f->handler)(&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8);
            break;
        }
        // 其他参数数量的情况...
        default:
        {

            printf("Unsupported argument count: %d\n", f->args_count);
            break;
        }
    }
}
