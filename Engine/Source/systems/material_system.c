#include "material_system.h"

#include "memory_system.h"
#include "resource_system.h"


// #include "containers/hash_table.h"
// #include "core/logger.h"
// #include "core/string_utils.h"
// #include "renderer/renderer_frontend.h"
// #include "systems/texture_system.h"
// #include "third_party/cglm/cglm.h"

typedef struct Material_System_State
{
    u32 max_material_count;
    Material* materials;
    bool* empty_slots;

    // Material default_material;
    // hashtable material_reference_table;
} Material_System_State;

// typedef struct material_reference {
//     u64 reference_count;
//     u32 handle;
//     b8 auto_release;
// } material_reference;

static Material_System_State* state;

// static b8 create_default_material();
// static b8 load_material(Material_Config config, Material* m);
// static void destroy_material(Material* m);



static bool load_from_config(const char* filename);

bool material_system_startup(Material_System_Config* config)
{
    if (!config || config->max_material_count == 0)
    {
        LOG_FATAL("material_system_startup: Invalid parameters");
        return false;
    }

    state = memory_system_allocate(sizeof(*state), MEMORY_TAG_SYSTEMS);
    state->max_material_count = config->max_material_count;
    state->materials = memory_system_allocate(config->max_material_count * sizeof(*state->materials), MEMORY_TAG_SYSTEMS);
    state->empty_slots = memory_system_allocate(config->max_material_count * sizeof(*state->empty_slots), MEMORY_TAG_SYSTEMS);
    memory_system_set(state->empty_slots, true, config->max_material_count * sizeof(*state->empty_slots));

    // Create default material



    void* array_block = (char*)state + state_struct_size_in_bytes;
    state->materials = array_block;

    void* hash_table_block = (char*)array_block + array_size_in_bytes;

    hashtable_create(
        sizeof(material_reference),
        config.max_material_count,
        hash_table_block, FALSE,
        &state->material_reference_table);

    material_reference invalid_ref;
    invalid_ref.auto_release = FALSE;
    invalid_ref.handle = INVALID_ID;  // Primary reason for needing default values.
    invalid_ref.reference_count = 0;
    hashtable_fill(&state->material_reference_table, &invalid_ref);

    for (u32 i = 0; i < state->config.max_material_count; ++i) {
        state->materials[i].id = INVALID_ID;
        state->materials[i].generation = INVALID_ID;
        state->materials[i].backend_id = INVALID_ID;
    }

    if (!create_default_material(state)) {
        LOG_FATAL("Failed to create default material. Application cannot continue.");
        return FALSE;
    }

    return TRUE;
}

void material_system_shutdown()
{
    if (state) {
        for (u32 i = 0; i < state->config.max_material_count; ++i) {
            if (state->materials[i].id != INVALID_ID) {
                destroy_material(&state->materials[i]);
            }
        }

        destroy_material(&state->default_material);
    }

    state = 0;
}

Material* material_system_acquire(char const* name)
{
    Resource_Data mat_resource;
    if (!resource_system_load(name, RESOURCE_TYPE_MATERIAL, &mat_resource))
    {
        LOG_ERROR("material_system_acquire: Failed to load material resource, returning nullptr");
        return 0;
    }

    Material* material;
    if (mat_resource.data)
    {
        material = material_system_acquire_from_config(*(Material_Config*)mat_resource.data);
    }

    resource_system_unload(&mat_resource);

    if (!material)
    {
        LOG_ERROR("material_system_acquire: Failed to load material resource, returning nullptr");
        return 0;
    }

    return material;
}

Material* material_system_acquire_from_config(Material_Config config)
{
    // Return default material.
    if (string_equali(config.name, DEFAULT_MATERIAL_NAME)) {
        return &state->default_material;
    }

    material_reference ref;
    if (state && hashtable_get(&state->material_reference_table, config.name, &ref)) {
        // This can only be changed the first time a material is loaded.
        if (ref.reference_count == 0) {
            ref.auto_release = config.auto_release;
        }
        ref.reference_count++;

        if (ref.handle == INVALID_ID) {
            // This means no material exists here. Find a free index first.
            u32 count = state->config.max_material_count;
            Material* m = 0;
            for (u32 i = 0; i < count; ++i) {
                if (state->materials[i].id == INVALID_ID) {
                    // A free slot has been found. Use its index as the handle.
                    ref.handle = i;
                    m = &state->materials[i];
                    break;
                }
            }

            // Make sure an empty slot was actually found.
            if (!m || ref.handle == INVALID_ID) {
                LOG_FATAL("material_system_acquire_material - Material system cannot hold anymore materials. Adjust configuration to allow more.");
                return 0;
            }

            // Create new material.
            if (!load_material(config, m)) {
                LOG_ERROR("Failed to load material '%s'.", config.name);
                return 0;
            }

            if (m->generation == INVALID_ID) {
                m->generation = 0;
            } else {
                m->generation++;
            }

            // Also use the handle as the material id.
            m->id = ref.handle;
            LOG_TRACE("Material '%s' does not yet exist. Created, and ref_count is now %i.", config.name, ref.reference_count);
        } else {
            LOG_TRACE("Material '%s' already exists, ref_count increased to %i.", config.name, ref.reference_count);
        }

        // Update the entry.
        hashtable_set(&state->material_reference_table, config.name, &ref);
        return &state->materials[ref.handle];
    }

    // NOTE: This would only happen in the event something went wrong with the state.
    LOG_ERROR("material_system_acquire_from_config failed to acquire material '%s'. Null pointer will be returned.", config.name);
    return 0;
}

void material_system_release(char const* name)
{
    // Ignore release requests for the default material.
    if (string_equali(name, DEFAULT_MATERIAL_NAME)) {
        return;
    }
    material_reference ref;
    if (state && hashtable_get(&state->material_reference_table, name, &ref)) {
        if (ref.reference_count == 0) {
            LOG_WARNING("Tried to release non-existent material: '%s'", name);
            return;
        }
        ref.reference_count--;
        if (ref.reference_count == 0 && ref.auto_release) {
            Material* m = &state->materials[ref.handle];

            // Destroy/reset material.
            destroy_material(m);

            // Reset the reference.
            ref.handle = INVALID_ID;
            ref.auto_release = FALSE;
            LOG_TRACE("Released material '%s'., Material unloaded because reference count=0 and auto_release=TRUE.", name);
        } else {
            LOG_TRACE("Released material '%s', now has a reference count of '%i' (auto_release=%s).", name, ref.reference_count, ref.auto_release ? "TRUE" : "FALSE");
        }

        // Update the entry.
        hashtable_set(&state->material_reference_table, name, &ref);
    } else {
        LOG_ERROR("material_system_release failed to release material '%s'.", name);
    }
}

Material* material_system_get_default_material()
{
    if (state) {
        return &state->default_material;
    }

    LOG_FATAL("material_system_get_default_material: Called before system_state is initialized");
    return 0;
}

b8 load_material(Material_Config config, Material* m)
{
    memory_zero(m, sizeof(Material));

    // name
    string_ncopy(m->name, config.name, MAX_MATERIAL_NAME_LENGTH);

    m->type = config.type;

    // Diffuse colour
    glm_vec4_copy(config.diffuse_color, m->diffuse_color);

    // Diffuse map
    if (string_length(config.diffuse_texture_name) > 0) {
        m->diffuse_map.use = TEXTURE_USE_MAP_DIFFUSE;
        m->diffuse_map.texture = texture_system_acquire(config.diffuse_texture_name, TRUE);
        if (!m->diffuse_map.texture) {
            LOG_WARNING("Unable to load texture '%s' for material '%s', using default.", config.diffuse_texture_name, m->name);
            m->diffuse_map.texture = texture_system_get_default_texture();
        }
    } else {
        // NOTE: Only set for clarity, as call to memory_zero above does this already.
        m->diffuse_map.use = TEXTURE_USE_UNKNOWN;
        m->diffuse_map.texture = 0;
    }

    // TODO: other maps

    // Send it off to the renderer to acquire resources.
    if (!renderer_frontend_create_material(m)) {
        LOG_ERROR("Failed to acquire renderer resources for material '%s'.", m->name);
        return FALSE;
    }

    return TRUE;
}

void destroy_material(Material* m)
{
    LOG_TRACE("Destroying material '%s'...", m->name);

    // Release texture references.
    if (m->diffuse_map.texture) {
        texture_system_release(m->diffuse_map.texture->name);
    }

    // Release renderer resources.
    renderer_frontend_destroy_material(m);

    // Zero it out, invalidate IDs.
    memory_zero(m, sizeof(Material));
    m->id = INVALID_ID;
    m->generation = INVALID_ID;
    m->backend_id = INVALID_ID;
}

b8 create_default_material()
{
    memory_zero(&state->default_material, sizeof(Material));
    state->default_material.id = INVALID_ID;
    state->default_material.generation = INVALID_ID;

    string_ncopy(state->default_material.name, DEFAULT_MATERIAL_NAME, MAX_MATERIAL_NAME_LENGTH);
    glm_vec4_one(state->default_material.diffuse_colour);

    state->default_material.diffuse_map.use = TEXTURE_USE_MAP_DIFFUSE;
    state->default_material.diffuse_map.texture = texture_system_get_default_texture();

    if (!renderer_frontend_create_material(&state->default_material)) {
        LOG_FATAL("create_default_material: Failed to acquire renderer resources for default texture");
        return FALSE;
    }

    return TRUE;
}


bool load_from_config(const char* filename)
{
    i32 slot = -1;
    for (u32 i = 0; i < state->max_material_count; ++i)
    {
        if (state->empty_slots[i])
        {
            slot = i;
            state->empty_slots[i] = false;
            break;
        }
    }

    if (slot == -1)
    {
        LOG_FATAL("load_from_config: Failed to find empty slot");
        return false;
    }

    Material_Config config;
    if (!resource_system_load(filename, slot, true, &config))
    {
        LOG_FATAL("load_from_config: Failed to load material from config");
        return false;
    }

    Material* material = &state->materials[slot];
    Material_Config* config = resource.data;
    new_material->diffuse_color = config->diffuse_color;




}
