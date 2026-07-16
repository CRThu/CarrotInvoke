/**
 * test_dispatch.c — Unit tests for dispatch module
 */
#include "unity.h"
#include "test_helpers.h"
#include "dispatch.h"
#include <string.h>

/* ===== init ===== */

void test_dispatch_init_empty(void)
{
    dispatch_init();
    function_info_t* f = dispatch_find("print_hello");
    TEST_ASSERT_NULL(f);
}

void test_dispatch_init_clears(void)
{
    dispatch_init();
    dispatch_register(&mock_func_group);

    dispatch_init(); /* clear */
    function_info_t* f = dispatch_find("print_hello");
    TEST_ASSERT_NULL(f);
}

/* ===== register + find ===== */

void test_dispatch_register_and_find(void)
{
    dispatch_init();
    dispatch_register(&mock_func_group);

    function_info_t* f = dispatch_find("print_hello");
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL_STRING("print_hello", f->name);
}

void test_dispatch_find_dec(void)
{
    dispatch_init();
    dispatch_register(&mock_func_group);

    function_info_t* f = dispatch_find("print_dec");
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL_INT(T_DEC64, f->args_type[0]);
}

void test_dispatch_find_hex(void)
{
    dispatch_init();
    dispatch_register(&mock_func_group);

    function_info_t* f = dispatch_find("print_hex");
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL_INT(T_HEX64, f->args_type[0]);
}

void test_dispatch_find_string(void)
{
    dispatch_init();
    dispatch_register(&mock_func_group);

    function_info_t* f = dispatch_find("print_string");
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL_INT(T_STRING, f->args_type[0]);
}

void test_dispatch_find_add(void)
{
    dispatch_init();
    dispatch_register(&mock_func_group);

    function_info_t* f = dispatch_find("print_add");
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL_INT(T_DEC64, f->ret_type);
    TEST_ASSERT_EQUAL_UINT8(2, f->args_count);
}

void test_dispatch_find_args(void)
{
    dispatch_init();
    dispatch_register(&mock_func_group);

    function_info_t* f = dispatch_find("print_args");
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL_UINT8(3, f->args_count);
    TEST_ASSERT_EQUAL_INT(T_STRING, f->args_type[0]);
    TEST_ASSERT_EQUAL_INT(T_STRING, f->args_type[1]);
    TEST_ASSERT_EQUAL_INT(T_STRING, f->args_type[2]);
}

/* ===== find not found ===== */

void test_dispatch_find_not_found(void)
{
    dispatch_init();
    dispatch_register(&mock_func_group);

    function_info_t* f = dispatch_find("nonexistent");
    TEST_ASSERT_NULL(f);
}

void test_dispatch_find_null_name(void)
{
    dispatch_init();
    dispatch_register(&mock_func_group);

    function_info_t* f = dispatch_find(NULL);
    TEST_ASSERT_NULL(f);
}

/* ===== find_len ===== */

void test_dispatch_find_len_basic(void)
{
    dispatch_init();
    dispatch_register(&mock_func_group);

    function_info_t* f = dispatch_find_len("print_hello", 11);
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL_STRING("print_hello", f->name);
}

void test_dispatch_find_len_partial(void)
{
    dispatch_init();
    dispatch_register(&mock_func_group);

    /* "print_h" is not a registered name */
    function_info_t* f = dispatch_find_len("print_h", 7);
    TEST_ASSERT_NULL(f);
}

void test_dispatch_find_len_not_found(void)
{
    dispatch_init();
    dispatch_register(&mock_func_group);

    function_info_t* f = dispatch_find_len("nonexistent", 10);
    TEST_ASSERT_NULL(f);
}

void test_dispatch_find_len_zero_len(void)
{
    dispatch_init();
    dispatch_register(&mock_func_group);

    function_info_t* f = dispatch_find_len("print_hello", 0);
    TEST_ASSERT_NULL(f);
}

void test_dispatch_find_len_null_name(void)
{
    dispatch_init();
    dispatch_register(&mock_func_group);

    function_info_t* f = dispatch_find_len(NULL, 5);
    TEST_ASSERT_NULL(f);
}

/* ===== multiple groups ===== */

void test_dispatch_multiple_groups(void)
{
    dispatch_init();

    /* Register same group twice (simulates multiple groups) */
    dispatch_register(&mock_func_group);
    dispatch_register(&mock_func_group);

    function_info_t* f = dispatch_find("print_hello");
    TEST_ASSERT_NOT_NULL(f);
}

/* ===== register NULL ===== */

void test_dispatch_register_null(void)
{
    dispatch_init();
    int result = dispatch_register(NULL);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

/* ===== register overflow ===== */

void test_dispatch_register_overflow(void)
{
    dispatch_init();
    for (int i = 0; i < DYNCALL_MAX_GROUPS; i++)
    {
        dispatch_register(&mock_func_group);
    }
    int result = dispatch_register(&mock_func_group);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

int run_dispatch_tests(void)
{
    UNITY_BEGIN();

    /* init */
    RUN_TEST(test_dispatch_init_empty);
    RUN_TEST(test_dispatch_init_clears);

    /* register + find */
    RUN_TEST(test_dispatch_register_and_find);
    RUN_TEST(test_dispatch_find_dec);
    RUN_TEST(test_dispatch_find_hex);
    RUN_TEST(test_dispatch_find_string);
    RUN_TEST(test_dispatch_find_add);
    RUN_TEST(test_dispatch_find_args);

    /* not found */
    RUN_TEST(test_dispatch_find_not_found);
    RUN_TEST(test_dispatch_find_null_name);

    /* find_len */
    RUN_TEST(test_dispatch_find_len_basic);
    RUN_TEST(test_dispatch_find_len_partial);
    RUN_TEST(test_dispatch_find_len_not_found);
    RUN_TEST(test_dispatch_find_len_zero_len);
    RUN_TEST(test_dispatch_find_len_null_name);

    /* edge cases */
    RUN_TEST(test_dispatch_multiple_groups);
    RUN_TEST(test_dispatch_register_null);
    RUN_TEST(test_dispatch_register_overflow);

    return UNITY_END();
}
