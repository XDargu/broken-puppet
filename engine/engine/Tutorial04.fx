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
float4 PS( VS_OUTPUT input ) : SV_Target
{
	return float4( input.Color.rgb, 0.5 );
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSTextured(VS_TEXTURED_OUTPUT input) : SV_Target
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
			// Sin atenuación
			//float attenuation = dist < OmniLightRadius[i].x ? 1 : 0;

			// Luz acumulada por ángulo, atenuación y distancia
			lightAccum += max(dot(input.Normal, normalizedLightDirection), 0) * OmniLightColors[i] * (OmniLightColors[i].w * 10) * attenuation;
		}
	}

	float4 color = txDiffuse.Sample(samWrapLinear, input.UV);

	// Blur
	float pixelInc = 0.001;
	/*float4 sum = float4(0, 0, 0, 0);
	sum += txDiffuse.Sample(samWrapLinear, float2(input.UV.x - pixelInc, input.UV.y - pixelInc));
	sum += txDiffuse.Sample(samWrapLinear, float2(input.UV.x + 0, input.UV.y - pixelInc));
	sum += txDiffuse.Sample(samWrapLinear, float2(input.UV.x + pixelInc, input.UV.y - pixelInc));
	sum += txDiffuse.Sample(samWrapLinear, float2(input.UV.x - pixelInc, input.UV.y + 0));
	sum += txDiffuse.Sample(samWrapLinear, float2(input.UV.x + 0, input.UV.y + 0));
	sum += txDiffuse.Sample(samWrapLinear, float2(input.UV.x + pixelInc, input.UV.y + 0));
	sum += txDiffuse.Sample(samWrapLinear, float2(input.UV.x - pixelInc, input.UV.y + pixelInc));
	sum += txDiffuse.Sample(samWrapLinear, float2(input.UV.x + 0, input.UV.y + pixelInc));
	sum += txDiffuse.Sample(samWrapLinear, float2(input.UV.x + pixelInc, input.UV.y + pixelInc));
	color = sum / 9;*/

	// Prewitt
	/*float4 sum = float4(0, 0, 0, 0);
	float4 sum2 = float4(0, 0, 0, 0);
	sum += txDiffuse.Sample(samWrapLinear, float2(input.UV.x - pixelInc, input.UV.y - pixelInc)) * -1;
	sum += txDiffuse.Sample(samWrapLinear, float2(input.UV.x + 0, input.UV.y - pixelInc)) * 0;
	sum += txDiffuse.Sample(samWrapLinear, float2(input.UV.x + pixelInc, input.UV.y - pixelInc)) * 1;
	sum += txDiffuse.Sample(samWrapLinear, float2(input.UV.x - pixelInc, input.UV.y + 0)) * -1;
	sum += txDiffuse.Sample(samWrapLinear, float2(input.UV.x + 0, input.UV.y + 0)) * 0;
	sum += txDiffuse.Sample(samWrapLinear, float2(input.UV.x + pixelInc, input.UV.y + 0)) * 1;
	sum += txDiffuse.Sample(samWrapLinear, float2(input.UV.x - pixelInc, input.UV.y + pixelInc)) * -1;
	sum += txDiffuse.Sample(samWrapLinear, float2(input.UV.x + 0, input.UV.y + pixelInc)) * 0;
	sum += txDiffuse.Sample(samWrapLinear, float2(input.UV.x + pixelInc, input.UV.y + pixelInc)) * 1;
	
	sum2 += txDiffuse.Sample(samWrapLinear, float2(input.UV.x - pixelInc, input.UV.y - pixelInc)) * -1;
	sum2 += txDiffuse.Sample(samWrapLinear, float2(input.UV.x + 0, input.UV.y - pixelInc)) * -1;
	sum2 += txDiffuse.Sample(samWrapLinear, float2(input.UV.x + pixelInc, input.UV.y - pixelInc)) * -1;
	sum2 += txDiffuse.Sample(samWrapLinear, float2(input.UV.x - pixelInc, input.UV.y + 0)) * 0;
	sum2 += txDiffuse.Sample(samWrapLinear, float2(input.UV.x + 0, input.UV.y + 0)) * 0;
	sum2 += txDiffuse.Sample(samWrapLinear, float2(input.UV.x + pixelInc, input.UV.y + 0)) * 0;
	sum2 += txDiffuse.Sample(samWrapLinear, float2(input.UV.x - pixelInc, input.UV.y + pixelInc)) * 1;
	sum2 += txDiffuse.Sample(samWrapLinear, float2(input.UV.x + 0, input.UV.y + pixelInc)) * 1;
	sum2 += txDiffuse.Sample(samWrapLinear, float2(input.UV.x + pixelInc, input.UV.y + pixelInc)) * 1;
	
	color = sum / 9 + sum2 / 9;*/


	float4 result = lightAccum * color /* Tint*/;

	// Pruebas de cámara
	/*float3 normalizeCameraDirection = normalize(CameraPosition - input.WorldPos);
	float distCamera = distance(input.WorldPos, CameraPosition);
	float normalCamera = max(0, dot(input.Normal, normalizeCameraDirection));

	float lightAttenuation = max(0, (20 - distCamera) / 20);
	float4 lightChange = float4(1, 1, 1, 1);

	float3 greyScaleConversion = float3(0.3f, 0.59f, 0.11f);

	float gamma = 3;
	float contrast = 2;
	float lightCorrection = (pow(cos(normalCamera) * gamma, contrast) / gamma);
	result = lightCorrection * result;*/

	/*float Gamma = 1;
	float Regions = 3;

	lightAccum = pow(lightAccum, Gamma);
	lightAccum = floor(lightAccum * Regions) / Regions;
	lightAccum = pow(lightAccum, 1.0 / Gamma);*/

	// Iluminación de rim
	/*float rim = 1 - max(dot(normalizeCameraDirection, input.Normal), 0.0);
	rim = smoothstep(0.5, 1.0, rim);
	float4 finalRim = float4(1, 1, 1, 1) * float4(rim, rim, rim, 1);*/

	float checker = (fmod(floor(input.UV.x * 10) + floor(input.UV.y * 10), 2) < 1) ? 0.5 : 1;
	return result /* checker /*+ finalRim*/;

  //return txDiffuse.Sample(samWrapLinear, input.UV);
}

