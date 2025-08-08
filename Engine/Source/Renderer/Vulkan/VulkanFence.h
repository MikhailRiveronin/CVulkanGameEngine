#pragma once

#include "vulkan_types.inl"

void vulkanFenceCreate(vulkan_context* context, b8 createSignaled, VulkanFence* fence);
void vulkanFenceDestroy(vulkan_context* context, VulkanFence* fence);

b8 vulkan_fence_wait(vulkan_context* context, VulkanFence* fence, u64 timeoutNs);
void vulkan_fence_reset(vulkan_context* context, VulkanFence* fence);
