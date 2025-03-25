#include "func.h"
//#include "../inc/dynamic_pool.h"
#include "../inc/dynamic_call.h"
//#include "../inc/cmd_parse.h"
#include <stdio.h>

void dynpool_internal_test()
{
    printf("----- DYNPOOL INTERNAL TEST -----\r\n");
    dynpool_t pool;
    int64_t dec = 'X';
    uint8_t str[100];

    dynpool_init(&pool);
    dynpool_set(&pool, T_STRING, "HELLOWORLD", sizeof("HELLOWORLD"));
    printf("[DYNPOOL SET]: %s\n", "HELLOWORLD");
    dynpool_set(&pool, T_STRING, "123", sizeof("123"));
    printf("[DYNPOOL SET]: %s\n", "123");
    dynpool_set(&pool, T_DEC64, &dec, sizeof(dec));
    printf("[DYNPOOL SET]: %lld\n", dec);

    dynpool_print(&pool);

    uint16_t used_size;
    dynpool_get(&pool, T_STRING, str, sizeof(str), &used_size);
    printf("[DYNPOOL GET]: %s\n", str);
    dynpool_get(&pool, T_DEC64, &dec, sizeof(dec), &used_size);
    printf("[DYNPOOL GET]: %lld\n", dec);
    dynpool_get(&pool, T_STRING, str, sizeof(str), &used_size);
    printf("[DYNPOOL GET]: %s\n", str);


}

/*
void invoke_test(dynamic_pool_t* pool)
{
    invoke(pool, &(delegates[0]));
    invoke(pool, &(delegates[1]));
    invoke(pool, &(delegates[2]));
    invoke(pool, &(delegates[3]));
}

void dyncall_test(dynamic_pool_t* pool)
{
    char s[256];

    dynamic_pool_init(pool);
    printf("\nWrite a command to execute:");
    scanf("%s", &s);
    cmd_parse_one(pool, s, 256);

    char funcname[256];
    dynamic_pool_get(pool, 0, T_STRING, funcname, 256);
    delegate_t* sel = find_delegate_by_name(delegates, delegates_count, funcname);

    printf("found function: %s\n", sel->name);

    invoke(pool, sel);
}

*/


int main()
{
    dynpool_internal_test();

    function_info_t* hello = get_func_by_name(&default_func_group, "print_hello");
    function_info_t* add = get_func_by_name(&default_func_group, "print_add");
    if (hello == NULL) printf("print_hello() not found");
    if (add == NULL) printf("print_add() not found");
    int64_t a = 123;
    int64_t b = 456;

    invoke_by_cmd(&default_func_group, "print_hello");
    invoke_by_cmd(&default_func_group, "print_add", &a, &b);

    /*
    invoke_test(&pool);
    dyncall_test(&pool);
    */
}
