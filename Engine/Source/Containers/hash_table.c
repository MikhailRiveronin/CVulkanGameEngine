#include "hash_table.h"

#include "core/logger.h"
#include "core/memory_utils.h"

static u64 hash_name(char const* name, u32 element_count);

void hash_table_create(u64 element_size, u32 element_count, void* memory, b8 is_pointer_type, hash_table* table)
{
    if (!memory || !table) {
        LOG_ERROR("hashtable_create failed! Pointer to memory and out_hashtable are required.");
        return;
    }
    if (!element_count || !element_size) {
        LOG_ERROR("element_size and element_count must be a positive non-zero value.");
        return;
    }

    // TODO: Might want to require an allocator and allocate this memory instead.
    table->memory = memory;
    table->element_count = element_count;
    table->element_size = element_size;
    table->is_pointer_type = is_pointer_type;
    memory_zero(table->memory, element_size * element_count);
}

void hash_table_destroy(hash_table* table)
{
    if (table) {
        // TODO: If using allocator above, free memory here.
        memory_zero(table, sizeof(*table));
    }
}

b8 hash_table_set(hash_table* table, char const* name, void* value)
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

b8 hash_table_set_ptr(hash_table* table, char const* name, void** value)
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

b8 hash_table_get(hash_table* table, char const* name, void* value)
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

b8 hash_table_get_ptr(hash_table* table, char const* name, void** value)
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

b8 hash_table_fill(hash_table* table, void* value)
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
