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
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR0;
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
VS_OUTPUT VS( float4 Pos : POSITION, float4 Color : COLOR )
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Pos = mul( Pos, World );
    output.Pos = mul( output.Pos, ViewProjection );
    output.Color = Color;
    return output;
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT VSNormal(float4 Pos : POSITION
  , float2 UV : TEXCOORD0
  , float3 Normal : NORMAL
  )
{
  VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;
  output.Pos = mul(Pos, World);
  output.Pos = mul(output.Pos, ViewProjection);
  output.Normal = mul(Normal, (float3x3)World);
  output.UV = UV + sin(world_time.x) * Normal.xz;
  output.WorldPos = mul(Pos, World);
  return output;
}



//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_Target
{
	return float4( input.Color.rgb, 0.5 );
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSTextured(VS_TEXTURED_OUTPUT input) : SV_Target
{
	/*
	//float Gamma = 2;
	//float Regions = 3;

	//lightIntensity = pow(lightIntensity, Gamma);
	//lightIntensity = floor(lightIntensity * Regions) / Regions;
	//lightIntensity = pow(lightIntensity, 1.0 / Gamma);

	*/

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
		float3 normalizedLightDirection = normalize(OmniLightPositions[i] - input.WorldPos);
		float attenuation = max(0, (OmniLightRadius[i].x - dist) / OmniLightRadius[i].x);
		//float attenuation = 1;

		lightAccum += max(dot(input.Normal, normalizedLightDirection), 0) * OmniLightColors[i] * (OmniLightColors[i].w * 10) * attenuation;
		
	}

	float4 color = txDiffuse.Sample(samWrapLinear, input.UV);
	return lightAccum * Tint * color;	

  //return txDiffuse.Sample(samWrapLinear, input.UV);
}

