#include "test_helpers.h"

/* ---- fff global state ---- */
DEFINE_FFF_GLOBALS;

/* ---- fff definitions ---- */
DEFINE_FAKE_VOID_FUNC(print_hello_mock);
DEFINE_FAKE_VALUE_FUNC(int64_t, print_add_mock, void*, void*);
DEFINE_FAKE_VOID_FUNC(print_dec_mock, void*);
DEFINE_FAKE_VOID_FUNC(print_hex_mock, void*);
DEFINE_FAKE_VOID_FUNC(print_string_mock, void*);
DEFINE_FAKE_VOID_FUNC(print_args_mock, void*, void*, void*);

/* ---- 注册 mock 函数 (新 dispatch API) ---- */
static void _register_mock_funcs(void)
{
    dispatch_reg(print_hello_mock,  "print_hello()");
    dispatch_reg(print_dec_mock,    "print_dec(i)");
    dispatch_reg(print_hex_mock,    "print_hex(u)");
    dispatch_reg(print_string_mock, "print_string(s)");
    dispatch_reg(print_add_mock,    "print_add(i, i) -> i");
    dispatch_reg(print_args_mock,   "print_args(s, s, s)");
}

/* ---- Reset all fff fakes and dispatch registrations ---- */
void test_helpers_reset(void)
{
    dispatch_init();
    _register_mock_funcs();
    RESET_FAKE(print_hello_mock);
    RESET_FAKE(print_add_mock);
    RESET_FAKE(print_dec_mock);
    RESET_FAKE(print_hex_mock);
    RESET_FAKE(print_string_mock);
    RESET_FAKE(print_args_mock);
}
