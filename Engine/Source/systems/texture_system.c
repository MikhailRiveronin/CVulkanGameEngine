#include "texture_system.h"

#include "containers/hash_table.h"
#include "core/logger.h"
#include "core/memory_utils.h"
#include "core/string_utils.h"
#include "renderer/renderer_frontend.h"
#include "third_party/stb_image.h"

typedef struct texture_system_state {
    texture_system_config config;

    texture default_texture;
    texture* registered_textures;
    hash_table texture_reference_table;
} texture_system_state;
static texture_system_state* system_state;

typedef struct texture_reference {
    u64 reference_count;
    u32 handle;
    b8 auto_release;
} texture_reference;

static b8 create_default_textures();
static void destroy_default_textures();
static b8 load_texture(char const* filename, texture* t);
static void destroy_texture(texture* t);

b8 texture_system_startup(u64* required_memory, void* memory, texture_system_config config)
{
    if (config.max_texture_count == 0) {
        LOG_FATAL("texture_system_initialize - config.max_texture_count must be > 0.");
        return FALSE;
    }

    u64 state_required_memory = sizeof(*system_state);
    u64 array_required_memory = sizeof(*system_state->registered_textures) * config.max_texture_count;
    u64 hash_table_required_memory = sizeof(texture_reference) * config.max_texture_count;
    *required_memory = state_required_memory + array_required_memory + hash_table_required_memory;

    if (!memory) {
        return TRUE;
    }

    system_state = memory;
    system_state->config = config;

    void* array_block = (char*)memory + state_required_memory;
    system_state->registered_textures = array_block;

    void* hash_table_block = (char*)array_block + array_required_memory;

    for (u32 i = 0; i < system_state->config.max_texture_count; ++i) {
        system_state->registered_textures[i].id = INVALID_ID;
        system_state->registered_textures[i].generation = INVALID_GEN;
    }

    hash_table_create(
        sizeof(texture_reference),
        config.max_texture_count,
        hash_table_block,
        FALSE,
        &system_state->texture_reference_table);

    texture_reference invalid;
    invalid.auto_release = FALSE;
    invalid.handle = INVALID_ID;
    invalid.reference_count = 0;
    hash_table_fill(&system_state->texture_reference_table, &invalid);

    create_default_textures(system_state);

    return TRUE;
}

void texture_system_shutdown()
{
    if (system_state) {
        for (u32 i = 0; i < system_state->config.max_texture_count; ++i) {
            texture* t = &system_state->registered_textures[i];
            if (t->generation != INVALID_ID) {
                renderer_frontend_destroy_texture(t);
            }
        }

        destroy_default_textures(system_state);

        system_state = 0;
    }
}

texture* texture_acquire(char const* name, b8 auto_release)
{
    if (system_state != 0 && string_equali(name, DEFAULT_TEXTURE_NAME)) {
        LOG_WARNING("texture_acquire: Called for default texture. Use texture_get_default for default texture");
        return &system_state->default_texture;
    }

    texture_reference ref;
    if (system_state != 0 && hash_table_get(&system_state->texture_reference_table, name, &ref)) {
        // auto_release can only be set the first time a texture is loaded.
        if (ref.reference_count == 0) {
            ref.auto_release = auto_release;
        }
        ref.reference_count++;

        if (ref.handle == INVALID_ID) {
            texture* t = 0;
            for (u32 i = 0; i < system_state->config.max_texture_count; ++i) {
                if (system_state->registered_textures[i].id == INVALID_ID) {
                    t = &system_state->registered_textures[i];
                    t->id = i;
                    ref.handle = t->id;
                    break;
                }
            }

            if (!t || ref.handle == INVALID_ID) {
                LOG_FATAL("texture_acquire: Texture system cannot hold anymore textures");
                return 0;
            }

            if (!load_texture(name, t)) {
                return 0;
            }

            LOG_TRACE("texture_acquire: Texture '%s' created", name);
        } else {
            LOG_TRACE("texture_acquire: Texture '%s' acquired. reference_count %llu", name, ref.reference_count);
        }

        hash_table_set(&system_state->texture_reference_table, name, &ref);

        return &system_state->registered_textures[ref.handle];
    }

    LOG_ERROR("texture_acquire: Failed to acquire texture '%s'. NULL will be returned", name);
    return 0;
}

void texture_release(char const* name)
{
    if (string_equali(name, DEFAULT_TEXTURE_NAME)) {
        LOG_TRACE("texture_release: No need to release a default texture '%s'", name);
        return;
    }

    char name_copy[TEXTURE_NAME_MAX_LENGTH];
    string_ncopy(name_copy, name, TEXTURE_NAME_MAX_LENGTH);

    texture_reference ref;
    if (system_state != 0 && hash_table_get(&system_state->texture_reference_table, name, &ref)) {
        if (ref.reference_count == 0) {
            LOG_WARNING("texture_release: Tried to release non-existent texture '%s'", name);
            return;
        }

        ref.reference_count--;
        if (ref.reference_count == 0 && ref.auto_release) {
            texture* t = &system_state->registered_textures[ref.handle];
            destroy_texture(t);

            LOG_TRACE("texture_release: Texture '%s' released", name_copy);
        } else {
            LOG_TRACE("texture_release: Texture '%s' not released. reference_count %llu, auto_release %s",
                name_copy,
                ref.reference_count,
                ref.auto_release ? "TRUE" : "FALSE");
        }

        hash_table_set(&system_state->texture_reference_table, name_copy, &ref);
    }

    LOG_ERROR("texture_release: Failed to release texture '%s'", name_copy);
}

texture* texture_get_default()
{
    if (system_state != 0) {
        return &system_state->default_texture;
    }

    LOG_ERROR("texture_get_default: Texture system not initialized. NULL will be returned");
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
    system_state->default_texture.generation = INVALID_GEN;

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
    char* format = "assets/textures/%s.%s";
    char absolute_path[512];
    string_format(absolute_path, format, filename, "png");

    texture temp;
    i32 desired_channels = 4;
    u8* data = stbi_load(
        absolute_path,
        &temp.width,
        &temp.height,
        (i32*)&temp.channel_count,
        desired_channels);
    temp.channel_count = desired_channels;

    if (data) {
        u32 total_size = temp.width * temp.height * temp.channel_count;
        b8 has_transparency = FALSE;
        for (u32 i = 0; i < total_size; i += temp.channel_count) {
            u8 alpha = data[i + 3];
            if (alpha < 255) {
                has_transparency = TRUE;
                break;
            }
        }

        string_ncopy(temp.name, filename, TEXTURE_NAME_MAX_LENGTH);
        temp.generation = INVALID_ID;
        temp.has_transparency = has_transparency;

        renderer_frontend_create_texture(data, &temp);

        texture old = *t;
        *t = temp;
        renderer_frontend_destroy_texture(&old);

        if (t->generation == INVALID_ID) {
            t->generation = 0;
        } else {
            t->generation = t->generation + 1;
        }

        stbi_image_free(data);

        return TRUE;
    }

    char const* failure = stbi_failure_reason();
    if (failure != 0) {
        LOG_WARNING("load_texture: Failed to load texture '%s': %s", absolute_path, failure);
    }

    return FALSE;
}

void destroy_texture(texture* t)
{
    renderer_frontend_destroy_texture(t);

    memory_zero(t->name, sizeof(char) * TEXTURE_NAME_MAX_LENGTH);
    memory_zero(t, sizeof(texture));
    t->id = INVALID_ID;
    t->generation = INVALID_GEN;
}
