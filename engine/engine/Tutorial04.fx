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
  output.Normal = Normal;
  output.UV = UV + sin(world_time.x) * Normal.xz;
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
	const float3 XAXIS = { 1, 0, 0 };
	const float3 YAXIS = { 0, 1, 0 };
	const float3 ZAXIS = { 0, 0, 1 };

	const float lXZ = (dot(input.Normal, XAXIS) + 1) / 2;
	const float lYZ = (dot(input.Normal, YAXIS) + 1) / 2;

	float3 lightDir = { Tint.x, -Tint.y, Tint.z };

	//float3 local_lightDirection = mul(lightDirection, World);
	//local_lightDirection = mul(local_lightDirection, ViewProjection);

	float lightIntensity = dot(input.Normal, -lightDir);

	//float Gamma = 2;
	//float Regions = 3;

	//lightIntensity = pow(lightIntensity, Gamma);
	//lightIntensity = floor(lightIntensity * Regions) / Regions;
	//lightIntensity = pow(lightIntensity, 1.0 / Gamma);

	float light = lXZ / 2 + lYZ / 2;
	float4 lv = { light, light, light, 1.f };
	float4 color = txDiffuse.Sample(samWrapLinear, input.UV);
	color = lv;
	return color * lightIntensity;

  //return txDiffuse.Sample(samWrapLinear, input.UV);
}

