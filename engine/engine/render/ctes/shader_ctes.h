#ifndef _SHADERS_CTES_H
#define _SHADERS_CTES_H

#include "render/ctes/shader_platform.h"

cbuffer TCtesObject SHADER_REGISTER(b0)
{
  matrix World;
  float4 Tint;
};

cbuffer TCtesCamera SHADER_REGISTER(b1)
{
	matrix ViewProjection;
	matrix cameraView;
	float4 cameraWorldPos;
	float4 cameraWorldFront;
	float4 cameraWorldUp;
	float4 cameraWorldLeft;
	float  cameraZNear, cameraZFar, cameraViewD, cameraDummy1;
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
	float  plight_dummy1, plight_dummy2;
};

cbuffer TCtesDirLight SHADER_REGISTER(b5)   // using the same register as the point light
{
	float4 dir_light_world_pos;
	float4 dir_light_color;
	//float  dir_light_max_radius, dir_light_inv_delta_radius;
	//float  dir_light_dummy1, dir_light_dummy2;
};


cbuffer TCtesGlobal SHADER_REGISTER(b2)
{
  float  world_time;
  float  dummy3[3];
  
  // Aligned to 16 bytes  
};

cbuffer TCtesBones SHADER_REGISTER(b3)
{
	matrix bones[256];
};

cbuffer TCtesBlur SHADER_REGISTER(b3)
{
	float4 blur_delta;
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

#endif