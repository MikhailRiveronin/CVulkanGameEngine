#include "renderer.h"

#include "core/logger.h"
#include "systems/resource_system.h"
#include "systems/memory_system.h"
#include "renderer_utils.h"
#include "vulkan_structure_initializers.h"
#include "platform/filesystem.h"
#include "third_party/SPIRV-Reflect/spirv_reflect.h"

static Renderer_Context context;

bool renderer_create_shader(Shader* shader)
{
    struct
    {
        Dynamic_Array* modules; // destroyed
        Dynamic_Array* create_infos; // destroyed
    } pipeline_shader_stages;
    pipeline_shader_stages.modules = DYNAMIC_ARRAY_CREATE(VkShaderModule);
    pipeline_shader_stages.create_info = DYNAMIC_ARRAY_CREATE(VkPipelineShaderStageCreateInfo);

    VkPipelineVertexInputStateCreateInfo input_state_create_info = {};

    typedef struct Descriptor_Set_Layout_Binding
    {
        Dynamic_Array* layout_bindings; // destroyed
        Dynamic_Array* binding_flags; // destroyed
    } Descriptor_Set_Layout_Binding;

    // Simplifying assumption: All descriptors are organized into 3 sets
    Descriptor_Set_Layout_Binding set_layout_bindings[3];
    for (u32 set_index = 0; set_index < 3; ++set_index)
    {
        set_layout_bindings[set_index].layout_bindings = DYNAMIC_ARRAY_CREATE(VkDescriptorSetLayoutBinding);
        set_layout_bindings[set_index].binding_flags = DYNAMIC_ARRAY_CREATE(VkDescriptorBindingFlags);
    }

    for (u32 path_index = 0; path_index < shader->spv_paths->size; ++path_index)
    {
        char const* spv_path = DYNAMIC_ARRAY_AT_AS(shader->spv_paths, path_index, char const*);
        Resource_Data spv;
        if (!resource_system_load(RESOURCE_TYPE_BINARY, spv_path, &spv))
        {
            LOG_FATAL("renderer_create_shader: Failed to load '%s' resource", spv_path);
            return false;
        }

        SpvReflectShaderModule reflect_module = {};
        SPV_REFLECT_CHECK_RESULT(spvReflectCreateShaderModule(spv.size, spv.data, &reflect_module));

        VkShaderModuleCreateInfo module_create_info = shader_module_create_info(reflect_module._internal->spirv_size, reflect_module._internal->spirv_code);
        VkShaderModule module;
        VULKAN_CHECK_RESULT(vkCreateShaderModule(context.device.handle, &module_create_info, context.allocator, &module));
        VkPipelineShaderStageCreateInfo stage_create_info = pipeline_shader_stage_create_info((VkShaderStageFlagBits)reflect_module.shader_stage, module);
        dynamic_array_push_back(pipeline_shader_stages.modules, &module);
        dynamic_array_push_back(pipeline_shader_stages.create_infos, &stage_create_info);

        if (reflect_module.shader_stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT)
        {
            u32 input_var_count = 0;
            SPV_REFLECT_CHECK_RESULT(spvReflectEnumerateInputVariables(&reflect_module, &input_var_count, 0));
            assert(input_var_count > 0);
            SpvReflectInterfaceVariable** input_vars = memory_system_allocate(input_var_count * sizeof(*input_vars), MEMORY_TAG_UNKNOWN);
            SPV_REFLECT_CHECK_RESULT(spvReflectEnumerateInputVariables(&reflect_module, &input_var_count, input_vars));

            // Simplifying assumption: Each vertex input attribute is sourced from a separate vertex buffer, bound sequentially starting from binding 0
            VkVertexInputBindingDescription* binding_descriptions = memory_system_allocate(input_var_count * sizeof(*binding_descriptions), MEMORY_TAG_UNKNOWN);
            VkVertexInputAttributeDescription* attribute_descriptions = memory_system_allocate(input_var_count * sizeof(*attribute_descriptions), MEMORY_TAG_UNKNOWN);
            for (u32 var_index = 0; var_index < input_var_count; ++var_index)
            {
                SpvReflectInterfaceVariable* var = input_vars[var_index];
                binding_descriptions[var_index] = vertex_input_binding_description(var_index, vertex_attribute_size((VkFormat)var->format));
                attribute_descriptions[var_index] = vertex_input_attribute_description(var->location, var_index, (VkFormat)var->format, 0);
            }

            memory_system_free(input_vars, input_var_count * sizeof(*input_vars), MEMORY_TAG_UNKNOWN);
            input_state_create_info = pipeline_vertex_input_state_create_info(input_var_count, binding_descriptions, input_var_count, attribute_descriptions);
        }

        u32 descriptor_set_count = 0;
        SPV_REFLECT_CHECK_RESULT(spvReflectEnumerateDescriptorSets(&reflect_module, &descriptor_set_count, 0));
        assert(descriptor_set_count > 0);
        SpvReflectDescriptorSet** descriptor_sets = memory_system_allocate(descriptor_set_count * sizeof(*descriptor_sets), MEMORY_TAG_UNKNOWN);
        SPV_REFLECT_CHECK_RESULT(spvReflectEnumerateDescriptorSets(&reflect_module, &descriptor_set_count, descriptor_sets));

        // Simplifying assumption: All descriptors are organized into 3 sets
        assert(descriptor_set_count == 3);
        for (u32 set_index = 0; set_index < descriptor_set_count; ++set_index)
        {
            SpvReflectDescriptorSet* descriptor_set = descriptor_sets[set_index];
            Dynamic_Array* layout_bindings = set_layout_bindings[set_index].layout_bindings;
            Dynamic_Array* binding_flags = set_layout_bindings[set_index].binding_flags;
            dynamic_array_resize(layout_bindings, layout_bindings->size + descriptor_set->binding_count);
            dynamic_array_resize(binding_flags, binding_flags->size + descriptor_set->binding_count);
            for (u32 binding_index = 0; binding_index < descriptor_set->binding_count; ++binding_index)
            {
                SpvReflectDescriptorBinding* descriptor_binding = descriptor_set->bindings[binding_index];
                if (descriptor_binding->binding > layout_bindings->size)
                {
                    dynamic_array_resize(layout_bindings, descriptor_binding->binding);
                }

                if (descriptor_binding->binding > binding_flags->size)
                {
                    dynamic_array_resize(binding_flags, descriptor_binding->binding);
                }

                // Simplifying assumption: For runtime sized arrays, an upper bound of 256 is enough
                bool is_runtime_sized_array = descriptor_binding->type_description->op == SpvOpTypeRuntimeArray;
                DYNAMIC_ARRAY_AT_AS(layout_bindings, descriptor_binding->binding, VkDescriptorSetLayoutBinding) = descriptor_set_layout_binding(descriptor_binding->binding, (VkDescriptorType)descriptor_binding->descriptor_type, is_runtime_sized_array ? 256 : descriptor_binding->count, (VkShaderStageFlagBits)reflect_module.shader_stage);
                DYNAMIC_ARRAY_AT_AS(binding_flags, descriptor_binding->binding, VkDescriptorBindingFlags) = is_runtime_sized_array ? VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT : 0;
            }
        }
    }

    // Simplifying assumption: All descriptors are organized into 3 sets
    for (u32 set_index = 0; set_index < 3; ++i)
    {
        Dynamic_Array* layout_bindings = set_layout_bindings[set_index].layout_bindings;
        Dynamic_Array* binding_flags = set_layout_bindings[set_index].binding_flags;
        VkDescriptorSetLayoutBindingFlagsCreateInfo layout_binding_flags_create_info = descriptor_set_layout_binding_flags_create_info(binding_flags->size, binding_flags->data);
        VkDescriptorSetLayoutCreateInfo layout_create_info = descriptor_set_layout_create_info(layout_bindings->size, layout_bindings->data, &layout_binding_flags_create_info);
        VULKAN_CHECK_RESULT(vkCreateDescriptorSetLayout(context.device.handle, &layout_create_info, context.allocator, &shader->descriptor_set_layouts[set_index]));

        dynamic_array_destroy(layout_bindings);
        dynamic_array_destroy(binding_flags);
    }

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state = pipeline_input_assembly_state_create_info();
    VkPipelineTessellationStateCreateInfo tessellation_state = pipeline_tessellation_state_create_info();
    VkPipelineRasterizationStateCreateInfo rasterization_state = pipeline_rasterization_state_create_info();
    VkPipelineMultisampleStateCreateInfo multisample_state = pipeline_multisample_state_create_info();
    VkPipelineDepthStencilStateCreateInfo depth_stencil_state = pipeline_depth_stencil_state_create_info();
    VkPipelineColorBlendStateCreateInfo color_blend_state = pipeline_color_blend_state_create_info(pipeline_color_blend_attachment_state());
    VkPipelineDynamicStateCreateInfo dynamic_state = pipeline_dynamic_state_create_info();

    VkPipelineLayoutCreateInfo layout_create_info = pipeline_layout_create_info(_countof(shader->descriptor_set_layouts), shader->descriptor_set_layouts, 0, 0);
    VULKAN_CHECK_RESULT(vkCreatePipelineLayout(context.device.handle, &layout_create_info, context.allocator, &shader->pipeline.pipeline_layout));

    VkGraphicsPipelineCreateInfo pipeline_create_info = graphics_pipeline_create_info(stage_create_infos->size, stage_create_infos->data, &input_state_create_info, &input_assembly_state, &tessellation_state, 0, &rasterization_state, &multisample_state, &depth_stencil_state, &color_blend_state, &dynamic_state, shader->pipeline.pipeline_layout, shader->renderpass->handle);
    VULKAN_CHECK_RESULT(vkCreateGraphicsPipelines(context.device.handle, VK_NULL_HANDLE, 1, &pipeline_create_info, context.allocator, &shader->pipeline.handle));

    dynamic_array_destroy(pipeline_shader_stages.modules);
    dynamic_array_destroy(pipeline_shader_stages.create_infos);
}
