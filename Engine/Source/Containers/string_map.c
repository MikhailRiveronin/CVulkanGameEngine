#include "string_map.h"

#include "core/string_utils.h"
#include "systems/memory_system.h"

/**
 * @brief DJB2 algorithm implementation.
 */
static u32 hash(char* str);
static u32 round_up_to_next_pow2(u32 value);
static u32 probe(u32 hash_key, u32 i, u32 map_size);

/**
 * @brief Find the bucket containing key, if the key is in the table, or the first one past the end of its probe.
 */
static Bucket* find_by_key(String_Map* map, char const* key);
static Bucket* find_empty(String_Map* map, u32 hash_key);
static bool contains(String_Map* map, char const* key);
static void resize(String_Map* map, u32 new_size);

String_Map* string_map_create(char const* type, u32 size, u32 data_size)
{
    String_Map* map = memory_system_allocate(sizeof(*map), MEMORY_TAG_CONTAINERS);
    strncpy(map->type, type, sizeof(map->type) - 1);
    map->size = round_up_to_next_pow2(size);
    map->data_size = data_size;
    map->used = 0;
    map->buckets = memory_system_allocate(map->size * sizeof(*map->buckets), MEMORY_TAG_CONTAINERS);
    for (u32 i = 0; i < map->size; ++i)
    {
        memory_system_zero(&map->buckets[i], sizeof(map->buckets[i]));
        map->buckets[i].is_empty = true;
    }

    return map;
}

void string_map_destroy(String_Map* map)
{
    for (u32 i = 0; i < map->size; ++i)
    {
        memory_system_free(&map->buckets[i], sizeof(map->buckets[i]), MEMORY_TAG_CONTAINERS);
    }

    memory_system_free(map->buckets, map->size * sizeof(*map->buckets), MEMORY_TAG_CONTAINERS);
    memory_system_free(map, sizeof(*map), MEMORY_TAG_CONTAINERS);
}

void string_map_insert(String_Map* map, char const* key, void const* value)
{
    if (!contains(map, key))
    {
        Bucket* bucket = find_empty(map, hash(key));
        if (!bucket->in_probe)
        {
            ++map->used;
        }

        bucket->is_empty = false;
        bucket->in_probe = true;
        strncpy(bucket->key, key, sizeof(bucket->key) - 1);
        memory_system_copy(bucket->value, value, map->data_size);

        if (map->used > map->size / 2)
        {
            resize(map, map->size * 2);
        }
    }
}

void string_map_erase(String_Map* map, char const* key)
{
    Bucket* bucket = find_by_key(map, key);
    if (string_equal(bucket->key, key))
    {
        memory_system_free(bucket->value, map->data_size, MEMORY_TAG_CONTAINERS);
        bucket->is_empty = true;
    }
}

void* string_map_at(String_Map const* map, char const* key)
{
    Bucket* bucket = find_by_key(map, key);
    return bucket->in_probe ? bucket->value : 0;
}

u32 hash(char* str)
{
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

u32 probe(u32 hash_key, u32 i, u32 map_size)
{
    return (hash_key + i * ((hash_key << 1) | 1)) & (map_size - 1);
}

Bucket* find_by_key(String_Map* map, char const* key)
{
    for (u32 i = 0; i < map->size; ++i)
    {
        Bucket* bucket = map->buckets + probe(hash(key), i, map->size);
        if (string_equal(bucket->key, key) || !bucket->in_probe)
        {
            return bucket;
        }
    }

    return 0;
}

Bucket* find_empty(String_Map* map, u32 hash_key)
{
    for (u32 i = 0; i < map->size; ++i)
    {
        Bucket* bucket = map->buckets + probe(hash_key, i, map->size);
        if (bucket->is_empty)
        {
            return bucket;
        }
    }

    return 0;
}

bool contains(String_Map* map, char const* key)
{
    Bucket* bucket = find_by_key(map, key);
    return string_equal(bucket->key, key);
}

void resize(String_Map* map, u32 new_size)
{
    u32 old_size = map->size;
    Bucket* old_buckets = map->buckets;

    map->size = new_size;
    map->used = 0;
    map->buckets = memory_system_allocate(map->size * sizeof(*map->buckets), MEMORY_TAG_CONTAINERS);
    for (u32 i = 0; i < map->size; ++i)
    {
        memory_system_zero(&map->buckets[i], sizeof(map->buckets[i]));
        map->buckets[i].is_empty = true;
    }

    for (u32 i = 0; i < old_size; ++i)
    {
        if (!old_buckets[i].is_empty)
        {
            string_map_insert(map, old_buckets[i].key, old_buckets[i].value);
        }

        memory_system_free(old_buckets[i].value, map->data_size, MEMORY_TAG_CONTAINERS);
    }

    memory_system_free(old_buckets, old_size * sizeof(*old_buckets), MEMORY_TAG_CONTAINERS);
}
