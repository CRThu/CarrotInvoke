#include "cmdparse.h"


cmd_parse_status_t cmdparse_from_string(dynpool_t* pool, char* cmd, uint16_t* cmd_size)
{
    cmdparse_from_buffer(pool);
}

/// <summary>
/// 解析指令
/// </summary>
/// <param name="pool">dynpool_t结构体</param>
/// <param name="cmdbuf">指令缓冲区</param>
/// <param name="offset">指令缓冲区偏移量</param>
/// <param name="size">指令缓冲区总长度</param>
/// <param name="cmd_size">解析字符串长度</param>
/// <returns>解析状态</returns>
cmd_parse_status_t cmdparse_from_buffer(dynpool_t* pool, uint8_t* cmdbuf, uint16_t offset, uint16_t size, uint16_t* cmd_size)
{
    uint8_t args_index = 0;
    uint16_t cursor = 0;
    uint16_t start_pos = 0;
    dynpool_init(pool);
    while (cursor < size)
    {
        if (CMD_PARSE_END(cmdbuf[cursor]))
        {
            break;
        }

        if (CMD_PARSE_ELEMENT_DELIMITER(cmdbuf[cursor]))
        {
            char* indata = &cmdbuf[start_pos];
            uint16_t inlen = cursor - start_pos;

            dynpool_set(pool, T_STRING, indata, inlen);

            args_index++;
            start_pos = cursor + 1;
        }
        cursor++;
    }

    return 0;
}
