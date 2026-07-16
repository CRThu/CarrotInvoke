/**
 * test_invoke_helpers.h — Mock functions for invoke module tests
 *
 * These mocks use void* parameters (not void**) because the new invoke module
 * passes p[i] directly to handlers, not &p[i].
 */
#ifndef TEST_INVOKE_HELPERS_H
#define TEST_INVOKE_HELPERS_H

#include "dispatch.h"
#include "fff.h"

/* ---- fff declarations (void* params, matching invoke calling convention) ---- */
DECLARE_FAKE_VOID_FUNC(invoke_mock_hello);
DECLARE_FAKE_VALUE_FUNC(int64_t, invoke_mock_add, void*, void*);
DECLARE_FAKE_VOID_FUNC(invoke_mock_dec, void*);
DECLARE_FAKE_VOID_FUNC(invoke_mock_hex, void*);
DECLARE_FAKE_VOID_FUNC(invoke_mock_string, void*);
DECLARE_FAKE_VOID_FUNC(invoke_mock_args, void*, void*, void*);

/* ---- Static capture variables (copy values during mock call, safe after invoke_call returns) ---- */
typedef struct {
    int64_t  dec_val;
    uint64_t hex_val;
    int64_t  add_a;
    int64_t  add_b;
    int64_t  add_ret;
    char     str_val[64];
    char     arg0[64];
    char     arg1[64];
    char     arg2[64];
} invoke_captured_t;

extern invoke_captured_t invoke_captured;

/* ---- Reset all invoke mocks + capture ---- */
void invoke_test_helpers_reset(void);

#endif /* TEST_INVOKE_HELPERS_H */
