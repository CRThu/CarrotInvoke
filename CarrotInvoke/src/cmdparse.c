#include "cmdparse.h"

static inline bool is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\r';
}

static inline bool is_terminator(char c)
{
    return c == '\n' || c == '\0';
}

static inline bool is_separator(char c)
{
    return c == ',' || c == ';' || is_whitespace(c);
}

static inline bool is_left_bracket(char c)
{
    return c == '(' || c == '[' || c == '{';
}

static inline bool is_right_bracket(char c)
{
    return c == ')' || c == ']' || c == '}';
}

static void store_token(dynpool_t* pool, const char* start, uint16_t len)
{
    // 去除两端空白
    while (len > 0 && is_whitespace(*start)) { start++; len--; }
    while (len > 0 && is_whitespace(start[len - 1])) { len--; }

    if (len > 0)
    {
        // 允许空参数但跳过函数名前空白
        dynpool_set(pool, T_STRING, start, len);
    }
}

cmd_parse_status_t cmdparse_from_buffer(dynpool_t* pool, const uint8_t* buf,
                                        uint16_t offset, uint16_t size, uint16_t* len)
{
    uint16_t i = offset;
    uint16_t start = offset;
    uint16_t depth = 0;

    dynpool_init(pool);

    while (i <= offset + size)
    {
        char c = buf[i];

        if (is_whitespace(c))
        {
        }
        if (is_terminator(c))
        {
            store_token(pool, (char*)&buf[start], i - start);
            i = i + 1;
            *len = i - offset;
            break;
        }
        if (is_left_bracket(c))
        {
            depth++;
            store_token(pool, (char*)&buf[start], i - start);
            start = i + 1;
        }
        if (is_right_bracket(c))
        {
            depth--;
            store_token(pool, (char*)&buf[start], i - start);
            start = i + 1;
        }
        if (is_separator(c))
        {
            store_token(pool, (char*)&buf[start], i - start);
            start = i + 1;
        }
        i = i + 1;
    }
    return (depth == 0) ? CMDPARSE_OK : CMDPARSE_INVALID_FORMAT;
}

cmd_parse_status_t cmdparse_from_string(dynpool_t* pool, const char* str, uint16_t* len)
{
    return cmdparse_from_buffer(pool, (uint8_t*)str, 0, (uint16_t)strlen(str), len);
}