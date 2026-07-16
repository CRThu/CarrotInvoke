/**
 * test_dyncall.c — Unit tests for dyncall module (old API)
 *
 * Self-contained: defines its own mock functions and groups
 * to avoid depending on test_helpers.h (which now uses dispatch v2).
 */
#include "unity.h"
#include "fff.h"
#include "dyncall.h"
#include "dynpool.h"
#include "cmdparse.h"
#include <string.h>

/* ---- fff declarations for old dyncall tests (void** params) ---- */
DECLARE_FAKE_VOID_FUNC(print_hello_mock);
DECLARE_FAKE_VALUE_FUNC(int64_t, print_add_mock, void**, void**);
DECLARE_FAKE_VOID_FUNC(print_dec_mock, void**);
DECLARE_FAKE_VOID_FUNC(print_hex_mock, void**);
DECLARE_FAKE_VOID_FUNC(print_string_mock, void**);
DECLARE_FAKE_VOID_FUNC(print_args_mock, void**, void**, void**);

/* ---- fff global state ---- */
DEFINE_FFF_GLOBALS;

/* ---- fff definitions ---- */
DEFINE_FAKE_VOID_FUNC(print_hello_mock);
DEFINE_FAKE_VALUE_FUNC(int64_t, print_add_mock, void**, void**);
DEFINE_FAKE_VOID_FUNC(print_dec_mock, void**);
DEFINE_FAKE_VOID_FUNC(print_hex_mock, void**);
DEFINE_FAKE_VOID_FUNC(print_string_mock, void**);
DEFINE_FAKE_VOID_FUNC(print_args_mock, void**, void**, void**);

/* ---- Old mock function group ---- */
static function_group_t mock_func_group =
{
    FUNCTION_GROUP("mock_func_group",
        FUNCTION_INFO_NAME("print_hello",  print_hello_mock,  T_NULL, T_VOID),
        FUNCTION_INFO_NAME("print_dec",    print_dec_mock,    T_NULL, T_DEC64),
        FUNCTION_INFO_NAME("print_hex",    print_hex_mock,    T_NULL, T_HEX64),
        FUNCTION_INFO_NAME("print_string", print_string_mock, T_NULL, T_STRING),
        FUNCTION_INFO_NAME("print_add",    print_add_mock,    T_DEC64, T_DEC64, T_DEC64),
        FUNCTION_INFO_NAME("print_args",   print_args_mock,   T_NULL, T_STRING, T_STRING, T_STRING),
    )
};

/* ---- Old default function group (same mocks, different group name) ---- */
static function_group_t default_func_group =
{
    FUNCTION_GROUP("default_func_group",
        FUNCTION_INFO_NAME("print_hello",  print_hello_mock,  T_NULL, T_VOID),
        FUNCTION_INFO_NAME("print_dec",    print_dec_mock,    T_NULL, T_DEC64),
        FUNCTION_INFO_NAME("print_hex",    print_hex_mock,    T_NULL, T_HEX64),
        FUNCTION_INFO_NAME("print_string", print_string_mock, T_NULL, T_STRING),
        FUNCTION_INFO_NAME("print_add",    print_add_mock,    T_DEC64, T_DEC64, T_DEC64),
        FUNCTION_INFO_NAME("print_args",   print_args_mock,   T_NULL, T_STRING, T_STRING, T_STRING),
    )
};

/* ===== find_func ===== */
void test_find_func_exists(void)
{
    function_info_t* f = find_func("print_hello");
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL_STRING("print_hello", f->name);
}

void test_find_func_not_exists(void)
{
    function_info_t* f = find_func("nonexistent");
    TEST_ASSERT_NULL(f);
}

void test_find_func_empty_name(void)
{
    function_info_t* f = find_func("");
    TEST_ASSERT_NULL(f);
}

void test_find_func_after_reset(void)
{
    dyncall_reset();
    function_info_t* f = find_func("print_hello");
    TEST_ASSERT_NULL(f);
}

/* ===== register_rpc_group ===== */
void test_register_group(void)
{
    dyncall_reset();
    dyncall_status_t s = register_rpc_group(&mock_func_group);
    TEST_ASSERT_EQUAL_INT(DYNCALL_NO_ERROR, s);

    function_info_t* f = find_func("print_hello");
    TEST_ASSERT_NOT_NULL(f);
}

void test_register_multiple_groups(void)
{
    dyncall_reset();
    register_rpc_group(&mock_func_group);
    register_rpc_group(&default_func_group);

    /* Both groups should be findable */
    TEST_ASSERT_NOT_NULL(find_func("print_hello"));
    TEST_ASSERT_NOT_NULL(find_func("print_hello"));
}

/* ===== invoke (variadic) ===== */
void test_invoke_hello(void)
{
    dyncall_status_t s = invoke("print_hello");
    TEST_ASSERT_EQUAL_INT(DYNCALL_NO_ERROR, s);
    TEST_ASSERT_EQUAL_INT(1, print_hello_mock_fake.call_count);
}

void test_invoke_not_found(void)
{
    dyncall_status_t s = invoke("nonexistent");
    TEST_ASSERT_EQUAL_INT(DYNCALL_ERR_FUNC_NOT_FOUND, s);
}

/* ===== invoke_by_pool ===== */
void test_invoke_by_pool_hello(void)
{
    dynpool_t pool;
    dynpool_init(&pool);

    function_info_t* f = find_func("print_hello");
    TEST_ASSERT_NOT_NULL(f);

    dyncall_status_t s = invoke_by_pool(&pool, f);
    TEST_ASSERT_EQUAL_INT(DYNCALL_NO_ERROR, s);
    TEST_ASSERT_EQUAL_INT(1, print_hello_mock_fake.call_count);
}

void test_invoke_by_pool_dec(void)
{
    dynpool_t pool;
    dynpool_init(&pool);

    int64_t val = 42;
    dynpool_set(&pool, T_DEC64, &val, 0);

    function_info_t* f = find_func("print_dec");
    TEST_ASSERT_NOT_NULL(f);

    dyncall_status_t s = invoke_by_pool(&pool, f);
    TEST_ASSERT_EQUAL_INT(DYNCALL_NO_ERROR, s);
    TEST_ASSERT_EQUAL_INT(1, print_dec_mock_fake.call_count);
    int64_t received = *(int64_t*)*print_dec_mock_fake.arg0_val;
    TEST_ASSERT_EQUAL_INT64(42, received);
}

void test_invoke_by_pool_hex(void)
{
    dynpool_t pool;
    dynpool_init(&pool);

    uint64_t val = 0xDEAD;
    dynpool_set(&pool, T_HEX64, &val, 0);

    function_info_t* f = find_func("print_hex");
    TEST_ASSERT_NOT_NULL(f);

    dyncall_status_t s = invoke_by_pool(&pool, f);
    TEST_ASSERT_EQUAL_INT(DYNCALL_NO_ERROR, s);
    TEST_ASSERT_EQUAL_INT(1, print_hex_mock_fake.call_count);
}

void test_invoke_by_pool_string(void)
{
    dynpool_t pool;
    dynpool_init(&pool);

    const char* val = "test_value";
    dynpool_set(&pool, T_STRING, (void*)val, strlen(val) + 1);

    function_info_t* f = find_func("print_string");
    TEST_ASSERT_NOT_NULL(f);

    dyncall_status_t s = invoke_by_pool(&pool, f);
    TEST_ASSERT_EQUAL_INT(DYNCALL_NO_ERROR, s);
    TEST_ASSERT_EQUAL_INT(1, print_string_mock_fake.call_count);
}

void test_invoke_by_pool_add(void)
{
    dynpool_t pool;
    dynpool_init(&pool);

    int64_t a = 10, b = 20;
    dynpool_set(&pool, T_DEC64, &a, 0);
    dynpool_set(&pool, T_DEC64, &b, 0);

    function_info_t* f = find_func("print_add");
    TEST_ASSERT_NOT_NULL(f);

    dyncall_status_t s = invoke_by_pool(&pool, f);
    TEST_ASSERT_EQUAL_INT(DYNCALL_NO_ERROR, s);
    TEST_ASSERT_EQUAL_INT(1, print_add_mock_fake.call_count);
    TEST_ASSERT_EQUAL_INT64(10, *(int64_t*)*print_add_mock_fake.arg0_val);
    TEST_ASSERT_EQUAL_INT64(20, *(int64_t*)*print_add_mock_fake.arg1_val);
}

void test_invoke_by_pool_args_3(void)
{
    dynpool_t pool;
    dynpool_init(&pool);

    dynpool_set(&pool, T_STRING, "alpha", 6);
    dynpool_set(&pool, T_STRING, "beta", 5);
    dynpool_set(&pool, T_STRING, "gamma", 6);

    function_info_t* f = find_func("print_args");
    TEST_ASSERT_NOT_NULL(f);

    dyncall_status_t s = invoke_by_pool(&pool, f);
    TEST_ASSERT_EQUAL_INT(DYNCALL_NO_ERROR, s);
    TEST_ASSERT_EQUAL_INT(1, print_args_mock_fake.call_count);
}

void test_invoke_by_pool_null_pool(void)
{
    function_info_t* f = find_func("print_hello");
    dyncall_status_t s = invoke_by_pool(NULL, f);
    TEST_ASSERT_EQUAL_INT(DYNCALL_ERR_NULL_OBJECT, s);
}

void test_invoke_by_pool_null_func(void)
{
    dynpool_t pool;
    dynpool_init(&pool);
    dyncall_status_t s = invoke_by_pool(&pool, NULL);
    TEST_ASSERT_EQUAL_INT(DYNCALL_ERR_NULL_OBJECT, s);
}

/* ===== invoke_by_cmd ===== */
void test_invoke_by_cmd_hello(void)
{
    dynpool_t pool;
    uint16_t len;
    cmdparse_from_string(&pool, "print_hello()", &len);

    dyncall_status_t s = invoke_by_cmd(&pool);
    TEST_ASSERT_EQUAL_INT(DYNCALL_NO_ERROR, s);
    TEST_ASSERT_EQUAL_INT(1, print_hello_mock_fake.call_count);
}

void test_invoke_by_cmd_not_found(void)
{
    dynpool_t pool;
    uint16_t len;
    cmdparse_from_string(&pool, "nonexistent()", &len);

    dyncall_status_t s = invoke_by_cmd(&pool);
    TEST_ASSERT_EQUAL_INT(DYNCALL_ERR_FUNC_NOT_FOUND, s);
}

void test_invoke_by_cmd_null_pool(void)
{
    dyncall_status_t s = invoke_by_cmd(NULL);
    TEST_ASSERT_EQUAL_INT(DYNCALL_ERR_NULL_OBJECT, s);
}

int run_dyncall_tests(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_find_func_exists);
    RUN_TEST(test_find_func_not_exists);
    RUN_TEST(test_find_func_empty_name);
    RUN_TEST(test_find_func_after_reset);
    RUN_TEST(test_register_group);
    RUN_TEST(test_register_multiple_groups);
    RUN_TEST(test_invoke_hello);
    RUN_TEST(test_invoke_not_found);
    RUN_TEST(test_invoke_by_pool_hello);
    RUN_TEST(test_invoke_by_pool_dec);
    RUN_TEST(test_invoke_by_pool_hex);
    RUN_TEST(test_invoke_by_pool_string);
    RUN_TEST(test_invoke_by_pool_add);
    RUN_TEST(test_invoke_by_pool_args_3);
    RUN_TEST(test_invoke_by_pool_null_pool);
    RUN_TEST(test_invoke_by_pool_null_func);
    RUN_TEST(test_invoke_by_cmd_hello);
    RUN_TEST(test_invoke_by_cmd_not_found);
    RUN_TEST(test_invoke_by_cmd_null_pool);

    return UNITY_END();
}
