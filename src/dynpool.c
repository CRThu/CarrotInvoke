#include "dynpool.h"


/// <summary>
/// 初始化动态类型数组
/// </summary>
/// <param name="pool">空动态类型数组结构体</param>
/// <returns>返回状态</returns>
dynpool_status_t dynpool_init(dynpool_t* pool)
{
    if (pool == NULL)   return DYNPOOL_ERR_NULL_OBJECT;

    memset(pool->buf, 0u, DYNPOOL_MAX_BYTES);
    memset(pool->elements, 0u, DYNPOOL_MAX_VARS);
    pool->wr_count = 0;
    pool->rd_count = 0;
    pool->cursor = 0;

    return DYNPOOL_NO_ERROR;
}

/// <summary>
/// 添加元素通用函数
/// </summary>
/// <param name="pool">动态类型数组结构体</param>
/// <param name="intype">变量类型</param>
/// <param name="indata">变量首地址指针</param>
/// <param name="len">变量长度,若为值类型则未使用</param>
/// <returns>返回状态</returns>
dynpool_status_t dynpool_set(dynpool_t* pool, dtypes_t intype, void* indata, uint16_t len)
{
    if (pool == NULL)   return DYNPOOL_ERR_NULL_OBJECT;
    // check pool params count
    if (pool->wr_count + 1 >= DYNPOOL_MAX_VARS)     return DYNPOOL_ERR_FULL_POOL;

    // get len
    if (!DTYPES_IS_REF(intype))
    {
        len = DTYPES_GET_LEN(intype);
        if (len < 0)
            return DYNPOOL_ERR_UNKNOWN_LEN;
    }

    // check pool buffer count
    if (pool->cursor + len >= DYNPOOL_MAX_BYTES)
    {
        return DYNPOOL_ERR_FULL_POOL;
    }

    if (DTYPES_IS_REF(intype))
    {
        memcpy(&pool->buf[pool->cursor], (uint8_t*)indata, len);
    }
    else
    {
        memcpy(&pool->buf[pool->cursor], indata, len);
    }

    dyn_info_t* ele = &pool->elements[pool->wr_count];
    ele->offset = pool->cursor;
    ele->len = len;
    ele->type = intype;
    pool->wr_count++;
    pool->cursor += len;

    return DYNPOOL_NO_ERROR;
}

/// <summary>
/// 按FIFO顺序获取元素
/// </summary>
/// <param name="pool">存储结构</param>
/// <param name="type">类型</param>
/// <param name="data">数据指针</param>
/// <param name="size">数据长度</param>
/// <param name="used_size">实际使用数据长度</param>
/// <returns>返回状态</returns>
dynpool_status_t dynpool_get(dynpool_t* pool, dtypes_t type, void* data, uint16_t size, uint16_t* used_size)
{
    if (pool->rd_count >= pool->wr_count)   return DYNPOOL_ERR_NO_DATA;

    dyn_info_t* ele = &pool->elements[pool->rd_count];
    dtype_conversion(&pool->buf[ele->offset], data, ele->type, type, ele->len, size, &used_size);
    pool->rd_count++;

    return DYNPOOL_NO_ERROR;
}

/// <summary>
/// 按index随机读取元素
/// </summary>
/// <param name="pool">存储结构</param>
/// <param name="index">数据索引</param>
/// <param name="type">类型</param>
/// <param name="data">数据指针</param>
/// <param name="size">数据长度</param>
/// <param name="used_size">实际使用数据长度</param>
/// <returns>返回状态</returns>
dynpool_status_t dynpool_peek(dynpool_t* pool, uint16_t index, dtypes_t type, void* data, uint16_t size, uint16_t* used_size)
{
    if (index >= pool->wr_count)   return DYNPOOL_ERR_NO_DATA;

    dyn_info_t* ele = &pool->elements[index];
    dtype_conversion(&pool->buf[ele->offset], data, ele->type, type, ele->len, size, &used_size);

    return DYNPOOL_NO_ERROR;
}


/// <summary>
/// 打印数据存储池
/// </summary>
/// <param name="pool">存储结构</param>
void dynpool_print(dynpool_t* pool)
{
    printf("----- DYNPOOL PRINT -----\r\n");
    uint8_t outbuf[DYNPOOL_MAX_BYTES];
    char* typename;
    for (uint16_t i = 0; i < pool->wr_count; i++)
    {
        dyn_info_t* ele = &pool->elements[i];
        uint16_t used_size;
        dtype_conversion(&pool->buf[ele->offset], outbuf, ele->type, T_STRING, ele->len, sizeof(outbuf), &used_size);

        switch (ele->type)
        {
            case T_NULL:    typename = "T_NULL";    break;
            case T_DEC64:   typename = "T_DEC64";   break;
            case T_HEX64:   typename = "T_HEX64";   break;
            case T_KV:      typename = "T_KV";      break;
            case T_STRING:  typename = "T_STRING";  break;
            case T_BYTES:   typename = "T_BYTES";   break;
            case T_JSON:    typename = "T_JSON";    break;
            default:        typename = "T_UNKNWON"; break;
        }
        printf("DYNPOOL[%2d](TYPE: %-9s): %s\r\n", i, typename, outbuf);
    }
    printf("----- DYNPOOL PRINT -----\r\n");
}

/// <summary>
/// 类型转换
/// </summary>
/// <param name="input">输入数据指针</param>
/// <param name="output">输出数据指针</param>
/// <param name="intype">输入数据类型</param>
/// <param name="outtype">输出数据类型</param>
/// <param name="input_size">输入数据长度</param>
/// <param name="output_size">输出数据长度</param>
/// <param name="used_size">实际使用数据长度</param>
/// <returns>返回状态</returns>
dynpool_status_t dtype_conversion(const void* input, void* output, dtypes_t intype, dtypes_t outtype, uint16_t input_size, uint16_t output_size, uint16_t* used_size)
{
    char buffer[256]; // 中间缓冲区，足够大以处理大部分情况
    *used_size = 0;

    // 处理输入类型，转换为中间格式
    switch (intype)
    {
        case T_NULL:
            buffer[0] = '\0';
            break;

        case T_DEC64: // 有符号十进制
            snprintf(buffer, sizeof(buffer), "%" PRId64, *(const int64_t*)input);
            break;

        case T_HEX64: // 无符号十六进制
            snprintf(buffer, sizeof(buffer), "%" PRIX64, *(const uint64_t*)input);
            break;

        case T_STRING: // 字符串类型
        {
            // 安全拷贝并保证终止符
            size_t copy_len = input_size < sizeof(buffer) - 1 ? input_size : sizeof(buffer) - 1;
            strncpy(buffer, (const char*)input, copy_len);
            buffer[copy_len] = '\0';
        }
        break;

        case T_BYTES: // 二进制类型转十六进制字符串
        {
            const uint8_t* bytes = (const uint8_t*)input;
            char* p = buffer;
            for (uint16_t i = 0; i < input_size && (p - buffer) < sizeof(buffer) - 2; i++)
            {
                snprintf(p, 3, "%02X", bytes[i]);
                p += 2;
            }
            *p = '\0';
        }
        break;

        default:
            return DYNPOOL_ERR_UNSUPPORTED_TYPE;
    }

    // 处理输出类型，进行类型转换
    switch (outtype)
    {
        case T_NULL: // 无需转换
            *used_size = 0;
            return DYNPOOL_NO_ERROR;

        case T_DEC64: // 转有符号十进制
        {
            if (output_size < sizeof(int64_t))
                return DYNPOOL_ERR_INSUFFICIENT_BUFFER;

            char* endptr;
            int64_t value = strtoll(buffer, &endptr, 10);
            if (*endptr != '\0')
                return DYNPOOL_ERR_CONVERSION_FAILED;

            *(int64_t*)output = value;
            *used_size = sizeof(int64_t);
        }
        break;

        case T_HEX64: // 转无符号十六进制
        {
            if (output_size < sizeof(uint64_t))
                return DYNPOOL_ERR_INSUFFICIENT_BUFFER;

            char* endptr;
            uint64_t value = strtoull(buffer, &endptr, 16);
            if (*endptr != '\0')
                return DYNPOOL_ERR_CONVERSION_FAILED;

            *(uint64_t*)output = value;
            *used_size = sizeof(uint64_t);
        }
        break;

        case T_STRING: // 转字符串类型
        {
            size_t req_size = strlen(buffer) + 1; // 包含终止符
            if (output_size < req_size)
            {
                *used_size = req_size;
                return DYNPOOL_ERR_INSUFFICIENT_BUFFER;
            }

            strncpy((char*)output, buffer, output_size);
            ((char*)output)[output_size - 1] = '\0'; // 强制终止
            *used_size = strlen(buffer) + 1;
        }
        break;

        case T_BYTES: // 转二进制类型（需十六进制解析）
        {
            size_t hex_len = strlen(buffer);
            if (hex_len % 2 != 0)
                return DYNPOOL_ERR_CONVERSION_FAILED;

            size_t byte_count = hex_len / 2;
            if (output_size < byte_count)
            {
                *used_size = byte_count;
                return DYNPOOL_ERR_INSUFFICIENT_BUFFER;
            }

            uint8_t* bytes = (uint8_t*)output;
            for (size_t i = 0; i < byte_count; i++)
            {
                if (sscanf(buffer + i * 2, "%2hhX", &bytes[i]) != 1)
                    return DYNPOOL_ERR_CONVERSION_FAILED;
            }
            *used_size = byte_count;
        }
        break;

        default:
            return DYNPOOL_ERR_UNSUPPORTED_TYPE;
    }

    return DYNPOOL_NO_ERROR;

}