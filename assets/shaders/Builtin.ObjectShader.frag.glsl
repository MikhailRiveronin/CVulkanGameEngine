#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 out_colour;

layout(set = 1, binding = 0) uniform object_uniform_buffer {
    vec4 diffuse_color;
} object_ubo;

layout(set = 1, binding = 1) uniform sampler2D diffuse_texture;

void main()
{
    out_colour = object_ubo.diffuse_color * texture(diffuse_texture, vec2(0.5f, 0.5f));
}
