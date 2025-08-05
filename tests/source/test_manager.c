#include "test_manager.h"

#include <Containers/darray.h>
#include <Core/logger.h>
#include <Core/String.h>
#include <Core/clock.h>

typedef struct test_entry {
    PFN_test func;
    char* desc;
} test_entry;

static DARRAY(test_entry) tests;

void test_manager_init()
{
    DARRAY_INIT(tests, MEMORY_TAG_APPLICATION);
}

void test_manager_register_test(u8 (* PFN_test)(), char* desc)
{
    test_entry e;
    e.func = PFN_test;
    e.desc = desc;
    DARRAY_PUSH(tests, e);
}

void test_manager_run_tests()
{
    u32 passed = 0;
    u32 failed = 0;
    u32 skipped = 0;

    u32 count = tests.size;

    clock total_time;
    clock_start(&total_time);

    for (u32 i = 0; i < count; ++i) {
        clock test_time;
        clock_start(&test_time);
        u8 result = tests.data[i].func();
        clock_update(&test_time);

        if (result == TRUE) {
            ++passed;
        } else if (result == BYPASS) {
            LOG_WARNING("[SKIPPED]: %s", tests.data[i].desc);
            ++skipped;
        } else {
            LOG_ERROR("[FAILED]: %s", tests.data[i].desc);
            ++failed;
        }
        char status[20];
        string_format(status, failed ? "*** %d FAILED ***" : "SUCCESS", failed);
        clock_update(&total_time);
        LOG_INFO("Executed %d of %d (skipped %d) %s (%.6f sec / %.6f sec total",
            i + 1, count, skipped, status, test_time.elapsed, total_time.elapsed);
    }

    clock_stop(&total_time);

    LOG_INFO("Results: %d passed, %d failed, %d skipped.", passed, failed, skipped);

    DARRAY_DESTROY(tests);
}
