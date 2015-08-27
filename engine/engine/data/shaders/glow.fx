#include "render/ctes/shader_ctes.h"

Texture2D txDiffuse : register(t0);
Texture2D txLuminance : register(t1);
Texture2D txDepth     : register(t2);
Texture2D txAccLight  : register(t3);

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
VS_TEXTURED_OUTPUT VSGlow(
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

static float3x3 conv =
{
	1. / 16., 1. / 8., 1. / 16.,
	1. / 8., 1. / 4., 1. / 8.,
	1. / 16., 1. / 8., 1. / 16.
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSBlur(VS_TEXTURED_OUTPUT input) : SV_Target
{
	if (blur_amount > 0) {
		float4 color = float4(0, 0, 0, 0);
			float2 delta = float2(0, 0);
			float factor = 1.f;		

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				factor = conv[i][j];
				delta = float2(blur_delta.x * (i - 1), blur_delta.y * (j - 1)) * blur_amount;
				color += txDiffuse.Sample(samClampLinear, input.UV + delta * 0.3) * factor;
			}
		}

		return color * 1;
	}
}
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSGlowLights(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 color = txDiffuse.Sample(samClampLinear, input.UV);
	float luminance = txAccLight.Sample(samClampLinear, input.UV);
	return color * (luminance - 1);
	if (luminance > 3) {
		return color;
	}
	else {
		return float4(0, 0, 0, 0);
	}
}



//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSGlow(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 original = txDiffuse.Sample(samClampLinear, input.UV);
	float4 depth = txDepth.Sample(samClampLinear, input.UV);
	return original;
  //return txLuminance.Sample(samClampLinear, input.UV);
  // Blur the glow image
  float4 luminance = float4(0, 0, 0, 0);
  float2 delta = float2(0, 0);
  float factor = 1.f;

  for (int i = 0; i < 3; i++) {
	  for (int j = 0; j < 3; j++) {
		  factor = conv[i][j];
		  delta = float2(glow_delta.x * (i - 1), glow_delta.y * (j - 1)) * 20;
		  luminance += txLuminance.Sample(samClampLinear, input.UV + delta * 0.1) * factor;
	  }
  }

  
  
  return original + luminance * 0.5;
  //return original;
  //return blurred;
  //return luminance;
  if (luminance.r > 0.1) {
	  float alfa = 0.8;
	  return original *(1 - alfa) + luminance * alfa;
  }
  return original;
  
}

