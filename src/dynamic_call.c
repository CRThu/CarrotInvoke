#include "../Inc/dynamic_call.h"

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


void invoke_by_cmd(function_group_t* group, char* cmd)
{
    function_info_t* f = get_func_by_name(group, cmd);
    uint8_t args[10];
    switch (f->args_count)
    {
        case 0:
            ((delegate_a0r0)f->handler)();
            break;
        case 1:
            ((delegate_a1r0)f->handler)(args[0]);
            break;
        case 2:
            ((delegate_a2r0)f->handler)(args[0], args[1]);
            break;
        case 3:
            ((delegate_a3r0)f->handler)(args[0], args[1], args[2]);
            break;
        case 4:
            ((delegate_a4r0)f->handler)(args[0], args[1], args[2], args[3]);
            break;
        case 5:
            ((delegate_a5r0)f->handler)(args[0], args[1], args[2], args[3], args[4]);
            break;
        case 6:
            ((delegate_a6r0)f->handler)(args[0], args[1], args[2], args[3], args[4], args[5]);
            break;
        case 7:
            ((delegate_a7r0)f->handler)(args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
            break;
        case 8:
            ((delegate_a8r0)f->handler)(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
            break;
        case 9:
            ((delegate_a9r0)f->handler)(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]);
            break;
        default:
            // TODO
            break;
    }
}

/// <summary>
/// 调用函数
/// </summary>
/// <param name="pool">数据存储结构</param>
/// <param name="f">函数</param>
void invoke(dynpool_t* pool, function_info_t* f)
{
    // method invoke
    uint8_t args[1024];
    uint16_t args_cursor = 0;
    uint16_t used_size = 0;

    for (uint16_t i = 1; i <= f->args_count; i++)
    {
        dynpool_get(pool, f->args_type[i - 1], args[args_cursor], 1024, &used_size);
        args_cursor += used_size;
    }

    switch (f->args_count)
    {
        case 0:
            ((delegate_a0r0)f->handler)();
            break;
        case 1:
            ((delegate_a1r0)f->handler)(args[0]);
            break;
        case 2:
            ((delegate_a2r0)f->handler)(args[0], args[1]);
            break;
        case 3:
            ((delegate_a3r0)f->handler)(args[0], args[1], args[2]);
            break;
        case 4:
            ((delegate_a4r0)f->handler)(args[0], args[1], args[2], args[3]);
            break;
        case 5:
            ((delegate_a5r0)f->handler)(args[0], args[1], args[2], args[3], args[4]);
            break;
        case 6:
            ((delegate_a6r0)f->handler)(args[0], args[1], args[2], args[3], args[4], args[5]);
            break;
        case 7:
            ((delegate_a7r0)f->handler)(args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
            break;
        case 8:
            ((delegate_a8r0)f->handler)(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
            break;
        case 9:
            ((delegate_a9r0)f->handler)(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]);
            break;
        default:
            // TODO
            break;
    }
}
