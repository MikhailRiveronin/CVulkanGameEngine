#include "vulkan_material_shader.h"
#include "core/logger.h"
#include "renderer/vulkan/vulkan_shader_utils.h"
#include "math/math_types.h"
#include "renderer/vulkan/vulkan_pipeline.h"
#include "renderer/vulkan/vulkan_buffer.h"
#include "math/math_types.h"
#include "math/kmath.h"

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"

VkDeviceSize calculateUniformBufferAlignment(vulkan_context* context, VkDeviceSize size);

b8 vulkan_material_shader_create(vulkan_context* context, vulkan_material_shader* shader)
{
    char stage_type_strs[MATERIAL_SHADER_STAGE_COUNT][5] = { "vert", "frag" };
    VkShaderStageFlagBits stage_types[MATERIAL_SHADER_STAGE_COUNT] = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT };

    for (u32 i = 0; i < MATERIAL_SHADER_STAGE_COUNT; ++i) {
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

    // Per object descriptors
    VkDescriptorType descriptor_types[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT] = {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // binding 0
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER // binding 1
    };

    VkDescriptorSetLayoutBinding bindings[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT];
    memory_zero(bindings, sizeof(bindings));
    for (u32 i = 0; i < VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT; ++i ) {
        bindings[i].binding = i;
        bindings[i].descriptorCount = 1;
        bindings[i].descriptorType = descriptor_types[i];
        bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    VkDescriptorSetLayoutCreateInfo object_set_layout_create_info = {};
    object_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    object_set_layout_create_info.pNext = 0;
    object_set_layout_create_info.flags = 0;
    object_set_layout_create_info.bindingCount = _countof(bindings);
    object_set_layout_create_info.pBindings = bindings;
    VK_CHECK(vkCreateDescriptorSetLayout(
        context->device.handle,
        &object_set_layout_create_info,
        context->allocator,
        &shader->object_descriptor_set_layout));

    shader->sampler_uses[0] = TEXTURE_USE_MAP_DIFFUSE;

    VkDescriptorPoolSize object_pool_sizes[2];
    object_pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    object_pool_sizes[0].descriptorCount = VULKAN_MAX_MATERIAL_COUNT;
    object_pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    object_pool_sizes[1].descriptorCount = VULKAN_MATERIAL_SHADER_SAMPLER_COUNT * VULKAN_MAX_MATERIAL_COUNT;

    VkDescriptorPoolCreateInfo object_pool_create_info = {};
    object_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    object_pool_create_info.pNext = 0;
    object_pool_create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    object_pool_create_info.maxSets = VULKAN_MAX_MATERIAL_COUNT;
    object_pool_create_info.poolSizeCount = _countof(object_pool_sizes);
    object_pool_create_info.pPoolSizes = object_pool_sizes;
    VK_CHECK(vkCreateDescriptorPool(
        context->device.handle,
        &object_pool_create_info,
        context->allocator,
        &shader->object_descriptor_pool));

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

    VkVertexInputAttributeDescription vertexAttributeDescriptions[2];
    vertexAttributeDescriptions[0].location = 0;
    vertexAttributeDescriptions[0].binding = 0;
    vertexAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttributeDescriptions[0].offset = offsetof(vertex_3d, position);
    vertexAttributeDescriptions[1].location = 1;
    vertexAttributeDescriptions[1].binding = 0;
    vertexAttributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    vertexAttributeDescriptions[1].offset = offsetof(vertex_3d, tex_coord);

    VkPipelineShaderStageCreateInfo shader_stage_create_infos[MATERIAL_SHADER_STAGE_COUNT];
    for (u32 i = 0; i < MATERIAL_SHADER_STAGE_COUNT; ++i) {
        shader_stage_create_infos[i] = shader->stages[i].shader_stage_create_info;
    }

    VkDescriptorSetLayout descriptor_set_layouts[] = {
        shader->global_descriptor_set_layout,
        shader->object_descriptor_set_layout };

    if(!vulkan_pipeline_create(
        context,
        &context->main_renderpass,
        sizeof(vertex_3d),
        _countof(vertexAttributeDescriptions),
        vertexAttributeDescriptions,
        _countof(descriptor_set_layouts),
        descriptor_set_layouts,
        MATERIAL_SHADER_STAGE_COUNT,
        shader_stage_create_infos,
        viewport,
        scissor,
        FALSE,
        TRUE,
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
        LOG_ERROR("Failed to create global uniform buffer");
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
    VK_CHECK(vkAllocateDescriptorSets(
        context->device.handle,
        &descriptor_set_allocate_info,
        shader->global_descriptor_sets));

    if (!vulkan_buffer_create(
        context,
        sizeof(shader->object_data) * VULKAN_MAX_MATERIAL_COUNT,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        TRUE, &shader->object_ubo)) {
        LOG_ERROR("Failed to create object uniform buffer");
        return FALSE;
    }

    return TRUE;
}

void vulkan_material_shader_destroy(vulkan_context* context, vulkan_material_shader* shader)
{
    VkDevice device = context->device.handle;

    vulkan_buffer_destroy(context, &shader->global_ubo);
    vulkan_buffer_destroy(context, &shader->object_ubo);
    vulkan_pipeline_destroy(context, &shader->pipeline); 
    vkDestroyDescriptorPool(context->device.handle, shader->global_descriptor_pool, context->allocator);
    vkDestroyDescriptorSetLayout(context->device.handle, shader->global_descriptor_set_layout, context->allocator);
    vkDestroyDescriptorPool(context->device.handle, shader->object_descriptor_pool, context->allocator);
    vkDestroyDescriptorSetLayout(context->device.handle, shader->object_descriptor_set_layout, context->allocator);


    for (u32 i = 0; i < MATERIAL_SHADER_STAGE_COUNT; ++i) {
        vkDestroyShaderModule(context->device.handle, shader->stages[i].handle, context->allocator);
        shader->stages[i].handle = 0;
    }
}

void vulkan_material_shader_use(vulkan_context* context, vulkan_material_shader* shader)
{
    u32 frame_index = context->current_frame;
    vulkan_pipeline_bind(
        &context->command_buffers.data[frame_index],
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        &shader->pipeline);
}

void vulkan_material_shader_update_global_state(vulkan_context* context, vulkan_material_shader* shader, f32 delta_time)
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

void vulkan_material_shader_set_model(vulkan_context* context, struct vulkan_material_shader* shader, mat4 model)
{
    if (context && shader) {
        u32 image_index = context->current_image;
        VkCommandBuffer command_buffer = context->command_buffers.data[image_index].handle;

        vkCmdPushConstants(command_buffer, shader->pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), &model);
    }
}

void vulkan_material_shader_apply_material(vulkan_context* context, struct vulkan_material_shader* shader, material* material)
{
    if (context && shader) {
        u32 current_frame = context->current_frame;
        VkCommandBuffer command_buffer = context->command_buffers.data[current_frame].handle;

        vulkan_material_shader_instance_state* object_state = &shader->instance_states[material->internal_id];
        VkDescriptorSet descriptor_set = object_state->descriptor_sets[current_frame];

        VkWriteDescriptorSet descriptor_writes[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT];
        memory_zero(descriptor_writes, sizeof(descriptor_writes));

        u32 descriptor_count = 0;
        u32 descriptor_index = 0;

        // Descriptor 0
        u32 range = sizeof(material_uniform_data);
        u32 offset = calculateUniformBufferAlignment(context, sizeof(material_uniform_data)) * material->internal_id;

        material_uniform_data uniform_data;
        static f32 accumulator = 0.f;
        accumulator += context->frame_delta_time;
        f32 s = (ksin(accumulator) + 1.f) / 2.f;
        uniform_data.diffuse_color = vec4_create(s, s, s, 1.f);

        vulkan_buffer_load_data(context, &shader->object_ubo, offset, range, 0, &uniform_data);

        u32* global_ubo_generation = &object_state->descriptor_states[descriptor_index].generations[current_frame];
        if (*global_ubo_generation == INVALID_ID || *global_ubo_generation != material->generation) {
            VkDescriptorBufferInfo bufferInfo = {};
            bufferInfo.buffer = shader->object_ubo.handle;
            bufferInfo.offset = offset;
            bufferInfo.range = range;

            descriptor_writes[descriptor_count].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_writes[descriptor_count].pNext = 0;
            descriptor_writes[descriptor_count].dstSet = descriptor_set;
            descriptor_writes[descriptor_count].dstBinding = 0;
            descriptor_writes[descriptor_count].dstArrayElement = 0;
            descriptor_writes[descriptor_count].descriptorCount = 1;
            descriptor_writes[descriptor_count].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_writes[descriptor_count].pImageInfo = 0;
            descriptor_writes[descriptor_count].pBufferInfo = &bufferInfo;
            descriptor_writes[descriptor_count].pTexelBufferView = 0;

            descriptor_count++;
            global_ubo_generation = &material->generation;
        }

        descriptor_index++;

        u32 sampler_count = 1;
        VkDescriptorImageInfo image_infos[1];
        for (u32 sampler_index = 0; sampler_index < sampler_count; ++sampler_index) {
            texture_use use = shader->sampler_uses[sampler_index];
            texture* t = 0;
            switch (use) {
                case TEXTURE_USE_MAP_DIFFUSE:
                    t = material->diffuse_map.texture;
                    break;
                default:
                    LOG_FATAL("Unable to bind sampler to unknown use.");
                    return;
            }

            u32* descriptor_generation = &object_state->descriptor_states[descriptor_index].generations[current_frame];
            u32* descriptor_id = &object_state->descriptor_states[descriptor_index].ids[current_frame];

            if (t->generation == INVALID_ID) {
                *descriptor_generation = INVALID_ID;
            }

            if (t && (*descriptor_generation != t->generation || *descriptor_generation == INVALID_ID || *descriptor_id != t->id)) {
                vulkan_texture_data* internal_data = t->internal_data;
                image_infos[sampler_index].sampler = internal_data->sampler;
                image_infos[sampler_index].imageView = internal_data->image.view;
                image_infos[sampler_index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                descriptor_writes[descriptor_count].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptor_writes[descriptor_count].pNext = 0;
                descriptor_writes[descriptor_count].dstSet = descriptor_set;
                descriptor_writes[descriptor_count].dstBinding = descriptor_index;
                descriptor_writes[descriptor_count].dstArrayElement = 0;
                descriptor_writes[descriptor_count].descriptorCount = 1;
                descriptor_writes[descriptor_count].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptor_writes[descriptor_count].pImageInfo = &image_infos[sampler_index];
                descriptor_writes[descriptor_count].pBufferInfo = 0;
                descriptor_writes[descriptor_count].pTexelBufferView = 0;

                if (t->generation != INVALID_ID) {
                    *descriptor_generation = t->generation;
                    *descriptor_id = t->id;
                }

                descriptor_count++;
                descriptor_index++;
            }
        }

        if (descriptor_count > 0) {
            vkUpdateDescriptorSets(context->device.handle, descriptor_count, descriptor_writes, 0, 0);
        }

        vkCmdBindDescriptorSets(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            shader->pipeline.layout,
            1, 1, &descriptor_set,
            0, 0);
    }
}

b8 vulkan_material_shader_acquire_resources(vulkan_context* context, vulkan_material_shader* shader, material* material)
{
    material->internal_id = shader->object_ubo_index++;
    vulkan_material_shader_instance_state* instance_state = &shader->instance_states[material->internal_id];
    for (u32 i = 0; i < VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT; ++i) {
        for (u32 j = 0; j < 3; ++j) {
            instance_state->descriptor_states[i].generations[j] = INVALID_ID;
            instance_state->descriptor_states[i].ids[j] = INVALID_ID;
        }
    }

    VkDescriptorSetLayout layouts[3] = {
        shader->object_descriptor_set_layout,
        shader->object_descriptor_set_layout,
        shader->object_descriptor_set_layout };

    VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {};
    descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_set_allocate_info.pNext = 0;
    descriptor_set_allocate_info.descriptorPool = shader->object_descriptor_pool;
    descriptor_set_allocate_info.descriptorSetCount = _countof(layouts);
    descriptor_set_allocate_info.pSetLayouts = layouts;
    VkResult result = vkAllocateDescriptorSets(
        context->device.handle,
        &descriptor_set_allocate_info,
        instance_state->descriptor_sets);

    if (result != VK_SUCCESS) {
        LOG_ERROR("Error allocating descriptor sets in shader!");
        return FALSE;
    }

    return TRUE;
}

void vulkan_material_shader_release_resources(vulkan_context* context, vulkan_material_shader* shader, material* material)
{
    vulkan_material_shader_instance_state* instance_state = &shader->instance_states[material->internal_id];

    vkDeviceWaitIdle(context->device.handle);

    u32 descriptor_set_count = 3;
    VK_CHECK(vkFreeDescriptorSets(
        context->device.handle,
        shader->object_descriptor_pool,
        descriptor_set_count,
        instance_state->descriptor_sets));

    for (u32 i = 0; i < VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT; ++i) {
        for (u32 j = 0; j < 3; ++j) {
            instance_state->descriptor_states[i].generations[j] = INVALID_ID;
            instance_state->descriptor_states[i].ids[j] = INVALID_ID;
        }
    }

    material->internal_id = INVALID_ID;
}

VkDeviceSize calculateUniformBufferAlignment(vulkan_context* context, VkDeviceSize size)
{
    VkDeviceSize min_alignment = context->device.properties.limits.minUniformBufferOffsetAlignment;
    return (size + min_alignment - 1) & ~(min_alignment - 1);
}
