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

float2 getDistorsion(float2 tex, float k, float kcube) {
	float r2 = (tex.x - 0.5) * (tex.x - 0.5) + (tex.y - 0.5) * (tex.y - 0.5);
	float f = 0;


	//only compute the cubic distortion if necessary
	if (kcube == 0.0){
		f = 1 + r2 * k;
	}
	else{
		f = 1 + r2 * (k + kcube * sqrt(r2));
	};

	// get the right pixel for the current position
	float x = f*(tex.x - 0.5) + 0.5;
	float y = f*(tex.y - 0.5) + 0.5;

	return float2(x, y);
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSSSAO(VS_TEXTURED_OUTPUT input,
	in float4 iPosition : SV_Position
	) : SV_Target
{
	float4 color = txDiffuse.Sample(samClampLinear, input.UV);
	float2 tex = input.UV;

	float2 rDist = getDistorsion(tex, -0.05, 0.05);
	float2 gDist = getDistorsion(tex, -0.08, 0.08);
	float2 bDist = getDistorsion(tex, -0.11, 0.11);
  
	float3 inputDistordr = txDiffuse.Sample(samClampLinear, rDist);
	float3 inputDistordg = txDiffuse.Sample(samClampLinear, gDist);
	float3 inputDistordb = txDiffuse.Sample(samClampLinear, bDist);


  return float4(inputDistordr.r, inputDistordg.g, inputDistordb.b, 1);
  

  //return original;
  //return blurred;
  //alfa = 1.5;
  //return original *(1 - alfa) + blurred * alfa;  
  return color;
}

