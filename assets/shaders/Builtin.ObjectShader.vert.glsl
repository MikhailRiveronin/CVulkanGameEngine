#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_position;

layout(set = 0, binding = 0) uniform global_uniform_object {
    mat4 view;
    mat4 proj;
} global;

layout(push_constant) uniform object_push_constants {
    mat4 world;
} object_push;

void main()
{
    gl_Position = global.proj * global.view * object_push.world * vec4(in_position, 1.0);
}
