#include "dynpool.h"
#include "dyncall.h"
#include "cmdparse.h"
#include "CppUnitTest.h"
#include "../CarrotInvoke.Demo/func.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CarrotInvokeTest
{
    TEST_CLASS(DynCallTest)
    {
    public:
        TEST_METHOD(TestGetFuncByName)
        {
            function_info_t* func = get_func_by_name(&default_func_group, "print_hello");
            Assert::IsNotNull(func);
            Assert::AreEqual("print_hello", func->name);

            func = get_func_by_name(&default_func_group, "print_add");
            Assert::IsNotNull(func);
            Assert::AreEqual("print_add", func->name);

            func = get_func_by_name(&default_func_group, "nonexistent_func");
            Assert::IsNull(func);
        }

        TEST_METHOD(TestInvokeBasic)
        {
            // Test void function with no args
            invoke(&default_func_group, "print_hello");

            // Test function with DEC64 arg
            invoke(&default_func_group, "print_dec", (int64_t)12345);

            // Test function with HEX64 arg
            invoke(&default_func_group, "print_hex", (uint64_t)0xABCDEF);

            // Test function with STRING arg
            invoke(&default_func_group, "print_string", "test string");
        }

        TEST_METHOD(TestInvokeWithReturn)
        {
            // Test function with return value
            // Note: The return value isn't captured in the current implementation
            invoke(&default_func_group, "print_add", (int64_t)100, (int64_t)200);
        }

        TEST_METHOD(TestInvokeByPool)
        {
            dynpool_t pool;
            dynpool_init(&pool);

            // Set up arguments for print_args
            dynpool_set(&pool, T_STRING, "arg1", strlen("arg1") + 1);
            dynpool_set(&pool, T_STRING, "arg2", strlen("arg2") + 1);
            dynpool_set(&pool, T_STRING, "arg3", strlen("arg3") + 1);

            function_info_t* func = get_func_by_name(&default_func_group, "print_args");
            Assert::IsNotNull(func);

            invoke_by_pool(&pool, func);
        }
    };
}