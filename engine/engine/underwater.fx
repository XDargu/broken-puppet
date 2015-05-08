#include "render/ctes/shader_ctes.h"

Texture2D txDiffuse : register(t0);
Texture2D txLuminance : register(t1);

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
VS_TEXTURED_OUTPUT VSUnderwater(
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
float4 PSUnderwater(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float distortion = uw_amount;
	float2 centerCoord = float2(0.5f, 0.5f);
		
		if (distortion > 0) {
		float2 distance = abs(input.UV - centerCoord);
		float scalar = length(distance);

		// invert the scale so 1 is centerpoint
		scalar = abs(1 - scalar);
		float wave = sin(world_time);
		// calculate how far to distort for this pixel    
		float sinoffset = sin(wave / scalar);
		sinoffset = clamp(sinoffset, -1, 1);

		// calculate which direction to distort
		float sinsign = cos(wave / scalar);

		// reduce the distortion effect
		sinoffset = sinoffset * distortion / 32;

		// pick a pixel on the screen for this pixel, based on
		// the calculated offset and direction
		float4 color = txDiffuse.Sample(samClampLinear, input.UV + (sinoffset*sinsign));
		return color + float4(0, 0.0, 0.05, 1);
	}

	return txDiffuse.Sample(samClampLinear, input.UV);
}