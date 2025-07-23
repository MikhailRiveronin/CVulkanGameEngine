file(GLOB_RECURSE sources *.c)
add_executable(Main ${sources})

target_link_libraries(Main PRIVATE Engine)

add_custom_command(TARGET Main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/Engine/Source/Debug ${CMAKE_BINARY_DIR}/Samples/Debug)
