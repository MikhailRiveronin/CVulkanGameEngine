#include "binary_loader.h"

#include "config.h"
#include "core/logger.h"
#include "core/string_utils.h"
#include "platform/filesystem.h"
#include "systems/memory_system.h"

static bool load(char const* filename, Resource* resource);
static void unload(Resource* resource);

Resource_Loader* binary_loader_create()
{
    Resource_Loader* loader = memory_system_allocate(sizeof(*loader), MEMORY_TAG_LOADERS);
    loader->load = load;
    loader->unload = unload;
    return loader;
}

bool load(char const* filename, Resource* resource)
{
    if (!filename || !resource)
    {
        LOG_FATAL("binary_loader load: Invalid parameters");
        return false;
    }

    char path[256];
    string_format(path, "%s/%s/%s", ASSETS_DIR, "shaders/spirv", filename);
    File_Handle handle;
    if (!filesystem_open(path, FILE_ACCESS_MODE_READ_BINARY, &handle))
    {
        LOG_FATAL("binary_loader load: Failed to open %s for binary reading", path);
        return false;
    }

    u32 data_size = filesystem_size(&handle);
    u8* data = memory_system_allocate(data_size, MEMORY_TAG_RESOURCES);
    if (!filesystem_read(&handle, data, data_size))
    {
        LOG_FATAL("binary_loader load: Failed to read %s", path);
        filesystem_close(&handle);
        return false;
    }

    filesystem_close(&handle);
    resource->data = data;
    resource->data_size = data_size;
    return true;
}

void unload(Resource* resource)
{
    if (!resource)
    {
        LOG_WARNING("binary_loader unload: Invalid parameters");
    }

    memory_system_free(resource->data, resource->data_size, MEMORY_TAG_RESOURCES);
    resource->data = 0;
    resource->data_size = 0;
}
