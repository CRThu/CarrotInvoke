#include "dynpool.h"
#include "dyncall.h"
#include "cmdparse.h"
#include "CppUnitTest.h"
#include "../CarrotInvoke.Demo/func.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CarrotInvokeTest
{
    TEST_CLASS(DynPoolTest)
    {
    public:
        TEST_METHOD(TestInit)
        {
            dynpool_t pool;
            dynpool_status_t status = dynpool_init(&pool);

            Assert::AreEqual((int)DYNPOOL_NO_ERROR, (int)status);
            Assert::AreEqual((uint16_t)0u, pool.wr_count);
            Assert::AreEqual((uint16_t)0u, pool.rd_count);
            Assert::AreEqual((uint16_t)0u, pool.cursor);
        }

        TEST_METHOD(TestSetGetBasicTypes)
        {
            dynpool_t pool;
            dynpool_init(&pool);

            // Test DEC64
            int64_t decVal = -123456789;
            dynpool_status_t status = dynpool_set(&pool, T_DEC64, &decVal, 0);
            Assert::AreEqual((int)DYNPOOL_NO_ERROR, (int)status);

            // Test HEX64
            uint64_t hexVal = 0xABCDEF;
            status = dynpool_set(&pool, T_HEX64, &hexVal, 0);
            Assert::AreEqual((int)DYNPOOL_NO_ERROR, (int)status);

            // Test STRING
            const char* strVal = "test string";
            status = dynpool_set(&pool, T_STRING, (void*)strVal, strlen(strVal) + 1);
            Assert::AreEqual((int)DYNPOOL_NO_ERROR, (int)status);

            // Verify values
            int64_t outDec;
            uint16_t usedSize;
            status = dynpool_get(&pool, T_DEC64, &outDec, sizeof(outDec), &usedSize);
            Assert::AreEqual((int)DYNPOOL_NO_ERROR, (int)status);
            Assert::AreEqual(decVal, outDec);

            uint64_t outHex;
            status = dynpool_get(&pool, T_HEX64, &outHex, sizeof(outHex), &usedSize);
            Assert::AreEqual((int)DYNPOOL_NO_ERROR, (int)status);
            Assert::AreEqual(hexVal, outHex);

            char outStr[50];
            status = dynpool_get(&pool, T_STRING, outStr, sizeof(outStr), &usedSize);
            Assert::AreEqual((int)DYNPOOL_NO_ERROR, (int)status);
            Assert::AreEqual(strVal, outStr);
        }

        TEST_METHOD(TestPeek)
        {
            dynpool_t pool;
            dynpool_init(&pool);

            int64_t val1 = 123;
            int64_t val2 = 456;
            dynpool_set(&pool, T_DEC64, &val1, 0);
            dynpool_set(&pool, T_DEC64, &val2, 0);

            dtypes_t type;
            void* data;
            uint16_t size;

            dynpool_status_t status = dynpool_peek(&pool, 0, &type, &data, &size);
            Assert::AreEqual((int)DYNPOOL_NO_ERROR, (int)status);
            Assert::AreEqual((int)T_DEC64, (int)type);
            Assert::AreEqual(val1, *(int64_t*)data);

            status = dynpool_peek(&pool, 1, &type, &data, &size);
            Assert::AreEqual((int)DYNPOOL_NO_ERROR, (int)status);
            Assert::AreEqual((int)T_DEC64, (int)type);
            Assert::AreEqual(val2, *(int64_t*)data);
        }

        TEST_METHOD(TestTypeConversion)
        {
            // Test DEC64 to STRING
            int64_t decVal = -123456789;
            char strBuf[50];
            uint16_t usedSize;

            dynpool_status_t status = dtype_conversion(&decVal, strBuf, T_DEC64, T_STRING, sizeof(decVal), sizeof(strBuf), &usedSize);
            Assert::AreEqual((int)DYNPOOL_NO_ERROR, (int)status);
            Assert::AreEqual("-123456789", strBuf);

            // Test HEX64 to STRING
            uint64_t hexVal = 0xABCDEF;
            status = dtype_conversion(&hexVal, strBuf, T_HEX64, T_STRING, sizeof(hexVal), sizeof(strBuf), &usedSize);
            Assert::AreEqual((int)DYNPOOL_NO_ERROR, (int)status);
            Assert::AreEqual("ABCDEF", strBuf);

            // Test STRING to DEC64
            const char* testStr = "987654321";
            int64_t outDec;
            status = dtype_conversion(testStr, &outDec, T_STRING, T_DEC64, strlen(testStr) + 1, sizeof(outDec), &usedSize);
            Assert::AreEqual((int)DYNPOOL_NO_ERROR, (int)status);
            Assert::AreEqual(987654321LL, outDec);
        }

        TEST_METHOD(TestPoolFull)
        {
            dynpool_t pool;
            dynpool_init(&pool);

            // Fill the pool
            for (int i = 0; i < DYNPOOL_MAX_VARS; i++)
            {
                int64_t val = i;
                dynpool_status_t status = dynpool_set(&pool, T_DEC64, &val, 0);
                Assert::AreEqual((int)DYNPOOL_NO_ERROR, (int)status);
            }

            // Try to add one more - should fail
            int64_t val = 999;
            dynpool_status_t status = dynpool_set(&pool, T_DEC64, &val, 0);
            Assert::AreEqual((int)DYNPOOL_ERR_FULL_POOL, (int)status);
        }
    };
}