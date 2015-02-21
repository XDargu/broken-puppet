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

#define MaxDirLights 10
#define MaxOmniLights 10

cbuffer TCtesGlobal SHADER_REGISTER(b2)
{
  float4 AmbientLight;

  float4 LightDirections[MaxDirLights];
  float4 LightColors[MaxDirLights];

  float4 OmniLightPositions[MaxOmniLights];
  float4 OmniLightColors[MaxOmniLights];
  float4 OmniLightRadius[MaxOmniLights];

  int OmniLightCount;  
  int LightCount;
  float  dummy3[3];
  
  // Aligned to 16 bytes
  float  world_time;
  float  dummy[3];
};
