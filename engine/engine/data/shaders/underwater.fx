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
float4 PSUnderwater(in float4 iPosition : SV_Position, VS_TEXTURED_OUTPUT input) : SV_Target
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

	// ÑAPA de render de frames
	float4 res = float4(0, 0, 0, 0);
	if ((input.UV.y < cameraCinematicBands) || (input.UV.y >(1 - cameraCinematicBands)))
		return res;
	else
		res = txDiffuse.Sample(samClampLinear, input.UV);

	int3 screenCoords = uint3(iPosition.xy, 0);
	
	int yres = cameraHalfYRes * 2;
	int xres = cameraHalfXRes * 2;
	int xpos = input.UV.x * xres;
	int ypos = input.UV.y * yres;
	float fps = 0;
	fps = frame_list[xpos].x;

	if (yres - ypos == 60) {
		return float4(1, 0, 0, 1);
	}

	if (yres - ypos == 60) {
		return float4(0, 0, 1, 1);
	}

	if (yres - ypos < fps) {
		return float4(1, 1, 1, 1);
	}


	return res;
}