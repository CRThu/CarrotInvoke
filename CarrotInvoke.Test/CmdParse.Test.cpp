#include "dynpool.h"
#include "dyncall.h"
#include "cmdparse.h"
#include "CppUnitTest.h"
#include "../CarrotInvoke.Demo/func.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CarrotInvokeTest
{
    TEST_CLASS(CmdParseTest)
    {
    public:
        TEST_METHOD(TestParseSimpleCommand)
        {
            dynpool_t pool;
            uint16_t len;

            const char* cmd = "print_hello()";
            cmd_parse_status_t status = cmdparse_from_string(&pool, cmd, &len);

            Assert::AreEqual((int)CMDPARSE_OK, (int)status);
            Assert::AreEqual((uint16_t)13, len);
            Assert::AreEqual((uint16_t)1u, pool.wr_count); // Only function name

            // Verify function name
            char funcName[50];
            uint16_t usedSize;
            dynpool_status_t dstatus = dynpool_get(&pool, T_STRING, funcName, sizeof(funcName), &usedSize);
            Assert::AreEqual((int)DYNPOOL_NO_ERROR, (int)dstatus);
            Assert::AreEqual("print_hello", funcName);
        }

        TEST_METHOD(TestParseCommandWithArgs)
        {
            dynpool_t pool;
            uint16_t len;

            const char* cmd = "print_args(arg1,arg2,arg3)";
            cmd_parse_status_t status = cmdparse_from_string(&pool, cmd, &len);
            
            Assert::AreEqual((int)CMDPARSE_OK, (int)status);
            Assert::AreEqual((uint16_t)4u, pool.wr_count); // Function name + 3 args

            // Verify function name and arguments
            char buf[50];
            uint16_t usedSize;

            dynpool_get(&pool, T_STRING, buf, sizeof(buf), &usedSize);
            Assert::AreEqual("print_args", buf);

            dynpool_get(&pool, T_STRING, buf, sizeof(buf), &usedSize);
            Assert::AreEqual("arg1", buf);

            dynpool_get(&pool, T_STRING, buf, sizeof(buf), &usedSize);
            Assert::AreEqual("arg2", buf);

            dynpool_get(&pool, T_STRING, buf, sizeof(buf), &usedSize);
            Assert::AreEqual("arg3", buf);
        }

        TEST_METHOD(TestParseInvalidCommand)
        {
            dynpool_t pool;
            uint16_t len;

            // Missing closing parenthesis
            const char* cmd1 = "print_hello(";
            cmd_parse_status_t status = cmdparse_from_string(&pool, cmd1, &len);
            Assert::AreEqual((int)CMDPARSE_INVALID_FORMAT, (int)status);

            // Missing opening parenthesis
            const char* cmd2 = "print_hello)";
            status = cmdparse_from_string(&pool, cmd2, &len);
            Assert::AreEqual((int)CMDPARSE_INVALID_FORMAT, (int)status);

            // Empty string
            const char* cmd3 = "";
            status = cmdparse_from_string(&pool, cmd3, &len);
            Assert::AreEqual((int)CMDPARSE_EMPTY_STRING, (int)status);
        }

        TEST_METHOD(TestInvokeByCmd)
        {
            dynpool_t pool;
            uint16_t len;

            const char* cmd = "print_args(arg1,arg2,arg3)";
            cmd_parse_status_t status = cmdparse_from_string(&pool, cmd, &len);
            Assert::AreEqual((int)CMDPARSE_OK, (int)status);

            invoke_by_cmd(&default_func_group, &pool);
        }
    };
}