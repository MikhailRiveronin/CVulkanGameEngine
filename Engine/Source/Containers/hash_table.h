#pragma once

#include "defines.h"

typedef struct Bucket
{
    bool is_empty;
    bool in_probe;
    char* key;
    u32 hash_key;
    void* value;
} Bucket;

/**
 * @brief A non-shrinking hash table. The keys are exclusively strings. Open addressing collision resolution. Double probing.
 */
typedef struct Hash_Table
{
    u32 size;
    u32 data_size;
    u32 used;
    Bucket* buckets;
} Hash_Table;

LIB_API Hash_Table* hash_table_create(u32 size, u32 data_size);
LIB_API void hash_table_destroy(Hash_Table* table);
LIB_API void hash_table_insert(Hash_Table* table, char const* key, void const* value);
LIB_API void hash_table_erase(Hash_Table* table, char const* key);
LIB_API void* hash_table_at(Hash_Table* table, char const* key);

#define HASH_TABLE_CREATE(type, size) hash_table_create((size), sizeof(type))
#define HASH_TABLE_AT_AS(table, key, type) *(type*)hash_table_at((table), (key))
