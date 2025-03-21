#include "pch.h"
#include "CppUnitTest.h"
#include "../inc/dynamic_pool.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CarrotInvokeTest
{
    /*
    TEST_CLASS(dynamic_type_array_test)
    {
    public:

        TEST_METHOD(TEST_NUM)
        {
            dynamic_pool_t dyn;
            dynamic_pool_init(&dyn);

            uint32_t num1 = 0x66778899;
            int32_t num2 = 0xAABBCCDD;
            double num3 = 1.23456789;

            uint32_t* actual_num1 = NULL;
            uint32_t* actual_num2 = NULL;
            double* actual_num3 = NULL;

            dynamic_pool_add(&dyn, &num1, sizeof(num1), UINT32TYPE);
            dynamic_pool_add(&dyn, &num2, sizeof(num2), INT32TYPE);
            dynamic_pool_add(&dyn, &num3, sizeof(num3), FLOAT64TYPE);

            dynamic_pool_get(&dyn, 0, (void**)&actual_num1);
            dynamic_pool_get(&dyn, 1, (void**)&actual_num2);
            dynamic_pool_get(&dyn, 2, (void**)&actual_num3);

            Assert::IsTrue(num1 == *actual_num1);
            Assert::IsTrue(num2 == *actual_num2);
            Assert::IsTrue(num3 == *actual_num3);
        }


        TEST_METHOD(TEST_STR)
        {
            dynamic_pool_t dyn;
            dynamic_pool_init(&dyn);

            char* str1 = "ABC";
            char* str2 = "12345678";
            char* str3 = "";

            char* actual_str1 = (char*)malloc(256);
            char* actual_str2 = (char*)malloc(256);
            char* actual_str3 = (char*)malloc(256);

            dynamic_pool_add_value(&dyn, str1, sizeof(str1), STRINGTYPE);
            dynamic_pool_add_value(&dyn, str2, sizeof(str2), STRINGTYPE);
            dynamic_pool_add_value(&dyn, str3, sizeof(str3), STRINGTYPE);

            dynamic_pool_get(&dyn, 0, (void**)&actual_str1);
            dynamic_pool_get(&dyn, 1, (void**)&actual_str2);
            dynamic_pool_get(&dyn, 2, (void**)&actual_str3);

            Assert::IsTrue(strcmp(str1, actual_str1) == 0);
            Assert::IsTrue(strcmp(str2, actual_str2) == 0);
            Assert::IsTrue(strcmp(str3, actual_str3) == 0);
        }
    };
    */
}
