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
TextureCube txCubemap : register(t8);
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
	// Luz inicial
	/*float4 lightAccum = AmbientLight * AmbientLight.w;

		// Luz direccional
		for (int i = 0; i < LightCount; i++)
		{
			lightAccum += max(dot(input.Normal, -LightDirections[i]), 0) * LightColors[i] * (LightColors[i].w * 10);
		}

	// Luz puntual
	for (int i = 0; i < OmniLightCount; i++)
	{
		float dist = distance(input.WorldPos, OmniLightPositions[i]);
		if (dist < OmniLightRadius[i].x) {
			float3 normalizedLightDirection = normalize(OmniLightPositions[i] - input.WorldPos);
				// Atenuación lineal
				float attenuation = max(0, 1 - (dist / OmniLightRadius[i].x));
			// Sin atenuación
			//float attenuation = dist < OmniLightRadius[i].x ? 1 : 0;

			// Luz acumulada por ángulo, atenuación y distancia
			lightAccum += max(dot(input.Normal, normalizedLightDirection), 0) * OmniLightColors[i] * (OmniLightColors[i].w * 10) * attenuation;
		}
	}
	*/
	float3 N = normalize(input.Normal);
	float3 I = input.WorldPos - cameraWorldPos.xyz;
	I = normalize(I);
	float3 R = refract(I, N, 1);

		float3 reflectedColor = txCubemap.Sample(samCube, R);

		return float4(reflectedColor.xyz, 0);
		//float4 color = txDiffuse.Sample(samWrapLinear, input.UV);

		//float4 finalColor = float4(reflectedColor * color, 1) * lightAccum * 10;

	/*float3 dir_to_eye = normalize(cameraWorldPos.xyz - input.WorldPos);
	float3 N = normalize(input.Normal.xyz);

	
		float3 N_reflected = reflect(-dir_to_eye, N);
		float4 env = txCubemap.Sample(samWrapLinear, N_reflected);
		return env;*/
	
	
	//return float4(reflectedColor, 0);
}