#include "render/ctes/shader_ctes.h"

Texture2D txDiffuse : register(t0);
Texture2D txDepth     : register(t2);
Texture2D txNoise    : register(t9);

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
VS_TEXTURED_OUTPUT VSFog(
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
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSFog(VS_TEXTURED_OUTPUT input, in float4 iPosition : SV_Position) : SV_Target
{
  float4 original = txDiffuse.Sample(samClampLinear, input.UV);
  float depth = txDepth.Sample(samClampLinear, input.UV).x;
  
  int3 screenCoords = uint3(iPosition.xy, 0);
  float3 wPos = getWorldCoords(screenCoords, depth.x);
  float3 I = wPos - cameraWorldPos.xyz;

  float4 fogColor = fog_color;
  float atten = 1;
  atten = clamp((fog_distance / 50) - depth, 0, 1);
  
  float noise = txNoise.Sample(samWrapLinear, wPos.xz * 0.1	 + world_time.xx*0.03) * 2 - 1;

  noise = 0;
  if (wPos.y < fog_level + noise) {
	  float dist = fog_level - wPos.y;
	  atten = 1 - (dist / 15 - noise);
  }  
  //return original;
  return lerp(original, fogColor, min(1, (1-atten)));
}

