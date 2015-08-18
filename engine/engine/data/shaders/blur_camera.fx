#include "render/ctes/shader_ctes.h"

Texture2D txDiffuse : register(t0);
Texture2D txDepth   : register(t2);
SamplerState samWrapLinear : register(s0);
SamplerState samClampLinear : register(s1);

//--------------------------------------------------------------------------------------
struct VS_TEXTURED_OUTPUT
{
  float4 Pos    : SV_POSITION;
  float2 UV     : TEXCOORD0;
};

float3 getWorldCoords(float2 screen_coords, float depth) {
	float3 view_dir = float3(cameraHalfXRes - screen_coords.x
		, cameraHalfYRes - screen_coords.y
		, cameraViewD);
	view_dir = view_dir / cameraViewD * cameraZFar * depth;

	float3 world_dir = view_dir.z * cameraWorldFront.xyz
		+ view_dir.y * cameraWorldUp.xyz
		+ view_dir.x * cameraWorldLeft.xyz;
	float3 wPos = cameraWorldPos.xyz + world_dir;
		return wPos;
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT VSBlurCamera(
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
float4 PSBlurCamera(
	VS_TEXTURED_OUTPUT input,
	in float4 iPosition : SV_Position
	) : SV_Target
{
	
	int3 ss_load_coords = uint3(iPosition.xy, 0);
	float depth = txDepth.Load(ss_load_coords).x;
	
	float4 wpos = float4(getWorldCoords(iPosition.xy, depth), 1);

	float4 H = float4(input.UV.x * 2 - 1, (1 - input.UV.y) * 2 - 1, depth, 1);
	
	float4 currentPos = mul(wpos, GameViewProjection);
	currentPos /= currentPos.w;

	float4 previousPos = mul(wpos, PrevViewProjection);
	previousPos /= previousPos.w;

	//return currentPos;
	float2 velocity = (currentPos - previousPos) / 2.f;

	// Get the initial color at this pixel.  
	float2 texCoord = input.UV;
	float4 origColor = txDiffuse.Sample(samClampLinear, texCoord);
	float4 color = txDiffuse.Sample(samClampLinear, texCoord);

	//return float4(velocity.xy, 0, 0);
	texCoord += velocity;
	float numSamples = 5;

	for (int i = 1; i < numSamples; ++i)
	{
		// Sample the color buffer along the velocity vector.  
		float4 currentColor = txDiffuse.Sample(samClampLinear, texCoord);

		// Add the current color to our color sum.  
		color += currentColor;

		texCoord += velocity * 1;
	}

	// Average all of the samples to get the final blur color.	
	float4 finalColor = color / numSamples;
	float radial = saturate(abs(H + 0.4));
	
	return finalColor * radial + origColor * (1 - radial);

	
	return txDiffuse.Sample(samClampLinear, input.UV);
}