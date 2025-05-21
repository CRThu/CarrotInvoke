/****************************
* DYNPOOL
* CRTHu
* 2025.03.18
*****************************/
#pragma once
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#ifndef _DYNPOOL_H_
#define _DYNPOOL_H_

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS

#ifdef __cplusplus
extern "C"
{
    #endif

    #define DYNPOOL_VERSION             "1.1.1"

    #define DYNPOOL_PRINTF              printf

    #define DYNPOOL_MAX_BYTES           1024
    #define DYNPOOL_MAX_VARS            4

    // DYNAMIC TYPES IS VALUE OR POINTER
    #define DTYPES_BITS_OFFSET          (7)
    #define DTYPES_STORE_REF_MASK       (0x01 << DTYPES_BITS_OFFSET)
    #define DTYPES_STORE_VAL            (0x00 << DTYPES_BITS_OFFSET)
    #define DTYPES_STORE_REF            (0x01 << DTYPES_BITS_OFFSET)
    #define DTYPES_IS_REF(x)            (((x) & DTYPES_STORE_REF_MASK) == DTYPES_STORE_REF)

    // DYNAMIC TYPES LENGTH
    #define DTYPES_STORE_LEN_OFFSET     (5)
    #define DTYPES_STORE_LEN_MASK       (0x03 << DTYPES_STORE_LEN_OFFSET)
    #define DTYPES_STORE_LEN_1B         (0x00 << DTYPES_STORE_LEN_OFFSET)
    #define DTYPES_STORE_LEN_8B         (0x01 << DTYPES_STORE_LEN_OFFSET)
    #define DTYPES_STORE_LEN_RESERVED   (0x02 << DTYPES_STORE_LEN_OFFSET)
    #define DTYPES_STORE_LEN_PTR        (0x03 << DTYPES_STORE_LEN_OFFSET)
    #define DTYPES_GET_LEN(x)           ((((x) & DTYPES_STORE_LEN_MASK) == DTYPES_STORE_LEN_1B) ? 1 \
                                        : ((((x) & DTYPES_STORE_LEN_MASK) == DTYPES_STORE_LEN_8B) ? 8 \
                                            : ((((x) & DTYPES_STORE_LEN_MASK) == DTYPES_STORE_LEN_RESERVED) ? DYNPOOL_ERR_UNKNOWN_LEN \
                                                : ((((x) & DTYPES_STORE_LEN_MASK) == DTYPES_STORE_LEN_PTR) ? DYNPOOL_ERR_UNKNOWN_LEN \
                                                    : DYNPOOL_ERR_UNKNOWN_LEN))))
// DYNAMIC CALLS PARAMS DATATYPES
    #define DTYPES_STORE_DTYPE_OFFSET   (0)
    #define DTYPES_STORE_DTYPE_MASK     (0x1F << DTYPES_STORE_DTYPE_OFFSET)
    #define DTYPES_STORE_NULL           (0x00 << DTYPES_STORE_DTYPE_OFFSET)
    #define DTYPES_STORE_DEC64          (0x01 << DTYPES_STORE_DTYPE_OFFSET)
    #define DTYPES_STORE_HEX64          (0x02 << DTYPES_STORE_DTYPE_OFFSET)
    #define DTYPES_STORE_STRING         (0x03 << DTYPES_STORE_DTYPE_OFFSET)
    #define DTYPES_STORE_KV             (0x04 << DTYPES_STORE_DTYPE_OFFSET)
    #define DTYPES_STORE_BYTES          (0x05 << DTYPES_STORE_DTYPE_OFFSET)
    #define DTYPES_STORE_JSON           (0x06 << DTYPES_STORE_DTYPE_OFFSET)

    /*
        TYPES       REPRESENT                   EQUALS TYPE
        |- NULL     NULL/VOID/PLACEHOLDER       VOID/NULL/0
        |- DEC64    DECIMAL NUMBERS             uint8_t/int8_t/uint16_t/int16_t/uint32_t/int32_t/int64_t
        |- HEX64    HEXADECIMAL NUMBERS         uint8_t/uint16_t/uint32_t/uint64_t
        |- KV       VALUES MATCHED BY KEYS      uint8_t/int8_t/uint16_t/int16_t/uint32_t/int32_t/int64_t
        |- STRING   ASCII STRING                char*
        |- BYTES    BYTE ARRAY                  uint8_t*
        |- JSON     JSON OBJECT                 json_t*
    */
    #define T_VOID                      0
    #define T_NULL                      (DTYPES_STORE_VAL | DTYPES_STORE_LEN_1B | DTYPES_STORE_NULL)
    #define T_DEC64                     (DTYPES_STORE_VAL | DTYPES_STORE_LEN_8B | DTYPES_STORE_DEC64)
    #define T_HEX64                     (DTYPES_STORE_VAL | DTYPES_STORE_LEN_8B | DTYPES_STORE_HEX64)
    #define T_KV                        (DTYPES_STORE_VAL | DTYPES_STORE_LEN_8B | DTYPES_STORE_KV)
    #define T_STRING                    (DTYPES_STORE_REF | DTYPES_STORE_LEN_PTR | DTYPES_STORE_STRING)
    #define T_BYTES                     (DTYPES_STORE_REF | DTYPES_STORE_LEN_PTR | DTYPES_STORE_BYTES)
    #define T_JSON                      (DTYPES_STORE_REF | DTYPES_STORE_LEN_PTR | DTYPES_STORE_JSON)

    #define DYNPOOL_NO_ERROR                    (0)
    #define DYNPOOL_ERR_NULL_OBJECT             (-1)
    #define DYNPOOL_ERR_FULL_POOL               (-2)
    #define DYNPOOL_ERR_UNKNOWN_LEN             (-3)
    #define DYNPOOL_ERR_NO_DATA                 (-4)
    #define DYNPOOL_ERR_UNSUPPORTED_TYPE        (-5)
    #define DYNPOOL_ERR_INSUFFICIENT_BUFFER     (-6)
    #define DYNPOOL_ERR_CONVERSION_FAILED       (-7)

    typedef uint8_t dtypes_t;
    typedef int8_t dynpool_status_t;

    typedef struct
    {
        /// <summary>
        /// 元素开始地址
        /// </summary>
        uint16_t offset;

        /// <summary>
        /// 元素长度
        /// </summary>
        uint16_t len;

        /// <summary>
        /// 元素类型
        /// </summary>
        dtypes_t type;
    } dyn_info_t;

    /// <summary>
    /// 动态类型数组结构体
    /// </summary>
    typedef struct
    {
        /// <summary>
        /// 数据存储池
        /// </summary>
        uint8_t buf[DYNPOOL_MAX_BYTES];

        /// <summary>
        /// 每个元素信息
        /// </summary>
        dyn_info_t elements[DYNPOOL_MAX_VARS];

        /// <summary>
        /// buf写入游标
        /// </summary>
        uint16_t cursor;

        /// <summary>
        /// 目前存储元素数量
        /// </summary>
        uint16_t wr_count;

        /// <summary>
        /// 目前读取元素数量
        /// </summary>
        uint16_t rd_count;
    }dynpool_t;

    dynpool_status_t dynpool_init(dynpool_t* pool);
    dynpool_status_t dynpool_set(dynpool_t* pool, dtypes_t intype, void* indata, uint16_t len);
    dynpool_status_t dynpool_get(dynpool_t* pool, dtypes_t type, void* data, uint16_t size, uint16_t* used_size);
    dynpool_status_t dynpool_peek(dynpool_t* pool, uint16_t index, dtypes_t* type, void** data, uint16_t* size);

    void dynpool_print(dynpool_t* pool);

    dynpool_status_t dtype_conversion(const void* input, void* output, dtypes_t intype, dtypes_t outtype, uint16_t input_size, uint16_t output_size, uint16_t* used_size);

    #ifdef __cplusplus
}
#endif

#endif /* _DYNPOOL_H_ */