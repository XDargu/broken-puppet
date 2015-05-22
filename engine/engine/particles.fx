#include "render/ctes/shader_ctes.h"

Texture2D txDiffuse : register(t0);
Texture2D txDepth   : register(t2);
SamplerState samWrapLinear : register(s0);
SamplerState samClampLinear : register(s1);

//--------------------------------------------------------------------------------------
struct VS_TEXTURED_OUTPUT
{
  float4 Pos    : SV_POSITION;
  float2 UV     : TEXCOORD0;
  float3 wPos : TEXCOORD1;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT VS(
  float4 Pos : POSITION0              // Stream 0
, float2 UV : TEXCOORD0
, float3 InstancePos : POSITION1      // Stream 1
, float  InstanceAge : TEXCOORD1    // Stream 1
/*, float  InstanceLifespan : TEXCOORD2    // Stream 1
, float  InstanceSize : TEXCOORD3    // Stream 1*/
)
{
  VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;
  float3 wpos = InstancePos
    + ( cameraWorldUp.xyz * Pos.y
      + cameraWorldLeft.xyz * Pos.x
      );
  output.Pos = mul(float4( wpos, 1 ), ViewProjection);
  
  // Animate the UV's. Assuming 4x4 frames
  float nmod16 = fmod(InstanceAge*32, 16.0);
  //float nmod16 = 1;
  int   idx = int(nmod16);
  float coords_x = fmod(idx, 4);
  float coords_y = int( idx / 4);

  output.UV.x = (coords_x + UV.x) / 4.0;
  output.UV.y = (coords_y + UV.y) / 4.0;

  output.wPos = wpos;

  return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_TEXTURED_OUTPUT input
  , in float4 iPosition : SV_Position
  ) : SV_Target
{

  float my_depth = dot(input.wPos - cameraWorldPos, cameraWorldFront) / cameraZFar;
  
  int3 ss_load_coords = uint3(iPosition.xy, 0);
  float pixel_detph = txDepth.Load(ss_load_coords).x;

  float delta_z = abs(pixel_detph - my_depth);
  delta_z = saturate(delta_z * 1000);

  float4 color = txDiffuse.Sample(samClampLinear, input.UV);
  color.a *= delta_z;
  return color;
}

