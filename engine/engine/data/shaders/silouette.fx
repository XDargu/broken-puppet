#include "render/ctes/shader_ctes.h"

Texture2D txDiffuse : register(t0);
Texture2D txNormal    : register(t1);
Texture2D txDepth   : register(t2);

Texture2D txType   : register(t9);

SamplerState samWrapLinear : register(s0);
SamplerState samClampLinear : register(s1);
SamplerState samClampPoint : register(s4);

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

static float3x3 conv =
{
	1. / 16., 1. / 8., 1. / 16.,
	1. / 8., 1. / 4., 1. / 8.,
	1. / 16., 1. / 8., 1. / 16.
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
	float3 wPos = getWorldCoords(screenCoords.xy, depth);

	float4 color_x = float4(0, 0, 0, 0);
	float4 color_y = float4(0, 0, 0, 0);

	float2 deltaaa = float2(1.0 / cameraHalfXRes, 1.0 / cameraHalfYRes) * 0.5;
	float2 delta = float2(0, 0);
	float factor = 1.f;
	bool near_player = false;

	// SOBEL, Manual TABS (for optimization purposes)
	color_x += txType.Sample(samClampPoint, input.UV + float2(deltaaa.x * -1, deltaaa.y * -1) * 1.5) * sobel_x[0][0];
	color_x += txType.Sample(samClampPoint, input.UV + float2(deltaaa.x * -1, deltaaa.y * 0)  * 1.5) * sobel_x[0][1];
	color_x += txType.Sample(samClampPoint, input.UV + float2(deltaaa.x * -1, deltaaa.y * 1)  * 1.5) * sobel_x[0][2];

	color_x += txType.Sample(samClampPoint, input.UV + float2(deltaaa.x * 1, deltaaa.y * -1)  * 1.5) * sobel_x[2][0];
	color_x += txType.Sample(samClampPoint, input.UV + float2(deltaaa.x * 1, deltaaa.y * 0)   * 1.5) * sobel_x[2][1];
	color_x += txType.Sample(samClampPoint, input.UV + float2(deltaaa.x * 1, deltaaa.y * 1)   * 1.5) * sobel_x[2][2];

	color_y += txType.Sample(samClampPoint, input.UV + float2(deltaaa.x * -1, deltaaa.y * -1) * 1.5) * sobel_y[0][0];
	color_y += txType.Sample(samClampPoint, input.UV + float2(deltaaa.x * 0, deltaaa.y * -1)  * 1.5) * sobel_y[1][0];
	color_y += txType.Sample(samClampPoint, input.UV + float2(deltaaa.x * 1, deltaaa.y * -1)  * 1.5) * sobel_y[2][0];
		  
	color_y += txType.Sample(samClampPoint, input.UV + float2(deltaaa.x * -1, deltaaa.y * -1) * 1.5) * sobel_y[0][0];
	color_y += txType.Sample(samClampPoint, input.UV + float2(deltaaa.x * 0, deltaaa.y * -1)  * 1.5) * sobel_y[1][0];
	color_y += txType.Sample(samClampPoint, input.UV + float2(deltaaa.x * 1, deltaaa.y * -1)  * 1.5) * sobel_y[2][0];


	// Old loops
	/*
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			factor = sobel_x[i][j];
			if (factor != 0) {
				float type_aux = txDepth.Sample(samClampPoint, input.UV + delta * 0.3).y;
				if (type_aux == 0.2) {
					near_player = true;
				}

				//float m_depth = txDepth.Sample(samClampPoint, input.UV + delta * 0.3).x;
				//if (m_depth < depth && (type_aux < 0.8 || type_aux > 0.9) && type_aux > 0) {
				//near_player = true;
				//}

				delta = float2(deltaaa.x * (i - 1), deltaaa.y * (j - 1)) * 5;
				float4 samp = txType.Sample(samClampPoint, input.UV + delta * 0.3);

				color_acum += color_x;

				color_x += samp * factor;
			}
		}
	}

	if (near_player) { return diffuse; }
	// There is no color, don't make the next step
	if (length(color_acum) == 0) { return diffuse; }
	
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			factor = sobel_y[i][j];
			if (factor != 0) {
				delta = float2(deltaaa.x * (i - 1), deltaaa.y * (j - 1)) * 5;
				float4 samp = txType.Sample(samClampPoint, input.UV + delta * 0.3);

					color_y += samp * factor;
			}
		}
	}*/
	float border = abs(color_x.x + color_y.x);
	//return abs(border);
	/*if (border > 0)
		return float4(0.45,0.8,0.63,0) * 0.6;	
	return float4(0, 0, 0, 0);*/

	if (border > 0)
		return float4(0.45, 0.8, 0.63, 0) * 0.6 + diffuse;
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


float4 PSSilouetteGlow(
	VS_TEXTURED_OUTPUT input,
	in float4 iPosition : SV_Position
	) : SV_Target
{

	// New code
	int3 screenCoords = uint3(iPosition.xy, 0);

	float depth = txDepth.Load(screenCoords).x;
	float4 diffuse = txDiffuse.Load(screenCoords);
	float4 silouette = txType.Load(screenCoords);
	float3 wPos = getWorldCoords(screenCoords.xy, depth);

	float3 normal = txNormal.Load(screenCoords).xyz;
	float3 wNormal = normalize(normal * 2.0f - 1.0f);

	float4 color = silouette;
	float2 delta = float2(0, 0);
	int samples = 3;
	float factor = 1.f / (samples * samples);


	for (int i = 0; i < samples; i++) {
		for (int j = 0; j < samples; j++) {
			//factor = conv[i][j];

			delta = float2(blur_delta.y * (i - (samples / 2)), blur_delta.y * (j - (samples / 2)))	;

			color += txType.Sample(samClampLinear, input.UV + delta * 0.0002) * factor;
		}
	}

	return diffuse + color;
	

	//return abs(border);
	return diffuse;
}