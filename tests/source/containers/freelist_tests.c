#include "freelist_tests.h"

#include <containers/freelist.h>
#include <systems/memory_system.h>
#include "expect.h"
#include "test_manager.h"

static u8 freelist_test_create_and_destroy();

void freelist_register_tests()
{
    test_manager_register_test(freelist_test_create_and_destroy, "freelist_test_create_and_destroy");

}

u8 freelist_test_create_and_destroy()
{
    freelist list;
    u64 required_memory;
    u32 size = 1024;
    freelist_create(&required_memory, 0, size, 0);

    void* memory = memory_allocate(required_memory, MEMORY_TAG_APPLICATION);
    freelist_create(&required_memory, memory, size, &list);
    EXPECT_NOT_EQUAL(memory, 0);

    u64 space = freelist_free_space(&list);
    EXPECT_EQUAL(size, space);

    freelist_destroy(&list);
    EXPECT_EQUAL(list.internal, 0);

    memory_free(memory, required_memory, MEMORY_TAG_APPLICATION);
    return TRUE;
}
