#include "render/ctes/shader_platform.h"

cbuffer TCtesObject SHADER_REGISTER(b0)
{
  matrix World;
  float4 Tint;
};

cbuffer TCtesCamera SHADER_REGISTER(b1)
{
  matrix ViewProjection;
};

#define MaxLights 10

cbuffer TCtesGlobal SHADER_REGISTER(b2)
{
  float4 lightDirection;
  float3 LightDirections[MaxLights];
  float4 LightColors[MaxLights];
  int LightCount;
  // Aligned to 16 bytes
  float  world_time;
  float  dummy[3];
};
