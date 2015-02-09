#include "render/ctes/shader_platform.h"

cbuffer TCtesObject SHADER_REGISTER(b0)
{
  matrix World;
};

cbuffer TCtesCamera SHADER_REGISTER(b1)
{
  matrix ViewProjection;
};

cbuffer TCtesGlobal SHADER_REGISTER(b2)
{
  // Aligned to 16 bytes
  float  world_time;
  float  dummy[3];
};
