file(GLOB_RECURSE sources *.c)
add_executable(tests ${sources})

target_link_libraries(tests PRIVATE Engine)
target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source)

add_custom_command(TARGET tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_BINARY_DIR}/Engine/Source/Debug ${CMAKE_BINARY_DIR}/tests/Debug)
