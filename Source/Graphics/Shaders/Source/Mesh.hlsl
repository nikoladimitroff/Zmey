#include "Platform.hlsl"

struct VertexInput
{
	float3 Position : POSITION0;
};

cbuffer VertexPushs : register(b0) PUSH_CONSTANT
{
	float x;
	float y;
	float width;
	float height;
};

cbuffer PixelPushs : register(b0) PUSH_CONSTANT
{
	float4 color : packoffset(c1);
};

float4 VertexShaderMain(VertexInput i) : SV_POSITION
{
	float4 pos = float4(i.Position, 1.0);
	pos.x += 60.0;
	pos.x /= 120.0;

	pos.y += 6.0;
	pos.y /= 12.0;

	pos.z += 80.0;
	pos.z /= 160.0;
	return pos;
}

float4 PixelShaderMain(float4 pos : SV_POSITION) : SV_TARGET
{
	return float4(1.0, 0.0, 0.0, 1.0);
}
