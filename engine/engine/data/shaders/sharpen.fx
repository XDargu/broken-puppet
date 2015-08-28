#include "render/ctes/shader_ctes.h"

Texture2D txDiffuse : register(t0);

SamplerState samWrapLinear : register(s0);
SamplerState samClampLinear : register(s1);

//--------------------------------------------------------------------------------------
struct VS_TEXTURED_OUTPUT
{
  float4 Pos    : SV_POSITION;
  float2 UV     : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT VSSharpen(
  float4 Pos : POSITION
, float2 UV : TEXCOORD0
)
{
  VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;
  output.Pos = mul(Pos, World);
  output.Pos = mul(output.Pos, ViewProjection);
  output.UV = UV;
  return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSSharpen(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 original = txDiffuse.Sample(samClampLinear, input.UV);
	
	float4 color = float4(0, 0, 0, 0);
	float2 delta = float2(0, 0);
	float factor = 1.f;

	/*float3x3 conv =
	{
		1. / 16., 1. / 8., 1. / 16.,
		1. / 8., 1. / 4., 1. / 8.,
		1. / 16., 1. / 8., 1. / 16.
	};*/

	float3x3 conv =
	{
		0, -1, 0,
		-1, 5, -1,
		0, -1, 0
	};

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			factor = conv[i][j];
			delta = float2(sharpen_delta.x * (i - 1), sharpen_delta.y * (j - 1));
			color += txDiffuse.Sample(samClampLinear, input.UV + delta * 0.3) * factor;
		}
	}

	float alfa = amount;
	return original *(1 - alfa) + color * alfa;
}

