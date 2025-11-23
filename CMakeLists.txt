cmake_minimum_required(VERSION 3.25)

project(CVulkanGameEngine LANGUAGES C)

set(ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/Engine/Source/config.h)

add_subdirectory(Engine/Source)
add_subdirectory(Samples)
add_subdirectory(tests)
