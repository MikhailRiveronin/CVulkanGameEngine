#include "texture_system.h"

#include "containers/hash_table.h"
#include "core/logger.h"
#include "core/string_utils.h"
#include "renderer/renderer_frontend.h"
#include "systems/memory_system.h"
#include "systems/resource_system.h"

typedef struct Texture_Reference
{
    u32 internal_id;
    u64 reference_count;
    b8 auto_release;
} Texture_Reference;

typedef struct Texture_System_State
{
    Texture_System_Config config;
    Texture* registered_textures;
    hashtable texture_references;
    Texture default_texture;
} Texture_System_State;

static Texture_System_State* state;

static b8 create_texture(char const* name, Texture* t);
static void destroy_texture(Texture* t);
static b8 create_default_textures();
static void destroy_default_textures();

b8 texture_system_startup(u64* required_memory, void* block, Texture_System_Config config)
{
    if (config.max_texture_count == 0)
    {
        LOG_FATAL("texture_system_startup: Invalid input parameters");
        return FALSE;
    }

    u64 state_struct_required_memory = sizeof(*state);
    u64 textures_reqired_memory = config.max_texture_count * sizeof(*state->registered_textures);
    u64 texture_references_required_memory = config.max_texture_count * sizeof(Texture_Reference);
    *required_memory = state_struct_required_memory + textures_reqired_memory + texture_references_required_memory;
    if (!block)
    {
        return TRUE;
    }

    state = block;
    state->config = config;
    state->registered_textures = (char*)state + state_struct_required_memory;

    for (u32 i = 0; i < state->config.max_texture_count; ++i)
    {
        state->registered_textures[i].id = INVALID_ID;
        state->registered_textures[i].generation = INVALID_ID;
    }

    void* texture_references_block = (char*)state->registered_textures + textures_reqired_memory;
    hashtable_create(sizeof(Texture_Reference), config.max_texture_count, texture_references_block, FALSE, &state->texture_references);
    Texture_Reference invalid_ref;
    invalid_ref.internal_id = INVALID_ID;
    invalid_ref.reference_count = 0;
    invalid_ref.auto_release = FALSE;
    hashtable_fill(&state->texture_references, &invalid_ref);

    create_default_textures(state);
    return TRUE;
}

void texture_system_shutdown()
{
    if (state)
    {
        for (u32 i = 0; i < state->config.max_texture_count; ++i)
        {
            Texture* t = &state->registered_textures[i];
            if (t->generation != INVALID_ID)
            {
                destroy_texture(t);
            }
        }

        destroy_default_textures(state);
        state = 0;
    }
}

Texture* texture_system_acquire(char const* name, b8 auto_release)
{
    Texture_Reference ref;
    if (state && hashtable_get(&state->texture_references, name, &ref))
    {
        if (ref.reference_count == 0)
        {
            ref.auto_release = auto_release;
        }
        ref.reference_count++;

        if (ref.internal_id == INVALID_ID)
        {
            Texture* tex = 0;
            for (u32 i = 0; i < state->config.max_texture_count; ++i)
            {
                if (state->registered_textures[i].id == INVALID_ID)
                {
                    tex = &state->registered_textures[i];
                    tex->id = i;
                    ref.internal_id = tex->id;
                    break;
                }
            }

            if (!tex || ref.internal_id == INVALID_ID)
            {
                LOG_FATAL("texture_system_acquire: Texture system cannot hold anymore textures");
                return 0;
            }

            if (!create_texture(name, tex))
            {
                return 0;
            }

            LOG_TRACE("texture_system_acquire: Texture '%s' created. reference_count %llu", name, ref.reference_count);
        }
        else
        {
            LOG_TRACE("texture_system_acquire: Texture '%s' acquired. reference_count %llu", name, ref.reference_count);
        }

        hashtable_set(&state->texture_references, name, &ref);
        return &state->registered_textures[ref.internal_id];
    }

    LOG_ERROR("texture_system_acquire: Failed to acquire texture '%s'. NULL will be returned", name);
    return 0;
}

void texture_system_release(char const* name)
{
    char name_copy[TEXTURE_NAME_MAX_LENGTH];
    string_ncopy(name_copy, name, TEXTURE_NAME_MAX_LENGTH);

    Texture_Reference ref;
    if (state && hashtable_get(&state->texture_references, name_copy, &ref))
    {
        if (ref.reference_count == 0)
        {
            LOG_WARNING("texture_system_release: Tried to release non-existent texture '%s'", name_copy);
            return;
        }

        ref.reference_count--;
        if (ref.reference_count == 0 && ref.auto_release)
        {
            Texture* t = &state->registered_textures[ref.internal_id];
            destroy_texture(t);

            ref.internal_id = INVALID_ID;
            ref.auto_release = FALSE;

            LOG_TRACE("texture_system_release: Texture '%s' released", name_copy);
        }
        else
        {
            LOG_TRACE("texture_system_release: Texture '%s' not released. reference_count %llu, auto_release %s", name_copy, ref.reference_count, ref.auto_release ? "TRUE" : "FALSE");
        }

        hashtable_set(&state->texture_references, name_copy, &ref);
        return;
    }

    LOG_ERROR("texture_system_release: Failed to release texture '%s'", name_copy);
}

Texture* texture_system_get_default_texture()
{
    if (state) {
        return &state->default_texture;
    }

    LOG_ERROR("texture_system_get_default_texture: Texture system not initialized. NULL will be returned");
    return 0;
}

b8 create_texture(char const* name, Texture* t)
{
    Resource resource;
    if (!resource_system_load(name, RESOURCE_TYPE_IMAGE, &resource))
    {
        LOG_ERROR("create_texture: Failed to load image resource for texture '%s'", name);
        return FALSE;
    }

    image_resource_data* resource_data = (image_resource_data*)resource.data;
    Texture temp_texture;
    temp_texture.width = resource_data->width;
    temp_texture.height = resource_data->height;
    temp_texture.channel_count = resource_data->channel_count;
    temp_texture.generation = INVALID_ID;

    string_ncopy(temp_texture.name, name, TEXTURE_NAME_MAX_LENGTH);

    u32 total_size = temp_texture.width * temp_texture.height * temp_texture.channel_count;
    b8 has_transparency = FALSE;
    for (u32 i = 0; i < total_size; i += temp_texture.channel_count)
    {
        u8 alpha = resource_data->pixels[i + 3];
        if (alpha < 255)
        {
            has_transparency = TRUE;
            break;
        }
    }
    temp_texture.has_transparency = has_transparency;

    u32 current_generation = t->generation;
    t->generation = INVALID_ID;

    renderer_frontend_create_texture(resource_data->pixels, &temp_texture);

    Texture old = *t;
    *t = temp_texture;
    renderer_frontend_destroy_texture(&old);

    if (current_generation == INVALID_ID)
    {
        t->generation = 0;
    }
    else
    {
        t->generation = current_generation + 1;
    }

    resource_system_unload(&resource);
    return TRUE;
}

void destroy_texture(Texture* t)
{
    renderer_frontend_destroy_texture(t);

    memory_zero(t->name, sizeof(char) * TEXTURE_NAME_MAX_LENGTH);
    memory_zero(t, sizeof(*t));
    t->id = INVALID_ID;
    t->generation = INVALID_ID;
}

b8 create_default_textures()
{
    u32 dimension = 256;
    u32 channel_count = 4;
    u32 pixel_count = dimension * dimension;
    u8* pixels = memory_allocate(pixel_count * channel_count, MEMORY_TAG_TEXTURE);
    memory_set(pixels, 255, sizeof(u8) * pixel_count * channel_count);

    for (u64 row = 0; row < dimension; ++row) {
        for (u64 col = 0; col < dimension; ++col) {
            u64 index = (row * dimension) + col;
            u64 channel_index = index * channel_count;
            if (row % 2 != 0) {
                if (col % 2 != 0) {
                    pixels[channel_index + 0] = 0;
                    pixels[channel_index + 1] = 0;
                }
            } else {
                if (col % 2 == 0) {
                    pixels[channel_index + 0] = 0;
                    pixels[channel_index + 1] = 0;
                }
            }
        }
    }

    string_ncopy(state->default_texture.name, DEFAULT_TEXTURE_NAME, TEXTURE_NAME_MAX_LENGTH);
    state->default_texture.width = dimension;
    state->default_texture.height = dimension;
    state->default_texture.channel_count = 4;
    state->default_texture.generation = INVALID_ID;
    state->default_texture.has_transparency = FALSE;
    renderer_frontend_create_texture(pixels, &state->default_texture);

    // Set to invalid id since this is a default texture
    state->default_texture.generation = INVALID_ID;

    return TRUE;
}

void destroy_default_textures()
{
    if (state)
    {
        destroy_texture(&state->default_texture);
    }
}
