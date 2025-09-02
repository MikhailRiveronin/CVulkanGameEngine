#include "texture_system.h"

#include "containers/hash_table.h"
#include "core/logger.h"
#include "core/memory_utils.h"
#include "core/string_utils.h"
#include "renderer/renderer_frontend.h"
#include "third_party/stb_image.h"

#include "systems/resource_system.h"

typedef struct texture_reference {
    u32 id;
    u64 reference_count;
    b8 auto_release;
} texture_reference;

typedef struct texture_system_state {
    texture_system_config config;
    texture* registered_textures;
    hash_table texture_references;
    texture default_texture;
} texture_system_state;

static texture_system_state* system_state;

static b8 create_default_textures();
static void destroy_default_textures();

static b8 load_texture(char const* filename, texture* t);
static void destroy_texture(texture* t);

b8 texture_system_startup(u64* state_size_in_bytes, void* memory, texture_system_config config)
{
    if (config.max_texture_count == 0) {
        LOG_FATAL("texture_system_initialize: config.max_texture_count must be > 0");
        return FALSE;
    }

    u64 struct_size_in_bytes = sizeof(*system_state);
    u64 array_size_in_bytes = sizeof(*system_state->registered_textures) * config.max_texture_count;
    u64 hash_table_size_in_bytes = sizeof(texture_reference) * config.max_texture_count;
    *state_size_in_bytes = struct_size_in_bytes + array_size_in_bytes + hash_table_size_in_bytes;

    if (!memory) {
        return TRUE;
    }

    system_state = memory;
    system_state->config = config;
    u32 texture_count = system_state->config.max_texture_count;

    // State layout: structure, texture array, texture reference hash table
    void* array_block = (char*)system_state + struct_size_in_bytes;
    system_state->registered_textures = array_block;

    for (u32 i = 0; i < texture_count; ++i) {
        system_state->registered_textures[i].id = INVALID_ID;
        system_state->registered_textures[i].generation = INVALID_ID;
    }

    void* hash_table_block = (char*)array_block + array_size_in_bytes;
    hash_table_create(
        sizeof(texture_reference), texture_count,
        hash_table_block, FALSE, &system_state->texture_references);

    texture_reference invalid_ref;
    invalid_ref.id = INVALID_ID;
    invalid_ref.reference_count = 0;
    invalid_ref.auto_release = FALSE;
    hash_table_fill(&system_state->texture_references, &invalid_ref);

    create_default_textures(system_state);
    return TRUE;
}

void texture_system_shutdown()
{
    if (system_state) {
        u32 texture_count = system_state->config.max_texture_count;
        for (u32 i = 0; i < texture_count; ++i) {
            texture* t = &system_state->registered_textures[i];
            if (t->generation != INVALID_ID) {
                destroy_texture(t);
            }
        }

        destroy_default_textures(system_state);
        system_state = 0;
    }
}

texture* texture_system_acquire_texture(char const* name, b8 auto_release)
{
    if (system_state && string_equali(name, DEFAULT_TEXTURE_NAME)) {
        LOG_WARNING(
            "texture_system_acquire_texture: Called for default texture. "
            "Use texture_system_get_default_texture for default texture");
        return &system_state->default_texture;
    }

    texture_reference ref;
    if (system_state && hash_table_get(&system_state->texture_references, name, &ref)) {
        // auto_release can only be set the first time a texture is loaded
        if (ref.reference_count == 0) {
            ref.auto_release = auto_release;
        }
        ref.reference_count++;

        if (ref.id == INVALID_ID) {
            u32 texture_count = system_state->config.max_texture_count;
            texture* t = 0;
            for (u32 i = 0; i < texture_count; ++i) {
                if (system_state->registered_textures[i].id == INVALID_ID) {
                    t = &system_state->registered_textures[i];
                    t->id = i;
                    ref.id = t->id;
                    break;
                }
            }

            if (!t || ref.id == INVALID_ID) {
                LOG_FATAL("texture_system_acquire_texture: Texture system cannot hold anymore textures");
                return 0;
            }

            if (!load_texture(name, t)) {
                return 0;
            }

            LOG_TRACE(
                "texture_system_acquire_texture: Texture '%s' created. reference_count %llu",
                name, ref.reference_count);
        } else {
            LOG_TRACE(
                "texture_system_acquire_texture: Texture '%s' acquired. reference_count %llu",
                name, ref.reference_count);
        }

        hash_table_set(&system_state->texture_references, name, &ref);
        return &system_state->registered_textures[ref.id];
    }

    LOG_ERROR("texture_system_acquire_texture: Failed to acquire texture '%s'. NULL will be returned", name);
    return 0;
}

void texture_system_release_texture(char const* name)
{
    if (string_equali(name, DEFAULT_TEXTURE_NAME)) {
        LOG_TRACE("texture_system_release_texture: No need to release a default texture '%s'", name);
        return;
    }

    // Original name will be wiped out by destroy_texture
    char name_copy[TEXTURE_NAME_MAX_LENGTH];
    string_ncopy(name_copy, name, TEXTURE_NAME_MAX_LENGTH);

    texture_reference ref;
    if (system_state && hash_table_get(&system_state->texture_references, name_copy, &ref)) {
        if (ref.reference_count == 0) {
            LOG_WARNING("texture_system_release_texture: Tried to release non-existent texture '%s'", name_copy);
            return;
        }

        ref.reference_count--;
        if (ref.reference_count == 0 && ref.auto_release) {
            texture* t = &system_state->registered_textures[ref.id];
            destroy_texture(t);

            ref.id = INVALID_ID;
            ref.auto_release = FALSE;

            LOG_TRACE("texture_system_release_texture: Texture '%s' released", name_copy);
        } else {
            LOG_TRACE(
                "texture_system_release_texture: Texture '%s' not released. reference_count %llu, auto_release %s",
                name_copy, ref.reference_count, ref.auto_release ? "TRUE" : "FALSE");
        }

        hash_table_set(&system_state->texture_references, name_copy, &ref);
        return;
    }

    LOG_ERROR("texture_system_release_texture: Failed to release texture '%s'", name_copy);
}

texture* texture_system_get_default_texture()
{
    if (system_state) {
        return &system_state->default_texture;
    }

    LOG_ERROR("texture_system_get_default_texture: Texture system not initialized. NULL will be returned");
    return 0;
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

    string_ncopy(system_state->default_texture.name, DEFAULT_TEXTURE_NAME, TEXTURE_NAME_MAX_LENGTH);
    system_state->default_texture.width = dimension;
    system_state->default_texture.height = dimension;
    system_state->default_texture.channel_count = 4;
    system_state->default_texture.generation = INVALID_ID;
    system_state->default_texture.has_transparency = FALSE;
    renderer_frontend_create_texture(pixels, &system_state->default_texture);

    // Set to invalid id since this is a default texture
    system_state->default_texture.generation = INVALID_ID;

    return TRUE;
}

void destroy_default_textures()
{
    if (system_state) {
        destroy_texture(&system_state->default_texture);
    }
}

b8 load_texture(char const* filename, texture* t)
{
    resource img_resource;
    if (!resource_system_load(filename, RESOURCE_TYPE_IMAGE, &img_resource)) {
        LOG_ERROR("Failed to load image resource for texture '%s'", filename);
        return FALSE;
    }

    image_resource_data* resource_data = img_resource.data;

    texture temp_texture;
    temp_texture.width = resource_data->width;
    temp_texture.height = resource_data->height;
    temp_texture.channel_count = resource_data->channel_count;

    u32 current_generation = t->generation;
    t->generation = INVALID_ID;

    u32 total_size = temp_texture.width * temp_texture.height * temp_texture.channel_count;
    b8 has_transparency = FALSE;
    for (u32 i = 0; i < total_size; i += temp_texture.channel_count) {
        u8 alpha = resource_data->pixels[i + 3];
        if (alpha < 255) {
            has_transparency = TRUE;
            break;
        }
    }

    string_ncopy(temp_texture.name, filename, TEXTURE_NAME_MAX_LENGTH);
    temp_texture.generation = INVALID_ID;
    temp_texture.has_transparency = has_transparency;

    renderer_frontend_create_texture(resource_data->pixels, &temp_texture);

    texture old = *t;
    *t = temp_texture;
    renderer_frontend_destroy_texture(&old);

    if (current_generation == INVALID_ID) {
        t->generation = 0;
    } else {
        t->generation = current_generation + 1;
    }

    resource_system_unload(&img_resource);
    return TRUE;
}

void destroy_texture(texture* t)
{
    renderer_frontend_destroy_texture(t);

    memory_zero(t->name, sizeof(char) * TEXTURE_NAME_MAX_LENGTH);
    memory_zero(t, sizeof(*t));
    t->id = INVALID_ID;
    t->generation = INVALID_ID;
}
