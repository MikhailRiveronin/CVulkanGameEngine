#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texcoord;

layout(set = 0, binding = 0) uniform global_uniform_object {
	mat4 view;
    mat4 projection;
} global_ubo;

layout(set = 3, binding = 5) uniform sampler2D myTextures[568];
layout(set = 12, binding = 2) uniform sampler2D UnboundBindless[];

layout(push_constant) uniform push_constants {
	
	// Only guaranteed a total of 128 bytes.
	mat4 model; // 64 bytes
	int ind;
} u_push_constants;

layout(location = 0) out int out_mode;

// Data Transfer Object
layout(location = 1) out struct dto {
	vec2 tex_coord;
} out_dto;

void main()
{
	vec4 outColor = texture(UnboundBindless[u_push_constants.ind], in_texcoord);

	out_dto.tex_coord = in_texcoord;
    gl_Position = global_ubo.projection * global_ubo.view * u_push_constants.model * vec4(in_position, 1.0);
}
