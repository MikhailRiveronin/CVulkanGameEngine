#include "vulkan_shader_utils.h"

#include "core/string_utils.h"
#include "core/logger.h"
#include "core/memory_utils.h"

#include "systems/resource_system.h"

b8 create_shader_module(
    vulkan_context* context,
    char const* name,
    char const* type_str,
    VkShaderStageFlagBits shader_stage_flag,
    u32 stage_index,
    vulkan_shader_stage* shader_stages)
{
    // Build file name.
    char file_name[512];
    string_format(file_name, "shaders/%s.%s.spv", name, type_str);

    // Read the resource.
    resource binary_resource;
    if (!resource_system_load(file_name, RESOURCE_TYPE_BINARY, &binary_resource)) {
        LOG_ERROR("Unable to read shader module: %s.", file_name);
        return FALSE;
    }

    memory_zero(&shader_stages[stage_index].create_info, sizeof(VkShaderModuleCreateInfo));
    shader_stages[stage_index].create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    // Use the resource's size and data directly.
    shader_stages[stage_index].create_info.codeSize = binary_resource.data_size;
    shader_stages[stage_index].create_info.pCode = (u32*)binary_resource.data;


    VK_CHECK(vkCreateShaderModule(
        context->device.handle,
        &shader_stages[stage_index].create_info,
        context->allocator,
        &shader_stages[stage_index].handle));

    // Release the resource.
    resource_system_unload(&binary_resource);

    // Shader stage info
    memory_zero(&shader_stages[stage_index].shader_stage_create_info, sizeof(VkPipelineShaderStageCreateInfo));
    shader_stages[stage_index].shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[stage_index].shader_stage_create_info.stage = shader_stage_flag;
    shader_stages[stage_index].shader_stage_create_info.module = shader_stages[stage_index].handle;
    shader_stages[stage_index].shader_stage_create_info.pName = "main";

    return TRUE;
}
