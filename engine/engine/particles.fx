#include "render/ctes/shader_ctes.h"

Texture2D txDiffuse : register(t0);
Texture2D txDepth   : register(t2);
Texture2D txParticle   : register(t7);
Texture2D txNoise    : register(t9);

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
, float rotation : TEXCOORD3
/*, float  InstanceAge : TEXCOORD1    // Stream 1
, float  InstanceLifespan : COLOR0    // Stream 1
, float  InstanceSize : POSITION2    // Stream 1*/
)
{
  VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;

  float real_stretch = 1;
  
  float s, c;
  sincos(rotation, s, c);
  
  /*float3 wpos = InstancePos
	  + (axis_1.xyz * Pos.y * InstanceAgeLifeSpanSize.z * real_stretch
	  + axis_2.xyz * Pos.x * InstanceAgeLifeSpanSize.z
	  );*/
    
  float3 particles_left = cameraWorldLeft.xyz;
  float3 particles_up = cameraWorldUp.xyz;

  // V-BILLBOARD
  if (render_mode == 1) {
	  particles_up = float3(0, 1, 0);
  }
  // H-BILLBOARD
  if (render_mode == 2) {
	  particles_left = float3(0, 0, 1);
	  particles_up = -float3(1, 0, 0);
  }
  // H-DIR-BILLBOARD
  if (render_mode == 3) {
	  particles_up = normalize(InstanceSpeed);
	  particles_left = normalize(cross(particles_up, float3(0, -1, 0)));
  }
  // STRETCHED-BILLBOARD
  if (render_mode == 4) {	  
	  particles_up = InstanceSpeed;

	  // STRETCH WITH SPEED
	  if (stretch_mode == 1) {
		  real_stretch = stretch * length(particles_up);
	  }

	  // STRETCH NORMAL
	  if (stretch_mode == 0) {
		  particles_up = normalize(particles_up);
		  real_stretch = stretch;
	  }
	  particles_left = normalize(cross(particles_up, cameraWorldFront));
	  
  }

  // BILLBOARDS DIRECCIONALES
  /*float3 particles_left = normalize(InstanceSpeed);
  float3 aux = float3(0, 1, 0); // cameraWorldLeft;
  float3 left = normalize(cross(aux, particles_left));
  float3 particles_up = -cross(particles_left, left); // normalize(cross(particles_up, aux));

  particles_up = cross(cameraWorldFront, particles_left); // float3(1, 0, 0);
  //particles_left = float3(0,1,0);
  */

  Pos.y = Pos.y * real_stretch;

  float3 wpos = InstancePos
	  + ((particles_up.xyz * (Pos.y * c + Pos.x * s))
	  + (particles_left.xyz * (Pos.x * c - Pos.y * s))
	  ) * InstanceAgeLifeSpanSize.z
	  ; 

  output.Pos = mul(float4(wpos, 1), ViewProjection);
  
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

  float4 color = txParticle.Sample(samClampLinear, input.UV);
  color.a *= delta_z;

  // 0.2 = % of life during the begining and the end of the particle with opacity fade in/out
  float opacity_change = input.ageLife.y * 0.2;
  float begining_opacity_modifier = saturate(input.ageLife.x / opacity_change);
  float end_opacity_modifier = 1 - saturate((input.ageLife.x - opacity_change) / (input.ageLife.y - opacity_change));

  color.a *= begining_opacity_modifier; // Begining of life opacity change
  color.a *= end_opacity_modifier; // End of life opacity change

  color.xyz *= input.color;

  // Noise test
  float3 wpos = input.wPos.xyz;
  float4 noise = txNoise.Sample(samWrapLinear, input.UV * 10 + world_time.xx*0.9) * 2 - 1;
  float4 noise2 = txNoise.Sample(samWrapLinear, float2(1, 1) - input.UV * 2.32) * 2 - 1;

  noise *= 0.06;
  noise2 *= 0.06;

  //wpos.x += noise.x * cos(world_time * 0.1) * 30;
  //wpos.z += noise.y * sin(world_time  * 0.12) * 30;
  //wpos.y += noise2.x * cos(world_time*0.23) * 30;
  //wpos.z += noise2.y * sin(world_time*1.7 + .123f);
  wpos += float3(noise.x, noise2.y * 3, noise.y);

  // ++add noise
  float4 hpos = mul(float4(wpos, 1), ViewProjection);
	  hpos.xyz /= hpos.w;   // -1 .. 1
  hpos.x = (hpos.x + 1) * 0.5;
  hpos.y = (1 - hpos.y) * 0.5;
  float4 albedo = txDiffuse.Sample(samClampLinear, hpos.xy);

  color.xyz = albedo.xyz;
  
  
  //color.a = 0.5;
  //color.xyz = txDiffuse.Load(ss_load_coords).xyz;
  
  return color;
}

