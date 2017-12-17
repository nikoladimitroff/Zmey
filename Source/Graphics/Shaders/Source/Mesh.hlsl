#include "Platform.hlsl"

struct VertexInput
{
	float3 Position : POSITION0;
	float3 Normal : NORMAL0;
};

struct VertexOutput
{
	float4 Position : SV_POSITION;
	float3 WorldPosition : POSITION0;
	float3 Normal : NORMAL0;
};

cbuffer VertexPushs : register(b0) PUSH_CONSTANT
{
	float4x4 WorldViewProjectionMatrix;
	float4x4 WorldMatrix;
	float3 Color;
	float3 LightDirection; // Directional Light
	float3 EyePosition;
};

VertexOutput VertexShaderMain(VertexInput input)
{
	VertexOutput result;
	result.Position = mul(WorldViewProjectionMatrix, float4(input.Position, 1.0));
	result.WorldPosition = mul(WorldMatrix, float4(input.Position, 1.0)).xyz;
	result.Normal = normalize(mul(WorldMatrix, float4(input.Normal, 0.0)).xyz);

	return result;
}

float4 PixelShaderMain(VertexOutput input) : SV_TARGET
{
	// Simple diffuse lighting
	float4 color = float4(Color, 1.0);

	// diffuse
	float diffuseFactor = saturate(dot(input.Normal, -LightDirection));

	// specular
	float3 eyeDirection = normalize(EyePosition - input.WorldPosition);
	float3 reflected = reflect(LightDirection, input.Normal);
	float specularFactor = pow(saturate(dot(reflected, eyeDirection)), 24);

	// ambient
	float ambientFactor = 0.1;

	return (color * diffuseFactor)
		+ (color * specularFactor)
		+ (color * ambientFactor);
}
