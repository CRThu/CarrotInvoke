/**
 * main_test.c — Unity test runner entry point
 *
 * Global setUp/tearDown + runs all test suites.
 */
#include "unity.h"
#include "test_helpers.h"

/* Global setUp: reset all mocks + register mock function group */
void setUp(void)
{
    test_helpers_reset();
    register_rpc_group(&mock_func_group);
}

void tearDown(void) {}

/* External test suite entry points */
extern int run_dynpool_tests(void);
extern int run_cmdparse_tests(void);
extern int run_cmdscan_tests(void);
extern int run_dyncall_tests(void);
extern int run_cmdqueue_tests(void);
extern int run_e2e_tests(void);

int main(void)
{
    int failures = 0;

    printf("\n========== DynPool Tests ==========\n");
    failures += run_dynpool_tests();

    printf("\n========== CmdParse Tests ==========\n");
    failures += run_cmdparse_tests();

    printf("\n========== CmdScan Tests ==========\n");
    failures += run_cmdscan_tests();

    printf("\n========== DynCall Tests ==========\n");
    failures += run_dyncall_tests();

    printf("\n========== CmdQueue Tests ==========\n");
    failures += run_cmdqueue_tests();

    printf("\n========== E2E Tests ==========\n");
    failures += run_e2e_tests();

    printf("\n========== SUMMARY ==========\n");
    if (failures == 0)
        printf("All test suites PASSED.\n");
    else
        printf("Some test suites FAILED.\n");

    return failures;
}
