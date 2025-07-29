# file(GLOB_RECURSE sources *.c)

set(sources
    Core/Logger.c
    Core/Application.c
    Core/Memory.c
    Core/Events.c
    Core/Input.c
    Core/Clock.c
    Core/String.c
    Renderer/RendererFrontend.c
    Renderer/RendererBackend.c
    Renderer/Vulkan/VulkanBackend.c
    Renderer/Vulkan/VulkanPlatform.c
    Renderer/Vulkan/VulkanDevice.c
    Renderer/Vulkan/VulkanSwapchain.c
    Renderer/Vulkan/VulkanImage.c
    Renderer/Vulkan/VulkanRenderpass.c
    Renderer/Vulkan/VulkanCommandBuffer.c
    Renderer/Vulkan/VulkanFramebuffer.c
    Renderer/Vulkan/VulkanFence.c
    Renderer/Vulkan/VulkanUtils.c)

if(WIN32)
    list(APPEND sources
        Platform/PlatformWin32.c)
endif()

find_package(Vulkan)
add_library(Engine SHARED ${sources})
target_link_libraries(Engine PRIVATE Vulkan::Vulkan Vulkan::shaderc_combined)
target_compile_definitions(Engine PRIVATE VK_USE_PLATFORM_WIN32_KHR)

target_compile_definitions(Engine PRIVATE EXPORT)

# Remove /RTC1.
set(CMAKE_C_FLAGS_DEBUG "/Ob0 /Od")

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
