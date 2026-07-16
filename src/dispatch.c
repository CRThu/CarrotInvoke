/****************************
 * DISPATCH - 函数注册与查找
 * CarrotRPC
 *
 * 独立注册表，不依赖旧模块
 *****************************/
#include "dispatch.h"
#include <string.h>

/* 独立于旧 dyncall.c 的注册表 */
static function_group_t* _groups[DYNCALL_MAX_GROUPS];
static uint8_t _group_count = 0;

void dispatch_init(void)
{
    _group_count = 0;
    memset(_groups, 0, sizeof(_groups));
}

int dispatch_register(function_group_t* group)
{
    if (group == NULL)
        return -1;
    if (_group_count >= DYNCALL_MAX_GROUPS)
        return -1;

    _groups[_group_count++] = group;
    return 0;
}

function_info_t* dispatch_find(const char* name)
{
    if (name == NULL)
        return NULL;

    for (uint8_t i = 0; i < _group_count; i++)
    {
        for (uint16_t j = 0; j < _groups[i]->func_count; j++)
        {
            if (strcmp(_groups[i]->func_table[j].name, name) == 0)
            {
                return &_groups[i]->func_table[j];
            }
        }
    }
    return NULL;
}

function_info_t* dispatch_find_len(const char* name, uint16_t len)
{
    if (name == NULL || len == 0)
        return NULL;

    char name_buf[DYNCALL_ARGS_MAX_SIZE];
    uint16_t copy_len = len < (DYNCALL_ARGS_MAX_SIZE - 1) ? len : (DYNCALL_ARGS_MAX_SIZE - 1);
    memcpy(name_buf, name, copy_len);
    name_buf[copy_len] = '\0';

    return dispatch_find(name_buf);
}
