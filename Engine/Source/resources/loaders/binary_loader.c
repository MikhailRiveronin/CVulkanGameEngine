#include "binary_loader.h"
#include "loader_utils.h"

#include "core/logger.h"
#include "systems/memory_system.h"
#include "core/string_utils.h"
#include "resources/resources.h"
#include "systems/resource_system.h"

#include "platform/filesystem.h"

b8 binary_loader_load(struct Resource_Loader* self, char const* name, Resource* out_resource) {
    if (!self || !name || !out_resource) {
        return FALSE;
    }

    char* format_str = "%s/%s";
    char full_file_path[512];
    string_format(full_file_path, format_str, "D:/Projects/CVulkanGameEngine/build/assets", name);

    File_Handle f;
    if (!filesystem_open(full_file_path, ACCESS_MODE_READ, &f)) {
        LOG_ERROR("binary_loader_load - unable to open file for binary reading: '%s'.", full_file_path);
        return FALSE;
    }

    // TODO: Should be using an allocator here.
    out_resource->complete_path = string_duplicate(full_file_path);

    u64 file_size = 0;
    if (!filesystem_size(&f, &file_size)) {
        LOG_ERROR("Unable to binary read file: %s.", full_file_path);
        filesystem_close(&f);
        return FALSE;
    }

    // TODO: Should be using an allocator here.
    u8* resource_data = memory_allocate(sizeof(u8) * file_size, MEMORY_TAG_ARRAY);
    u64 read_size = 0;
    if (!filesystem_read_all(&f, resource_data, &read_size)) {
        LOG_ERROR("Unable to binary read file: %s.", full_file_path);
        filesystem_close(&f);
        return FALSE;
    }

    filesystem_close(&f);

    out_resource->data = resource_data;
    out_resource->data_size = read_size;
    out_resource->name = name;

    return TRUE;
}

void binary_loader_unload(struct Resource_Loader* self, Resource* resource) {
    if (!resource_unload(self, resource, MEMORY_TAG_ARRAY)) {
        LOG_WARNING("binary_loader_unload called with nullptr for self or resource.");
    }
}

Resource_Loader binary_loader_create() {
    Resource_Loader loader;
    loader.type = RESOURCE_TYPE_BINARY;
    loader.load = binary_loader_load;
    loader.unload = binary_loader_unload;
    loader.resource_type_subfolder = "";

    return loader;
}