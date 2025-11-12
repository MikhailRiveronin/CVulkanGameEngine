#pragma once

#include "renderer/renderer_types.h"
#include "renderer/vulkan/vulkan_types.h"

b8 vulkan_material_shader_create(vulkan_context* context, vulkan_material_shader* shader);
void vulkan_material_shader_destroy(vulkan_context* context, vulkan_material_shader* shader);

void vulkan_material_shader_use(vulkan_context* context, vulkan_material_shader* shader);

void vulkan_material_shader_update_global_state(vulkan_context* context, vulkan_material_shader* shader, f32 delta_time);

void vulkan_material_shader_set_world_matrix(vulkan_context* context, vulkan_material_shader* shader, mat4 world);
void vulkan_material_shader_apply_material(vulkan_context* context, vulkan_material_shader* shader, material_resource* material);

b8 vulkan_material_shader_acquire_resources(vulkan_context* context, vulkan_material_shader* shader, material_resource* material);
void vulkan_material_shader_release_resources(vulkan_context* context, vulkan_material_shader* shader, material_resource* material);
