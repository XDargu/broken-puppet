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
  float3 WorldPos     : TEXCOORD1;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT VS(
    float4 ipos     : POSITION
  , float2 iuv      : TEXCOORD0
  , float3 inormal  : NORMAL
  , uint4  bone_ids : BONEIDS
  , float4 weights  : WEIGHTS
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
  output.Normal = mul(inormal, (float3x3) skin_mtx);
  output.UV = iuv;
  output.UV = bone_ids.xx / 50.;
  output.WorldPos = output.Pos;
  return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSTextured(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 color = txDiffuse.Sample(samWrapLinear, input.UV);
	//return float4(input.UV, 0, 1); // txDiffuse.Sample(samWrapLinear, input.UV);
	return color;
}

float4 PSTexturedLighting(VS_TEXTURED_OUTPUT input) : SV_Target
{

	// Luz inicial
	float4 lightAccum = AmbientLight * AmbientLight.w;

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

			// Luz acumulada por ángulo, atenuación y distancia
			lightAccum += max(dot(input.Normal, normalizedLightDirection), 0) * OmniLightColors[i] * (OmniLightColors[i].w * 10) * attenuation;
		}
	}

	float4 color = txDiffuse.Sample(samWrapLinear, input.UV);

	float4 result = lightAccum * color /* Tint*/;

	float checker = (fmod(floor(input.UV.x * 10) + floor(input.UV.y * 10), 2) < 1) ? 0.5 : 1;
	return result /* checker /*+ finalRim*/;

	//return txDiffuse.Sample(samWrapLinear, input.UV);
}

