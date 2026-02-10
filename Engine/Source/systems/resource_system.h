#pragma once

#include "defines.h"
#include "resources/resource_types.h"

bool resource_system_startup();
void resource_system_shutdown();

bool resource_system_load(Resource_Type type, char const* filename, Resource* resource);
void resource_system_unload(Resource* resource);
