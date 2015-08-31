#ifndef _SHADERS_CTES_H
#define _SHADERS_CTES_H

#include "render/ctes/shader_platform.h"

cbuffer TCtesObject SHADER_REGISTER(b0)
{
  float4 t_type;
  matrix World;
  float4 Tint;
};

cbuffer TCtesCamera SHADER_REGISTER(b1)
{
	matrix ViewProjection;
	matrix GameViewProjection;
	matrix cameraView;
	matrix PrevViewProjection;
	float4 cameraWorldPos;
	float4 cameraWorldFront;
	float4 cameraWorldUp;
	float4 cameraWorldLeft;
	float  cameraZNear, cameraZFar, cameraViewD, cameraCinematicBands;
	float  cameraHalfXRes, cameraHalfYRes, cameraXRes, cameraYRes;
};

cbuffer TCtesLight SHADER_REGISTER(b4)
{
	matrix LightViewProjection;
	matrix LightViewProjectionOffset;
	float4 LightWorldPos;
};

cbuffer TCtesPointLight SHADER_REGISTER(b5)
{
	float4 plight_world_pos;
	float4 plight_color;
	float  plight_max_radius, plight_inv_delta_radius;
	float  plight_intensity, plight_dummy2;
};

cbuffer TCtesDirLight SHADER_REGISTER(b5)   // using the same register as the point light
{
	float4 dir_light_direction;
	float4 dir_light_world_pos;
	float4 dir_light_color;
	float dir_light_angle, dir_light_dummy, dir_light_dummy1, dir_light_dummy2;
	//float  dir_light_max_radius, dir_light_inv_delta_radius;
	//float  dir_light_dummy1, dir_light_dummy2;
};

cbuffer TCtesSpotLight SHADER_REGISTER(b5)   // using the same register as the point light
{
	float4 spot_light_world_pos;
	float4 spot_light_color;
	float4 spot_light_direction;
	float  spot_light_max_radius;
	float  spot_light_dummy1, spot_light_dummy2, spot_light_dummy3;
};

cbuffer TCtesParticleSystem SHADER_REGISTER(b5)   // using the same register as the lights
{
	int n_imgs_x;
	int n_imgs_y;
	int stretch;
	int render_mode;
	int animation_mode;
	int stretch_mode;
	float distorsion_amount;
	float dummy_ps1;
};

cbuffer TCtesGUI SHADER_REGISTER(b5)   // using the same register as the lights and particles
{
	float gui_height;
	float gui_width;
	float gui_offset;
	float gui_dummy;
};

cbuffer TCtesGlobal SHADER_REGISTER(b2)
{
  float4 added_ambient_color;
  float4 static_needles[4];
  float  world_time;
  float use_lightmaps;
  float elapsed, dummy_global1;
  
  // Aligned to 16 bytes  
};

cbuffer TCtesBones SHADER_REGISTER(b3)
{
	matrix bones[256];
};

cbuffer TCtesBlur SHADER_REGISTER(b3)
{
	float4 blur_delta;
	float blur_amount;
	float dummy_blur, dummy_blur2, dummy_blur3;
};

cbuffer TCtesBlurCamera SHADER_REGISTER(b3)
{
	float4 blur_camera_delta;
	float blur_camera_amount;
	float dummy_blur_camera, dummy_blur_camera2, dummy_blur_camera3;
};

cbuffer TCtesSharpen SHADER_REGISTER(b3)
{
	float4 sharpen_delta;
	float amount;
	float dummy_sharpen, dummy_sharpen2, dummy_sharpen3;
};

cbuffer TCtesSSAO SHADER_REGISTER(b3)
{
	float4 ssao_delta;
	float radius;
	float dummy_ssao, dummy_ssao2, dummy_ssao3;
};

cbuffer TCtesChromaticAberration SHADER_REGISTER(b3)
{
	float4 ca_delta;
	float chromatic_amount;
	float dummy_ca, dummy_ca2, dummy_ca3;
};

cbuffer TCtesGlow SHADER_REGISTER(b3)
{
	float4 glow_pos;
	float4 glow_delta;
	float glow_amount;
	float dummy_glow, dummy_glow2, dummy_glow3;
};

cbuffer TCtesUnderwater SHADER_REGISTER(b3)
{
	float4 uw_pos;
	float4 uw_delta;
	float uw_amount;
	float water_level;
	float dummy_uw, dummy_uw2;
};

cbuffer TCtesSSRR SHADER_REGISTER(b3)
{
	float ssrr_amount;
	float ssrr_quality;
	float dummy_ssrr, dummy_ssrr2;
};

#endif