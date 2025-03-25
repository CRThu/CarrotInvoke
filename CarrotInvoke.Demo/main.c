#include "func.h"
#include "dyncall.h"
#include <stdio.h>
#include <stdlib.h>

void dynpool_internal_test()
{
    printf("--------------- DYNPOOL INTERNAL TEST ---------------\r\n");
    dynpool_t pool;
    int64_t dec = 'X';
    uint8_t str[100];

    dynpool_init(&pool);
    dynpool_set(&pool, T_STRING, "HELLOWORLD", sizeof("HELLOWORLD"));
    printf("[dynpool set]: %s\n", "HELLOWORLD");
    dynpool_set(&pool, T_STRING, "123", sizeof("123"));
    printf("[dynpool set]: %s\n", "123");
    dynpool_set(&pool, T_DEC64, &dec, sizeof(dec));
    printf("[dynpool set]: %lld\n", dec);

    #if(DYNCALL_DEBUG)
    dynpool_print(&pool);
    #endif

    uint16_t used_size;
    dynpool_get(&pool, T_STRING, str, sizeof(str), &used_size);
    printf("[dynpool get]: %s\n", str);
    dynpool_get(&pool, T_DEC64, &dec, sizeof(dec), &used_size);
    printf("[dynpool get]: %lld\n", dec);
    dynpool_get(&pool, T_STRING, str, sizeof(str), &used_size);
    printf("[dynpool get]: %s\n", str);


}

void invoke_test()
{
    printf("--------------- INVOKE TEST ---------------\r\n");
    function_info_t* hello = get_func_by_name(&default_func_group, "print_hello");
    function_info_t* add = get_func_by_name(&default_func_group, "print_add");
    if (hello == NULL)
        printf("print_hello() not found\n");
    else
        printf("print_hello() found\n");
    if (add == NULL)
        printf("print_add() not found\n");
    else
        printf("print_add() found\n");

}

void dyncall_test()
{
    printf("--------------- DYNCALL TEST ---------------\r\n");
    int64_t a = 123;
    int64_t b = 456;
    char* str = "test string";

    invoke(&default_func_group, "print_hello");
    invoke(&default_func_group, "print_add", a, b);
    invoke(&default_func_group, "print_string", str);
    invoke(&default_func_group, "print_dec", (int64_t)123456789);
    invoke(&default_func_group, "print_hex", (uint64_t)0xABCDEF);
}

int main()
{
    dynpool_internal_test();
    invoke_test();
    dyncall_test();
}
