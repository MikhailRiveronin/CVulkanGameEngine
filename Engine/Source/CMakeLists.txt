file(GLOB_RECURSE sources *.c)

# if(WIN32)
#     list(APPEND sources
#         platform/PlatformWin32.c)
# endif()

find_package(Vulkan)
add_library(Engine SHARED ${sources})
target_link_libraries(Engine PRIVATE Vulkan::Vulkan)
target_compile_definitions(Engine PRIVATE EXPORT VK_USE_PLATFORM_WIN32_KHR)

# Remove /RTC1.
set(CMAKE_C_FLAGS_DEBUG "/Ob0 /Od")

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
