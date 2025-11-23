#pragma once

#include "defines.h"
#include "vulkan_structures.h"

/**
 * Returns the string representation of result.
 * @param result The result to get the string for.
 * @param get_extended Indicates whether to also return an extended result.
 * @return The error code and/or extended error message in string form. Defaults to success for unknown result types.
 */
char const* vulkan_result_string(VkResult result, b8 get_extended);

/**
 * Inticates if the passed result is a success or an error as defined by the Vulkan spec.
 * @return True if success; otherwise false. Defaults to true for unknown result types.
 */
b8 vulkan_result_is_success(VkResult result);

b8 create_shader_module(
    vulkan_context* context,
    char const* name,
    char const* stage_str,
    VkShaderStageFlagBits stage_type,
    u32 stage_index,
    vulkan_shader_stage* stages);
