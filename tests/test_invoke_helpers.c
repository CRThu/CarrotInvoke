/**
 * test_invoke_helpers.c — Mock functions for invoke module tests
 *
 * Uses void* parameters (not void**) to match the new invoke calling convention
 * where p[i] points directly to data.
 *
 * Custom fakes copy pointed-to data into static capture variables so tests can
 * safely read them after invoke_call returns (avoids dangling stack pointers).
 *
 * NOTE: No DEFINE_FFF_GLOBALS here — it's defined in test_helpers.c.
 */
#include "test_invoke_helpers.h"
#include "dispatch.h"
#include <string.h>

/* ---- Capture variables ---- */
invoke_captured_t invoke_captured;

/* ---- fff definitions (void* params) ---- */
DEFINE_FAKE_VOID_FUNC(invoke_mock_hello);
DEFINE_FAKE_VALUE_FUNC(int64_t, invoke_mock_add, void*, void*);
DEFINE_FAKE_VOID_FUNC(invoke_mock_dec, void*);
DEFINE_FAKE_VOID_FUNC(invoke_mock_hex, void*);
DEFINE_FAKE_VOID_FUNC(invoke_mock_string, void*);
DEFINE_FAKE_VOID_FUNC(invoke_mock_args, void*, void*, void*);

/* ---- Custom fakes that copy data into capture struct ---- */
static void capture_dec(void* a)
{
    if (a) invoke_captured.dec_val = *(int64_t*)a;
}

static void capture_hex(void* a)
{
    if (a) invoke_captured.hex_val = *(uint64_t*)a;
}

static int64_t capture_add(void* a, void* b)
{
    if (a) invoke_captured.add_a = *(int64_t*)a;
    if (b) invoke_captured.add_b = *(int64_t*)b;
    invoke_captured.add_ret = 0;
    return 0;
}

static void capture_string(void* a)
{
    if (a) {
        const char* s = (const char*)a;
        uint16_t i = 0;
        while (s[i] && i < sizeof(invoke_captured.str_val) - 1) {
            invoke_captured.str_val[i] = s[i];
            i++;
        }
        invoke_captured.str_val[i] = '\0';
    }
}

static void capture_args(void* a0, void* a1, void* a2)
{
    if (a0) {
        const char* s = (const char*)a0;
        uint16_t i = 0;
        while (s[i] && i < sizeof(invoke_captured.arg0) - 1) {
            invoke_captured.arg0[i] = s[i]; i++;
        }
        invoke_captured.arg0[i] = '\0';
    }
    if (a1) {
        const char* s = (const char*)a1;
        uint16_t i = 0;
        while (s[i] && i < sizeof(invoke_captured.arg1) - 1) {
            invoke_captured.arg1[i] = s[i]; i++;
        }
        invoke_captured.arg1[i] = '\0';
    }
    if (a2) {
        const char* s = (const char*)a2;
        uint16_t i = 0;
        while (s[i] && i < sizeof(invoke_captured.arg2) - 1) {
            invoke_captured.arg2[i] = s[i]; i++;
        }
        invoke_captured.arg2[i] = '\0';
    }
}

/* ---- Mock function group ---- */
function_group_t invoke_mock_group =
{
    FUNCTION_GROUP("invoke_mock_group",
        FUNCTION_INFO_NAME("hello",  invoke_mock_hello,  T_NULL, T_VOID),
        FUNCTION_INFO_NAME("dec",    invoke_mock_dec,    T_NULL, T_DEC64),
        FUNCTION_INFO_NAME("hex",    invoke_mock_hex,    T_NULL, T_HEX64),
        FUNCTION_INFO_NAME("str",    invoke_mock_string, T_NULL, T_STRING),
        FUNCTION_INFO_NAME("add",    invoke_mock_add,    T_DEC64, T_DEC64, T_DEC64),
        FUNCTION_INFO_NAME("args",   invoke_mock_args,   T_NULL, T_STRING, T_STRING, T_STRING),
    )
};

void invoke_test_helpers_reset(void)
{
    dispatch_init();
    memset(&invoke_captured, 0, sizeof(invoke_captured));
    RESET_FAKE(invoke_mock_hello);
    RESET_FAKE(invoke_mock_add);
    RESET_FAKE(invoke_mock_dec);
    RESET_FAKE(invoke_mock_hex);
    RESET_FAKE(invoke_mock_string);
    RESET_FAKE(invoke_mock_args);

    /* Set custom fakes to capture data into static variables */
    invoke_mock_dec_fake.custom_fake = capture_dec;
    invoke_mock_hex_fake.custom_fake = capture_hex;
    invoke_mock_add_fake.custom_fake = capture_add;
    invoke_mock_string_fake.custom_fake = capture_string;
    invoke_mock_args_fake.custom_fake = capture_args;
}
