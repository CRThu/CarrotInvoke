cmake_minimum_required(VERSION 3.14)
project(CarrotInvoke C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ---- Static library ----
add_library(carrot
    src/dynpool.c
    src/cmdparse.c
    src/cmdscan.c
    src/dyncall.c
    src/typeconv.c
    src/dispatch.c
    src/invoke.c
    src/cmdqueue.c
    src/ringbuf.c
)
target_include_directories(carrot PUBLIC inc)

# ---- Demo executable ----
add_executable(carrot_demo
    examples/main.c
    examples/func.c
)
target_link_libraries(carrot_demo PRIVATE carrot)
target_include_directories(carrot_demo PRIVATE examples)

# ---- CMD SCAN Demo executable ----
add_executable(cmdscan_demo
    examples/cmdscan_demo.c
)
target_link_libraries(cmdscan_demo PRIVATE carrot)
target_include_directories(cmdscan_demo PRIVATE examples)

# ---- CMD QUEUE Demo executable ----
add_executable(cmdqueue_demo
    examples/cmdqueue_demo.c
)
target_link_libraries(cmdqueue_demo PRIVATE carrot)
target_include_directories(cmdqueue_demo PRIVATE examples)

# ---- Test executable ----
add_executable(carrot_tests
    vendor/unity/unity.c
    examples/func.c
    tests/test_helpers.c
    tests/test_dynpool.c
    tests/test_cmdparse.c
    tests/test_cmdscan.c
    # tests/test_dyncall.c  # old dyncall tests, disabled
    tests/test_cmdqueue.c
    tests/test_ringbuf.c
    # tests/test_e2e.c      # old dyncall e2e tests, disabled
    tests/test_typeconv.c
    tests/test_dispatch.c
    tests/test_invoke_helpers.c
    tests/test_invoke.c
    tests/main_test.c
)
target_link_libraries(carrot_tests PRIVATE carrot)
target_include_directories(carrot_tests PRIVATE
    examples
    tests
    vendor/unity
    vendor/fff
)
target_compile_definitions(carrot_tests PRIVATE UNITY_INCLUDE_DOUBLE)
