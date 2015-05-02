//--------------------------------------------------------------------------------------
// File: Tutorial04.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "render/ctes/shader_ctes.h"

Texture2D txDiffuse : register(t0);
SamplerState samWrapLinear : register(s0);
SamplerState samClampLinear : register(s1);

//--------------------------------------------------------------------------------------
struct VS_TEXTURED_OUTPUT
{
	float4 Pos    : SV_POSITION;
	float2 UV     : TEXCOORD0;
	float3 Normal : NORMAL;
	float4 wPos    : TEXCOORD1;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT VS(
	float4 ipos     : POSITION
	, float2 iuv : TEXCOORD0
	, float3 inormal : NORMAL
	, uint4  bone_ids : BONEIDS
	, float4 weights : WEIGHTS
	)
{
	VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;

	matrix skin_mtx = bones[bone_ids.x] * weights[0]
		+ bones[bone_ids.y] * weights[1]
		+ bones[bone_ids.z] * weights[2]
		+ bones[bone_ids.w] * weights[3]
		;

	float4 skinned_pos = mul(ipos, skin_mtx);

	output.Pos = mul(skinned_pos, ViewProjection);
	output.wPos = mul(skinned_pos, World);
	output.Normal = mul(inormal, (float3x3) skin_mtx);
	output.UV = float2(iuv.x, 1 - iuv.y);
	//output.UV = bone_ids.xy / 50.;
	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSTextured(
	VS_TEXTURED_OUTPUT input
	, out float4 normal : SV_Target1
	, out float4 acc_light : SV_Target2
	, out float  depth : SV_Target3
	) : SV_Target
{
	//return txDiffuse.Sample(samWrapLinear, input.UV);
	//return float4(input.UV, 0, 1); // txDiffuse.Sample(samWrapLinear, input.UV);

	// Basic diffuse lighting
	float3 L = LightWorldPos.xyz - input.wPos.xyz;
	L = normalize(L);
	float3 N = normalize(input.Normal);
	float  diffuse_amount = saturate(dot(N, L));

	// Speculares
	float3 E = normalize(cameraWorldPos.xyz - input.wPos.xyz);
	//float3 H = normalize(E + L);
	//float  cos_beta = saturate( dot(H, N) );
	float3 ER = reflect(-E, N);
	float  cos_beta = saturate(dot(ER, L));
	float  spec_amount = pow(cos_beta, 20.);
	depth = dot(input.wPos - cameraWorldPos, cameraWorldFront) / cameraZFar;
	acc_light = float4(1,1,1,1);
	
	normal = (float4(N, 1) + 1.) * 0.5;

	float4 albedo = txDiffuse.Sample(samWrapLinear, input.UV);
	//return (albedo + spec_amount) * diffuse_amount;
	return albedo * diffuse_amount;
}

