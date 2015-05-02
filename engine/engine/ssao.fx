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

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT VSSSAO(
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
float4 PSSSAO(VS_TEXTURED_OUTPUT input,
	in float4 iPosition : SV_Position
	) : SV_Target
{
  float4 color = txDiffuse.Sample(samClampLinear, input.UV);
  
  
  

  //return original;
  //return blurred;
  //alfa = 1.5;
  //return original *(1 - alfa) + blurred * alfa;  
  return color;
}

