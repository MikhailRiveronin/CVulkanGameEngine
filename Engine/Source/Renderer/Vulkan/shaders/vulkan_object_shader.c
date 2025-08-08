#include "vulkan_object_shader.h"
#include "Core/logger.h"
#include "Renderer/Vulkan/vulkan_shader_utils.h"
#include "math/math_types.h"
#include "Renderer/Vulkan/vulkan_pipeline.h"

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"

b8 vulkan_object_shader_create(vulkan_context* context, vulkan_object_shader* shader)
{
    char stage_type_strs[OBJECT_SHADER_STAGE_COUNT][5] = { "vert", "frag" };
    VkShaderStageFlagBits stage_types[OBJECT_SHADER_STAGE_COUNT] = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT };

    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        if (!create_shader_module(context, BUILTIN_SHADER_NAME_OBJECT, stage_type_strs[i], stage_types[i], i, shader->stages)) {
            LOG_ERROR("Unable to create %s shader module for '%s'.", stage_type_strs[i], BUILTIN_SHADER_NAME_OBJECT);
            return FALSE;
        }
    }

    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = context->framebuffer_width;
    viewport.height = context->framebuffer_height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = context->framebuffer_width;
    scissor.extent.height = context->framebuffer_height;

    VkVertexInputAttributeDescription vertexAttributeDescriptions[1];
    vertexAttributeDescriptions[0].location = 0;
    vertexAttributeDescriptions[0].binding = 0;
    vertexAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttributeDescriptions[0].offset = offsetof(vertex_3d, position);

    VkPipelineShaderStageCreateInfo shader_stage_create_info[OBJECT_SHADER_STAGE_COUNT];
    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        shader_stage_create_info[i] = shader->stages[i].shader_stage_create_info;
    }

    if(!vulkan_pipeline_create(
        context,
        &context->main_renderpass,
        _countof(vertexAttributeDescriptions), vertexAttributeDescriptions,
        0, 0,
        OBJECT_SHADER_STAGE_COUNT, shader_stage_create_info,
        viewport, scissor,
        FALSE,
        &shader->pipeline)) {
        LOG_ERROR("Failed to create pipeline");
        return FALSE;
    }
    return TRUE;
}

void vulkan_object_shader_destroy(vulkan_context* context, vulkan_object_shader* shader)
{
    vulkan_graphics_pipeline_destroy(context, &shader->pipeline);

    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        vkDestroyShaderModule(context->device.handle, shader->stages[i].handle, context->allocator);
        shader->stages[i].handle = 0;
    }
}

void vulkan_object_shader_use(vulkan_context* context, vulkan_object_shader* shader)
{
    u32 frame_index = context->current_frame;
    vulkan_pipeline_bind(
        &context->command_buffers.data[frame_index],
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        &shader->pipeline);
}
