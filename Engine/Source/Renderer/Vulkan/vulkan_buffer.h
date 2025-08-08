#pragma once

#include "defines.h"
#include "vulkan_types.inl"

b8 vulkan_buffer_create(
    vulkan_context* context,
    u64 size,
    VkBufferUsageFlagBits usage,
    u32 memory_property_flags,
    b8 bind_on_create,
    vulkan_buffer* buffer);
void vulkan_buffer_destroy(vulkan_context* context, vulkan_buffer* buffer);

b8 vulkan_buffer_resize(
    vulkan_context* context,
    vulkan_buffer* buffer,
    u64 new_size,
    VkCommandPool pool,
    VkQueue queue);

void vulkan_buffer_bind(vulkan_context* context, vulkan_buffer* buffer, u64 offset);

void* vulkan_buffer_map_memory(
    vulkan_context* context,
    vulkan_buffer* buffer,
    u64 offset,
    u64 size,
    u32 flags);
void vulkan_buffer_unmap_memory(vulkan_context* context, vulkan_buffer* buffer);

void vulkan_buffer_load_data(
    vulkan_context* context,
    vulkan_buffer* buffer,
    u64 offset,
    u64 size,
    u32 flags,
    void const* data);

void vulkan_buffer_copy(
    vulkan_context* context,
    VkCommandPool pool,
    VkFence fence,
    VkQueue queue,
    VkBuffer source,
    u64 source_offset,
    VkBuffer dest,
    u64 dest_offset,
    u64 size);

void vulkan_buffer_upload_data(
    vulkan_context* context,
    VkCommandPool pool,
    VkFence fence,
    VkQueue queue,
    vulkan_buffer* buffer,
    u64 offset,
    u64 size,
    void const* data);
