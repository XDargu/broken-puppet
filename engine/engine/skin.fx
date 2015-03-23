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
  return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSTextured(VS_TEXTURED_OUTPUT input) : SV_Target
{
  return float4(input.UV, 0, 1); // txDiffuse.Sample(samWrapLinear, input.UV);
}

