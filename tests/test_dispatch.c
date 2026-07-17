/**
 * test_dispatch.c — Unit tests for dispatch v2 module
 */
#include "unity.h"
#include "dispatch.h"
#include <string.h>

/* ---- mock functions ---- */
static void mock_hello(void) {}
static int64_t mock_add(void* a, void* b) { (void)a; (void)b; return 0; }
static void mock_proc(void* a, void* b, void* c) { (void)a; (void)b; (void)c; }

/* ===== init ===== */

void test_dispatch_init_empty(void)
{
    dispatch_init();
    dispatch_func_t* f = dispatch_find("hello", 5);
    TEST_ASSERT_NULL(f);
}

void test_dispatch_init_clears(void)
{
    dispatch_init();
    dispatch_reg(mock_hello, "hello()");

    dispatch_init(); /* clear */
    dispatch_func_t* f = dispatch_find("hello", 5);
    TEST_ASSERT_NULL(f);
}

/* ===== register + find ===== */

void test_dispatch_register_and_find(void)
{
    dispatch_init();
    dispatch_reg(mock_hello, "hello()");

    dispatch_func_t* f = dispatch_find("hello", 5);
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL_STRING("hello", f->name);
    TEST_ASSERT_EQUAL_UINT8(DV, f->ret_type);
    TEST_ASSERT_EQUAL_UINT8(0, f->args_count);
}

void test_dispatch_find_add(void)
{
    dispatch_init();
    dispatch_reg(mock_add, "add(i, i) -> i");

    dispatch_func_t* f = dispatch_find("add", 3);
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL_UINT8(DI, f->ret_type);
    TEST_ASSERT_EQUAL_UINT8(2, f->args_count);
    TEST_ASSERT_EQUAL_UINT8(DI, f->args_type[0]);
    TEST_ASSERT_EQUAL_UINT8(DI, f->args_type[1]);
}

void test_dispatch_find_hex(void)
{
    dispatch_init();
    dispatch_reg(mock_add, "add(u, u) -> u");

    dispatch_func_t* f = dispatch_find("add", 3);
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL_UINT8(DU, f->ret_type);
    TEST_ASSERT_EQUAL_UINT8(DU, f->args_type[0]);
    TEST_ASSERT_EQUAL_UINT8(DU, f->args_type[1]);
}

void test_dispatch_find_string(void)
{
    dispatch_init();
    dispatch_reg(mock_hello, "echo(s) -> s");

    dispatch_func_t* f = dispatch_find("echo", 4);
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL_UINT8(DS, f->ret_type);
    TEST_ASSERT_EQUAL_UINT8(1, f->args_count);
    TEST_ASSERT_EQUAL_UINT8(DS, f->args_type[0]);
}

void test_dispatch_find_proc(void)
{
    dispatch_init();
    dispatch_reg(mock_proc, "proc(s, i, u)");

    dispatch_func_t* f = dispatch_find("proc", 4);
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL_UINT8(DV, f->ret_type);
    TEST_ASSERT_EQUAL_UINT8(3, f->args_count);
    TEST_ASSERT_EQUAL_UINT8(DS, f->args_type[0]);
    TEST_ASSERT_EQUAL_UINT8(DI, f->args_type[1]);
    TEST_ASSERT_EQUAL_UINT8(DU, f->args_type[2]);
}

/* ===== signature format variants ===== */

void test_dispatch_sig_no_parens(void)
{
    dispatch_init();
    dispatch_reg(mock_add, "i, i -> i");

    // 无 '(' 时, 名字用外部参数 "mock_add"
    dispatch_func_t* f = dispatch_find("mock_add", 8);
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL_UINT8(2, f->args_count);
}

void test_dispatch_sig_empty_parens(void)
{
    dispatch_init();
    dispatch_reg(mock_hello, "()");

    // 无函数名时, 名字用外部参数 "mock_hello"
    dispatch_func_t* f = dispatch_find("mock_hello", 10);
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL_UINT8(0, f->args_count);
}

void test_dispatch_sig_empty_string(void)
{
    dispatch_init();
    dispatch_reg(mock_hello, "");

    // 空签名时, 名字用外部参数 "mock_hello"
    dispatch_func_t* f = dispatch_find("mock_hello", 10);
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL_UINT8(0, f->args_count);
    TEST_ASSERT_EQUAL_UINT8(DV, f->ret_type);
}

void test_dispatch_sig_types(void)
{
    dispatch_init();
    dispatch_reg(mock_add, "add(i64, u64) -> f64");

    dispatch_func_t* f = dispatch_find("add", 3);
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL_UINT8(DF, f->ret_type);
    TEST_ASSERT_EQUAL_UINT8(DI, f->args_type[0]);
    TEST_ASSERT_EQUAL_UINT8(DU, f->args_type[1]);
}

/* ===== find not found ===== */

void test_dispatch_find_not_found(void)
{
    dispatch_init();
    dispatch_reg(mock_hello, "hello()");

    dispatch_func_t* f = dispatch_find("nonexistent", 10);
    TEST_ASSERT_NULL(f);
}

void test_dispatch_find_null_name(void)
{
    dispatch_init();
    dispatch_reg(mock_hello, "hello()");

    dispatch_func_t* f = dispatch_find(NULL, 0);
    TEST_ASSERT_NULL(f);
}

/* ===== find with length ===== */

void test_dispatch_find_len_basic(void)
{
    dispatch_init();
    dispatch_reg(mock_hello, "hello()");

    dispatch_func_t* f = dispatch_find("hello", 5);
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL_STRING("hello", f->name);
}

void test_dispatch_find_len_partial(void)
{
    dispatch_init();
    dispatch_reg(mock_hello, "hello()");

    dispatch_func_t* f = dispatch_find("hel", 3);
    TEST_ASSERT_NULL(f);
}

void test_dispatch_find_len_not_found(void)
{
    dispatch_init();
    dispatch_reg(mock_hello, "hello()");

    dispatch_func_t* f = dispatch_find("nonexistent", 10);
    TEST_ASSERT_NULL(f);
}

void test_dispatch_find_len_zero_len(void)
{
    dispatch_init();
    dispatch_reg(mock_hello, "hello()");

    dispatch_func_t* f = dispatch_find("hello", 0);
    TEST_ASSERT_NULL(f);
}

void test_dispatch_find_len_null_name(void)
{
    dispatch_init();
    dispatch_reg(mock_hello, "hello()");

    dispatch_func_t* f = dispatch_find(NULL, 5);
    TEST_ASSERT_NULL(f);
}

/* ===== error cases ===== */

void test_dispatch_reg_null_handler(void)
{
    dispatch_init();
    dispatch_status_t st = _dispatch_add("bad", NULL, "()");
    TEST_ASSERT_EQUAL_INT(DISPATCH_ERR_NULL, st);
}

void test_dispatch_reg_null_sig(void)
{
    dispatch_init();
    dispatch_status_t st = _dispatch_add("bad", (void*)mock_hello, NULL);
    TEST_ASSERT_EQUAL_INT(DISPATCH_ERR_NULL, st);
}

void test_dispatch_reg_bad_sig(void)
{
    dispatch_init();
    dispatch_status_t st = _dispatch_add("bad", (void*)mock_hello, "xyz");
    TEST_ASSERT_EQUAL_INT(DISPATCH_ERR_SIG, st);
}

void test_dispatch_reg_overflow(void)
{
    dispatch_init();
    for (int i = 0; i < DISPATCH_MAX_FUNC_CNT; i++)
    {
        char name[16];
        snprintf(name, sizeof(name), "f%d", i);
        _dispatch_add(name, (void*)mock_hello, "()");
    }
    dispatch_status_t st = _dispatch_add("overflow", (void*)mock_hello, "()");
    TEST_ASSERT_EQUAL_INT(DISPATCH_ERR_FULL, st);
}

/* ===== test runner ===== */

int run_dispatch_tests(void)
{
    UNITY_BEGIN();

    /* init */
    RUN_TEST(test_dispatch_init_empty);
    RUN_TEST(test_dispatch_init_clears);

    /* register + find */
    RUN_TEST(test_dispatch_register_and_find);
    RUN_TEST(test_dispatch_find_add);
    RUN_TEST(test_dispatch_find_hex);
    RUN_TEST(test_dispatch_find_string);
    RUN_TEST(test_dispatch_find_proc);

    /* signature format */
    RUN_TEST(test_dispatch_sig_no_parens);
    RUN_TEST(test_dispatch_sig_empty_parens);
    RUN_TEST(test_dispatch_sig_empty_string);
    RUN_TEST(test_dispatch_sig_types);

    /* not found */
    RUN_TEST(test_dispatch_find_not_found);
    RUN_TEST(test_dispatch_find_null_name);

    /* find_len */
    RUN_TEST(test_dispatch_find_len_basic);
    RUN_TEST(test_dispatch_find_len_partial);
    RUN_TEST(test_dispatch_find_len_not_found);
    RUN_TEST(test_dispatch_find_len_zero_len);
    RUN_TEST(test_dispatch_find_len_null_name);

    /* error cases */
    RUN_TEST(test_dispatch_reg_null_handler);
    RUN_TEST(test_dispatch_reg_null_sig);
    RUN_TEST(test_dispatch_reg_bad_sig);
    RUN_TEST(test_dispatch_reg_overflow);

    return UNITY_END();
}
