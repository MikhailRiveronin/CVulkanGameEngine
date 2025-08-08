file(GLOB_RECURSE sources *.c)
add_executable(Main ${sources})

target_link_libraries(Main PRIVATE Engine)

add_custom_command(TARGET Main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_BINARY_DIR}/Engine/Source/Debug ${CMAKE_BINARY_DIR}/Samples/Debug)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets/shaders)

message("Compiling shaders...")

message("assets/shaders/Builtin.ObjectShader.vert.glsl -> bin/assets/shaders/Builtin.ObjectShader.vert.spv")
add_custom_command(TARGET Main POST_BUILD
    COMMAND $ENV{VULKAN_SDK}/Bin/glslc.exe
        -fshader-stage=vert
        ${CMAKE_SOURCE_DIR}/assets/shaders/Builtin.ObjectShader.vert.glsl
        -o ${CMAKE_BINARY_DIR}/assets/shaders/Builtin.ObjectShader.vert.spv
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    BYPRODUCTS ${CMAKE_BINARY_DIR}/assets/shaders/Builtin.ObjectShader.vert.spv)

message("assets/shaders/Builtin.ObjectShader.frag.glsl -> bin/assets/shaders/Builtin.ObjectShader.frag.spv")
add_custom_command(TARGET Main POST_BUILD
    COMMAND $ENV{VULKAN_SDK}/Bin/glslc.exe
        -fshader-stage=frag
        ${CMAKE_SOURCE_DIR}/assets/shaders/Builtin.ObjectShader.frag.glsl
        -o ${CMAKE_BINARY_DIR}/assets/shaders/Builtin.ObjectShader.frag.spv
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    BYPRODUCTS ${CMAKE_BINARY_DIR}/assets/shaders/Builtin.ObjectShader.frag.spv)

message("Copying assets...")
add_custom_command(TARGET Main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets)
