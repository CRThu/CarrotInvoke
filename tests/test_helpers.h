#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include "dyncall.h"
#include "fff.h"

/* ---- fff declarations for demo functions ---- */
/* fff: zero-arg void funcs use DECLARE_FAKE_VOID_FUNC(name) with NO type args */
DECLARE_FAKE_VOID_FUNC(print_hello_mock);
DECLARE_FAKE_VALUE_FUNC(int64_t, print_add_mock, void**, void**);
DECLARE_FAKE_VOID_FUNC(print_dec_mock, void**);
DECLARE_FAKE_VOID_FUNC(print_hex_mock, void**);
DECLARE_FAKE_VOID_FUNC(print_string_mock, void**);
DECLARE_FAKE_VOID_FUNC(print_args_mock, void**, void**, void**);

/* ---- Mock function group (for E2E tests) ---- */
extern function_group_t mock_func_group;

/* ---- Real function group (from demo, for integration tests) ---- */
extern function_group_t default_func_group;

/* ---- Reset all fff fakes and dyncall registrations ---- */
void test_helpers_reset(void);

#endif /* TEST_HELPERS_H */
