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
	float4 Pos     : SV_POSITION;
	float2 UV      : TEXCOORD0;
	float4 wPos    : TEXCOORD1;
	float3 wNormal : NORMAL;
	float4 wTangent : TANGENT;
	float2 UVL      : TEXCOORD2;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT VS(
	float4 Pos : POSITION
	, float2 UV : TEXCOORD0
	, float3 Normal : NORMAL
	, float2 UVL : TEXCOORD1
	, float4 Tangent : TANGENT
	)
{
	VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;
	output.wNormal = mul(Normal, (float3x3)World);
	float4 world_pos = mul(Pos, World);
	world_pos.xyz += output.wNormal;
	output.Pos = mul(world_pos, ViewProjection);	
	output.UV = UV * 1;
	output.UVL = UVL;
	output.wPos = world_pos;
	// Rotate the tangent and keep the w value
	output.wTangent.xyz = mul(Tangent.xyz, (float3x3)World);
	output.wTangent.w = Tangent.w;
	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(
	VS_TEXTURED_OUTPUT input,
	in float4 iPosition : SV_Position
	) : SV_Target
{	
	return float4(0, 1, 0, 1);
}