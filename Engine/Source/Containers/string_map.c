#include "string_map.h"

#include "core/logger.h"
#include "systems/memory_system.h"

typedef struct String_Map_Entry
{
    char key[32];
    void* data;
} String_Map_Entry;

/**
 * @brief DJB2 algorithm implementation.
 */
static u32 hash(unsigned char* str);
static u32 round_up_to_next_pow2(u32 value);
static Linked_List* get_bucket(String_Map const* map, u32 hash_key);
static void* insert(void const* data);
static void erase(void* data);
static bool compare(void const* node_data, void const* key);

String_Map* string_map_create(char const* type, u32 size, u32 data_size)
{
    String_Map* map = memory_system_allocate(sizeof(*map), MEMORY_TAG_CONTAINERS);
    strncpy(map->type, type, sizeof(map->type) - 1);
    map->size = round_up_to_next_pow2(size);
    map->data_size = data_size;
    map->buckets = memory_system_allocate(map->size * sizeof(*map->buckets), MEMORY_TAG_CONTAINERS);
    for (u32 i = 0; i < map->size; ++i)
    {
        map->buckets[i] = 0;
    }

    return map;
}

void string_map_destroy(String_Map* map)
{
    for (u32 i = 0; i < map->size; ++i)
    {
        linked_list_destroy(map->buckets[i]);
    }

    memory_system_free(map->buckets, map->size * sizeof(*map->buckets), MEMORY_TAG_CONTAINERS);
    memory_system_free(map, sizeof(*map), MEMORY_TAG_CONTAINERS);
}

void string_map_insert(String_Map* map, char const* key, void const* data)
{
    u32 hash_key = hash(key);
    Linked_List* bucket = get_bucket(map, hash_key);
    if (!bucket)
    {
        bucket = linked_list_create("String_Map_Entry", insert, erase, compare);
    }

    if (!linked_list_contains(bucket, key))
    {
        String_Map_Entry entry;
        strncpy(entry.key, key, sizeof(entry.key) - 1);
        entry.data = memory_system_allocate(map->data_size, MEMORY_TAG_CONTAINERS);
        memory_system_copy(entry.data, data, map->data_size);
        linked_list_insert_front(bucket, &entry);
    }
    else
    {
        LOG_WARNING("string_map_insert: Trying to insert already existing data. Not happening");
    }
}

void string_map_erase(String_Map* map, char const* key)
{
    u32 hash_key = hash(key);
    Linked_List* bucket = get_bucket(map, hash_key);
    if (bucket)
    {
        linked_list_erase(bucket, key);
    }
    else
    {
        LOG_WARNING("string_map_erase: Trying to delete non-existing data");
    }
}

bool string_map_contains(String_Map* map, char const* key)
{
    u32 hash_key = hash(key);
    Linked_List* bucket = get_bucket(map, hash_key);
    return bucket && linked_list_contains(bucket, &key);
}

void* string_map_at(String_Map* map, char const* key)
{
    u32 hash_key = hash(key);
    Linked_List* bucket = get_bucket(map, hash_key);
    if (bucket)
    {
        return linked_list_at(bucket, key);
    }

    LOG_WARNING("string_map_at: Trying to access non-existing data");
    return 0;
}

u32 hash(unsigned char* str)
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

Linked_List* get_bucket(String_Map const* map, u32 hash_key)
{
    u32 mask = map->size - 1;
    u32 index = hash_key & mask;
    return map->buckets[index];
}

void* insert(void const* data)
{
    void* new_data = memory_system_allocate(sizeof(String_Map_Entry), MEMORY_TAG_CONTAINERS);
    memory_system_copy(new_data, data, sizeof(String_Map_Entry));
    return new_data;
}

void erase(void* data)
{
    memory_system_free(data, sizeof(String_Map_Entry), MEMORY_TAG_CONTAINERS);
}

bool compare(void const* node_data, void const* key)
{
    return strcmp(((String_Map_Entry*)node_data)->key, key) == 0;
}
