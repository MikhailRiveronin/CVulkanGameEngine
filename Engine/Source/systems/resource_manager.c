#include "resource_manager.h"

#include "containers/hash_table.h"
#include "core/logger.h"
#include "core/math_utils.h"
#include "core/string_utils.h"
#include "memory/linear_allocator.h"
#include "resources/loaders.h"
#include "systems/memory_system.h"

typedef struct Material_Resource
{
    bool empty;
    char* name;
    char* filename;
    u32 id;
    u32 backend_id;
    Material_Type type;
    vec4s diffuse_color;
    Texture_Map diffuse_map;
} Material_Resource;

typedef enum Resource_Type
{
    RESOURCE_TYPE_TEXT,
    RESOURCE_TYPE_BINARY,
    RESOURCE_TYPE_IMAGE,
    RESOURCE_TYPE_MATERIAL,
    RESOURCE_TYPE_SHADER_CONFIG,
    RESOURCE_TYPE_STATIC_MESH,
    RESOURCE_TYPE_ENUM_COUNT
} Resource_Type;

typedef struct Resource
{
    char const* filepath;
    Resource_Type type;
    u32 reference_count;
    u32 slot; // index in the corresponding resource array
    bool auto_release; // if reference_count reaches zero
} Resource;

typedef struct Resource_Entry
{
    void* resource;
    // Resource resource;
} Resource_Entry;

/**
 * @brief LUT containing references to all resources.
 * A non-shrinkable (but growable) hash table.
 * The keys are resource file paths.
 * Open addressing collision resolution. Double probing.
 */
typedef struct Resource_Lookup_Table
{
    u32 size;
    u32 used;
    Resource_Entry* entries;
} Resource_Lookup_Table;

typedef struct Resource_System_State
{
    Linear_Allocator* allocator;
    Hash_Table* lut;
    Resource_Loader* loaders[RESOURCE_TYPE_ENUM_COUNT];
} Resource_System_State;



static Resource_System_State* state;

static bool load_resource(char const* filepath);


/**
 * @brief Finds the entry that contains a given key, if the key is in the table, or 0.
 */
static Resource_Entry* lookup_table_find_by_hash(Resource_Lookup_Table* lut, u32 hash_key);

/**
 * @brief Finds the first empty entry corresponding the key, or 0.
 */
static Resource_Entry* lookup_table_find_empty(Resource_Lookup_Table* lut, u32 hash_key);

/**
 * @brief Check if the table contains a given key.
 */
static bool lookup_table_contains(Resource_Lookup_Table* lut, u32 hash_key);

/**
 * @brief Doubles the table size.
 */
static bool lookup_table_resize(Resource_Lookup_Table* lut, u32 new_size);

/**
 * @brief Insert a new resource into the table.
 */
static bool lookup_table_insert(Resource_Lookup_Table* lut, u32 hash_key, Resource* resource);

/**
 * @brief Searching for the next available entry for inserting a new resource.
 */
static u32 lookup_table_probe(u32 hash_key, u32 i, u32 lut_size);

static i32 find_empty_slot(Resource_Type type);
static i32 find_empty_material_slot();


static bool load_material_config(char const* filename, Material_Config* material);

#define INITIAL_LOOKUP_TABLE_SIZE 1024

bool resource_manager_startup()
{
    if (!linear_allocator_create(1024 * 1024/*1 MiB*/, state->allocator))
    {
        LOG_FATAL("resource_manager_startup: Failed to create allocator");
        return false;
    }

    state->lut = HASH_TABLE_CREATE(Resource_Entry, INITIAL_LOOKUP_TABLE_SIZE);
    for (u32 i = 0; i < state->lut->size; ++i)
    {
        state->lut->entries[i].is_empty = true;
        state->lut->entries[i].in_probe = false;
        state->lut->entries[i].resource.auto_release = true;
    }

    for (u32 i; i < MAX_MATERIAL_RESOURCE_COUNT; ++i)
    {
        state->resources.materials[i].empty = true;
    }







    state->loaders[RESOURCE_TYPE_TEXT] = text_loader_create();
    state->loaders[RESOURCE_TYPE_BINARY] = binary_loader_create();
    state->loaders[RESOURCE_TYPE_IMAGE] = image_loader_create();
    state->loaders[RESOURCE_TYPE_MATERIAL] = material_loader_create();
    state->loaders[RESOURCE_TYPE_SHADER_CONFIG] = shader_config_loader_create();
    return true;
}

void resource_manager_shutdown()
{
    if (state)
    {
        for (u32 i = 0; i < RESOURCE_TYPE_ENUM_COUNT; ++i)
        {
            memory_system_free(&state->loaders[i], MEMORY_TAG_LOADERS);
        }

        memory_system_free(state, sizeof(*state), MEMORY_TAG_SYSTEMS);
        state = 0;
    }

    LOG_WARNING("resource_manager_shutdown: Resource system hasn't been started up yet");
}



bool load_resource(char const* filepath)
{
    Resource_Entry entry;
    entry.hash_key = DJB2_hash(filepath);
    if (string_equal(filepath, "materials"))
    {
        // entry.resource.type = RESOURCE_TYPE_MATERIAL;
        // for (u32 i = 0; i < MAX_MATERIAL_RESOURCE_COUNT; ++i)
        // {
        //     if (state->resources.materials[i].empty)
        //     {
        //         entry.resource.slot = i;
        //         state->resources.materials[i].empty = false;
        //     }
        // }

        Material_Resource* resource = linear_allocator_allocate(state->allocator, sizeof(*resource));
        if (!resource)
        {
            LOG_FATAL("load_resource: Failed to allocate memory for resource");
            return false;
        }

        if (!state->loaders[entry.resource.type]->load(filepath, resource))
        {
            LOG_FATAL("load_resource: Failed to load resource");
            return false;
        }
    }
    else
    {
        // other resource types
    }

    if (!lookup_table_insert(state->lut, entry.hash_key, &entry.resource))
    {
        LOG_FATAL("load_resource: Failed to insert new entry");
        return false;
    }

    return true;
}

void resource_manager_unload(Resource_Data* resource)
{
    if (!resource)
    {
        LOG_FATAL("resource_manager_unload: Invalid parameters");
        return;
    }

    if (state)
    {
        state->loaders[resource->type]->unload(resource);
    }

    LOG_WARNING("resource_manager_load: Resource system hasn't been started up yet");
}

bool resource_manager_acquire(char const* filepath, bool auto_release, void* resource)
{
    if (!state)
    {
        LOG_FATAL("resource_manager_acquire: Resource manager hasn't been started up yet");
        return false;
    }

    if (!filepath)
    {
        LOG_FATAL("resource_manager_acquire: Invalid parameters");
        return false;
    }

    u32 hash_key = DJB2_hash(filepath);
    Resource_Entry* entry = lookup_table_find_by_hash(state->lut, hash_key);
    if (!entry)
    {
        if (!load_resource(filepath))
        {
            LOG_FATAL("resource_manager_acquire: Failed to load resource");
            return false;
        }

        entry = lookup_table_find_by_hash(state->lut, hash_key);
    }

    if (!entry)
    {
        LOG_FATAL("resource_manager_acquire: Failed to acquire resource");
        return false;
    }

    if (!auto_release && entry->resource.auto_release)
    {
        entry->resource.auto_release = auto_release;
    }

    entry->resource.reference_count++;
    resource = // materials[entry->slot];

    return true;
}


Resource_Entry* lookup_table_find_by_hash(Resource_Lookup_Table* lut, u32 hash_key)
{
    for (u32 i = 0; i < lut->size; ++i)
    {
        Resource_Entry* entry = lut->entries + lookup_table_probe(hash_key, i, lut->size);
        if (entry->hash_key == hash_key)
        {
            return entry;
        }

        if (!entry->in_probe)
        {
            break;
        }
    }

    return 0;
}

Resource_Entry* lookup_table_find_empty(Resource_Lookup_Table* lut, u32 hash_key)
{
    for (u32 i = 0; i < lut->size; ++i)
    {
        Resource_Entry* entry = lut->entries + lookup_table_probe(hash_key, i, lut->size);
        if (entry->is_empty)
        {
            return entry;
        }
    }

    return 0;
}

bool lookup_table_contains(Resource_Lookup_Table* lut, u32 hash_key)
{
    Resource_Entry* entry = lookup_table_find_by_hash(lut, hash_key);
    return !!entry;
}

bool lookup_table_resize(Resource_Lookup_Table* lut, u32 new_size)
{
    u32 old_size = lut->size;
    Resource_Entry* old_entries = lut->entries;

    lut->size = new_size;
    lut->used = 0;
    lut->entries = memory_system_allocate(lut->size * sizeof(*lut->entries), MEMORY_TAG_RESOURCES);
    if (!lut->entries)
    {
        LOG_FATAL("lookup_table_resize: Failed to allocate memory for new entries")
        return false;
    }

    for (u32 i = 0; i < lut->size; ++i)
    {
        state->lut->entries[i].is_empty = true;
        state->lut->entries[i].in_probe = false;
        state->lut->entries[i].resource.auto_release = true;
    }

    for (u32 i = 0; i < old_size; ++i)
    {
        if (!old_buckets[i].is_empty)
        {
            lookup_table_insert(lut, old_entries[i].hash_key, &old_entries[i].resource);
        }
    }

    memory_system_free(old_entries, old_size * sizeof(*old_entries), MEMORY_TAG_RESOURCES);
}

bool lookup_table_insert(Resource_Lookup_Table* lut, u32 hash_key, Resource* resource)
{
    if (lookup_table_contains(lut, hash_key))
    {
        LOG_WARNING("lookup_table_insert: Entry already exists");
        return false;
    }

    Resource_Entry* entry = lookup_table_find_empty(lut, hash_key);
    if (!entry->in_probe)
    {
        lut->used++;
    }

    entry->is_empty = false;
    entry->in_probe = true;
    entry->hash_key = hash_key;
    memory_system_copy(&entry->resource, resource, sizeof(*resource));

    if ((lut->used > lut->size / 2) && !lookup_table_resize(state->lut, state->lut->size * 2))
    {
        LOG_WARNING("lookup_table_insert: Failed to resize lookup table. Ok?");
    }

    return true;
}

u32 lookup_table_probe(u32 hash_key, u32 i, u32 map_size)
{
    return (hash_key + i * ((hash_key << 1) | 1)) & (map_size - 1);
}

bool load_material_config(char const* filename, Material_Config* material)
{
    return true;
}

i32 find_empty_slot(Resource_Type type)
{
    switch (type)
    {
        case RESOURCE_TYPE_MATERIAL:
            return find_empty_material_slot();

        default:
            return -1;
    }
}

i32 find_empty_material_slot()
{
    for (u32 i = 0; i < state->materials.count; ++i)
    {
        if (state->materials.empty_slots[i])
        {
            state->materials.empty_slots[i] = false;
            return i;
        }
    }

    return -1;
}
