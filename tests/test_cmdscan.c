/**
 * test_cmdscan.c — Unit tests for cmdscan module
 */
#include "unity.h"
#include "test_helpers.h"
#include "cmdscan.h"
#include <string.h>

/* ===== cmdscan_init ===== */
void test_cmdscan_init(void)
{
    cmd_scanner_t scanner;
    uint8_t buf[] = "hello";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);

    TEST_ASSERT_EQUAL_UINT16(0, scanner.scan_pos);
    TEST_ASSERT_EQUAL_UINT16(0, scanner.cmd_start);
    TEST_ASSERT_EQUAL_UINT16(0, scanner.cmd_len);
    TEST_ASSERT_EQUAL_INT(SCAN_IDLE, scanner.state);
    TEST_ASSERT_EQUAL_UINT8(0, scanner.in_args);
    TEST_ASSERT_EQUAL_INT8(0, scanner.error);
}

void test_cmdscan_init_null(void)
{
    cmdscan_init(NULL, NULL, 0);  /* 不应崩溃 */
}

/* ===== cmdscan_reset ===== */
void test_cmdscan_reset(void)
{
    cmd_scanner_t scanner;
    uint8_t buf[] = "test";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);
    scanner.scan_pos = 5;
    scanner.cmd_start = 2;
    scanner.cmd_len = 3;
    scanner.state = SCAN_args;
    scanner.in_args = 1;

    cmdscan_reset(&scanner);

    TEST_ASSERT_EQUAL_UINT16(0, scanner.scan_pos);
    TEST_ASSERT_EQUAL_UINT16(0, scanner.cmd_start);
    TEST_ASSERT_EQUAL_UINT16(0, scanner.cmd_len);
    TEST_ASSERT_EQUAL_INT(SCAN_IDLE, scanner.state);
    TEST_ASSERT_EQUAL_UINT8(0, scanner.in_args);
}

/* ===== cmdscan_scan: 无参命令 ===== */
void test_cmdscan_no_args(void)
{
    cmd_scanner_t scanner;
    uint8_t buf[] = "print\n";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);

    scan_status_t s = cmdscan_scan(&scanner);
    TEST_ASSERT_EQUAL_INT(SCAN_COMPLETE, s);
    TEST_ASSERT_EQUAL_UINT16(0, scanner.cmd_start);
    TEST_ASSERT_EQUAL_UINT16(5, scanner.cmd_len);
}

void test_cmdscan_no_args_cr(void)
{
    cmd_scanner_t scanner;
    uint8_t buf[] = "print\r";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);

    /* \r 是空白不是终止符，返回 INCOMPLETE */
    scan_status_t s = cmdscan_scan(&scanner);
    TEST_ASSERT_EQUAL_INT(SCAN_INCOMPLETE, s);
}

void test_cmdscan_no_args_null(void)
{
    cmd_scanner_t scanner;
    uint8_t buf[] = "print";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);

    /* 没有终止符，应该返回 INCOMPLETE */
    scan_status_t s = cmdscan_scan(&scanner);
    TEST_ASSERT_EQUAL_INT(SCAN_INCOMPLETE, s);
}

/* ===== cmdscan_scan: 带括号命令 ===== */
void test_cmdscan_with_parens(void)
{
    cmd_scanner_t scanner;
    uint8_t buf[] = "print(1,2)\n";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);

    scan_status_t s = cmdscan_scan(&scanner);
    TEST_ASSERT_EQUAL_INT(SCAN_COMPLETE, s);
    TEST_ASSERT_EQUAL_UINT16(10, scanner.cmd_len);
}

void test_cmdscan_parens_unclosed(void)
{
    cmd_scanner_t scanner;
    uint8_t buf[] = "print(1,2\n";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);

    scan_status_t s = cmdscan_scan(&scanner);
    TEST_ASSERT_EQUAL_INT(SCAN_ERROR, s);
}

/* ===== cmdscan_scan: 空格分隔命令 ===== */
void test_cmdscan_space_separated(void)
{
    cmd_scanner_t scanner;
    uint8_t buf[] = "print 1 2\n";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);

    scan_status_t s = cmdscan_scan(&scanner);
    TEST_ASSERT_EQUAL_INT(SCAN_COMPLETE, s);
}

/* ===== cmdscan_scan: 分号分隔命令 ===== */
void test_cmdscan_semicolon_separated(void)
{
    cmd_scanner_t scanner;
    uint8_t buf[] = "print;1;2\n";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);

    scan_status_t s = cmdscan_scan(&scanner);
    TEST_ASSERT_EQUAL_INT(SCAN_COMPLETE, s);
}

/* ===== cmdscan_scan: 前导空白 ===== */
void test_cmdscan_leading_whitespace(void)
{
    cmd_scanner_t scanner;
    uint8_t buf[] = "  \tprint\n";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);

    scan_status_t s = cmdscan_scan(&scanner);
    TEST_ASSERT_EQUAL_INT(SCAN_COMPLETE, s);
    /* "  \t" = 3 chars (2空格+1tab), cmd_start=3 */
    TEST_ASSERT_EQUAL_UINT16(3, scanner.cmd_start);
}

/* ===== cmdscan_scan: 空行 ===== */
void test_cmdscan_empty_line(void)
{
    cmd_scanner_t scanner;
    uint8_t buf[] = "\nprint\n";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);

    scan_status_t s = cmdscan_scan(&scanner);
    TEST_ASSERT_EQUAL_INT(SCAN_COMPLETE, s);
    /* \n 是终止符被跳过，从位置1开始扫描 "print" */
    TEST_ASSERT_EQUAL_UINT16(1, scanner.cmd_start);
    TEST_ASSERT_EQUAL_UINT16(5, scanner.cmd_len);     /* "print" */
}

/* ===== cmdscan_scan: 多命令 ===== */
void test_cmdscan_multi_command(void)
{
    cmd_scanner_t scanner;
    uint8_t buf[] = "cmd1\ncmd2\n";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);

    /* 第一条 */
    scan_status_t s = cmdscan_scan(&scanner);
    TEST_ASSERT_EQUAL_INT(SCAN_COMPLETE, s);
    TEST_ASSERT_EQUAL_UINT16(4, scanner.cmd_len);

    /* 第二条 */
    s = cmdscan_scan(&scanner);
    TEST_ASSERT_EQUAL_INT(SCAN_COMPLETE, s);
    TEST_ASSERT_EQUAL_UINT16(4, scanner.cmd_len);
}

/* ===== cmdscan_next ===== */
void test_cmdscan_next(void)
{
    cmd_scanner_t scanner;
    uint8_t buf[] = "cmd1\ncmd2\n";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);

    cmdscan_scan(&scanner);  /* 扫描第一条 */

    uint16_t next_start;
    uint16_t remaining = cmdscan_next(&scanner, &next_start);

    TEST_ASSERT_EQUAL_UINT16(5, next_start);
    TEST_ASSERT_EQUAL_UINT16(5, remaining);
}

void test_cmdscan_next_end(void)
{
    cmd_scanner_t scanner;
    uint8_t buf[] = "cmd1\n";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);

    cmdscan_scan(&scanner);

    uint16_t next_start;
    uint16_t remaining = cmdscan_next(&scanner, &next_start);

    TEST_ASSERT_EQUAL_UINT16(0, remaining);
}

/* ===== cmdscan_prefetch: 四种命令格式 ===== */
void test_cmdscan_prefetch_parens(void)
{
    /* print(1)\n */
    cmd_scanner_t scanner;
    uint8_t buf[] = "print(1)\n";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);

    cmd_prefetch_t pf;
    scan_status_t s = cmdscan_prefetch(&scanner, &pf);
    TEST_ASSERT_EQUAL_INT(SCAN_COMPLETE, s);
    TEST_ASSERT_EQUAL_UINT16(0, pf.cmd_start);
    TEST_ASSERT_EQUAL_UINT16(8, pf.cmd_len);      /* "print(1)" */
    TEST_ASSERT_EQUAL_UINT8(5, pf.func_len);      /* "print" */
    TEST_ASSERT_EQUAL_MEMORY("print", pf.buf + pf.cmd_start, pf.func_len);
}

void test_cmdscan_prefetch_parens_with_semicolon(void)
{
    /* print(1);\n */
    cmd_scanner_t scanner;
    uint8_t buf[] = "print(1);\n";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);

    cmd_prefetch_t pf;
    scan_status_t s = cmdscan_prefetch(&scanner, &pf);
    TEST_ASSERT_EQUAL_INT(SCAN_COMPLETE, s);
    TEST_ASSERT_EQUAL_UINT16(8, pf.cmd_len);      /* "print(1)" 不含分号 */
    TEST_ASSERT_EQUAL_UINT8(5, pf.func_len);
}

void test_cmdscan_prefetch_space_separated(void)
{
    /* print 1\n */
    cmd_scanner_t scanner;
    uint8_t buf[] = "print 1\n";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);

    cmd_prefetch_t pf;
    scan_status_t s = cmdscan_prefetch(&scanner, &pf);
    TEST_ASSERT_EQUAL_INT(SCAN_COMPLETE, s);
    TEST_ASSERT_EQUAL_UINT16(7, pf.cmd_len);      /* "print 1" */
    TEST_ASSERT_EQUAL_UINT8(5, pf.func_len);
}

void test_cmdscan_prefetch_semicolon_separated(void)
{
    /* print;1;\n */
    cmd_scanner_t scanner;
    uint8_t buf[] = "print;1;\n";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);

    cmd_prefetch_t pf;
    scan_status_t s = cmdscan_prefetch(&scanner, &pf);
    TEST_ASSERT_EQUAL_INT(SCAN_COMPLETE, s);
    TEST_ASSERT_EQUAL_UINT16(5, pf.cmd_len);      /* "print" 不含分号 */
    TEST_ASSERT_EQUAL_UINT8(5, pf.func_len);
}

void test_cmdscan_prefetch_multi_args_parens(void)
{
    /* print(1,2,3)\n */
    cmd_scanner_t scanner;
    uint8_t buf[] = "print(1,2,3)\n";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);

    cmd_prefetch_t pf;
    scan_status_t s = cmdscan_prefetch(&scanner, &pf);
    TEST_ASSERT_EQUAL_INT(SCAN_COMPLETE, s);
    TEST_ASSERT_EQUAL_UINT16(12, pf.cmd_len);     /* "print(1,2,3)" */
    TEST_ASSERT_EQUAL_UINT8(5, pf.func_len);
}

void test_cmdscan_prefetch_multi_args_parens_semicolon(void)
{
    /* print(1,2,3);\n */
    cmd_scanner_t scanner;
    uint8_t buf[] = "print(1,2,3);\n";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);

    cmd_prefetch_t pf;
    scan_status_t s = cmdscan_prefetch(&scanner, &pf);
    TEST_ASSERT_EQUAL_INT(SCAN_COMPLETE, s);
    TEST_ASSERT_EQUAL_UINT16(12, pf.cmd_len);     /* "print(1,2,3)" 不含分号 */
    TEST_ASSERT_EQUAL_UINT8(5, pf.func_len);
}

void test_cmdscan_prefetch_multi_args_space(void)
{
    /* print 1 2 3\n */
    cmd_scanner_t scanner;
    uint8_t buf[] = "print 1 2 3\n";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);

    cmd_prefetch_t pf;
    scan_status_t s = cmdscan_prefetch(&scanner, &pf);
    TEST_ASSERT_EQUAL_INT(SCAN_COMPLETE, s);
    TEST_ASSERT_EQUAL_UINT16(11, pf.cmd_len);     /* "print 1 2 3" */
    TEST_ASSERT_EQUAL_UINT8(5, pf.func_len);
}

void test_cmdscan_prefetch_multi_args_semicolon(void)
{
    /* print;1;2;3;\n */
    cmd_scanner_t scanner;
    uint8_t buf[] = "print;1;2;3;\n";
    cmdscan_init(&scanner, buf, sizeof(buf) - 1);

    cmd_prefetch_t pf;
    scan_status_t s = cmdscan_prefetch(&scanner, &pf);
    TEST_ASSERT_EQUAL_INT(SCAN_COMPLETE, s);
    TEST_ASSERT_EQUAL_UINT16(5, pf.cmd_len);      /* "print" 不含分号 */
    TEST_ASSERT_EQUAL_UINT8(5, pf.func_len);
}

/* ===== cmdparse_args: 基本 ===== */
void test_cmdparse_args_no_args(void)
{
    cmd_parse_result_t result;
    uint8_t count = cmdparse_args("print", 5, &result);

    TEST_ASSERT_EQUAL_UINT8(0, count);
    TEST_ASSERT_EQUAL_UINT16(5, result.func_name_len);
    TEST_ASSERT_EQUAL_MEMORY("print", result.func_name, 5);
}

void test_cmdparse_args_with_parens(void)
{
    cmd_parse_result_t result;
    uint8_t count = cmdparse_args("print(1,2)", 10, &result);

    TEST_ASSERT_EQUAL_UINT8(2, count);
    TEST_ASSERT_EQUAL_MEMORY("print", result.func_name, 5);
    TEST_ASSERT_EQUAL_MEMORY("1", result.args[0].ptr, result.args[0].len);
    TEST_ASSERT_EQUAL_MEMORY("2", result.args[1].ptr, result.args[1].len);
}

void test_cmdparse_args_space_separated(void)
{
    cmd_parse_result_t result;
    uint8_t count = cmdparse_args("print 1 2", 9, &result);

    TEST_ASSERT_EQUAL_UINT8(2, count);
    TEST_ASSERT_EQUAL_MEMORY("print", result.func_name, 5);
    TEST_ASSERT_EQUAL_MEMORY("1", result.args[0].ptr, result.args[0].len);
    TEST_ASSERT_EQUAL_MEMORY("2", result.args[1].ptr, result.args[1].len);
}

void test_cmdparse_args_semicolon_separated(void)
{
    cmd_parse_result_t result;
    uint8_t count = cmdparse_args("print;1;2", 9, &result);

    TEST_ASSERT_EQUAL_UINT8(2, count);
    TEST_ASSERT_EQUAL_MEMORY("print", result.func_name, 5);
    TEST_ASSERT_EQUAL_MEMORY("1", result.args[0].ptr, result.args[0].len);
    TEST_ASSERT_EQUAL_MEMORY("2", result.args[1].ptr, result.args[1].len);
}

void test_cmdparse_args_with_spaces(void)
{
    cmd_parse_result_t result;
    uint8_t count = cmdparse_args("print( 1 , 2 )", 14, &result);

    TEST_ASSERT_EQUAL_UINT8(2, count);
    TEST_ASSERT_EQUAL_MEMORY("1", result.args[0].ptr, result.args[0].len);
    TEST_ASSERT_EQUAL_MEMORY("2", result.args[1].ptr, result.args[1].len);
}

void test_cmdparse_args_empty(void)
{
    cmd_parse_result_t result;
    uint8_t count = cmdparse_args("", 0, &result);
    TEST_ASSERT_EQUAL_UINT8(0xFF, count);
}

void test_cmdparse_args_null(void)
{
    uint8_t count = cmdparse_args(NULL, 0, NULL);
    TEST_ASSERT_EQUAL_UINT8(0xFF, count);
}

void test_cmdparse_args_func_with_space(void)
{
    cmd_parse_result_t result;
    uint8_t count = cmdparse_args("print 123", 9, &result);

    TEST_ASSERT_EQUAL_UINT8(1, count);
    TEST_ASSERT_EQUAL_MEMORY("print", result.func_name, 5);
    TEST_ASSERT_EQUAL_MEMORY("123", result.args[0].ptr, result.args[0].len);
}

/* ===== 零拷贝验证 ===== */
void test_cmdparse_zero_copy(void)
{
    char buf[] = "print(1,2)";
    cmd_parse_result_t result;
    cmdparse_args(buf, strlen(buf), &result);

    /* 指针应指向原始缓冲区 */
    TEST_ASSERT_TRUE(result.func_name >= buf && result.func_name < buf + strlen(buf));
    TEST_ASSERT_TRUE(result.args[0].ptr >= buf && result.args[0].ptr < buf + strlen(buf));
    TEST_ASSERT_TRUE(result.args[1].ptr >= buf && result.args[1].ptr < buf + strlen(buf));
}

/* ===== runner ===== */
int run_cmdscan_tests(void)
{
    UnityBegin("test_cmdscan.c");

    /* cmdscan_init */
    RUN_TEST(test_cmdscan_init);
    RUN_TEST(test_cmdscan_init_null);

    /* cmdscan_reset */
    RUN_TEST(test_cmdscan_reset);

    /* cmdscan_scan */
    RUN_TEST(test_cmdscan_no_args);
    RUN_TEST(test_cmdscan_no_args_cr);
    RUN_TEST(test_cmdscan_no_args_null);
    RUN_TEST(test_cmdscan_with_parens);
    RUN_TEST(test_cmdscan_parens_unclosed);
    RUN_TEST(test_cmdscan_space_separated);
    RUN_TEST(test_cmdscan_semicolon_separated);
    RUN_TEST(test_cmdscan_leading_whitespace);
    RUN_TEST(test_cmdscan_empty_line);
    RUN_TEST(test_cmdscan_multi_command);

    /* cmdscan_next */
    RUN_TEST(test_cmdscan_next);
    RUN_TEST(test_cmdscan_next_end);

    /* cmdscan_prefetch */
    RUN_TEST(test_cmdscan_prefetch_parens);
    RUN_TEST(test_cmdscan_prefetch_parens_with_semicolon);
    RUN_TEST(test_cmdscan_prefetch_space_separated);
    RUN_TEST(test_cmdscan_prefetch_semicolon_separated);
    RUN_TEST(test_cmdscan_prefetch_multi_args_parens);
    RUN_TEST(test_cmdscan_prefetch_multi_args_parens_semicolon);
    RUN_TEST(test_cmdscan_prefetch_multi_args_space);
    RUN_TEST(test_cmdscan_prefetch_multi_args_semicolon);

    /* cmdparse_args */
    RUN_TEST(test_cmdparse_args_no_args);
    RUN_TEST(test_cmdparse_args_with_parens);
    RUN_TEST(test_cmdparse_args_space_separated);
    RUN_TEST(test_cmdparse_args_semicolon_separated);
    RUN_TEST(test_cmdparse_args_with_spaces);
    RUN_TEST(test_cmdparse_args_empty);
    RUN_TEST(test_cmdparse_args_null);
    RUN_TEST(test_cmdparse_args_func_with_space);

    /* 零拷贝 */
    RUN_TEST(test_cmdparse_zero_copy);

    return UnityEnd();
}
