#include "test_helpers.h"

/* ---- fff global state ---- */
DEFINE_FFF_GLOBALS;

/* ---- fff definitions ---- */
DEFINE_FAKE_VOID_FUNC(print_hello_mock);
DEFINE_FAKE_VALUE_FUNC(int64_t, print_add_mock, void**, void**);
DEFINE_FAKE_VOID_FUNC(print_dec_mock, void**);
DEFINE_FAKE_VOID_FUNC(print_hex_mock, void**);
DEFINE_FAKE_VOID_FUNC(print_string_mock, void**);
DEFINE_FAKE_VOID_FUNC(print_args_mock, void**, void**, void**);

/* ---- Mock function group using fff stubs ---- */
/* Use FUNCTION_INFO_NAME to register mock functions under the original names
   so E2E tests can call "print_hello()" etc. */
function_group_t mock_func_group =
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

/* ---- Default function group (real functions from demo) ---- */

void test_helpers_reset(void)
{
    dyncall_reset();
    RESET_FAKE(print_hello_mock);
    RESET_FAKE(print_add_mock);
    RESET_FAKE(print_dec_mock);
    RESET_FAKE(print_hex_mock);
    RESET_FAKE(print_string_mock);
    RESET_FAKE(print_args_mock);
}
