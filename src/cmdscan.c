/****************************
* CMD SCAN - 零拷贝状态机解析器
* CRTHu
* 2025.07.15
*****************************/
#include "cmdscan.h"

/*=============================================================
 * 内部辅助函数（无 std 库依赖）
 *=============================================================*/

/**
 * @brief 检查字符是否为空白字符
 */
static inline int8_t is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\r';
}

/**
 * @brief 检查字符是否为终止符
 */
static inline int8_t is_terminator(char c)
{
    return c == '\n' || c == '\0';
}

/**
 * @brief 检查字符是否为左括号
 */
static inline int8_t is_left_bracket(char c)
{
    return c == '(';
}

/**
 * @brief 检查字符是否为右括号
 */
static inline int8_t is_right_bracket(char c)
{
    return c == ')';
}

/**
 * @brief 检查字符是否为分隔符
 */
static inline int8_t is_separator(char c)
{
    return c == ',' || c == ';';
}

/*=============================================================
 * 扫描器 API
 *=============================================================*/

void cmdscan_init(cmd_scanner_t* scanner, const uint8_t* buf, uint16_t buf_size)
{
    if (scanner == NULL) return;

    scanner->buf = buf;
    scanner->buf_size = buf_size;
    scanner->scan_pos = 0;
    scanner->cmd_start = 0;
    scanner->cmd_len = 0;
    scanner->func_len = 0;
    scanner->state = SCAN_IDLE;
    scanner->in_args = 0;
    scanner->error = 0;
}

void cmdscan_reset(cmd_scanner_t* scanner)
{
    if (scanner == NULL) return;

    scanner->scan_pos = 0;
    scanner->cmd_start = 0;
    scanner->cmd_len = 0;
    scanner->func_len = 0;
    scanner->state = SCAN_IDLE;
    scanner->in_args = 0;
    scanner->error = 0;
}

scan_status_t cmdscan_scan(cmd_scanner_t* scanner)
{
    if (scanner == NULL || scanner->buf == NULL)
        return SCAN_ERROR;

    /* 从当前扫描位置继续 */
    while (scanner->scan_pos < scanner->buf_size)
    {
        char c = (char)scanner->buf[scanner->scan_pos];

        switch (scanner->state)
        {
        case SCAN_IDLE:
            /* 跳过前导空白 */
            if (is_space(c))
            {
                scanner->scan_pos++;
                break;
            }

            /* 遇到终止符，忽略空行 */
            if (is_terminator(c))
            {
                scanner->scan_pos++;
                break;
            }

            /* 遇到非空白字符，开始扫描命令 */
            scanner->cmd_start = scanner->scan_pos;
            scanner->cmd_len = 0;
            scanner->func_len = 0;
            scanner->in_args = 0;
            scanner->state = SCAN_func_name;
            break;

        case SCAN_func_name:
            /* 扫描函数名，直到遇到左括号或空白 */
            if (is_left_bracket(c))
            {
                scanner->in_args = 1;
                scanner->state = SCAN_args;
                scanner->scan_pos++;
            }
            else if (is_space(c))
            {
                /* 函数名后的空白，继续扫描直到遇到左括号 */
                scanner->scan_pos++;
            }
            else if (is_terminator(c))
            {
                /* 无参函数，直接结束 */
                scanner->cmd_len = scanner->scan_pos - scanner->cmd_start;
                scanner->scan_pos++; /* 跳过 \n */
                scanner->state = SCAN_IDLE;
                return SCAN_COMPLETE;
            }
            else
            {
                /* 普通字符，属于函数名 */
                scanner->scan_pos++;
            }
            break;

        case SCAN_args:
            /* 扫描参数区域 */
            if (is_terminator(c))
            {
                /* 命令结束 */
                scanner->cmd_len = scanner->scan_pos - scanner->cmd_start;
                scanner->scan_pos++; /* 跳过 \n */

                /* 检查括号是否匹配 */
                if (scanner->in_args)
                {
                    scanner->error = -1; /* 括号不匹配 */
                    scanner->state = SCAN_error;
                    return SCAN_ERROR;
                }

                scanner->state = SCAN_IDLE;
                return SCAN_COMPLETE;
            }
            else if (is_right_bracket(c))
            {
                scanner->in_args = 0;
                scanner->scan_pos++;
            }
            else
            {
                /* 普通字符 */
                scanner->scan_pos++;
            }
            break;

        case SCAN_error:
            /* 错误状态，跳过剩余字符直到终止符 */
            if (is_terminator(c))
            {
                scanner->scan_pos++;
                scanner->state = SCAN_IDLE;
                return SCAN_ERROR;
            }
            scanner->scan_pos++;
            break;
        }
    }

    /* 缓冲区扫描完毕，未找到完整命令 */
    return SCAN_INCOMPLETE;
}

scan_status_t cmdscan_prefetch(cmd_scanner_t* scanner, cmd_prefetch_t* prefetch)
{
    if (scanner == NULL || prefetch == NULL || scanner->buf == NULL)
        return SCAN_ERROR;

    prefetch->buf = scanner->buf;
    prefetch->buf_len = scanner->buf_size;
    prefetch->cmd_start = 0;
    prefetch->cmd_len = 0;
    prefetch->func_len = 0;

    /* 跳过前导空白和终止符 */
    while (scanner->scan_pos < scanner->buf_size)
    {
        char c = (char)scanner->buf[scanner->scan_pos];

        if (is_space(c) || is_terminator(c))
        {
            scanner->scan_pos++;
            continue;
        }

        break;
    }

    if (scanner->scan_pos >= scanner->buf_size)
    {
        return SCAN_INCOMPLETE;
    }

    uint16_t cmd_start = scanner->scan_pos;

    /* 扫描函数名 */
    while (scanner->scan_pos < scanner->buf_size)
    {
        char c = (char)scanner->buf[scanner->scan_pos];

        if (is_terminator(c) || is_separator(c))
        {
            /* 遇到终止符或分隔符，命令结束 */
            prefetch->cmd_start = cmd_start;
            prefetch->cmd_len = scanner->scan_pos - cmd_start;
            prefetch->func_len = prefetch->cmd_len;
            scanner->scan_pos++; /* 跳过终止符/分隔符 */
            return SCAN_COMPLETE;
        }

        if (is_left_bracket(c))
        {
            /* 遇到左括号，函数名结束 */
            prefetch->cmd_start = cmd_start;
            prefetch->func_len = scanner->scan_pos - cmd_start;

            /* 扫描到右括号或终止符 */
            scanner->scan_pos++; /* 跳过左括号 */
            while (scanner->scan_pos < scanner->buf_size)
            {
                c = (char)scanner->buf[scanner->scan_pos];
                if (is_terminator(c))
                {
                    prefetch->cmd_len = scanner->scan_pos - cmd_start;
                    scanner->scan_pos++; /* 跳过终止符 */
                    return SCAN_COMPLETE;
                }
                if (is_right_bracket(c))
                {
                    scanner->scan_pos++;
                    /* 继续扫描到终止符或分隔符 */
                    while (scanner->scan_pos < scanner->buf_size &&
                           !is_terminator((char)scanner->buf[scanner->scan_pos]) &&
                           !is_separator((char)scanner->buf[scanner->scan_pos]))
                        scanner->scan_pos++;
                    prefetch->cmd_len = scanner->scan_pos - cmd_start;
                    if (scanner->scan_pos < scanner->buf_size)
                        scanner->scan_pos++; /* 跳过终止符/分隔符 */
                    return SCAN_COMPLETE;
                }
                scanner->scan_pos++;
            }
            /* 缓冲区结束，未找到终止符 */
            prefetch->cmd_len = scanner->scan_pos - cmd_start;
            return SCAN_INCOMPLETE;
        }

        if (is_space(c))
        {
            /* 遇到空白，函数名结束，无括号形式 */
            prefetch->cmd_start = cmd_start;
            prefetch->func_len = scanner->scan_pos - cmd_start;

            /* 继续扫描到终止符、分隔符或缓冲区结束 */
            while (scanner->scan_pos < scanner->buf_size &&
                   !is_terminator((char)scanner->buf[scanner->scan_pos]) &&
                   !is_separator((char)scanner->buf[scanner->scan_pos]))
                scanner->scan_pos++;

            prefetch->cmd_len = scanner->scan_pos - cmd_start;
            if (scanner->scan_pos < scanner->buf_size)
                scanner->scan_pos++; /* 跳过终止符/分隔符 */
            return SCAN_COMPLETE;
        }

        scanner->scan_pos++;
    }

    /* 缓冲区结束，未找到终止符 - 命令本身完整 */
    prefetch->cmd_start = cmd_start;
    prefetch->cmd_len = scanner->scan_pos - cmd_start;
    if (prefetch->func_len == 0)
        prefetch->func_len = prefetch->cmd_len;
    return SCAN_COMPLETE;
}

uint16_t cmdscan_next(const cmd_scanner_t* scanner, uint16_t* next_start)
{
    if (scanner == NULL || next_start == NULL)
        return 0;

    /* 当前命令结束位置就是下一个命令的起始 */
    *next_start = scanner->scan_pos;

    /* 检查是否还有剩余数据 */
    if (*next_start >= scanner->buf_size)
        return 0;

    /* 计算剩余长度 */
    return scanner->buf_size - *next_start;
}

/*=============================================================
 * 参数解析 API（零拷贝）- 保留供执行时使用
 *=============================================================*/

/**
 * @brief 跳过空白字符
 */
static const char* skip_space(const char* p, uint16_t len, uint16_t* pos)
{
    while (*pos < len && is_space(p[*pos]))
    {
        (*pos)++;
    }
    return p;
}

/**
 * @brief 计算 token 长度（直到遇到分隔符或空白）
 */
static uint16_t token_len(const char* p, uint16_t len, uint16_t start)
{
    uint16_t i = start;

    while (i < len)
    {
        char c = p[i];

        if (is_separator(c) || is_space(c) || is_right_bracket(c))
        {
            break;
        }

        i++;
    }

    return i - start;
}

/**
 * @brief 去除首尾空白
 */
static void trim_token(const char* p, uint16_t len,
                       const char** start, uint16_t* trim_len)
{
    uint16_t i = 0;
    uint16_t j = len;

    /* 去除前导空白 */
    while (i < j && is_space(p[i]))
    {
        i++;
    }

    /* 去除尾部空白 */
    while (j > i && is_space(p[j - 1]))
    {
        j--;
    }

    *start = p + i;
    *trim_len = j - i;
}

uint8_t cmdparse_args(const char* cmd, uint16_t len, cmd_parse_result_t* result)
{
    if (cmd == NULL || result == NULL || len == 0)
        return 0xFF; /* 错误 */

    /* 清空结果 */
    result->func_name = NULL;
    result->func_name_len = 0;
    result->args_count = 0;

    uint16_t pos = 0;

    /* 1. 跳过前导空白 */
    skip_space(cmd, len, &pos);

    /* 2. 解析函数名（遇到括号、空白、分隔符停止） */
    uint16_t name_start = pos;
    while (pos < len && !is_left_bracket(cmd[pos]) && !is_space(cmd[pos]) && !is_separator(cmd[pos]))
    {
        pos++;
    }

    if (pos == name_start)
        return 0xFF; /* 没有函数名 */

    result->func_name = cmd + name_start;
    result->func_name_len = pos - name_start;

    /* 3. 跳过函数名后的空白 */
    skip_space(cmd, len, &pos);

    /* 4. 检查是否有左括号 */
    if (pos >= len || !is_left_bracket(cmd[pos]))
    {
        /* 无括号形式：检查是否有参数（空格分隔） */
        if (pos < len && !is_terminator(cmd[pos]))
        {
            /* 有参数，解析空格分隔的参数 */
            uint8_t arg_idx = 0;

            while (pos < len && arg_idx < CMD_SCAN_MAX_ARGS)
            {
                /* 跳过参数前空白 */
                skip_space(cmd, len, &pos);

                /* 检查是否到达终止符 */
                if (pos >= len || is_terminator(cmd[pos]))
                {
                    break;
                }

                /* 计算当前 token 长度 */
                uint16_t t_len = token_len(cmd, len, pos);

                if (t_len > 0)
                {
                    /* 去除首尾空白 */
                    const char* token_start;
                    uint16_t token_length;
                    trim_token(cmd + pos, t_len, &token_start, &token_length);

                    if (token_length > 0)
                    {
                        result->args[arg_idx].ptr = token_start;
                        result->args[arg_idx].len = token_length;
                        arg_idx++;
                    }
                }

                pos += t_len;

                /* 跳过分隔符（逗号/分号） */
                if (pos < len && is_separator(cmd[pos]))
                {
                    pos++;
                }
            }

            result->args_count = arg_idx;
            return arg_idx;
        }

        /* 无参函数 */
        return 0;
    }

    pos++; /* 跳过左括号 */

    /* 5. 解析参数 */
    uint8_t arg_idx = 0;

    while (pos < len && arg_idx < CMD_SCAN_MAX_ARGS)
    {
        /* 跳过参数前空白 */
        skip_space(cmd, len, &pos);

        /* 检查是否到达右括号 */
        if (pos < len && is_right_bracket(cmd[pos]))
        {
            break;
        }

        /* 检查是否到达终止符 */
        if (pos < len && is_terminator(cmd[pos]))
        {
            break;
        }

        /* 计算当前 token 长度 */
        uint16_t t_len = token_len(cmd, len, pos);

        if (t_len > 0)
        {
            /* 去除首尾空白 */
            const char* token_start;
            uint16_t token_length;
            trim_token(cmd + pos, t_len, &token_start, &token_length);

            if (token_length > 0)
            {
                result->args[arg_idx].ptr = token_start;
                result->args[arg_idx].len = token_length;
                arg_idx++;
            }
        }

        pos += t_len;

        /* 跳过分隔符 */
        if (pos < len && is_separator(cmd[pos]))
        {
            pos++;
        }
    }

    result->args_count = arg_idx;
    return arg_idx;
}
