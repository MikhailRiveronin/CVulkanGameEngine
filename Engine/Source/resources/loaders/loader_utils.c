#include "loader_utils.h"

#include "core/logger.h"
#include "core/string_utils.h"

b8 resource_unload(struct Resource_Loader* self, Resource* resource, memory_tag tag)
{
    if (!self || !resource) {
        LOG_WARNING("resource_unload called with nullptr for self or resource.");
        return FALSE;
    }

    u32 path_length = string_length(resource->complete_path);
    if (path_length) {
        memory_free(resource->complete_path, sizeof(char) * path_length + 1, MEMORY_TAG_STRING);
    }

    if (resource->data) {
        memory_free(resource->data, resource->data_size, tag);
        resource->data = 0;
        resource->data_size = 0;
        resource->loader_id = INVALID_ID;
    }

    return TRUE;
}
