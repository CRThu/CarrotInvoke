#include "func.h"


void print_hello()
{
    PRINT_FUNC_SIGNATURE();
    printf("[info]: print_hello() Called.\r\n");
}

void print_dec(dyn_dec64p_t a)
{
    PRINT_FUNC_SIGNATURE();
    printf("[info]: print_dec(%"PRId64") Called.\r\n", PVAL(a));
}

void print_hex(dyn_hex64p_t a)
{
    PRINT_FUNC_SIGNATURE();
    printf("[info]: print_hex(0x%"PRIX64") Called.\r\n", PVAL(a));
}

void print_string(dyn_string_t a)
{
    PRINT_FUNC_SIGNATURE();
    printf("[info]: print_string(%s) Called.\r\n", PREF(a));
}

ret_dec64_t print_add(dyn_dec64p_t a, dyn_dec64p_t b)
{
    PRINT_FUNC_SIGNATURE();
    printf("[info]: print_add(%"PRId64", %"PRId64") Called.\r\n", PVAL(a), PVAL(b));
    return PVAL(a) + PVAL(b);
}

void print_args(dyn_string_t a0,
                dyn_string_t a1,
                dyn_string_t a2)
{
    PRINT_FUNC_SIGNATURE();
    printf("[info]: print_string(%s,%s,%s) Called.\r\n"
           , PREF(a0)
           , PREF(a1)
           , PREF(a2));
}

function_group_t default_func_group =
{
    FUNCTION_GROUP("default_func_group",
        FUNCTION_INFO(print_hello, T_NULL, T_VOID),
        FUNCTION_INFO(print_dec, T_NULL, T_DEC64),
        FUNCTION_INFO(print_hex, T_NULL, T_HEX64),
        FUNCTION_INFO(print_string, T_NULL, T_STRING),
        FUNCTION_INFO(print_add, T_DEC64, T_DEC64, T_DEC64),
        FUNCTION_INFO(print_args, T_NULL, T_STRING, T_STRING, T_STRING),
    )
};
