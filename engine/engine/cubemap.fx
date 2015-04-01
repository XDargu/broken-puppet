//--------------------------------------------------------------------------------------
// File: Tutorial04.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "render/ctes/shader_ctes.h"

float PI = 3.14159;

Texture2D txDiffuse : register(t0);
TextureCube txCubemap : register(t3);
SamplerState samWrapLinear : register(s0);
SamplerState samClampLinear : register(s1);
SamplerState samCube : TEXUNIT3 {
	Texture = txCubemap;
	MIPFILTER = LINEAR;
	MINFILTER = LINEAR;
	MAGFILTER = LINEAR;
};

//--------------------------------------------------------------------------------------
struct VS_TEXTURED_OUTPUT
{
  float4 Pos    : SV_POSITION;
  float2 UV     : TEXCOORD0;
  float3 Normal : NORMAL;
  float3 WorldPos     : TEXCOORD1;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT VSNormal(float4 Pos : POSITION
	, float2 UV : TEXCOORD0
	, float3 Normal : NORMAL
	)
{
	VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;
	/*float3 wpos = mul(Pos, World);
	float3 dir = mul(float3(0, 1, 0), World);
	float distCamera = distance(wpos, CameraPosition);
	float3 mPos = Pos.xyz + float3(0, sin(world_time * 0.5), 0) * (pow(max(0, distCamera - 4), 2) * 0.035);
	Pos = float4(mPos.x, mPos.y, mPos.z, Pos.w);*/


	output.Pos = mul(Pos, World);
	output.Pos = mul(output.Pos, ViewProjection);
	output.Normal = mul(Normal, (float3x3)World);
	output.UV = UV /*+ world_time * Normal.xz*/;
	output.WorldPos = mul(Pos, World);
	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSTextured(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float3 N = normalize(input.Normal);
	float3 I = input.WorldPos - CameraPosition;
	I = normalize(I);
	float3 R = reflect(I, N);
	
	float3 reflectedColor = txCubemap.Sample(samCube, R);		
	
	float4 color = txDiffuse.Sample(samWrapLinear, input.UV);

	return float4(reflectedColor * color, 1);
}