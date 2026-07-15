/**
 * test_cmdparse.c — Unit tests for command parser
 */
#include "unity.h"
#include "test_helpers.h"
#include "cmdparse.h"
#include "dynpool.h"
#include <string.h>

/* Helper: parse string and return pool wr_count */
static uint16_t parse_count(const char* str, cmd_parse_status_t* out_status)
{
    dynpool_t pool;
    uint16_t len;
    cmd_parse_status_t s = cmdparse_from_string(&pool, str, &len);
    if (out_status) *out_status = s;
    return pool.wr_count;
}

static void get_first_token(const char* str, char* buf, size_t bufsize)
{
    dynpool_t pool;
    uint16_t len;
    cmdparse_from_string(&pool, str, &len);
    uint16_t used;
    dynpool_get(&pool, T_STRING, buf, (uint16_t)bufsize, &used);
}

/* ===== Basic parsing ===== */
void test_cmdparse_simple_no_args(void)
{
    cmd_parse_status_t s;
    uint16_t count = parse_count("print_hello()", &s);
    TEST_ASSERT_EQUAL_INT(CMDPARSE_OK, s);
    TEST_ASSERT_EQUAL_UINT16(1, count);

    char buf[50];
    get_first_token("print_hello()", buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("print_hello", buf);
}

void test_cmdparse_simple_no_parens(void)
{
    cmd_parse_status_t s;
    uint16_t count = parse_count("print_hello", &s);
    TEST_ASSERT_EQUAL_INT(CMDPARSE_OK, s);
    TEST_ASSERT_EQUAL_UINT16(1, count);
}

void test_cmdparse_with_args_parens(void)
{
    cmd_parse_status_t s;
    uint16_t count = parse_count("print_args(a,b,c)", &s);
    TEST_ASSERT_EQUAL_INT(CMDPARSE_OK, s);
    TEST_ASSERT_EQUAL_UINT16(4, count); /* func + 3 args */
}

void test_cmdparse_with_args_space(void)
{
    cmd_parse_status_t s;
    uint16_t count = parse_count("print_args a b c", &s);
    TEST_ASSERT_EQUAL_INT(CMDPARSE_OK, s);
    TEST_ASSERT_EQUAL_UINT16(4, count);
}

void test_cmdparse_two_args_comma(void)
{
    cmd_parse_status_t s;
    uint16_t count = parse_count("print_add(10,20)", &s);
    TEST_ASSERT_EQUAL_INT(CMDPARSE_OK, s);
    TEST_ASSERT_EQUAL_UINT16(3, count); /* func + 2 args */
}

void test_cmdparse_two_args_space(void)
{
    cmd_parse_status_t s;
    uint16_t count = parse_count("print_add 10 20", &s);
    TEST_ASSERT_EQUAL_INT(CMDPARSE_OK, s);
    TEST_ASSERT_EQUAL_UINT16(3, count);
}

/* ===== Whitespace handling ===== */
void test_cmdparse_spaces_around_args(void)
{
    cmd_parse_status_t s;
    uint16_t count = parse_count("print_add( 10 , 20 )", &s);
    TEST_ASSERT_EQUAL_INT(CMDPARSE_OK, s);
    TEST_ASSERT_EQUAL_UINT16(3, count);
}

void test_cmdparse_leading_whitespace(void)
{
    cmd_parse_status_t s;
    uint16_t count = parse_count("  print_hello()", &s);
    TEST_ASSERT_EQUAL_INT(CMDPARSE_OK, s);
    TEST_ASSERT_EQUAL_UINT16(1, count);
}

void test_cmdparse_tab_whitespace(void)
{
    cmd_parse_status_t s;
    uint16_t count = parse_count("\tprint_hello\t", &s);
    TEST_ASSERT_EQUAL_INT(CMDPARSE_OK, s);
    TEST_ASSERT_EQUAL_UINT16(1, count);
}

/* ===== Line endings ===== */
void test_cmdparse_crlf_terminator(void)
{
    dynpool_t pool;
    uint16_t len;
    cmd_parse_status_t s = cmdparse_from_string(&pool, "print_hello()\r\n", &len);
    TEST_ASSERT_EQUAL_INT(CMDPARSE_OK, s);
    TEST_ASSERT_EQUAL_UINT16(1, pool.wr_count);
}

void test_cmdparse_lf_terminator(void)
{
    dynpool_t pool;
    uint16_t len;
    cmd_parse_status_t s = cmdparse_from_string(&pool, "print_hello()\n", &len);
    TEST_ASSERT_EQUAL_INT(CMDPARSE_OK, s);
    TEST_ASSERT_EQUAL_UINT16(1, pool.wr_count);
}

void test_cmdparse_cr_terminator(void)
{
    /* \r is treated as whitespace, \0 terminates — should still parse */
    dynpool_t pool;
    uint16_t len;
    cmd_parse_status_t s = cmdparse_from_string(&pool, "print_hello()\r", &len);
    TEST_ASSERT_EQUAL_INT(CMDPARSE_OK, s);
    TEST_ASSERT_EQUAL_UINT16(1, pool.wr_count);
}

/* ===== Separators ===== */
void test_cmdparse_comma_separator(void)
{
    cmd_parse_status_t s;
    uint16_t count = parse_count("f(1,2,3)", &s);
    TEST_ASSERT_EQUAL_INT(CMDPARSE_OK, s);
    TEST_ASSERT_EQUAL_UINT16(4, count);
}

void test_cmdparse_semicolon_separator(void)
{
    cmd_parse_status_t s;
    uint16_t count = parse_count("f(1;2;3)", &s);
    TEST_ASSERT_EQUAL_INT(CMDPARSE_OK, s);
    TEST_ASSERT_EQUAL_UINT16(4, count);
}

void test_cmdparse_space_separator(void)
{
    cmd_parse_status_t s;
    uint16_t count = parse_count("f 1 2 3", &s);
    TEST_ASSERT_EQUAL_INT(CMDPARSE_OK, s);
    TEST_ASSERT_EQUAL_UINT16(4, count);
}

/* ===== Bracket types ===== */
void test_cmdparse_square_brackets(void)
{
    cmd_parse_status_t s;
    uint16_t count = parse_count("f[1,2]", &s);
    TEST_ASSERT_EQUAL_INT(CMDPARSE_OK, s);
    TEST_ASSERT_EQUAL_UINT16(3, count);
}

void test_cmdparse_curly_braces(void)
{
    cmd_parse_status_t s;
    uint16_t count = parse_count("f{1,2}", &s);
    TEST_ASSERT_EQUAL_INT(CMDPARSE_OK, s);
    TEST_ASSERT_EQUAL_UINT16(3, count);
}

/* ===== Token extraction ===== */
void test_cmdparse_token_values(void)
{
    dynpool_t pool;
    uint16_t len;
    cmdparse_from_string(&pool, "print_args(hello,world,test)", &len);

    char buf[50];
    uint16_t used;

    dynpool_get(&pool, T_STRING, buf, sizeof(buf), &used);
    TEST_ASSERT_EQUAL_STRING("print_args", buf);

    dynpool_get(&pool, T_STRING, buf, sizeof(buf), &used);
    TEST_ASSERT_EQUAL_STRING("hello", buf);

    dynpool_get(&pool, T_STRING, buf, sizeof(buf), &used);
    TEST_ASSERT_EQUAL_STRING("world", buf);

    dynpool_get(&pool, T_STRING, buf, sizeof(buf), &used);
    TEST_ASSERT_EQUAL_STRING("test", buf);
}

void test_cmdparse_token_trim_whitespace(void)
{
    dynpool_t pool;
    uint16_t len;
    cmdparse_from_string(&pool, "f( hello , world )", &len);

    char buf[50];
    uint16_t used;

    dynpool_get(&pool, T_STRING, buf, sizeof(buf), &used); /* skip func */
    dynpool_get(&pool, T_STRING, buf, sizeof(buf), &used);
    TEST_ASSERT_EQUAL_STRING("hello", buf);

    dynpool_get(&pool, T_STRING, buf, sizeof(buf), &used);
    TEST_ASSERT_EQUAL_STRING("world", buf);
}

/* ===== Error cases ===== */
void test_cmdparse_unclosed_paren(void)
{
    cmd_parse_status_t s;
    parse_count("print_hello(", &s);
    TEST_ASSERT_EQUAL_INT(CMDPARSE_INVALID_FORMAT, s);
}

void test_cmdparse_unmatched_close(void)
{
    cmd_parse_status_t s;
    parse_count("print_hello)", &s);
    TEST_ASSERT_EQUAL_INT(CMDPARSE_INVALID_FORMAT, s);
}

void test_cmdparse_multi_line_first_cmd(void)
{
    dynpool_t pool;
    uint16_t len;
    cmd_parse_status_t s = cmdparse_from_string(&pool, "print_hello()\nprint_hello()", &len);
    TEST_ASSERT_EQUAL_INT(CMDPARSE_OK, s);
    /* First command: "print_hello()" is 13 chars + \n = 14 */
    TEST_ASSERT_EQUAL_UINT16(14, len);
    TEST_ASSERT_EQUAL_UINT16(1, pool.wr_count);
}

int run_cmdparse_tests(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_cmdparse_simple_no_args);
    RUN_TEST(test_cmdparse_simple_no_parens);
    RUN_TEST(test_cmdparse_with_args_parens);
    RUN_TEST(test_cmdparse_with_args_space);
    RUN_TEST(test_cmdparse_two_args_comma);
    RUN_TEST(test_cmdparse_two_args_space);
    RUN_TEST(test_cmdparse_spaces_around_args);
    RUN_TEST(test_cmdparse_leading_whitespace);
    RUN_TEST(test_cmdparse_tab_whitespace);
    RUN_TEST(test_cmdparse_crlf_terminator);
    RUN_TEST(test_cmdparse_lf_terminator);
    RUN_TEST(test_cmdparse_cr_terminator);
    RUN_TEST(test_cmdparse_comma_separator);
    RUN_TEST(test_cmdparse_semicolon_separator);
    RUN_TEST(test_cmdparse_space_separator);
    RUN_TEST(test_cmdparse_square_brackets);
    RUN_TEST(test_cmdparse_curly_braces);
    RUN_TEST(test_cmdparse_token_values);
    RUN_TEST(test_cmdparse_token_trim_whitespace);
    RUN_TEST(test_cmdparse_unclosed_paren);
    RUN_TEST(test_cmdparse_unmatched_close);
    RUN_TEST(test_cmdparse_multi_line_first_cmd);

    return UNITY_END();
}
