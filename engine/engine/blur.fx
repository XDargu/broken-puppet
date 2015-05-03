#include "render/ctes/shader_ctes.h"

Texture2D txDiffuse : register(t0);
Texture2D txBlurred : register(t1);
Texture2D txDepth   : register(t2);
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
VS_TEXTURED_OUTPUT VSBlur(
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
float4 PSBlur(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 color = float4(0, 0, 0, 0);
	float2 delta = float2(0, 0);
	float factor = 1.f;

	float3x3 conv =
	{
		1. / 16., 1. / 8., 1. / 16.,
		1. / 8., 1. / 4., 1. / 8.,
		1. / 16., 1. / 8., 1. / 16.
	};

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			factor = conv[i][j];
			delta = float2(blur_delta.x * (i - 1), blur_delta.y * (j - 1));
			color += txDiffuse.Sample(samClampLinear, input.UV + delta * 0.3) * factor;
		}
	}

  return color * 1;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSBlurByZ(VS_TEXTURED_OUTPUT input) : SV_Target
{
  float4 original = txDiffuse.Sample(samClampLinear, input.UV);
  float4 blurred = txBlurred.Sample(samClampLinear, input.UV);
  float  depth = txDepth.Sample(samClampLinear, input.UV);
  float alfa = 1-saturate(abs( depth - 0.1) * 10);
  
  //return original;
  //return blurred;
  alfa = 1.5;
  return original *(1 - alfa) + blurred * alfa;
  
}

