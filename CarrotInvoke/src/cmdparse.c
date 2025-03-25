#include "cmdparse.h"

typedef enum
{
    STATE_FUNC,    // 解析函数名
    STATE_ARGS     // 解析参数
} parse_state_t;

static inline bool is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\r';
}

static inline bool is_terminator(char c)
{
    return c == '\n' || c == '\0' || c == ';';
}

static void store_token(dynpool_t* pool, const char* start, uint16_t len)
{
    // 去除两端空白
    while (len > 0 && is_whitespace(*start)) { start++; len--; }
    while (len > 0 && is_whitespace(start[len - 1])) { len--; }

    if (len > 0 || pool->wr_count > 1)
    {
        // 允许空参数但跳过函数名前空白
        dynpool_set(pool, T_STRING, start, len);
    }
}

cmd_parse_status_t cmdparse_from_buffer(dynpool_t* pool, const uint8_t* buf,
                                        uint16_t offset, uint16_t size, uint16_t* len)
{
    parse_state_t state = STATE_FUNC;
    uint16_t start = offset, depth = 0;
    bool has_func = false;

    dynpool_init(pool);

    for (uint16_t i = offset; i < offset + size; i++)
    {
        char c = buf[i];

        if (is_whitespace(c)) continue;
        if (is_terminator(c)) { *len = i - offset; break; }

        switch (state)
        {
            case STATE_FUNC:
                if (c == '(')
                {
                    store_token(pool, (char*)&buf[start], i - start);
                    state = STATE_ARGS;
                    start = i + 1;
                    has_func = true;
                }
                break;

            case STATE_ARGS:
                if (c == '(') depth++;
                else if (c == ')')
                {
                    if (depth-- == 0)
                    {
                        store_token(pool, (char*)&buf[start], i - start);
                        *len = i - offset + 1;
                        return has_func ? CMDPARSE_OK : CMDPARSE_INVALID_FORMAT;
                    }
                }
                else if (depth == 0 && c == ',')
                {
                    store_token(pool, (char*)&buf[start], i - start);
                    start = i + 1;
                }
                break;
        }
    }

    return (has_func && state == STATE_ARGS && depth == 0) ? CMDPARSE_OK : CMDPARSE_INVALID_FORMAT;
}

cmd_parse_status_t cmdparse_from_string(dynpool_t* pool, const char* str, uint16_t* len)
{
    return cmdparse_from_buffer(pool, (uint8_t*)str, 0, (uint16_t)strlen(str), len);
}