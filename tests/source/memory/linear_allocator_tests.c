#include "linear_allocator_tests.h"
#include "../test_manager.h"
#include "../expect.h"
#include "memory/linear_allocator.h"

u8 linear_allocator_should_create_and_destroy()
{
    linear_allocator alloc;
    linear_allocator_create(sizeof(u64), 0, &alloc);

    EXPECT_NOT_EQUAL(0, alloc.memory);
    EXPECT_EQUAL(sizeof(u64), alloc.tracked_memory);
    EXPECT_EQUAL(0, alloc.allocated_memory);

    linear_allocator_destroy(&alloc);

    EXPECT_EQUAL(0, alloc.memory);
    EXPECT_EQUAL(0, alloc.tracked_memory);
    EXPECT_EQUAL(0, alloc.allocated_memory);

    return TRUE;
}

u8 linear_allocator_single_allocation_all_space() {
    linear_allocator alloc;
    linear_allocator_create(sizeof(u64), 0, &alloc);

    // Single allocation.
    void* block = linear_allocator_allocate(&alloc, sizeof(u64));

    // Validate it
    EXPECT_NOT_EQUAL(0, block);
    EXPECT_EQUAL(sizeof(u64), alloc.allocated_memory);

    linear_allocator_destroy(&alloc);

    return TRUE;
}

u8 linear_allocator_multi_allocation_all_space() {
    u64 max_allocs = 1024;
    linear_allocator alloc;
    linear_allocator_create(sizeof(u64) * max_allocs, 0, &alloc);

    // Multiple allocations - full.
    void* block;
    for (u64 i = 0; i < max_allocs; ++i) {
        block = linear_allocator_allocate(&alloc, sizeof(u64));
        // Validate it
        EXPECT_NOT_EQUAL(0, block);
        EXPECT_EQUAL(sizeof(u64) * (i + 1), alloc.allocated_memory);
    }

    linear_allocator_destroy(&alloc);

    return TRUE;
}

u8 linear_allocator_multi_allocation_over_allocate() {
    u64 max_allocs = 3;
    linear_allocator alloc;
    linear_allocator_create(sizeof(u64) * max_allocs, 0, &alloc);

    // Multiple allocations - full.
    void* block;
    for (u64 i = 0; i < max_allocs; ++i) {
        block = linear_allocator_allocate(&alloc, sizeof(u64));
        // Validate it
        EXPECT_NOT_EQUAL(0, block);
        EXPECT_EQUAL(sizeof(u64) * (i + 1), alloc.allocated_memory);
    }

    LOG_DEBUG("Note: The following error is intentionally caused by this test.");

    // Ask for one more allocation. Should error and return 0.
    block = linear_allocator_allocate(&alloc, sizeof(u64));
    // Validate it - allocated should be unchanged.
    EXPECT_EQUAL(0, block);
    EXPECT_EQUAL(sizeof(u64) * (max_allocs), alloc.allocated_memory);

    linear_allocator_destroy(&alloc);

    return TRUE;
}

u8 linear_allocator_multi_allocation_all_space_then_free() {
    u64 max_allocs = 1024;
    linear_allocator alloc;
    linear_allocator_create(sizeof(u64) * max_allocs, 0, &alloc);

    // Multiple allocations - full.
    void* block;
    for (u64 i = 0; i < max_allocs; ++i) {
        block = linear_allocator_allocate(&alloc, sizeof(u64));
        // Validate it
        EXPECT_NOT_EQUAL(0, block);
        EXPECT_EQUAL(sizeof(u64) * (i + 1), alloc.allocated_memory);
    }

    // Validate that pointer is reset.
    linear_allocator_free_all(&alloc);
    EXPECT_EQUAL(0, alloc.allocated_memory);

    linear_allocator_destroy(&alloc);

    return TRUE;
}

void linear_allocator_register_tests()
{
    test_manager_register_test(linear_allocator_should_create_and_destroy, "Linear allocator should create and destroy");
    test_manager_register_test(linear_allocator_single_allocation_all_space, "Linear allocator single alloc for all space");
    test_manager_register_test(linear_allocator_multi_allocation_all_space, "Linear allocator multi alloc for all space");
    test_manager_register_test(linear_allocator_multi_allocation_over_allocate, "Linear allocator try over allocate");
    test_manager_register_test(linear_allocator_multi_allocation_all_space_then_free, "Linear allocator allocated should be 0 after free_all");
}
