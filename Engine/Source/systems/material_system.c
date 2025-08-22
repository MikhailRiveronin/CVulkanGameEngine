#include "material_system.h"

#include "core/logger.h"
#include "core/string_utils.h"
#include "containers/hash_table.h"
#include "math/kmath.h"
#include "renderer/renderer_frontend.h"
#include "systems/texture_system.h"

// TODO: temp: resource system
#include "platform/filesystem.h"
// end temp

typedef struct material_system_state {
    material_system_config config;

    material default_material;
    material* registered_materials;
    hash_table material_reference_table;
} material_system_state;

typedef struct material_reference {
    u64 reference_count;
    u32 handle;
    b8 auto_release;
} material_reference;

static material_system_state* system_state;

static b8 create_default_material();
static b8 load_material(material_config config, material* m);
static void destroy_material(material* m);
static b8 load_configuration_file(const char* path, material_config* config);

b8 material_system_startup(u64* required_memory, void* memory, material_system_config config)
{
    if (config.max_material_count == 0) {
        KFATAL("material_system_startup: config.max_material_count must be > 0");
        return FALSE;
    }

    u64 struct_requirement = sizeof(material_system_state);
    u64 array_requirement = sizeof(material) * config.max_material_count;
    u64 hashtable_requirement = sizeof(material_reference) * config.max_material_count;
    *required_memory = struct_requirement + array_requirement + hashtable_requirement;

    if (system_state == 0) {
        return TRUE;
    }

    system_state = memory;
    system_state->config = config;

    void* array_block = (char*)system_state + struct_requirement;
    system_state->registered_materials = array_block;

    void* hash_table_block = (char*)array_block + array_requirement;

    hash_table_create(
        sizeof(material_reference),
        config.max_material_count,
        hash_table_block, FALSE,
        &system_state->material_reference_table);

    material_reference invalid_ref;
    invalid_ref.auto_release = FALSE;
    invalid_ref.handle = INVALID_ID;  // Primary reason for needing default values.
    invalid_ref.reference_count = 0;
    hashtable_fill(&system_state->material_reference_table, &invalid_ref);

    for (u32 i = 0; i < system_state->config.max_material_count; ++i) {
        system_state->registered_materials[i].id = INVALID_ID;
        system_state->registered_materials[i].generation = INVALID_ID;
        system_state->registered_materials[i].internal_id = INVALID_ID;
    }

    if (!create_default_material(system_state)) {
        KFATAL("Failed to create default material. Application cannot continue.");
        return FALSE;
    }

    return TRUE;
}

void material_system_shutdown()
{
    if (system_state) {
        for (u32 i = 0; i < system_state->config.max_material_count; ++i) {
            if (system_state->registered_materials[i].id != INVALID_ID) {
                destroy_material(&system_state->registered_materials[i]);
            }
        }

        destroy_material(&system_state->default_material);
    }

    system_state = 0;
}

material* material_system_acquire(const char* name)
{
    material_config config;

    char* format_str = "assets/materials/%s.%s";
    char full_file_path[512];

    // TODO: try different extensions
    string_format(full_file_path, format_str, name, "kmt");
    if (!load_configuration_file(full_file_path, &config)) {
        LOG_ERROR(
            "material_system_acquire: Failed to load material file: '%s'. Null pointer will be returned",
            full_file_path);
        return 0;
    }

    return material_system_acquire_from_config(config);
}

material* material_system_acquire_from_config(material_config config)
{
    // Return default material.
    if (strings_equali(config.name, DEFAULT_MATERIAL_NAME)) {
        return &system_state->default_material;
    }

    material_reference ref;
    if (system_state && hash_table_get(&system_state->material_reference_table, config.name, &ref)) {
        // This can only be changed the first time a material is loaded.
        if (ref.reference_count == 0) {
            ref.auto_release = config.auto_release;
        }
        ref.reference_count++;

        if (ref.handle == INVALID_ID) {
            // This means no material exists here. Find a free index first.
            u32 count = system_state->config.max_material_count;
            material* m = 0;
            for (u32 i = 0; i < count; ++i) {
                if (system_state->registered_materials[i].id == INVALID_ID) {
                    // A free slot has been found. Use its index as the handle.
                    ref.handle = i;
                    m = &system_state->registered_materials[i];
                    break;
                }
            }

            // Make sure an empty slot was actually found.
            if (!m || ref.handle == INVALID_ID) {
                KFATAL("material_system_acquire - Material system cannot hold anymore materials. Adjust configuration to allow more.");
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
        hash_table_set(&system_state->material_reference_table, config.name, &ref);
        return &system_state->registered_materials[ref.handle];
    }

    // NOTE: This would only happen in the event something went wrong with the state.
    LOG_ERROR("material_system_acquire_from_config failed to acquire material '%s'. Null pointer will be returned.", config.name);
    return 0;
}

void material_system_release(const char* name)
{
    // Ignore release requests for the default material.
    if (strings_equali(name, DEFAULT_MATERIAL_NAME)) {
        return;
    }
    material_reference ref;
    if (system_state && hash_table_get(&system_state->material_reference_table, name, &ref)) {
        if (ref.reference_count == 0) {
            LOG_WARNING("Tried to release non-existent material: '%s'", name);
            return;
        }
        ref.reference_count--;
        if (ref.reference_count == 0 && ref.auto_release) {
            material* m = &system_state->registered_materials[ref.handle];

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
        hash_table_set(&system_state->material_reference_table, name, &ref);
    } else {
        LOG_ERROR("material_system_release failed to release material '%s'.", name);
    }
}

b8 load_material(material_config config, material* m)
{
    memory_zero(m, sizeof(material));

    // name
    string_ncopy(m->name, config.name, MATERIAL_NAME_MAX_LENGTH);

    // Diffuse colour
    m->diffuse_colour = config.diffuse_colour;

    // Diffuse map
    if (string_length(config.diffuse_map_name) > 0) {
        m->diffuse_map.use = TEXTURE_USE_MAP_DIFFUSE;
        m->diffuse_map.texture = texture_system_acquire(config.diffuse_map_name, TRUE);
        if (!m->diffuse_map.texture) {
            LOG_WARNING("Unable to load texture '%s' for material '%s', using default.", config.diffuse_map_name, m->name);
            m->diffuse_map.texture = texture_system_get_default_texture();
        }
    } else {
        // NOTE: Only set for clarity, as call to memory_zero above does this already.
        m->diffuse_map.use = TEXTURE_USE_UNKNOWN;
        m->diffuse_map.texture = 0;
    }

    // TODO: other maps

    // Send it off to the renderer to acquire resources.
    if (!renderer_create_material(m)) {
        LOG_ERROR("Failed to acquire renderer resources for material '%s'.", m->name);
        return FALSE;
    }

    return TRUE;
}

void destroy_material(material* m)
{
    LOG_TRACE("Destroying material '%s'...", m->name);

    // Release texture references.
    if (m->diffuse_map.texture) {
        texture_system_release(m->diffuse_map.texture->name);
    }

    // Release renderer resources.
    renderer_destroy_material(m);

    // Zero it out, invalidate IDs.
    memory_zero(m, sizeof(material));
    m->id = INVALID_ID;
    m->generation = INVALID_ID;
    m->internal_id = INVALID_ID;
}

b8 create_default_material()
{
    memory_zero(&system_state->default_material, sizeof(material));
    system_state->default_material.id = INVALID_ID;
    system_state->default_material.generation = INVALID_ID;
    string_ncopy(system_state->default_material.name, DEFAULT_MATERIAL_NAME, MATERIAL_NAME_MAX_LENGTH);
    system_state->default_material.diffuse_colour = vec4_one();  // white
    system_state->default_material.diffuse_map.use = TEXTURE_USE_MAP_DIFFUSE;
    system_state->default_material.diffuse_map.texture = texture_system_get_default_texture();

    if (!renderer_create_material(&system_state->default_material)) {
        KFATAL("Failed to acquire renderer resources for default texture. Application cannot continue.");
        return FALSE;
    }

    return TRUE;
}

b8 load_configuration_file(const char* path, material_config* config)
{
    file_handle f;
    if (!filesystem_open(path, FILE_MODE_READ, FALSE, &f)) {
        KERROR("load_configuration_file - unable to open material file for reading: '%s'.", path);
        return FALSE;
    }

    // Read each line of the file.
    char line_buf[512] = "";
    char* p = &line_buf[0];
    u64 line_length = 0;
    u32 line_number = 1;
    while (filesystem_read_line(&f, 511, &p, &line_length)) {
        // Trim the string.
        char* trimmed = string_trim(line_buf);

        // Get the trimmed length.
        line_length = string_length(trimmed);

        // Skip blank lines and comments.
        if (line_length < 1 || trimmed[0] == '#') {
            line_number++;
            continue;
        }

        // Split into var/value
        i32 equal_index = string_index_of(trimmed, '=');
        if (equal_index == -1) {
            LOG_WARNING("Potential formatting issue found in file '%s': '=' token not found. Skipping line %ui.", path, line_number);
            line_number++;
            continue;
        }

        // Assume a max of 64 characters for the variable name.
        char raw_var_name[64];
        memory_zero(raw_var_name, sizeof(char) * 64);
        string_mid(raw_var_name, trimmed, 0, equal_index);
        char* trimmed_var_name = string_trim(raw_var_name);

        // Assume a max of 511-65 (446) for the max length of the value to account for the variable name and the '='.
        char raw_value[446];
        memory_zero(raw_value, sizeof(char) * 446);
        string_mid(raw_value, trimmed, equal_index + 1, -1);  // Read the rest of the line
        char* trimmed_value = string_trim(raw_value);

        // Process the variable.
        if (strings_equali(trimmed_var_name, "version")) {
            // TODO: version
        } else if (strings_equali(trimmed_var_name, "name")) {
            string_ncopy(config->name, trimmed_value, MATERIAL_NAME_MAX_LENGTH);
        } else if (strings_equali(trimmed_var_name, "diffuse_map_name")) {
            string_ncopy(config->diffuse_map_name, trimmed_value, TEXTURE_NAME_MAX_LENGTH);
        } else if (strings_equali(trimmed_var_name, "diffuse_colour")) {
            // Parse the colour
            if (!string_to_vec4(trimmed_value, &config->diffuse_colour)) {
                LOG_WARNING("Error parsing diffuse_colour in file '%s'. Using default of white instead.", path);
                config->diffuse_colour = vec4_one();  // white
            }
        }

        // TODO: more fields.

        // Clear the line buffer.
        memory_zero(line_buf, sizeof(char) * 512);
        line_number++;
    }

    filesystem_close(&f);

    return TRUE;
}
