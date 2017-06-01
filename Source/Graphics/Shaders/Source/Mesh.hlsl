#include "Platform.hlsl"

struct VertexInput
{
	float3 Position : POSITION0;
	float3 Normal : NORMAL0;
};

struct VertexOutput
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR0;
};

cbuffer VertexPushs : register(b0) PUSH_CONSTANT
{
	float4x4 ModelViewProjectionMatrix;
};

VertexOutput VertexShaderMain(VertexInput input)
{
	VertexOutput result;
	const float3 LightDir = float3(1.0, 0.0, 0.0);
	result.Position = mul(ModelViewProjectionMatrix, float4(input.Position, 1.0));
	//result.Color = float4(input.Normal, 1.0);

	// Simple diffuse lighting
	result.Color = float4(1.0, 0.0, 0.0, 1.0);
	result.Color *= dot(input.Normal, LightDir) * 2.0;
	result.Color += float4(0.0, 1.0, 0.0, 1.0) * 0.05; // ambient

	return result;
}

float4 PixelShaderMain(VertexOutput input) : SV_TARGET
{
	return input.Color;
}
