#include "func.h"


const function_group_t default_func_group =
{
    FUNCTION_GROUP("default_func_group",
        FUNCTION_INFO(print_hello, T_NULL, T_VOID),
        FUNCTION_INFO(print_dec, T_NULL, T_DEC64),
        FUNCTION_INFO(print_hex, T_NULL, T_HEX64),
        FUNCTION_INFO(print_string, T_NULL, T_STRING),
        FUNCTION_INFO(print_add, T_DEC64, T_DEC64, T_DEC64)
    )
};

char dynamic_call_buf[256];

void print_hello()
{
    PRINT_FUNC_SIGNATURE();
    sprintf(dynamic_call_buf, "print_hello() Called.");
    puts(dynamic_call_buf);
}

void print_dec(dyn_dec64p_t a)
{
    PRINT_FUNC_SIGNATURE();
    sprintf(dynamic_call_buf, "print_dec(%"PRId64") Called.", PVAL(a));
    puts(dynamic_call_buf);
}

void print_hex(dyn_hex64p_t a)
{
    PRINT_FUNC_SIGNATURE();
    sprintf(dynamic_call_buf, "print_hex(0x%"PRIX64") Called.", PVAL(a));
    puts(dynamic_call_buf);
}

void print_string(dyn_string_t a)
{
    PRINT_FUNC_SIGNATURE();
    sprintf(dynamic_call_buf, "print_string(%s) Called.", a);
    puts(dynamic_call_buf);
}

ret_dec64_t print_add(dyn_dec64p_t a, dyn_dec64p_t b)
{
    PRINT_FUNC_SIGNATURE();
    return PVAL(a) + PVAL(b);
}