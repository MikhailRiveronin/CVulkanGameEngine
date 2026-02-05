#include "hash_table.h"

#include "core/logger.h"
#include "systems/memory_system.h"

static u64 hash_name(char const* name, u32 element_count);

b8 hashtable_create(u64 count, u64 element_size, void const* block, void const* default_value, String_Map* table)
{
    if (!table || count == 0 || element_size == 0)
    {
        LOG_FATAL("hashtable_create: Invalid input parameters");
        return FALSE;
    }

    table->count = count;
    table->element_size = element_size;

    if (block)
    {
        table->block = block;
    }
    else
    {
        table->block = memory_system_allocate(count * element_size, MEMORY_TAG_HASHTABLE);
        if (!table->block)
        {
            LOG_FATAL("hashtable_create: Failed to allocate memory for hashtable");
            return FALSE;
        }
    }

    if (default_value)
    {
        for (u32 i = 0; i < table->count; ++i)
        {
            // memory_system_copy((char*)table->block + (i * table->element_size), default_value, table->element_size);
        }
    }

    return TRUE;
}

void hashtable_destroy(String_Map* table)
{
    if (table) {
        // TODO: If using allocator above, free memory here.
        memory_zero(table, sizeof(*table));
    }
}

b8 hashtable_set(String_Map* table, char const* name, void* value)
{
    if (!table || !name || !value) {
        LOG_WARNING("hashtable_set requires table, name and value to exist.");
        return FALSE;
    }
    if (table->is_pointer_type) {
        LOG_ERROR("hashtable_set should not be used with tables that have pointer types. Use hashtable_set_ptr instead.");
        return FALSE;
    }

    u64 hash = hash_name(name, table->element_count);
    memory_copy((u8*)table->memory + (table->element_size * hash), value, table->element_size);
    return TRUE;
}

b8 hashtable_set_ptr(String_Map* table, char const* name, void** value)
{
    if (!table || !name) {
        LOG_WARNING("hashtable_set_ptr requires table and name  to exist.");
        return FALSE;
    }
    if (!table->is_pointer_type) {
        LOG_ERROR("hashtable_set_ptr should not be used with tables that do not have pointer types. Use hashtable_set instead.");
        return FALSE;
    }

    u64 hash = hash_name(name, table->element_count);
    ((void**)table->memory)[hash] = value ? *value : 0;
    return TRUE;
}

b8 hashtable_get(String_Map* table, char const* name, void* value)
{
    if (!table || !name || !value) {
        LOG_WARNING("hashtable_get requires table, name and value to exist.");
        return FALSE;
    }
    if (table->is_pointer_type) {
        LOG_ERROR("hashtable_get should not be used with tables that have pointer types. Use hashtable_set_ptr instead.");
        return FALSE;
    }

    u64 hash = hash_name(name, table->element_count);
    memory_copy(value, (char*)table->memory + (table->element_size * hash), table->element_size);
    return TRUE;
}

b8 hashtable_get_ptr(String_Map* table, char const* name, void** value)
{
    if (!table || !name) {
        LOG_WARNING("hashtable_set_ptr requires table and name  to exist.");
        return FALSE;
    }
    if (!table->is_pointer_type) {
        LOG_ERROR("hashtable_set_ptr should not be used with tables that do not have pointer types. Use hashtable_set instead.");
        return FALSE;
    }

    u64 hash = hash_name(name, table->element_count);
    *value = ((void**)table->memory)[hash];
    return *value;
}

b8 hashtable_fill(String_Map* table, void* value)
{
    if (!table || !value) {
        LOG_WARNING("hashtable_set requires table, name and value to exist.");
        return FALSE;
    }
    if (table->is_pointer_type) {
        LOG_ERROR("hashtable_set should not be used with tables that have pointer types. Use hashtable_set_ptr instead.");
        return FALSE;
    }

    for (u32 i = 0; i < table->element_count; ++i) {
        memory_copy((u8*)table->memory + (table->element_size * i), value, table->element_size);
    }
    return TRUE;
}

u64 hash_name(char const* name, u32 element_count)
{
    // A multipler to use when generating a hash. Prime to hopefully avoid collisions.
    static const u64 multiplier = 97;

    unsigned char const* us;
    u64 hash = 0;

    for (us = (unsigned char const*)name; *us; us++) {
        hash = hash * multiplier + *us;
    }

    // Mod it against the size of the table.
    hash %= element_count;

    return hash;
}
