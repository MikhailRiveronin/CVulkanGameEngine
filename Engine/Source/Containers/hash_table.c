#include "hash_table.h"

#include "core/string_utils.h"
#include "systems/memory_system.h"

/**
 * @brief DJB2 algorithm implementation.
 */
static u32 hash(char const* key);
static u32 round_up_to_next_pow2(u32 value);
static u32 probe(u32 hash_key, u32 i, u32 map_size);

/**
 * @brief Find the bucket containing key, if the key is in the table, or the first one past the end of its probe.
 */
static Bucket* find_by_key(Hash_Table* table, char const* key);
static Bucket* find_empty(Hash_Table* table, u32 hash_key);
static bool contains(Hash_Table* table, char const* key);
static void resize(Hash_Table* table, u32 new_size);

Hash_Table* hash_table_create(u32 size, u32 data_size)
{
    Hash_Table* table = memory_system_allocate(sizeof(*table), MEMORY_TAG_CONTAINERS);
    table->size = round_up_to_next_pow2(size);
    table->data_size = data_size;
    table->used = 0;
    table->buckets = memory_system_allocate(table->size * sizeof(*table->buckets), MEMORY_TAG_CONTAINERS);
    for (u32 i = 0; i < table->size; ++i)
    {
        table->buckets[i].is_empty = true;
        table->buckets[i].in_probe = false;
        table->buckets[i].value = memory_system_allocate(table->data_size, MEMORY_TAG_CONTAINERS);
    }

    return table;
}

void hash_table_destroy(Hash_Table* table)
{
    for (u32 i = 0; i < table->size; ++i)
    {
        free(table->buckets[i].key);
        memory_system_free(table->buckets[i].value, table->data_size, MEMORY_TAG_CONTAINERS);
    }

    memory_system_free(table->buckets, table->size * sizeof(*table->buckets), MEMORY_TAG_CONTAINERS);
    memory_system_free(table, sizeof(*table), MEMORY_TAG_CONTAINERS);
}

void hash_table_insert(Hash_Table* table, char const* key, void const* value)
{
    u32 hash_key = hash(key);
    if (!contains(table, key))
    {
        Bucket* bucket = find_empty(table, hash_key);
        if (!bucket->in_probe)
        {
            table->used++;
        }

        bucket->is_empty = false;
        bucket->in_probe = true;
        bucket->key = string_duplicate(key);
        bucket->hash_key = hash_key;
        memory_system_copy(bucket->value, value, table->data_size);

        if (table->used > table->size / 2)
        {
            resize(table, table->size * 2);
        }
    }
}

void hash_table_erase(Hash_Table* table, char const* key)
{
    u32 hash_key = hash(key);
    Bucket* bucket = find_by_key(table, key);
    if (bucket->hash_key == hash_key)
    {
        free(bucket->key);
        memory_system_free(bucket->value, table->data_size, MEMORY_TAG_CONTAINERS);
        bucket->is_empty = true;
    }
}

void* hash_table_at(Hash_Table const* table, char const* key)
{
    Bucket* bucket = find_by_key(table, key);
    return bucket->in_probe ? bucket->value : 0;
}

u32 hash(char const* key)
{
    unsigned char* str = (unsigned char*)key;
    u32 hash = 5381;
    int c;
    while (c = *str++)
    {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

u32 round_up_to_next_pow2(u32 value)
{
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value++;
    return value;
}

u32 probe(u32 hash_key, u32 i, u32 table_size)
{
    return (hash_key + i * ((hash_key << 1) | 1)) & (table_size - 1);
}

Bucket* find_by_key(Hash_Table* table, char const* key)
{
    u32 hash_key = hash(key);
    for (u32 i = 0; i < table->size; ++i)
    {
        Bucket* bucket = table->buckets + probe(hash_key, i, table->size);
        if (bucket->hash_key == hash_key || !bucket->in_probe)
        {
            return bucket;
        }
    }

    return 0;
}

Bucket* find_empty(Hash_Table* table, u32 hash_key)
{
    for (u32 i = 0; i < table->size; ++i)
    {
        Bucket* bucket = table->buckets + probe(hash_key, i, table->size);
        if (bucket->is_empty)
        {
            return bucket;
        }
    }

    return 0;
}

bool contains(Hash_Table* table, char const* key)
{
    Bucket* bucket = find_by_key(table, key);
    return bucket->hash_key == key && !bucket->is_empty;
}

void resize(Hash_Table* table, u32 new_size)
{
    u32 old_size = table->size;
    Bucket* old_buckets = table->buckets;

    table->size = new_size;
    table->used = 0;
    table->buckets = memory_system_allocate(table->size * sizeof(*table->buckets), MEMORY_TAG_CONTAINERS);
    for (u32 i = 0; i < table->size; ++i)
    {
        memory_system_zero(&table->buckets[i], sizeof(table->buckets[i]));
        table->buckets[i].is_empty = true;
    }

    for (u32 i = 0; i < old_size; ++i)
    {
        if (!old_buckets[i].is_empty)
        {
            hash_table_insert(table, old_buckets[i].key, old_buckets[i].value);
        }

        free(old_buckets[i].key);
        memory_system_free(old_buckets[i].value, table->data_size, MEMORY_TAG_CONTAINERS);
    }

    memory_system_free(old_buckets, old_size * sizeof(*old_buckets), MEMORY_TAG_CONTAINERS);
}
