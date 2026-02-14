#include "image_loader.h"

#include "config.h"
#include "core/logger.h"
#include "core/string_utils.h"
#include "systems/memory_system.h"
#include "third_party/stb_image.h"

static bool load(char const* filename, Resource* resource);
static void unload(Resource* resource);

Resource_Loader* image_loader_create()
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
        LOG_FATAL("image_loader load: Invalid parameters");
        return false;
    }

    char path[256];
    string_format(path, "%s/%s/%s", ASSETS_DIR, "materials", filename);
    i32 required_channel_count = 4;
    i32 width;
    i32 height;
    i32 channel_count;
    u8* pixels = stbi_load(path, &width, &height, &channel_count, required_channel_count);
    if (!pixels)
    {
        LOG_FATAL("image_loader load: Failed to load image %s", path);
        return false;
    }

    Image_Resource* image = memory_system_allocate(sizeof(*image), MEMORY_TAG_RESOURCES);
    image->pixels = pixels;
    image->width = width;
    image->height = height;
    image->channel_count = channel_count;
    stbi_image_free(pixels);

    resource->data = image;
    resource->size = sizeof(*image);
    return true;
}

void unload(Resource* resource)
{
    if (!resource)
    {
        LOG_WARNING("image_loader unload: Invalid parameters");
    }

    memory_system_free(resource->data, resource->size, MEMORY_TAG_RESOURCES);
    resource->data = 0;
    resource->size = 0;
}
