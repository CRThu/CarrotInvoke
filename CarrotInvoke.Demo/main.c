#include "func.h"
//#include "../inc/dynamic_pool.h"
#include "../inc/dynamic_call.h"
//#include "../inc/cmd_parse.h"
#include <stdio.h>

/*
void parse_test(dynamic_pool_t* pool)
{
    dynamic_pool_init(pool);

    uint8_t types[] = { T_STRING, T_DEC64, T_HEX64 };
    uint8_t types_len = sizeof(types);

    printf("TYPE:\nT_STRING=%02X\nT_DEC64=%02X\nT_HEX64=%02X\n", types[0], types[1], types[2]);

    char c[256] = "";
    char* p = c;
    strcpy(p, "func(32,10)\n");

    cmd_parse_one(pool, p, strlen(p));

    printf("CMD: %s\n", p);

    dynamic_pool_print(pool);

    printf("\n");
}

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
    function_info_t* hello = get_func_by_name(&default_func_group, "print_hello");
    function_info_t* add = get_func_by_name(&default_func_group, "print_add");
    if (hello == NULL) printf("print_hello() not found");
    if (add == NULL) printf("print_add() not found");

    invoke_by_cmd(&default_func_group, "print_hello");

    dynpool_t pool;
    int64_t dec;
    uint8_t str[100];

    dynpool_init(&pool);
    dynpool_set(&pool, T_STRING, "HELLOWORLD", sizeof("HELLOWORLD"));
    printf("[DYNPOOL SET]: %s\n", "HELLOWORLD");
    dynpool_set(&pool, T_STRING, "123", sizeof("123"));
    printf("[DYNPOOL SET]: %s\n", "123");
    dynpool_set(&pool, T_DEC64, &dec, sizeof(dec));
    printf("[DYNPOOL SET]: %lld\n", dec);

    dynpool_get(&pool, 0, T_STRING, str, sizeof(str));
    printf("[DYNPOOL GET]: %s\n", str);
    dynpool_get(&pool, 1, T_DEC64, &dec, sizeof(dec));
    printf("[DYNPOOL GET]: %lld\n", dec);
    dynpool_get(&pool, 2, T_STRING, str, sizeof(str));
    printf("[DYNPOOL GET]: %s\n", str);


    /*
    parse_test(&pool);
    invoke_test(&pool);
    dyncall_test(&pool);
    */
}
