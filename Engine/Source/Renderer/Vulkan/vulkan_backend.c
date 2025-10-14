#include "vulkan_backend.h"

#include "VulkanPlatform.h"
#include "vulkan_device.h"
#include "VulkanSwapchain.h"
#include "vulkan_renderpass.h"
#include "vulkan_command_buffer.h"
#include "vulkan_utils.h"
#include "shaders/vulkan_material_shader.h"
#include "shaders/vulkan_ui_shader.h"
#include "third_party/cglm/cglm.h"
#include "vulkan_buffer.h"
#include "vulkan_image.h"

#include "containers/darray.h"

#include "core/application.h"
#include "core/logger.h"
#include "core/string_utils.h"

#include "systems/material_system.h"

static vulkan_context context;
static u32 cached_framebuffer_width;
static u32 cached_framebuffer_height;

#ifdef _DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    VkDebugUtilsMessengerCallbackDataEXT const* callbackData,
    void* userData);
#endif

static i32 findMemoryType(u32 memoryTypeBits, VkMemoryPropertyFlags properties);
static void createCommandBuffers(renderer_backend* backend);
static void regenerate_framebuffers();
static b8 recreate_swapchain(renderer_backend* backend);

static b8 create_buffers(vulkan_context* context);

b8 vulkan_backend_create(renderer_backend* backend, char const* app_name)
{
    context.allocator = NULL;
    context.findMemoryType = findMemoryType;
    applicationGetFramebufferSize(&cached_framebuffer_width, &cached_framebuffer_height);
    context.framebuffer_width = cached_framebuffer_width;
    context.framebuffer_height = cached_framebuffer_height;

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = NULL;
    appInfo.pApplicationName = "Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    DARRAY_DEFINE(char const*, requiredLayers, 0, MEMORY_TAG_STRING);
#ifdef _DEBUG
    DARRAY_PUSH(requiredLayers, "VK_LAYER_KHRONOS_validation");

    LOG_DEBUG("Required layers:");
    for (u32 i = 0; i < requiredLayers.size; ++i) {
        LOG_DEBUG("    %s", DARRAY_AT(requiredLayers, i));
    }
#endif
    {
        u32 propertyCount;
        VK_CHECK(vkEnumerateInstanceLayerProperties(&propertyCount, NULL));
        DARRAY_DEFINE(VkLayerProperties, availableLayers, propertyCount, MEMORY_TAG_RENDERER);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&propertyCount, availableLayers.data));
        availableLayers.size = propertyCount;

        for (u32 i = 0; i < requiredLayers.size; ++i) {
            b8 found = FALSE;
            for (u32 j = 0; j < availableLayers.size; ++j) {
                if (string_equal(DARRAY_AT(requiredLayers, i), DARRAY_AT(availableLayers, j).layerName)) {
                    found = TRUE;
                    break;
                }
            }

            if (!found) {
                LOG_FATAL("Required validation layer '%s' not found", DARRAY_AT(requiredLayers, i));
                return FALSE;
            }
        }
        DARRAY_DESTROY(availableLayers);
    }

    DARRAY_CSTRING requiredExtensions;
    DARRAY_INIT(requiredExtensions, MEMORY_TAG_STRING);
    DARRAY_PUSH(requiredExtensions, VK_KHR_SURFACE_EXTENSION_NAME);
    vulkan_platform_get_required_extensions(&requiredExtensions);
#ifdef _DEBUG
    DARRAY_PUSH(requiredExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    LOG_DEBUG("Required extensions:");
    for (u32 i = 0; i < requiredExtensions.size; ++i) {
        LOG_DEBUG("    %s", DARRAY_AT(requiredExtensions, i));
    }
#endif
    {
        u32 propertyCount;
        VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &propertyCount, NULL));
        DARRAY_DEFINE(VkExtensionProperties, availableExtensions, propertyCount, MEMORY_TAG_RENDERER);
        VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &propertyCount, availableExtensions.data));
        availableExtensions.size = propertyCount;

        for (u32 i = 0; i < requiredExtensions.size; ++i) {
            b8 found = FALSE;
            for (u32 j = 0; j < availableExtensions.size; ++j) {
                if (string_equal(DARRAY_AT(requiredExtensions, i), DARRAY_AT(availableExtensions, j).extensionName)) {
                    found = TRUE;
                    break;
                }
            }

            if (!found) {
                LOG_FATAL("Required extension '%s' not found", DARRAY_AT(requiredExtensions, i));
                return FALSE;
            }
        }
        DARRAY_DESTROY(availableExtensions);
    }

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = NULL;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledLayerCount = requiredLayers.size;
    instanceCreateInfo.ppEnabledLayerNames = requiredLayers.data;
    instanceCreateInfo.enabledExtensionCount = requiredExtensions.size;
    instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data;
    VK_CHECK(vkCreateInstance(&instanceCreateInfo, context.allocator, &context.instance));
    LOG_INFO("Vulkan instance created");
    DARRAY_DESTROY(requiredLayers);
    DARRAY_DESTROY(requiredExtensions);

#ifdef _DEBUG
    VkDebugUtilsMessageSeverityFlagsEXT severity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    // severity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    VkDebugUtilsMessageTypeFlagsEXT messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    // messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {};
    debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugMessengerCreateInfo.pNext = NULL;
    debugMessengerCreateInfo.flags = 0;
    debugMessengerCreateInfo.messageSeverity = severity;
    debugMessengerCreateInfo.messageType = messageType;
    debugMessengerCreateInfo.pfnUserCallback = debugUtilsMessengerCallback;
    debugMessengerCreateInfo.pUserData = NULL;

    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");
    ASSERT_MSG(func, "Failed to load vkCreateDebugUtilsMessengerEXT");
    VK_CHECK(func(context.instance, &debugMessengerCreateInfo, context.allocator, &context.debugUtilsMessenger));
    LOG_INFO("Debug messenger created");
#endif

    if (!platform_create_vulkan_surface(&context)) {
        LOG_ERROR("Failed to create surface");
        return FALSE;
    }

    if (!vulkanDeviceCreate(&context)) {
        LOG_ERROR("Failed to create device");
        return FALSE;
    }

    if (!vulkanSwapchainCreate(&context, context.framebuffer_width, context.framebuffer_height, &context.swapchain)) {
        LOG_ERROR("Failed to create swapchain");
        return FALSE;
    }

    vulkan_renderpass_create(
        &context,
        &context.main_renderpass,
        (vec4){ 0.f, 0.f, context.framebuffer_width, context.framebuffer_height },
        (vec4){ 0.f, 0.f, 1.f, 1.f },
        1.f,
        0,
        RENDERPASS_CLEAR_COLOUR_BUFFER_FLAG | RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG | RENDERPASS_CLEAR_STENCIL_BUFFER_FLAG,
        FALSE,
        TRUE);

    vulkan_renderpass_create(
        &context,
        &context.ui_renderpass,
        (vec4){ 0.f, 0.f, context.framebuffer_width, context.framebuffer_height },
        (vec4){ 0.f, 0.f, 0.f, 1.f },
        1.f,
        0,
        RENDERPASS_CLEAR_COLOUR_BUFFER_FLAG | RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG | RENDERPASS_CLEAR_STENCIL_BUFFER_FLAG,
        TRUE,
        FALSE);

    regenerate_framebuffers();

    // Command buffer
    createCommandBuffers(backend);

    // Sync objects
    DARRAY_RESERVE(context.image_available_semaphors, context.swapchain.images.size, MEMORY_TAG_RENDERER);
    DARRAY_RESERVE(context.render_complete_semaphors, context.swapchain.images.size, MEMORY_TAG_RENDERER);

    for (u32 i = 0; i < context.image_available_semaphors.capacity; ++i) {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = NULL;
        semaphoreCreateInfo.flags = 0;
        VK_CHECK(vkCreateSemaphore(
            context.device.handle,
            &semaphoreCreateInfo,
            context.allocator,
            &DARRAY_AT(context.image_available_semaphors, i)));
        VK_CHECK(vkCreateSemaphore(
            context.device.handle,
            &semaphoreCreateInfo,
            context.allocator,
            &DARRAY_AT(context.render_complete_semaphors, i)));

        // Create the fence in a signaled state, indicating that the first frame has already been "rendered".
        // This will prevent the application from waiting indefinitely for the first frame to render since it
        // cannot be rendered until a frame is "rendered" before it.
        VkFenceCreateInfo fence_create_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VK_CHECK(vkCreateFence(context.device.handle, &fence_create_info, context.allocator, &context.fences_in_flight[i]));
    }

    // Create builtin shaders
    if (!vulkan_material_shader_create(&context, &context.material_shader)) {
        LOG_ERROR("Error loading built-in basic_lighting shader.");
        return FALSE;
    }

    if (!vulkan_ui_shader_create(&context, &context.ui_shader)) {
        LOG_ERROR("Error loading built-in basic_lighting shader.");
        return FALSE;
    }

    create_buffers(&context);

    // Mark all geometries as invalid
    for (u32 i = 0; i < VULKAN_MAX_GEOMETRY_COUNT; ++i) {
        context.geometries[i].id = INVALID_ID;
    }

    LOG_INFO("Vulkan renderer initialized");
    return TRUE;
}

void vulkan_backend_destroy(struct renderer_backend* backend)
{
    vkDeviceWaitIdle(context.device.handle);

    // Vertex/index buffers
    vulkan_buffer_destroy(&context, &context.object_vertex_buffer);
    vulkan_buffer_destroy(&context, &context.object_index_buffer);

    // Shader objects
    vulkan_material_shader_destroy(&context, &context.material_shader);
    vulkan_ui_shader_destroy(&context, &context.ui_shader);

    // Sync objects
    for (u32 i = 0; i < context.image_available_semaphors.capacity; ++i) {
        vkDestroySemaphore(context.device.handle, DARRAY_AT(context.image_available_semaphors, i), context.allocator);
        vkDestroySemaphore(context.device.handle, DARRAY_AT(context.render_complete_semaphors, i), context.allocator);
        vkDestroyFence(context.device.handle, context.fences_in_flight[i], context.allocator);

        context.image_available_semaphors.data[i] = VK_NULL_HANDLE;
        context.render_complete_semaphors.data[i] = VK_NULL_HANDLE;
    }
    DARRAY_DESTROY(context.image_available_semaphors);
    DARRAY_DESTROY(context.render_complete_semaphors);

    for (u8 i = 0; i < context.command_buffers.size; ++i) {
        vulkanCommandBufferFree(
            &context,
            context.device.graphics_command_pool,
            &DARRAY_AT(context.command_buffers, i));
    }
    DARRAY_DESTROY(context.command_buffers);

    // Framebuffers
    for (u32 i = 0; i < _countof(context.swapchain.framebuffers); ++i) {
        vkDestroyFramebuffer(context.device.handle, context.world_framebuffers[i], context.allocator);
        vkDestroyFramebuffer(context.device.handle, context.swapchain.framebuffers[i], context.allocator);
    }

    // Renderpass
    vulkan_renderpass_destroy(&context, &context.ui_renderpass);
    vulkan_renderpass_destroy(&context, &context.main_renderpass);

    // Swapchain
    vulkanSwapchainDestroy(&context, &context.swapchain);
    vulkanDeviceDestroy(&context);
    vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);
#ifdef _DEBUG
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            context.instance,
            "vkDestroyDebugUtilsMessengerEXT");
    ASSERT_MSG(func, "Failed to load vkDestroyDebugUtilsMessengerEXT");
    func(context.instance, context.debugUtilsMessenger, context.allocator);
#endif
    vkDestroyInstance(context.instance, context.allocator);
}

b8 vulkan_backend_begin_frame(renderer_backend* backend, f64 delta_time)
{
    context.frame_delta_time = delta_time;

    VkDevice device = context.device.handle;
    vulkan_command_buffer* command_buffer = &context.command_buffers.data[context.current_frame];

    // Return if recreating swapchain
    if (context.recreating_swapchain) {
        VkResult result = vkDeviceWaitIdle(device);
        if (!vulkan_result_is_success(result)) {
            LOG_ERROR("vulkan_backend_begin_frame: vkDeviceWaitIdle(1) failed: '%s'",
                vulkan_result_string(result, TRUE));
            return FALSE;
        }

        LOG_INFO("Recreating swapchain");
        return FALSE;
    }

    // If framebuffer has been resized, recreate swapchain
    if (context.framebuffer_generation != context.framebuffer_last_generation) {
        VkResult result = vkDeviceWaitIdle(device);
        if (!vulkan_result_is_success(result)) {
            LOG_ERROR("vulkan_backend_begin_frame: vkDeviceWaitIdle(2) failed: '%s'",
                vulkan_result_string(result, TRUE));
            return FALSE;
        }

        // If the swapchain recreation failed (because, for example, the window was minimized),
        // don't unset recreating_swapchain
        if (!recreate_swapchain(backend)) {
            return FALSE;
        }

        // Swapchain resized but render on the next frame
        LOG_INFO("Swapchain resized");
        return FALSE;
    }

    // Wait for the execution of the current frame to complete. The fence being free will allow this one to move on.
    VkResult result = vkWaitForFences(context.device.handle, 1, &context.fences_in_flight[context.current_frame], TRUE, UINT64_MAX);
    if (!vulkan_result_is_success(result)) {
        LOG_ERROR("In-flight fence wait failure! error: %s", vulkan_result_string(result, TRUE));
        return FALSE;
    }

    if (!vulkan_swapchain_acquire_next_image(
        &context, &context.swapchain, UINT64_MAX,
        context.image_available_semaphors.data[context.current_frame],
        VK_NULL_HANDLE, &context.current_image)) {
        return FALSE;
    }

    vulkan_command_buffer_reset(command_buffer);
    vulkan_command_buffer_begin(command_buffer, FALSE, FALSE, FALSE);

    VkViewport viewport = {};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = (f32)context.framebuffer_width;
    viewport.height = (f32)context.framebuffer_height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = context.framebuffer_width;
    scissor.extent.height = context.framebuffer_height;

    vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);

    context.main_renderpass.render_area.z = context.framebuffer_width;
    context.main_renderpass.render_area.w = context.framebuffer_height;

    return TRUE;
}

void vulkan_backend_update_global_state(mat4 view, mat4 proj, vec3 view_pos, vec4 ambient_color, i32 mode)
{
    vulkan_command_buffer* command_buffer = &context.command_buffers.data[context.current_frame];

    vulkan_material_shader_use(&context, &context.material_shader);

    context.material_shader.global_data.view = view;
    context.material_shader.global_data.proj = proj;

    vulkan_material_shader_update_global_state(&context, &context.material_shader, context.frame_delta_time);
}

void vulkan_renderer_update_global_ui_state(mat4 projection, mat4 view, i32 mode) {
    vulkan_command_buffer* command_buffer = &context.command_buffers.data[context.current_image];

    vulkan_ui_shader_use(&context, &context.ui_shader);

    context.ui_shader.global_ubo.proj = projection;
    context.ui_shader.global_ubo.view = view;

    // TODO: other ubo properties

    vulkan_ui_shader_update_global_state(&context, &context.ui_shader, context.frame_delta_time);
}

b8 vulkan_backend_end_frame(struct renderer_backend* backend, f64 deltaTime)
{
    vulkan_command_buffer* command_buffer = &context.command_buffers.data[context.current_image];
    vulkan_command_buffer_end(command_buffer);

    // Make sure the previous frame is not using this image
    if (context.images_in_flight[context.current_frame]) {
        VkResult result = vkWaitForFences(context.device.handle, 1, context.images_in_flight[context.current_image], VK_TRUE, UINT64_MAX);
        if (!vulkan_result_is_success(result)) {
            LOG_FATAL("vkWaitForFences: %s", vulkan_result_string(result, TRUE));
        }
    }

    // Mark the image fence as in-use by this frame.
    context.images_in_flight[context.current_image] = &context.fences_in_flight[context.current_frame];

    vkResetFences(context.device.handle, 1, &context.fences_in_flight[context.current_frame]);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &context.image_available_semaphors.data[context.current_frame];

    VkPipelineStageFlags wait_dst_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.pWaitDstStageMask = wait_dst_stages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffer->handle;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &context.render_complete_semaphors.data[context.current_frame];
    VK_CHECK(vkQueueSubmit(context.device.queues.graphics.handle, 1, &submitInfo, context.fences_in_flight[context.current_frame]));

    vulkanCommandBufferUpdateSubmitted(command_buffer);

    vulkan_swapchain_present(
        &context, &context.swapchain,
        context.device.queues.graphics.handle, context.device.queues.present.handle,
        context.render_complete_semaphors.data[context.current_frame], context.current_image);

    return TRUE;
}

void vulkan_backend_on_resize(renderer_backend* backend, i16 width, i16 height)
{
    cached_framebuffer_width = width;
    cached_framebuffer_height = height;
    context.framebuffer_generation++;

    LOG_INFO("vulkan_backend_on_resize: width %i, height %i, framebuffer generation %llu",
        width, height, context.framebuffer_generation);
}

void vulkan_backend_draw_geometry(geometry_render_data data)
{
    // Ignore non-uploaded geometries.
    if (data.geometry && data.geometry->internal_id == INVALID_ID) {
        return;
    }

    vulkan_geometry_data* buffer_data = &context.geometries[data.geometry->internal_id];
    vulkan_command_buffer* command_buffer = &context.command_buffers.data[context.current_frame];

    material* m = 0;
    if (data.geometry->material) {
        m = data.geometry->material;
    } else {
        m = material_system_get_default_material();
    }

    switch (m->type) {
        case MATERIAL_TYPE_WORLD:
            vulkan_material_shader_set_model(&context, &context.material_shader, data.world);
            vulkan_material_shader_apply_material(&context, &context.material_shader, m);
            break;
        case MATERIAL_TYPE_UI:
            vulkan_ui_shader_set_model(&context, &context.ui_shader, data.world);
            vulkan_ui_shader_apply_material(&context, &context.ui_shader, m);
            break;
        default:
            LOG_ERROR("vulkan_renderer_draw_geometry - unknown material type: %i", m->type);
            return;
    }

    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(command_buffer->handle, 0, 1, &context.object_vertex_buffer.handle, offsets);

    // Draw indexed or non-indexed.
    if (buffer_data->index_count > 0) {
        // Bind index buffer at offset.
        vkCmdBindIndexBuffer(command_buffer->handle, context.object_index_buffer.handle, buffer_data->index_buffer_offset, VK_INDEX_TYPE_UINT32);

        // Issue the draw.
        vkCmdDrawIndexed(command_buffer->handle, buffer_data->index_count, 1, 0, 0, 0);
    } else {
        vkCmdDraw(command_buffer->handle, buffer_data->vertex_count, 1, 0, 0);
    }
}

void vulkan_backend_create_texture(u8 const* pixels, texture* texture)
{
    texture->internal_data = memory_allocate(sizeof(vulkan_texture_data), MEMORY_TAG_TEXTURE);
    vulkan_texture_data* data = texture->internal_data;

    VkDeviceSize image_size = texture->width * texture->height * texture->channel_count;

    VkFormat image_format = VK_FORMAT_R8G8B8A8_SNORM;

    // Staging buffer
    VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlagBits memory_properties =
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vulkan_buffer staging;
    vulkan_buffer_create(&context, image_size, usage, memory_properties, TRUE, &staging);

    vulkan_buffer_load_data(&context, &staging, 0, image_size, 0, pixels);

    vulkan_image_create(
        &context,
        VK_IMAGE_TYPE_2D,
        texture->width,
        texture->height,
        image_format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        TRUE,
        VK_IMAGE_ASPECT_COLOR_BIT,
        &data->image);

    VkCommandPool pool = context.device.graphics_command_pool;
    VkQueue queue = context.device.queues.graphics.handle;
    vulkan_command_buffer temp_command_buffer;

    vulkan_command_buffer_allocate_and_begin_single_use(&context, pool, &temp_command_buffer);
    vulkan_image_transition_layout(
        &context,
        &temp_command_buffer,
        &data->image,
        image_format,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vulkan_image_copy_from_buffer(&context, &temp_command_buffer, &data->image, staging.handle);

    vulkan_image_transition_layout(
        &context,
        &temp_command_buffer,
        &data->image,
        image_format,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vulkan_command_buffer_end_single_use(&context, pool, &temp_command_buffer, queue);
    vulkan_buffer_destroy(&context, &staging);

    VkSamplerCreateInfo sampler_create_info = {};
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.pNext = 0;
    sampler_create_info.flags = 0;
    sampler_create_info.magFilter = VK_FILTER_LINEAR;
    sampler_create_info.minFilter = VK_FILTER_LINEAR;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.mipLodBias = 0.f;
    sampler_create_info.anisotropyEnable = VK_TRUE;
    sampler_create_info.maxAnisotropy = 1.f;
    sampler_create_info.compareEnable = VK_FALSE;
    sampler_create_info.compareOp = VK_COMPARE_OP_NEVER;
    sampler_create_info.minLod = 0.f;
    sampler_create_info.maxLod = 0.f;
    sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    sampler_create_info.unnormalizedCoordinates = VK_FALSE;
    VK_CHECK(vkCreateSampler(context.device.handle, &sampler_create_info, context.allocator, &data->sampler));

    texture->generation++;
}

void vulkan_backend_destroy_texture(texture* texture)
{
    vkDeviceWaitIdle(context.device.handle);

    vulkan_texture_data* data = texture->internal_data;

    if (data != 0) {
        vulkan_image_destroy(&context, &data->image);
        memory_zero(&data->image, sizeof(data->image));
        vkDestroySampler(context.device.handle, data->sampler, context.allocator);
        data->sampler = 0;

        memory_free(texture->internal_data, sizeof(vulkan_texture_data), MEMORY_TAG_TEXTURE);
    }

    memory_zero(texture, sizeof(*texture));
}

b8 vulkan_backend_create_material(material* material)
{
    if (material) {
        switch (material->type) {
            case MATERIAL_TYPE_WORLD:
                if (!vulkan_material_shader_acquire_resources(&context, &context.material_shader, material)) {
                    LOG_ERROR("vulkan_renderer_create_material - Failed to acquire world shader resources.");
                    return FALSE;
                }
                break;
            case MATERIAL_TYPE_UI:
                if (!vulkan_ui_shader_acquire_resources(&context, &context.ui_shader, material)) {
                    LOG_ERROR("vulkan_renderer_create_material - Failed to acquire UI shader resources.");
                    return FALSE;
                }
                break;
            default:
                LOG_ERROR("vulkan_renderer_create_material - unknown material type");
                return FALSE;
        }

        LOG_TRACE("Renderer: Material created.");
        return TRUE;
    }

    LOG_ERROR("vulkan_renderer_create_material called with nullptr. Creation failed.");
    return TRUE;

}

void vulkan_backend_destroy_material(material* material)
{
    if (material) {
        if (material->internal_id != INVALID_ID) {
            switch (material->type) {
                case MATERIAL_TYPE_WORLD:
                    vulkan_material_shader_release_resources(&context, &context.material_shader, material);
                    break;
                case MATERIAL_TYPE_UI:
                    vulkan_ui_shader_release_resources(&context, &context.ui_shader, material);
                    break;
                default:
                    LOG_ERROR("vulkan_renderer_destroy_material - unknown material type");
                    break;
            }
        } else {
            LOG_WARNING("vulkan_renderer_destroy_material called with internal_id=INVALID_ID. Nothing was done.");
        }
    } else {
        LOG_WARNING("vulkan_renderer_destroy_material called with nullptr. Nothing was done.");
    }
}

b8 vulkan_backend_create_geometry(geometry_resource* geometry, u32 vertex_size, u32 vertex_count, void const* vertices, u32 index_size, u32 index_count, u32 const* indices)
{
    if (vertex_count == 0 || !vertices) {
        LOG_ERROR("vulkan_renderer_create_geometry requires vertex data, and none was supplied. vertex_count=%d, vertices=%p", vertex_count, vertices);
        return FALSE;
    }

    // Check if this is a re-upload. If it is, need to free old data afterward.
    b8 is_reupload = geometry->internal_id != INVALID_ID;
    vulkan_geometry_data old_range;

    vulkan_geometry_data* internal_data = 0;
    if (is_reupload) {
        internal_data = &context.geometries[geometry->internal_id];

        // Take a copy of the old range.
        old_range.index_buffer_offset = internal_data->index_buffer_offset;
        old_range.index_count = internal_data->index_count;
        old_range.index_element_size = internal_data->index_element_size;
        old_range.vertex_buffer_offset = internal_data->vertex_buffer_offset;
        old_range.vertex_count = internal_data->vertex_count;
        old_range.vertex_element_size = internal_data->vertex_element_size;
    } else {
        for (u32 i = 0; i < VULKAN_MAX_GEOMETRY_COUNT; ++i) {
            if (context.geometries[i].id == INVALID_ID) {
                // Found a free index.
                geometry->internal_id = i;
                context.geometries[i].id = i;
                internal_data = &context.geometries[i];
                break;
            }
        }
    }
    if (!internal_data) {
        LOG_FATAL("vulkan_renderer_create_geometry failed to find a free index for a new geometry upload. Adjust config to allow for more.");
        return FALSE;
    }

    VkCommandPool pool = context.device.graphics_command_pool;
    VkQueue queue = context.device.queues.graphics.handle;

    // Vertex data.
    internal_data->vertex_buffer_offset = context.geometry_vertex_offset;
    internal_data->vertex_count = vertex_count;
    internal_data->vertex_element_size = sizeof(vertex_3d);
    u32 total_size = vertex_count * vertex_size;
    vulkan_buffer_upload_data(&context, pool, 0, queue, &context.object_vertex_buffer, internal_data->vertex_buffer_offset, total_size, vertices);
    // TODO: should maintain a free list instead of this.
    context.geometry_vertex_offset += total_size;

    // Index data, if applicable
    if (index_count != 0 && indices) {
        internal_data->index_buffer_offset = context.geometry_index_offset;
        internal_data->index_count = index_count;
        internal_data->index_element_size = sizeof(u32);
        total_size = index_count * index_size;
        vulkan_buffer_upload_data(&context, pool, 0, queue, &context.object_index_buffer, internal_data->index_buffer_offset, total_size, indices);
        // TODO: should maintain a free list instead of this.
        context.geometry_index_offset += total_size;
    }

    if (internal_data->generation == INVALID_ID) {
        internal_data->generation = 0;
    } else {
        internal_data->generation++;
    }

    if (is_reupload) {
        // Free vertex data
        vulkan_buffer_free_data(&context.object_vertex_buffer, old_range.vertex_buffer_offset, old_range.vertex_element_size * old_range.vertex_count);

        // Free index data, if applicable
        if (old_range.index_element_size > 0) {
            vulkan_buffer_free_data(&context.object_index_buffer, old_range.index_buffer_offset, old_range.index_element_size * old_range.index_count);
        }
    }

    return TRUE;
}

void vulkan_backend_destroy_geometry(geometry_resource* geometry)
{
    if (geometry && geometry->internal_id != INVALID_ID) {
        vkDeviceWaitIdle(context.device.handle);
        vulkan_geometry_data* internal_data = &context.geometries[geometry->internal_id];

        // Free vertex data
        vulkan_buffer_free_data(&context.object_vertex_buffer, internal_data->vertex_buffer_offset, internal_data->vertex_element_size * internal_data->vertex_count);

        // Free index data, if applicable
        if (internal_data->index_element_size > 0) {
            vulkan_buffer_free_data(&context.object_index_buffer, internal_data->index_buffer_offset, internal_data->index_element_size * internal_data->index_count);
        }

        // Clean up data.
        memory_zero(internal_data, sizeof(vulkan_geometry_data));
        internal_data->id = INVALID_ID;
        internal_data->generation = INVALID_ID;
    }
}

b8 vulkan_renderer_begin_renderpass(renderer_backend* backend, u8 renderpass_id)
{
    vulkan_renderpass* renderpass = 0;
    VkFramebuffer framebuffer = 0;
    vulkan_command_buffer* command_buffer = &context.command_buffers.data[context.current_image];

    // Choose a renderpass based on ID.
    switch (renderpass_id) {
        case BUILTIN_RENDERPASS_WORLD:
            renderpass = &context.main_renderpass;
            framebuffer = context.world_framebuffers[context.current_image];
            break;
        case BUILTIN_RENDERPASS_UI:
            renderpass = &context.ui_renderpass;
            framebuffer = context.swapchain.framebuffers[context.current_image];
            break;
        default:
            LOG_ERROR("vulkan_renderer_begin_renderpass called on unrecognized renderpass id: %#02x", renderpass_id);
            return FALSE;
    }

    // Begin the render pass.
    vulkan_renderpass_begin(command_buffer, renderpass, framebuffer);

    // Use the appropriate shader.
    switch (renderpass_id) {
        case BUILTIN_RENDERPASS_WORLD:
            vulkan_material_shader_use(&context, &context.material_shader);
            break;
        case BUILTIN_RENDERPASS_UI:
            vulkan_ui_shader_use(&context, &context.ui_shader);
            break;
    }

    return TRUE;
}

b8 vulkan_renderer_end_renderpass(renderer_backend* backend, u8 renderpass_id)
{
    vulkan_renderpass* renderpass = 0;
    vulkan_command_buffer* command_buffer = &context.command_buffers.data[context.current_image];

    // Choose a renderpass based on ID.
    switch (renderpass_id) {
        case BUILTIN_RENDERPASS_WORLD:
            renderpass = &context.main_renderpass;
            break;
        case BUILTIN_RENDERPASS_UI:
            renderpass = &context.ui_renderpass;
            break;
        default:
            LOG_ERROR("vulkan_renderer_end_renderpass called on unrecognized renderpass id:  %#02x", renderpass_id);
            return FALSE;
    }

    vulkan_renderpass_end(command_buffer, renderpass);
    return TRUE;
}

#ifdef _DEBUG
VkBool32 debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    VkDebugUtilsMessengerCallbackDataEXT const* callbackData,
    void* userData)
{
    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            LOG_ERROR("%s", callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            LOG_WARNING("%s", callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            LOG_INFO("%s", callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            LOG_TRACE("%s", callbackData->pMessage);
            break;
    }
    return VK_FALSE;
}
#endif

i32 findMemoryType(u32 memoryTypeBits, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(context.device.physical_device, &memoryProperties);
    for (u32 i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        if ((memoryTypeBits & (1 << i)) && ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)) {
            return i;
        }
    }
    return -1;
}

void createCommandBuffers(renderer_backend* backend)
{
    if (!context.command_buffers.capacity) {
        DARRAY_RESERVE(context.command_buffers, context.swapchain.images.size, MEMORY_TAG_RENDERER);
    }

    for (u8 i = 0; i < context.swapchain.images.size; ++i) {
        if (DARRAY_AT(context.command_buffers, i).handle) {
            vulkanCommandBufferFree(
                &context, context.device.graphics_command_pool,
                &DARRAY_AT(context.command_buffers, i));
        }
        vulkanCommandBufferAllocate(
            &context, context.device.graphics_command_pool,
            TRUE, &DARRAY_AT(context.command_buffers, i));
    }
    LOG_DEBUG("Vulkan command buffers created");
}

void regenerate_framebuffers()
{
    u32 image_count = context.swapchain.images.size;
    for (u8 i = 0; i < image_count; ++i) {
        VkImageView world_attachments[2] = { context.swapchain.image_views.data[i], context.swapchain.depth_buffer.view };
        VkFramebufferCreateInfo framebuffer_create_info = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        framebuffer_create_info.renderPass = context.main_renderpass.handle;
        framebuffer_create_info.attachmentCount = _countof(world_attachments);
        framebuffer_create_info.pAttachments = world_attachments;
        framebuffer_create_info.width = context.framebuffer_width;
        framebuffer_create_info.height = context.framebuffer_height;
        framebuffer_create_info.layers = 1;
        VK_CHECK(vkCreateFramebuffer(context.device.handle, &framebuffer_create_info, context.allocator, &context.world_framebuffers[i]));

        // Swapchain framebuffers (UI pass). Outputs to swapchain images
        VkImageView ui_attachments[1] = { context.swapchain.image_views.data[i] };
        VkFramebufferCreateInfo sc_framebuffer_create_info = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        sc_framebuffer_create_info.renderPass = context.ui_renderpass.handle;
        sc_framebuffer_create_info.attachmentCount = _countof(ui_attachments);
        sc_framebuffer_create_info.pAttachments = ui_attachments;
        sc_framebuffer_create_info.width = context.framebuffer_width;
        sc_framebuffer_create_info.height = context.framebuffer_height;
        sc_framebuffer_create_info.layers = 1;
        VK_CHECK(vkCreateFramebuffer(context.device.handle, &sc_framebuffer_create_info, context.allocator, &context.swapchain.framebuffers[i]));
    }
}

b8 recreate_swapchain(renderer_backend* backend)
{
    // If already being recreated, do not try again
    if (context.recreating_swapchain) {
        LOG_DEBUG("Swapchain is already being recreated");
        return FALSE;
    }

    if ((context.framebuffer_width) == 0 || (context.framebuffer_height == 0)) {
        LOG_DEBUG("Recreating swapchain when window is minimized");
        return FALSE;
    }

    context.recreating_swapchain = TRUE;
    vkDeviceWaitIdle(context.device.handle);

    vulkan_device_query_swapchain_support(
        context.device.physical_device,
        context.surface,
        &context.device.swapchain_support);
    vulkan_device_detect_depth_format(&context.device);

    vulkan_swapchain_recreate(&context, cached_framebuffer_width, cached_framebuffer_height, &context.swapchain);

    // Sync the framebuffer size with the cached sizes.
    context.framebuffer_width = cached_framebuffer_width;
    context.framebuffer_height = cached_framebuffer_height;
    context.main_renderpass.render_area.z = context.framebuffer_width;
    context.main_renderpass.render_area.w = context.framebuffer_height;
    cached_framebuffer_width = 0;
    cached_framebuffer_height = 0;
    context.framebuffer_last_generation = context.framebuffer_generation;

    // cleanup swapchain
    for (u32 i = 0; i < context.swapchain.images.size; ++i) {
        vulkanCommandBufferFree(&context, context.device.graphics_command_pool, &context.command_buffers.data[i]);
    }

    // Framebuffers.
    for (u32 i = 0; i < _countof(context.swapchain.framebuffers); ++i) {
        vkDestroyFramebuffer(context.device.handle, context.world_framebuffers[i], context.allocator);
        vkDestroyFramebuffer(context.device.handle, context.swapchain.framebuffers[i], context.allocator);
    }

    context.main_renderpass.render_area.x = 0;
    context.main_renderpass.render_area.y = 0;
    context.main_renderpass.render_area.z = context.framebuffer_width;
    context.main_renderpass.render_area.w = context.framebuffer_height;

    regenerate_framebuffers();

    createCommandBuffers(backend);

    // Clear the recreating flag.
    context.recreating_swapchain = FALSE;

    return TRUE;
}

b8 create_buffers(vulkan_context* context)
{
    VkMemoryPropertyFlagBits memory_property_flag_bits = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    u64 vertex_buffer_size = 1024 * 1024 * sizeof(vertex_3d);
    if (!vulkan_buffer_create(
        context,
        vertex_buffer_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        memory_property_flag_bits,
        TRUE,
        &context->object_vertex_buffer)) {
        LOG_ERROR("Failed to create vertex buffer");
        return FALSE;
    }

    context->geometry_vertex_offset = 0;

    u64 index_buffer_size = 1024 * 1024 * sizeof(u32);
    if (!vulkan_buffer_create(
        context,
        index_buffer_size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        memory_property_flag_bits,
        TRUE,
        &context->object_index_buffer)) {
        LOG_ERROR("Failed to create index buffer");
        return FALSE;
    }

    context->geometry_index_offset = 0;

    return TRUE;
}
