#include "dynpool.h"

/// <summary>
/// 初始化动态类型数组
/// </summary>
/// <param name="pool">空动态类型数组结构体</param>
dynpool_status_t dynpool_init(dynpool_t* pool)
{
    if (pool == NULL)
        return DYNPOOL_ERR_NULL_OBJECT;

    memset(pool->buf, 0u, DYNPOOL_MAX_BYTES);
    memset(pool->offset, 0u, DYNPOOL_MAX_PARAMS);
    memset(pool->len, 0u, DYNPOOL_MAX_PARAMS);
    pool->count = 0;
    pool->cursor = 0;

    return DYNPOOL_ERR_NONE;
}

/// <summary>
/// 添加元素通用函数
/// </summary>
/// <param name="pool">动态类型数组结构体</param>
/// <param name="intype">变量类型（未使用）</param>
/// <param name="data">变量首地址指针</param>
/// <param name="len">变量长度,若为值类型则未使用</param>
dynpool_status_t dynpool_set(dynpool_t* pool, dtypes_t intype, void* indata, uint16_t len)
{
    // check pool params count
    if (pool->count + 1 >= DYNPOOL_MAX_PARAMS)
    {
        return DYNPOOL_ERR_FULL_POOL;
    }

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

    pool->offset[pool->count] = pool->cursor;
    pool->len[pool->count] = len;
    pool->count++;
    pool->cursor = pool->cursor + len;

    return DYNPOOL_ERR_NONE;
}

/// <summary>
/// 获取元素
/// </summary>
/// <param name="pool">存储结构</param>
/// <param name="index">元素索引</param>
/// <param name="type">类型</param>
/// <param name="data">数据指针</param>
/// <param name="len">数据长度</param>
void dynpool_get(dynpool_t* pool, uint16_t index, dtypes_t type, void* data, uint16_t len)
{
    if (index < pool->count)
    {
        // TODO
        void* internal_data = &pool->buf[pool->offset[index]];
        uint16_t internal_len = pool->len[index];
        dtype_conversion(internal_data, (size_t*)data, T_BYTES, type, internal_len, len);
    }
    else
    {
        // NO DATA AT INDEX
    }

}

/// <summary>
/// 打印数据存储池
/// </summary>
/// <param name="pool">存储结构</param>
void dynpool_print(dynpool_t* pool)
{
    for (uint16_t i = 0; i < pool->count; i++)
    {
        void* pd = &(pool->buf[pool->offset[i]]);
        dtypes_t dt = T_STRING;
        uint16_t len = pool->len[i];

        char format[256] = "";
        memcpy(format, pd, len);

        switch (dt)
        {
        case T_STRING:
            break;
        case T_DEC64:
            sprintf(format, "%d", *((int32_t*)pd));
            break;
        case T_HEX64:
            sprintf(format, "0x%X", (int)*((uint64_t*)pd));
            break;
        }

        printf("INDEX:%d, TYPE:%02X, ADDR:%08X, LEN:%02X, DATA:%s\r\n", i, dt, (uint32_t)pd, len, format);
    }
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
void dtype_conversion(const void* input, void* output, dtypes_t intype, dtypes_t outtype, size_t input_size, size_t output_size)
{
    char buffer[100];

    // Convert input to string using sprintf
    switch (intype)
    {
    case T_NULL:
        buffer[0] = '\0';
        break;
    case T_DEC64:
        sprintf(buffer, "%"PRId64"", *(int64_t*)input);
        break;
    case T_HEX64:
        sprintf(buffer, "%"PRIX64"", *(int64_t*)input);
        break;
    case T_STRING:
        strncpy(buffer, (char*)input, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
        break;
        //case T_KV:
        //    if (sscanf((char*)input, "%d", (int*)output) == 1)
        //    {
        //        // Input is a number, convert to enum string
        //        strncpy(buffer, enum_to_string(*(int*)output), sizeof(buffer) - 1);
        //    }
        //    else
        //    {
        //        // Input is a string, convert to enum number
        //        *(int*)output = string_to_enum((char*)input);
        //        strncpy(buffer, (char*)input, sizeof(buffer) - 1);
        //    }
        //    buffer[sizeof(buffer) - 1] = '\0';
        //    break;
    case T_BYTES:
        memcpy(buffer, input, input_size);
        buffer[input_size] = '\0'; // Ensure null-terminated string for further processing
        break;
    default:
        printf("Unsupported input type\n");
        return;
    }

    // Convert string to output type using sscanf
    switch (outtype)
    {
    case T_NULL:
        // No conversion needed for T_NULL
        break;
    case T_DEC64:
        sscanf(buffer, "%"PRId64"", (int64_t*)output);
        break;
    case T_HEX64:
        sscanf(buffer, "%"PRIX64"", (uint64_t*)output);
        break;
    case T_STRING:
        strncpy((char*)output, buffer, output_size - 1);
        ((char*)output)[output_size - 1] = '\0';
        break;
        //case T_KV:
        //    if (sscanf(buffer, "%d", (int*)output) == 1)
        //    {
        //        // Output is a number, convert to enum string
        //        strncpy((char*)output, enum_to_string(*(int*)output), output_size - 1);
        //    }
        //    else
        //    {
        //        // Output is a string, convert to enum number
        //        *(int*)output = string_to_enum(buffer);
        //        strncpy((char*)output, buffer, output_size - 1);
        //    }
        //    ((char*)output)[output_size - 1] = '\0';
        //    break;
    case T_BYTES:
        memcpy(output, buffer, output_size);
        break;
    default:
        printf("Unsupported output type\n");
    }
}