#include "resource_system.h"

#include "core/logger.h"
#include "core/math_utils.h"
#include "resources/loaders.h"
#include "systems/memory_system.h"

#define INITIAL_RESOURCE_LUT_SIZE 1024

typedef struct Resource_Reference
{
    bool is_empty;
    bool in_probe;
    u32 hash_key;

    struct
    {
        char const* filename;
        char const* name;
        u32 generation; // TODO: Check if really needed
        u32 reference_count;
        u32 slot; // index in the corresponding resource array
        bool auto_release; // if reference_count reaches zero
    } data;
} Resource_Reference;

/**
 * @brief LUT containing references to all resources. A non-shrinkable (but growable) hash table. The keys are resource names. Open addressing collision resolution. Double probing.
 */
typedef struct Resource_LUT
{
    u32 size;
    u32 used;
    Resource_Reference* references;
} Resource_LUT;



typedef struct Resource_System_State
{
    Resource_LUT* lut;
    Resource_Loader* loaders[RESOURCE_TYPE_ENUM_COUNT];
} Resource_System_State;

static Resource_System_State* state;

/**
 * @brief Returns the reference containing key, if the key is in the table, or the first one past the end of its probe.
 */
static Resource_Reference* find_by_hash(Resource_LUT* lut, u32 hash_key);
static Resource_Reference* find_empty(Resource_LUT* lut, u32 hash_key);
static bool contains(Resource_System_State* table, char const* key);
static void resize(Resource_System_State* table, u32 new_size);
static u32 probe(u32 hash_key, u32 i, u32 lut_size);

bool resource_system_startup()
{
    state->lut = memory_system_allocate(sizeof(*state->lut), MEMORY_TAG_SYSTEMS);
    state->lut->size = round_up_to_next_pow2(INITIAL_RESOURCE_LUT_SIZE);
    state->lut->used = 0;
    state->lut->references = memory_system_allocate(state->lut->size * sizeof(*state->lut->references), MEMORY_TAG_SYSTEMS);
    for (u32 i = 0; i < state->lut->size; ++i)
    {
        state->lut->references[i].is_empty = true;
        state->lut->references[i].in_probe = false;
    }





    state = memory_system_allocate(sizeof(*state), MEMORY_TAG_SYSTEMS);
    state->loaders[RESOURCE_TYPE_TEXT] = text_loader_create();
    state->loaders[RESOURCE_TYPE_BINARY] = binary_loader_create();
    state->loaders[RESOURCE_TYPE_IMAGE] = image_loader_create();
    state->loaders[RESOURCE_TYPE_MATERIAL] = material_loader_create();
    state->loaders[RESOURCE_TYPE_SHADER_CONFIG] = shader_config_loader_create();
    return true;
}

void resource_system_shutdown()
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

    LOG_WARNING("resource_system_shutdown: Resource system hasn't been started up yet");
}

bool resource_system_load(Resource_Type type, char const* filename, i32 slot, Resource_Data* resource)
{
    if (!state)
    {
        LOG_FATAL("resource_system_load: Resource system hasn't been started up yet");
        return false;
    }

    if (!filename || !resource)
    {
        LOG_FATAL("resource_system_load: Invalid parameters");
        return false;
    }

    if (contains(state->lut, filename))
    {
        LOG_WARNING("resource_system_load: Resource already loaded");
        return false;
    }

    u32 hash = DJB2_hash(filename);
    Resource_Reference* reference = find_empty(state->lut, hash);
    if (!reference)
    {
        LOG_FATAL("resource_system_load: Failed to find empty slot");
        return false;
    }

    if (!reference->in_probe)
    {
        if (++state->lut->used > state->lut->size / 2)
        {
            resize(state->lut, state->lut->size * 2);
        }
    }

    resource->type = type;
    if (!state->loaders[resource->type]->load(filename, resource))
    {
        LOG_FATAL("resource_system_load: Failed to load resource");
        return false;
    }

    reference->is_empty = false;
    reference->in_probe = true;
    reference->hash_key = hash;
    reference->data.filename = resource->filename;
    reference->data.name = resource->name;
    reference->data.generation = resource->generation;
    reference->data.reference_count = 0;
    reference->data.slot = slot;
    reference->data.auto_release = resource->auto_release;
    return true;
}

void resource_system_unload(Resource_Data* resource)
{
    if (!resource)
    {
        LOG_FATAL("resource_system_unload: Invalid parameters");
        return;
    }

    if (state)
    {
        state->loaders[resource->type]->unload(resource);
    }

    LOG_WARNING("resource_system_load: Resource system hasn't been started up yet");
}

u32 resource_system_get_reference(char const* name)
{
    Resource_Reference* bucket = find_by_hash(state->lut, D);
    return bucket->in_probe ? bucket->value : 0;
}

Resource_Reference* find_by_hash(Resource_LUT* lut, u32 hash_key)
{
    for (u32 i = 0; i < lut->size; ++i)
    {
        Resource_Reference* reference = lut->references + probe(hash_key, i, lut->size);
        if (reference->hash_key == hash_key)
        {
            return reference;
        }

        if (!reference->in_probe)
        {
            break;
        }
    }

    return 0;
}

Resource_Reference* find_empty(Resource_LUT* lut, u32 hash_key)
{
    for (u32 i = 0; i < lut->size; ++i)
    {
        Resource_Reference* reference = lut->references + probe(hash_key, i, lut->size);
        if (reference->is_empty)
        {
            return reference;
        }
    }

    return 0;
}

u32 probe(u32 hash_key, u32 i, u32 map_size)
{
    return (hash_key + i * ((hash_key << 1) | 1)) & (map_size - 1);
}
