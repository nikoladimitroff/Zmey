#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(push_constant) uniform pushConstants
{
	float x;
	float y;
	float width;
	float height;
} u_push;

void main()
{
	if(gl_VertexIndex == 0)
	{
		gl_Position = vec4(u_push.x, u_push.y, 0.0, 1.0);
	}
	else if(gl_VertexIndex == 1)
	{
		gl_Position = vec4(u_push.x + u_push.width, u_push.y, 0.0, 1.0);
	}
	else if(gl_VertexIndex == 2)
	{
		gl_Position = vec4(u_push.x, u_push.y + u_push.height, 0.0, 1.0);
	}
	else
	{
		gl_Position = vec4(u_push.x + u_push.width, u_push.y + u_push.height, 0.0, 1.0);
	}
}