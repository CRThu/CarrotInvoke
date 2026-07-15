/**
 * test_dynpool.c — Unit tests for dynpool module
 */
#include "unity.h"
#include "dynpool.h"
#include <string.h>

/* ===== Init ===== */
void test_dynpool_init(void)
{
    dynpool_t pool;
    dynpool_status_t s = dynpool_init(&pool);
    TEST_ASSERT_EQUAL_INT(DYNPOOL_NO_ERROR, s);
    TEST_ASSERT_EQUAL_UINT16(0, pool.wr_count);
    TEST_ASSERT_EQUAL_UINT16(0, pool.rd_count);
    TEST_ASSERT_EQUAL_UINT16(0, pool.cursor);
}

void test_dynpool_init_null(void)
{
    dynpool_status_t s = dynpool_init(NULL);
    TEST_ASSERT_EQUAL_INT(DYNPOOL_ERR_NULL_OBJECT, s);
}

/* ===== Set / Get basic types ===== */
void test_dynpool_dec64(void)
{
    dynpool_t pool;
    dynpool_init(&pool);

    int64_t val = -123456789;
    dynpool_status_t s = dynpool_set(&pool, T_DEC64, &val, 0);
    TEST_ASSERT_EQUAL_INT(DYNPOOL_NO_ERROR, s);

    int64_t out;
    uint16_t used;
    s = dynpool_get(&pool, T_DEC64, &out, sizeof(out), &used);
    TEST_ASSERT_EQUAL_INT(DYNPOOL_NO_ERROR, s);
    TEST_ASSERT_EQUAL_INT64(-123456789, out);
}

void test_dynpool_hex64(void)
{
    dynpool_t pool;
    dynpool_init(&pool);

    uint64_t val = 0xABCDEF1234567890ULL;
    dynpool_status_t s = dynpool_set(&pool, T_HEX64, &val, 0);
    TEST_ASSERT_EQUAL_INT(DYNPOOL_NO_ERROR, s);

    uint64_t out;
    uint16_t used;
    s = dynpool_get(&pool, T_HEX64, &out, sizeof(out), &used);
    TEST_ASSERT_EQUAL_INT(DYNPOOL_NO_ERROR, s);
    TEST_ASSERT_EQUAL_HEX64(0xABCDEF1234567890ULL, out);
}

void test_dynpool_string(void)
{
    dynpool_t pool;
    dynpool_init(&pool);

    const char* val = "hello world";
    dynpool_status_t s = dynpool_set(&pool, T_STRING, (void*)val, strlen(val) + 1);
    TEST_ASSERT_EQUAL_INT(DYNPOOL_NO_ERROR, s);

    char out[50];
    uint16_t used;
    s = dynpool_get(&pool, T_STRING, out, sizeof(out), &used);
    TEST_ASSERT_EQUAL_INT(DYNPOOL_NO_ERROR, s);
    TEST_ASSERT_EQUAL_STRING("hello world", out);
}

/* ===== Peek ===== */
void test_dynpool_peek(void)
{
    dynpool_t pool;
    dynpool_init(&pool);

    int64_t v1 = 111;
    int64_t v2 = 222;
    dynpool_set(&pool, T_DEC64, &v1, 0);
    dynpool_set(&pool, T_DEC64, &v2, 0);

    dtypes_t type;
    void* data;
    uint16_t size;

    dynpool_status_t s = dynpool_peek(&pool, 0, &type, &data, &size);
    TEST_ASSERT_EQUAL_INT(DYNPOOL_NO_ERROR, s);
    TEST_ASSERT_EQUAL_INT(T_DEC64, type);
    TEST_ASSERT_EQUAL_INT64(111, *(int64_t*)data);

    s = dynpool_peek(&pool, 1, &type, &data, &size);
    TEST_ASSERT_EQUAL_INT(DYNPOOL_NO_ERROR, s);
    TEST_ASSERT_EQUAL_INT64(222, *(int64_t*)data);
}

void test_dynpool_peek_out_of_bounds(void)
{
    dynpool_t pool;
    dynpool_init(&pool);

    dtypes_t type;
    void* data;
    uint16_t size;
    dynpool_status_t s = dynpool_peek(&pool, 0, &type, &data, &size);
    TEST_ASSERT_EQUAL_INT(DYNPOOL_ERR_NO_DATA, s);
}

/* ===== Pool full ===== */
void test_dynpool_full(void)
{
    dynpool_t pool;
    dynpool_init(&pool);

    for (int i = 0; i < DYNPOOL_MAX_VARS; i++)
    {
        int64_t val = i;
        dynpool_status_t s = dynpool_set(&pool, T_DEC64, &val, 0);
        TEST_ASSERT_EQUAL_INT(DYNPOOL_NO_ERROR, s);
    }

    int64_t extra = 999;
    dynpool_status_t s = dynpool_set(&pool, T_DEC64, &extra, 0);
    TEST_ASSERT_EQUAL_INT(DYNPOOL_ERR_FULL_POOL, s);
}

/* ===== Type conversion ===== */
void test_dtype_conv_dec64_to_string(void)
{
    int64_t val = -123456789;
    char buf[50];
    uint16_t used;
    dynpool_status_t s = dtype_conversion(&val, buf, T_DEC64, T_STRING, sizeof(val), sizeof(buf), &used);
    TEST_ASSERT_EQUAL_INT(DYNPOOL_NO_ERROR, s);
    TEST_ASSERT_EQUAL_STRING("-123456789", buf);
}

void test_dtype_conv_hex64_to_string(void)
{
    uint64_t val = 0xABCDEF;
    char buf[50];
    uint16_t used;
    dynpool_status_t s = dtype_conversion(&val, buf, T_HEX64, T_STRING, sizeof(val), sizeof(buf), &used);
    TEST_ASSERT_EQUAL_INT(DYNPOOL_NO_ERROR, s);
    TEST_ASSERT_EQUAL_STRING("ABCDEF", buf);
}

void test_dtype_conv_string_to_dec64(void)
{
    const char* str = "987654321";
    int64_t out;
    uint16_t used;
    dynpool_status_t s = dtype_conversion(str, &out, T_STRING, T_DEC64, strlen(str) + 1, sizeof(out), &used);
    TEST_ASSERT_EQUAL_INT(DYNPOOL_NO_ERROR, s);
    TEST_ASSERT_EQUAL_INT64(987654321, out);
}

void test_dtype_conv_string_to_hex64(void)
{
    const char* str = "FF";
    uint64_t out;
    uint16_t used;
    dynpool_status_t s = dtype_conversion(str, &out, T_STRING, T_HEX64, strlen(str) + 1, sizeof(out), &used);
    TEST_ASSERT_EQUAL_INT(DYNPOOL_NO_ERROR, s);
    TEST_ASSERT_EQUAL_HEX64(0xFF, out);
}

int run_dynpool_tests(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_dynpool_init);
    RUN_TEST(test_dynpool_init_null);
    RUN_TEST(test_dynpool_dec64);
    RUN_TEST(test_dynpool_hex64);
    RUN_TEST(test_dynpool_string);
    RUN_TEST(test_dynpool_peek);
    RUN_TEST(test_dynpool_peek_out_of_bounds);
    RUN_TEST(test_dynpool_full);
    RUN_TEST(test_dtype_conv_dec64_to_string);
    RUN_TEST(test_dtype_conv_hex64_to_string);
    RUN_TEST(test_dtype_conv_string_to_dec64);
    RUN_TEST(test_dtype_conv_string_to_hex64);

    return UNITY_END();
}
