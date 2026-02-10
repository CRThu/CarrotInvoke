#include "dyncall.h"

static function_group_t* _registered_groups[DYNCALL_MAX_GROUPS];
static uint8_t _group_count = 0;

/// <summary>
/// 注册RPC方法组
/// </summary>
/// <param name="group">RPC方法组</param>
/// <returns>状态，成功返回DYNCALL_NO_ERROR</returns>
dyncall_status_t register_rpc_group(function_group_t* group)
{
    if (_group_count >= DYNCALL_MAX_GROUPS)
        return DYNCALL_ERR_INVOKE_UNSUPPORTED;
    _registered_groups[_group_count++] = group;
    return DYNCALL_NO_ERROR;
}

/// <summary>
/// 获取函数
/// </summary>
/// <param name="name">函数名称</param>
/// <returns>找到的函数,没有返回NULL</returns>
function_info_t* find_func(char* name)
{
    // 遍历RPC组
    for (uint8_t i = 0;i < _group_count;i++)
    {
        // 遍历注册方法
        for (uint16_t j = 0; j < _registered_groups[i]->func_count; j++)
        {
            // 寻找匹配方法名称
            if (NAME_ISEQUAL(_registered_groups[i]->func_table[j].name, name))
            {
                return &_registered_groups[i]->func_table[j];
            }
        }
    }
    return NULL;
}

 dyncall_status_t invoke(char* fname, ...)
 {
     function_info_t* f = find_func(fname);
     if (f == NULL)
     {
         DYNCALL_PRINTF("[ERROR]: Function '%s' not found.\r\n", fname);
         return DYNCALL_ERR_FUNC_NOT_FOUND;
     }

     // 准备参数
     va_list args;
     va_start(args, fname);

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
     return invoke_by_pool(&pool, f);
 }

dyncall_status_t invoke_by_cmd(dynpool_t* pool)
{
    if (pool == NULL)
    {
        DYNCALL_PRINTF("[ERROR]: Invalid pool.\r\n");
        return DYNCALL_ERR_NULL_OBJECT;
    }

    uint16_t used_size = 0;
    char fname_buf[DYNCALL_ARGS_MAX_SIZE];
    dynpool_status_t status = dynpool_get(pool, T_STRING, fname_buf, sizeof(fname_buf), &used_size);

    //#if(DYNCALL_DEBUG)
    //dynpool_print(pool);
    //#endif

    if (status != DYNPOOL_NO_ERROR)
    {
        DYNCALL_PRINTF("[ERROR]: Failed to get function '%s' from pool.\r\n", fname_buf);
        return DYNCALL_ERR_POOL;
    }

    function_info_t* f = find_func(fname_buf);
    if (f == NULL)
    {
        DYNCALL_PRINTF("[ERROR]: Function '%s' not found.\r\n", fname_buf);
        return DYNCALL_ERR_FUNC_NOT_FOUND;
    }

    return invoke_by_pool(pool, f);
}

/// <summary>
/// 调用函数
/// </summary>
/// <param name="pool">数据存储结构</param>
/// <param name="f">函数</param>
dyncall_status_t invoke_by_pool(dynpool_t* pool, function_info_t* f)
{
    if (f == NULL || pool == NULL)
    {
        DYNCALL_PRINTF("[ERROR]: Invalid function or pool.\r\n");
        return DYNCALL_ERR_NULL_OBJECT;
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
            if (status == DYNPOOL_ERR_NO_DATA)
            {
                DYNCALL_PRINTF("[WARNING]: No data at argument %d.\r\n", i);
            }
            else
            {
                DYNCALL_PRINTF("[ERROR]: Failed to get argument %d.\r\n", i);
                return DYNCALL_ERR_POOL;
            }
        }

        // 将参数存入新pool
        dynpool_set(&arg_pool, f->args_type[i], arg_buffer, used_size);
    }

    #if(DYNCALL_DEBUG)
    dynpool_print(&arg_pool);
    #endif

    // 根据参数数量调用函数
    dtypes_t dtype;
    uint16_t size;
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
        dynpool_peek(&arg_pool, 0, &dtype, &arg0, &size);
        ((delegate_a1r0)f->handler)(&arg0);
        break;
    }
    case 2:
    {
        void* arg0, * arg1;
        dynpool_peek(&arg_pool, 0, &dtype, &arg0, &size);
        dynpool_peek(&arg_pool, 1, &dtype, &arg1, &size);
        ((delegate_a2r0)f->handler)(&arg0, &arg1);
        break;
    }
    case 3:
    {
        void* arg0, * arg1, * arg2;
        dynpool_peek(&arg_pool, 0, &dtype, &arg0, &size);
        dynpool_peek(&arg_pool, 1, &dtype, &arg1, &size);
        dynpool_peek(&arg_pool, 2, &dtype, &arg2, &size);
        ((delegate_a3r0)f->handler)(&arg0, &arg1, &arg2);
        break;
    }
    case 4:
    {
        void* arg0, * arg1, * arg2, * arg3;
        dynpool_peek(&arg_pool, 0, &dtype, &arg0, &size);
        dynpool_peek(&arg_pool, 1, &dtype, &arg1, &size);
        dynpool_peek(&arg_pool, 2, &dtype, &arg2, &size);
        dynpool_peek(&arg_pool, 3, &dtype, &arg3, &size);
        ((delegate_a4r0)f->handler)(&arg0, &arg1, &arg2, &arg3);
        break;
    }
    case 5:
    {
        void* arg0, * arg1, * arg2, * arg3, * arg4;
        dynpool_peek(&arg_pool, 0, &dtype, &arg0, &size);
        dynpool_peek(&arg_pool, 1, &dtype, &arg1, &size);
        dynpool_peek(&arg_pool, 2, &dtype, &arg2, &size);
        dynpool_peek(&arg_pool, 3, &dtype, &arg3, &size);
        dynpool_peek(&arg_pool, 4, &dtype, &arg4, &size);
        ((delegate_a5r0)f->handler)(&arg0, &arg1, &arg2, &arg3, &arg4);
        break;
    }
    case 6:
    {
        void* arg0, * arg1, * arg2, * arg3, * arg4, * arg5;
        dynpool_peek(&arg_pool, 0, &dtype, &arg0, &size);
        dynpool_peek(&arg_pool, 1, &dtype, &arg1, &size);
        dynpool_peek(&arg_pool, 2, &dtype, &arg2, &size);
        dynpool_peek(&arg_pool, 3, &dtype, &arg3, &size);
        dynpool_peek(&arg_pool, 4, &dtype, &arg4, &size);
        dynpool_peek(&arg_pool, 5, &dtype, &arg5, &size);
        ((delegate_a6r0)f->handler)(&arg0, &arg1, &arg2, &arg3, &arg4, &arg5);
        break;
    }
    case 7:
    {
        void* arg0, * arg1, * arg2, * arg3, * arg4, * arg5, * arg6;
        dynpool_peek(&arg_pool, 0, &dtype, &arg0, &size);
        dynpool_peek(&arg_pool, 1, &dtype, &arg1, &size);
        dynpool_peek(&arg_pool, 2, &dtype, &arg2, &size);
        dynpool_peek(&arg_pool, 3, &dtype, &arg3, &size);
        dynpool_peek(&arg_pool, 4, &dtype, &arg4, &size);
        dynpool_peek(&arg_pool, 5, &dtype, &arg5, &size);
        dynpool_peek(&arg_pool, 6, &dtype, &arg6, &size);
        ((delegate_a7r0)f->handler)(&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6);
        break;
    }
    case 8:
    {
        void* arg0, * arg1, * arg2, * arg3, * arg4, * arg5, * arg6, * arg7;
        dynpool_peek(&arg_pool, 0, &dtype, &arg0, &size);
        dynpool_peek(&arg_pool, 1, &dtype, &arg1, &size);
        dynpool_peek(&arg_pool, 2, &dtype, &arg2, &size);
        dynpool_peek(&arg_pool, 3, &dtype, &arg3, &size);
        dynpool_peek(&arg_pool, 4, &dtype, &arg4, &size);
        dynpool_peek(&arg_pool, 5, &dtype, &arg5, &size);
        dynpool_peek(&arg_pool, 6, &dtype, &arg6, &size);
        dynpool_peek(&arg_pool, 7, &dtype, &arg7, &size);
        ((delegate_a8r0)f->handler)(&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7);
        break;
    }
    case 9:
    {
        void* arg0, * arg1, * arg2, * arg3, * arg4, * arg5, * arg6, * arg7, * arg8;
        dynpool_peek(&arg_pool, 0, &dtype, &arg0, &size);
        dynpool_peek(&arg_pool, 1, &dtype, &arg1, &size);
        dynpool_peek(&arg_pool, 2, &dtype, &arg2, &size);
        dynpool_peek(&arg_pool, 3, &dtype, &arg3, &size);
        dynpool_peek(&arg_pool, 4, &dtype, &arg4, &size);
        dynpool_peek(&arg_pool, 5, &dtype, &arg5, &size);
        dynpool_peek(&arg_pool, 6, &dtype, &arg6, &size);
        dynpool_peek(&arg_pool, 7, &dtype, &arg7, &size);
        dynpool_peek(&arg_pool, 8, &dtype, &arg8, &size);
        ((delegate_a9r0)f->handler)(&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8);
        break;
    }
    // 其他参数数量的情况...
    default:
    {

        DYNCALL_PRINTF("[ERROR]: Unsupported argument count: %d.\r\n", f->args_count);
        return DYNCALL_ERR_INVOKE_UNSUPPORTED;
    }
    }
    return DYNCALL_NO_ERROR;
}
