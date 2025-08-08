#pragma once

#include "Renderer/Vulkan/vulkan_types.inl"
#include "Renderer/RendererTypes.inl"

b8 vulkan_object_shader_create(vulkan_context* context, vulkan_object_shader* shader);
void vulkan_object_shader_destroy(vulkan_context* context, vulkan_object_shader* shader);

void vulkan_object_shader_use(vulkan_context* context, vulkan_object_shader* shader);
