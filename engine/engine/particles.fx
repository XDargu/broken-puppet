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
  float3 wPos   : TEXCOORD1;
  float2 ageLife: TEXCOORD2;
  float3 color	: COLOR0;
};

float rand_1_05(in float2 uv)
{
	float2 noise = (frac(sin(dot(uv, float2(12.9898, 78.233)*2.0)) * 43758.5453));
		return abs(noise.x + noise.y) * 0.5;
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT VS(
  float4 Pos : POSITION0              // Stream 0
, float2 UV : TEXCOORD0
, float3 InstancePos : POSITION1      // Stream 1
, float3 InstanceDir : POSITION2      // Stream 1
, float3 InstanceSpeed : POSITION3      // Stream 1
, float3 InstanceAgeLifeSpanSize : TEXCOORD1
, float3 Color : COLOR0
, int index : TEXCOORD2
/*, float  InstanceAge : TEXCOORD1    // Stream 1
, float  InstanceLifespan : COLOR0    // Stream 1
, float  InstanceSize : POSITION2    // Stream 1*/
)
{
  VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;

  float real_stretch = 1;
  if (render_mode == 3)
	  real_stretch = stretch;

  float3 wpos = InstancePos
	  + (cameraWorldUp.xyz * Pos.y * InstanceAgeLifeSpanSize.z * real_stretch
	  + cameraWorldLeft.xyz * Pos.x * InstanceAgeLifeSpanSize.z
      );
  output.Pos = mul(float4(wpos, 1), ViewProjection);
  //output.Pos = mul(output.Pos, ViewProjection);
  
  int idx = 0;

  if (animation_mode == 0) {
	  // Animate the UV's. Assuming 4x4 frames
	  float nmod16 = fmod(InstanceAgeLifeSpanSize.x * 16, n_imgs_x * n_imgs_y);
	  //float nmod16 = 1;
	  idx = int(nmod16);
  }

  if (animation_mode == 1) {
	  idx = fmod(index, n_imgs_x * n_imgs_y);
  }

  if (animation_mode == 2) {
	  int rnd_row = fmod(index, n_imgs_y);
	  int rnd_col = fmod(InstanceAgeLifeSpanSize.x * 16, n_imgs_x);
	  idx = rnd_row * n_imgs_x + rnd_col;
  }

  float coords_x = (int)fmod(idx, n_imgs_x);
  float coords_y = uint(idx / n_imgs_x);

  output.UV.x = (coords_x + UV.x) / n_imgs_x;
  output.UV.y = 1 - (coords_y + UV.y) / n_imgs_y;

  output.wPos = wpos;
  output.color = Color;

  if (InstanceAgeLifeSpanSize.z == -1)
	  output.Pos.w = 0.0;
  
  output.ageLife = InstanceAgeLifeSpanSize.xy;

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

  // 0.2 = % of life during the begining and the end of the particle with opacity fade in/out
  float opacity_change = input.ageLife.y * 0.2;
  float begining_opacity_modifier = saturate(input.ageLife.x / opacity_change);
  float end_opacity_modifier = 1 - saturate((input.ageLife.x - opacity_change) / (input.ageLife.y - opacity_change));

  color.a *= begining_opacity_modifier; // Begining of life opacity change
  color.a *= end_opacity_modifier; // End of life opacity change

  color.xyz *= input.color;
  
  return color;
}

