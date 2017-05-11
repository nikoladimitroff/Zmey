#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform pushConstants
{
	layout(offset = 16) vec4 color;
} u_push;

void main()
{
	outColor = u_push.color;
}