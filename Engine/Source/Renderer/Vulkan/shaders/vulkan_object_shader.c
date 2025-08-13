#include "vulkan_object_shader.h"
#include "core/logger.h"
#include "Renderer/Vulkan/vulkan_shader_utils.h"
#include "math/math_types.h"
#include "Renderer/Vulkan/vulkan_pipeline.h"
#include "Renderer/Vulkan/vulkan_buffer.h"

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

    VkDescriptorSetLayoutBinding descriptor_set_layout_binding = {};
    descriptor_set_layout_binding.binding = 0;
    descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_set_layout_binding.descriptorCount = 1;
    descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    descriptor_set_layout_binding.pImmutableSamplers = 0;

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {};
    descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_info.pNext = 0;
    descriptor_set_layout_create_info.flags = 0;
    descriptor_set_layout_create_info.bindingCount = 1;
    descriptor_set_layout_create_info.pBindings = &descriptor_set_layout_binding;
    VK_CHECK(vkCreateDescriptorSetLayout(
        context->device.handle,
        &descriptor_set_layout_create_info,
        context->allocator,
        &shader->global_descriptor_set_layout));

    VkDescriptorPoolSize global_pool_size = {};
    global_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    global_pool_size.descriptorCount = context->swapchain.images.size;

    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {};
    descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_create_info.pNext = 0;
    descriptor_pool_create_info.flags = 0;
    descriptor_pool_create_info.maxSets = context->swapchain.images.size;
    descriptor_pool_create_info.poolSizeCount = 1;
    descriptor_pool_create_info.pPoolSizes = &global_pool_size;
    VK_CHECK(vkCreateDescriptorPool(
        context->device.handle,
        &descriptor_pool_create_info,
        context->allocator,
        &shader->global_descriptor_pool));

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

    VkDescriptorSetLayout descriptor_set_layouts[] = { shader->global_descriptor_set_layout };

    if(!vulkan_pipeline_create(
        context,
        &context->main_renderpass,
        _countof(vertexAttributeDescriptions), vertexAttributeDescriptions,
        _countof(descriptor_set_layouts), descriptor_set_layouts,
        OBJECT_SHADER_STAGE_COUNT, shader_stage_create_info,
        viewport, scissor,
        FALSE,
        &shader->pipeline)) {
        LOG_ERROR("Failed to create pipeline");
        return FALSE;
    }

    if (!vulkan_buffer_create(
        context,
        sizeof(shader->global_data),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        TRUE, &shader->global_ubo)) {
        LOG_ERROR("Failed to create buffer");
        return FALSE;
    }

    VkDescriptorSetLayout global_layouts[3] = {
        shader->global_descriptor_set_layout,
        shader->global_descriptor_set_layout,
        shader->global_descriptor_set_layout };

    VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {};
    descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_set_allocate_info.pNext = 0;
    descriptor_set_allocate_info.descriptorPool = shader->global_descriptor_pool;
    descriptor_set_allocate_info.descriptorSetCount = _countof(global_layouts);
    descriptor_set_allocate_info.pSetLayouts = global_layouts;
    VK_CHECK(vkAllocateDescriptorSets(context->device.handle, &descriptor_set_allocate_info, shader->global_descriptor_sets));


    return TRUE;
}

void vulkan_object_shader_destroy(vulkan_context* context, vulkan_object_shader* shader)
{
    VkDevice device = context->device.handle;

    vulkan_buffer_destroy(context, &shader->global_ubo);
    vulkan_graphics_pipeline_destroy(context, &shader->pipeline); 
    vkDestroyDescriptorPool(context->device.handle, shader->global_descriptor_pool, context->allocator);
    vkDestroyDescriptorSetLayout(context->device.handle, shader->global_descriptor_set_layout, context->allocator);

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

void vulkan_object_shader_update_global_state(vulkan_context* context, vulkan_object_shader* shader)
{
    u32 current_frame = context->current_frame;
    VkCommandBuffer command_buffer = context->command_buffers.data[current_frame].handle;
    VkDescriptorSet descriptor_set = shader->global_descriptor_sets[current_frame];

    VkDeviceSize offset = 0;
    VkDeviceSize range = sizeof(shader->global_data);

    vulkan_buffer_load_data(context, &shader->global_ubo, offset, range, 0, &shader->global_data);

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = shader->global_ubo.handle;
    bufferInfo.offset = offset;
    bufferInfo.range = range;

    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.pNext = 0;
    descriptor_write.dstSet = descriptor_set;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.pImageInfo = 0;
    descriptor_write.pBufferInfo = &bufferInfo;
    descriptor_write.pTexelBufferView = 0;

    vkUpdateDescriptorSets(context->device.handle, 1, &descriptor_write, 0, 0);

    vkCmdBindDescriptorSets(
        command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        shader->pipeline.layout,
        0, 1, &descriptor_set,
        0, 0);
}

void vulkan_object_shader_update_object_state(vulkan_context* context, vulkan_object_shader* shader, mat4 world)
{
    u32 current_frame = context->current_frame;
    VkCommandBuffer command_buffer = context->command_buffers.data[current_frame].handle;

    vkCmdPushConstants(command_buffer, shader->pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(world), &world);
}
