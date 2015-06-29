#include "render/ctes/shader_ctes.h"

Texture2D txDiffuse : register(t0);
Texture2D txNormal    : register(t1);
Texture2D txDepth   : register(t2);

Texture2D txType   : register(t9);

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
VS_TEXTURED_OUTPUT VSSilouette(
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

static float3x3 sobel_x =
{
	-1, 0, 1,
	-2, 0, 2,
	-1, 0, 1
};

static float3x3 sobel_y =
{
	-1, -2, -1,
	0,  0,  0,
	1, 2,  1
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSSilouette(
	VS_TEXTURED_OUTPUT input,
	in float4 iPosition : SV_Position
	) : SV_Target
{

	// New code
	int3 screenCoords = uint3(iPosition.xy, 0);

	float depth = txDepth.Load(screenCoords).x;
	float4 diffuse = txDiffuse.Load(screenCoords);
	float4 type = txType.Load(screenCoords);
	float3 wPos = getWorldCoords(screenCoords, depth);

	float3 normal = txNormal.Load(screenCoords).xyz;
	float3 wNormal = normalize(normal * 2.0f - 1.0f);

	float4 color_x = float4(0, 0, 0, 0);
	float4 color_y = float4(0, 0, 0, 0);

	float2 delta = float2(0, 0);
	float factor = 1.f;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			delta = float2(glow_delta.x * (i - 1), glow_delta.y * (j - 1)) * 5;
			float4 samp = txType.Sample(samClampLinear, input.UV + delta * 0.3);

			factor = sobel_x[i][j];
			color_x += samp * factor;

			factor = sobel_y[i][j];
			color_y += samp * factor;
		}
	}

	float border = abs(color_x.x + color_y.x);
	//return abs(border);
	if (border > 0)
		diffuse = float4(0,0.7,0,0);
	
	return diffuse;
}

float4 PSSilouetteType(
VS_TEXTURED_OUTPUT input,
in float4 iPosition : SV_Position
) : SV_Target
	{
		// New code
		int3 screenCoords = uint3(iPosition.xy, 0);

		float type = txDepth.Load(screenCoords).y;

		if (type >= 0.8 && type <= 0.9)
			return float4(1, 0, 0, 1);
		return float4(0, 0, 0, 1);
	}
