#include "Platform.hlsl"

struct VertexInput
{
	uint index : SV_VertexID;
};

cbuffer VertexPushs : register(b0) PUSH_CONSTANT
{
	float x;
	float y;
	float width;
	float height;
};

cbuffer PixelPushs : register(b1) PUSH_CONSTANT
{
	float4 color : packoffset(c1);
};

float4 VertexShaderMain(VertexInput i) : SV_POSITION
{
	if(i.index == 0)
	{
		return float4(x, y, 0.0, 1.0);
	}
	else if(i.index == 1)
	{
		return float4(x + width, y, 0.0, 1.0);
	}
	else if(i.index == 2)
	{
		return float4(x, y + height, 0.0, 1.0);
	}
	else
	{
		return float4(x + width, y + height, 0.0, 1.0);
	}
}

float4 PixelShaderMain() : SV_TARGET
{
	return color;
}
