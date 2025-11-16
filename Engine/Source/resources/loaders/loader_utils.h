#pragma once

#include "defines.h"
#include "systems/memory_system.h"
#include "resources/resources.h"

struct Resource_Loader;

b8 resource_unload(struct Resource_Loader* self, Resource* resource, memory_tag tag);
