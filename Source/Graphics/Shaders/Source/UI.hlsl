#include "Platform.hlsl"

struct VertexInput
{
	float2 Position : POSITION0;
	float2 TextureUV : TEXCOORD0;
	float4 Color : COLOR0;
};

struct VertexOutput
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR0;
	float2 TextureUV: TEXCOORD0;
};

cbuffer VertexPushs : register(b0) PUSH_CONSTANT
{
	float2 Scale;
	float2 Translate;
};

Texture2D txBuffer : register(t0) SET_BINDING(0);
SamplerState txBufferSampler : register(s0) SET_BINDING(1);

VertexOutput VertexShaderMain(VertexInput input)
{
	VertexOutput result;
	result.Position = float4(input.Position * Scale + Translate, 0, 1);
	result.Color = input.Color;
	result.TextureUV = input.TextureUV;

	return result;
}

float4 PixelShaderMain(VertexOutput input) : SV_TARGET
{
	return input.Color * txBuffer.Sample(txBufferSampler, input.TextureUV);
}
