#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include "dispatch.h"
#include "fff.h"

/* ---- fff declarations for demo functions ---- */
DECLARE_FAKE_VOID_FUNC(print_hello_mock);
DECLARE_FAKE_VALUE_FUNC(int64_t, print_add_mock, void*, void*);
DECLARE_FAKE_VOID_FUNC(print_dec_mock, void*);
DECLARE_FAKE_VOID_FUNC(print_hex_mock, void*);
DECLARE_FAKE_VOID_FUNC(print_string_mock, void*);
DECLARE_FAKE_VOID_FUNC(print_args_mock, void*, void*, void*);

/* ---- Reset all fff fakes and dispatch registrations ---- */
void test_helpers_reset(void);

#endif /* TEST_HELPERS_H */
