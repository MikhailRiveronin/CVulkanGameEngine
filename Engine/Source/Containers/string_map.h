#pragma once

#include "defines.h"

/**
 * @brief A hash table in which the keys are exclusively strings with collision resolution using chaining.
 */
typedef struct String_Map
{
    char value_type[32];

    u64 size;
    u64 stride;

    void* block;
} String_Map;

/**
 * @brief Creates a hash_table.
 * 
 * @param count The maximum number of elements. Cannot be resized.
 * @param element_size The size of each element in bytes.
 * @param block A block of memory to be used. Should be equal in size to count * element_size. If 0 allocated dynamically.
 * @param is_pointer_type Indicates if this hash_table will hold pointer types.
 * @param table A pointer to a hash_table in which to hold relevant data.
 */
LIB_API b8 hashtable_create(u64 count, u64 element_size, void const* block, void const* default_value, String_Map* table);

/**
 * @brief Destroys the provided hash_table. Does not release memory for pointer types.
 * 
 * @param table A pointer to the table to be destroyed.
 */
LIB_API void hashtable_destroy(String_Map* table);

/**
 * @brief Stores a copy of the data in value in the provided hash_table. 
 * Only use for tables which were *NOT* created with is_pointer_type = true.
 * 
 * @param table A pointer to the table to get from. Required.
 * @param name The name of the entry to set. Required.
 * @param value The value to be set. Required.
 * @return TRUE, or FALSE if a null pointer is passed.
 */
LIB_API b8 hashtable_set(String_Map* table, char const* name, void* value);

/**
 * @brief Stores a pointer as provided in value in the hash_table.
 * Only use for tables which were created with is_pointer_type = true.
 * 
 * @param table A pointer to the table to get from. Required.
 * @param name The name of the entry to set. Required.
 * @param value A pointer value to be set. Can pass 0 to 'unset' an entry.
 * @return TRUE, or FALSE if a null pointer is passed or if the entry is 0.
 */
LIB_API b8 hashtable_set_ptr(String_Map* table, char const* name, void** value);

/**
 * @brief Obtains a copy of data present in the hash_table.
 * Only use for tables which were *NOT* created with is_pointer_type = true.
 * 
 * @param table A pointer to the table to retrieved from. Required.
 * @param name The name of the entry to retrieved. Required.
 * @param value A pointer to store the retrieved value. Required.
 * @return TRUE, or FALSE if a null pointer is passed.
 */
LIB_API b8 hashtable_get(String_Map* table, char const* name, void* value);

/**
 * @brief Obtains a pointer to data present in the hash_table.
 * Only use for tables which were created with is_pointer_type = true.
 * 
 * @param table A pointer to the table to retrieved from. Required.
 * @param name The name of the entry to retrieved. Required.
 * @param value A pointer to store the retrieved value. Required.
 * @return True if retrieved successfully; false if a null pointer is passed or is the retrieved value is 0.
 */
LIB_API b8 hashtable_get_ptr(String_Map* table, char const* name, void** value);

/**
 * @brief Fills all entries in the hash_table with the given value.
 * Useful when non-existent names should return some default value.
 * Should not be used with pointer table types.
 * 
 * @param table A pointer to the table filled. Required.
 * @param value The value to be filled with. Required.
 * @return True if successful; otherwise false.
 */
LIB_API b8 hashtable_fill(String_Map* table, void* value);

#define HASHTABLE_CREATE(type, count, default_value, table) hashtable_create(count, sizeof(type), 0, &(type){ default_value }, table)
