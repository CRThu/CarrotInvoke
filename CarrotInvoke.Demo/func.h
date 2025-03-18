#pragma once
#ifndef _FUNC_H_
#define _FUNC_H_

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS

#ifdef __cplusplus
extern "C"
{
#endif

    #include "../inc/dynamic_call.h"

    extern const function_group_t default_func_group;


    void print_hello();
    void print_dec(dyn_dec64p_t a);
    void print_hex(dyn_hex64p_t a);
    void print_string(dyn_string_t a);
    ret_dec64_t print_add(dyn_dec64p_t a, dyn_dec64p_t b);


#ifdef __cplusplus
}
#endif
#endif  // _FUNC_H_