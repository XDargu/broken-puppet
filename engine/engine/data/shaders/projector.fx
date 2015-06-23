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
SamplerState samBorderLinear : register(s2);

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
  float4 wPos   : TEXCOORD1;
  float3 Normal : NORMAL;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT VS(float4 Pos : POSITION
  , float2 UV : TEXCOORD0
  , float3 Normal : NORMAL
  )
{
  VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;
  float4 world_pos = mul(Pos, World);
  output.Pos = mul(world_pos, ViewProjection);
  output.Normal = Normal;
  output.UV = UV;

  output.wPos = world_pos;


  return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_TEXTURED_OUTPUT input) : SV_Target
{

  float4 light_proj_coords = mul(input.wPos, LightViewProjectionOffset);
  light_proj_coords.xyz /= light_proj_coords.w;
  //if (light_proj_coords.w < 0)
  //  return float4(1, 0, 0, 1);
  float2 uv2 = light_proj_coords.xy;
  //if (light_proj_coords.x < 0 || light_proj_coords.x > 1.f)
  //  return float4(0, 0, 1, 1);
  //if (light_proj_coords.y < 0 || light_proj_coords.y > 1.f)
  //  return float4(0, 0, 1, 1);
  return txDiffuse.Sample(samBorderLinear, uv2);
}

