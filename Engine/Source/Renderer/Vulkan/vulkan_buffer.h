#pragma once

#include "defines.h"
#include "vulkan_structures.h"

b8 vulkan_buffer_create(vulkan_context* context, u64 size, VkBufferUsageFlagBits usage, u32 memory_property, b8 bind_on_create, vulkan_buffer* buffer);
void vulkan_buffer_destroy(vulkan_context* context, vulkan_buffer* buffer);

b8 vulkan_buffer_resize(vulkan_context* context, vulkan_buffer* buffer, u64 new_size, VkCommandPool pool, VkQueue queue);
void vulkan_buffer_bind(vulkan_context* context, vulkan_buffer* buffer, u64 offset);

void vulkan_buffer_upload_to_host_visible_memory(
    vulkan_context* context,
    vulkan_buffer* buffer,
    u64 offset,
    u64 size,
    u32 flags,
    void const* data);

void vulkan_buffer_copy_to_buffer(
    vulkan_context* context,
    VkCommandPool pool,
    VkFence fence,
    VkQueue queue,
    VkBuffer source,
    u64 source_offset,
    VkBuffer dest,
    u64 dest_offset,
    u64 size);

void vulkan_buffer_upload_to_device_local_memory(
    vulkan_context* context,
    VkCommandPool pool,
    VkFence fence,
    VkQueue queue,
    vulkan_buffer* buffer,
    u64 offset,
    u64 size,
    void const* data);
void vulkan_buffer_free_data(vulkan_buffer* buffer, u64 offset, u64 size);

/**
 * @brief Sub-allocates space from the vulkan buffer.
 * @param buffer A pointer to the buffer.
 * @param size The size in bytes to be sub-allocated.
 * @param offset A pointer to the offset to the sub-allocated memory, in bytes.
 * @return TRUE on success, otherwise FALSE.
 */
b8 vulkan_buffer_suballocate(vulkan_buffer* buffer, u64 size, u64* offset);

/**
 * @brief Frees space in the vulkan buffer.
 * @param buffer A pointer to the buffer.
 * @param size The size in bytes to be freed.
 * @param offset The offset from the beginning of the buffer, in bytes.
 * @return TRUE on success, otherwise FALSE.
 */
b8 vulkan_buffer_free(vulkan_buffer* buffer, u64 size, u64 offset);
