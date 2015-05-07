#include "render/ctes/shader_ctes.h"

Texture2D txDiffuse   : register(t0);
Texture2D txNormal    : register(t1);
Texture2D txDepth     : register(t2);
Texture2D txAccLight  : register(t3);
Texture2D txShadowMap : register(t6);

TextureCube  txEnvironment : register(t4);

SamplerState samWrapLinear : register(s0);
SamplerState samClampLinear : register(s1);
SamplerState samBorderLinear : register(s2);
SamplerComparisonState samPCFShadows : register(s3);

//--------------------------------------------------------------------------------------
struct VS_TEXTURED_OUTPUT
{
  float4 Pos     : SV_POSITION;
  float2 UV      : TEXCOORD0;
  float4 wPos    : TEXCOORD1;
  float3 wNormal : NORMAL;
  float4 wTangent : TANGENT;
};

//--------------------------------------------------------------------------------------
void VSCommon(
        float4 iPos : POSITION
  , out float4 oPos : SV_POSITION
  ) {
  float4 world_pos = mul(iPos, World);
  oPos = mul(world_pos, ViewProjection);
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT VS(
    float4 Pos : POSITION
  , float2 UV : TEXCOORD0
  , float3 Normal : NORMAL
  , float4 Tangent : TANGENT
  )
{
  VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;
  float4 world_pos = mul(Pos, World);
  output.Pos = mul(world_pos, ViewProjection);
  output.wNormal = mul(Normal, (float3x3)World);
  output.UV = UV * 1;
  output.wPos = world_pos;
  // Rotate the tangent and keep the w value
  output.wTangent.xyz = mul(Tangent.xyz, (float3x3)World);
  output.wTangent.w = Tangent.w;
  return output;
}

void VSGenShadowsSkel(
	float4 ipos     : POSITION
	, float2 iuv : TEXCOORD0
	, float3 inormal : NORMAL
	, float4 Tangent : TANGENT
	, uint4  bone_ids : BONEIDS
	, float4 weights : WEIGHTS
	, out float4 oPos : SV_POSITION
	)
{
	matrix skin_mtx = bones[bone_ids.x] * weights[0]
		+ bones[bone_ids.y] * weights[1]
		+ bones[bone_ids.z] * weights[2]
		+ bones[bone_ids.w] * weights[3]
		;
	float4 skinned_pos = mul(ipos, skin_mtx);
		oPos = mul(skinned_pos, ViewProjection);
}

// Skels Vertex Shader
VS_TEXTURED_OUTPUT VSSkels(
	float4 ipos     : POSITION
	, float2 iuv : TEXCOORD0
	, float3 inormal : NORMAL
	, float4 Tangent : TANGENT
	, uint4  bone_ids : BONEIDS
	, float4 weights : WEIGHTS	
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
	output.wPos = skinned_pos;
	output.wNormal = mul(inormal, (float3x3) skin_mtx);
	output.UV = float2(iuv.x, 1 - iuv.y);
	//output.UV = bone_ids.xy / 50.;

	// Rotate the tangent and keep the w value
	output.wTangent.xyz = mul(Tangent.xyz, (float3x3)World);
	output.wTangent.w = Tangent.w;

	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSBasicLighting(VS_TEXTURED_OUTPUT input) : SV_Target
{

  // Basic diffuse lighting
  float3 L = LightWorldPos.xyz - input.wPos.xyz;
  L = normalize(L);
  float3 N = normalize(input.wNormal);
  float  diffuse_amount = saturate( dot(N, L) );

  // Speculares
  float3 E = normalize(cameraWorldPos.xyz - input.wPos.xyz);
  //float3 H = normalize(E + L);
  //float  cos_beta = saturate( dot(H, N) );
  float3 ER = reflect(-E, N);
  float  cos_beta = saturate(dot(ER, L));
  float  spec_amount = pow(cos_beta, 20.);

  float4 albedo = txDiffuse.Sample(samWrapLinear, input.UV);
  return albedo * diffuse_amount + spec_amount;
}


//--------------------------------------------------------------------------------------
float tapAt(float2 homo_coords, float depth) {
  float amount = txShadowMap.SampleCmpLevelZero(samPCFShadows
    , homo_coords, depth);
  return amount;
}

float rand_1_05(in float2 uv)
{
	float2 noise = (frac(sin(dot(uv, float2(12.9898, 78.233)*2.0)) * 43758.5453));
	return abs(noise.x + noise.y) * 0.5;
}

float2 rand_2_10(in float2 uv) {
	float noiseX = (frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
	float noiseY = sqrt(1 - noiseX * noiseX);
	return float2(noiseX, noiseY);
}

// -------------------------
float getShadowAt(float4 wPos) {

  // Move to homogeneous space of the light
  float4 light_proj_coords = mul(wPos, LightViewProjectionOffset);
  light_proj_coords.xyz /= light_proj_coords.w;
  if (light_proj_coords.z <= 1e-3)
    return 0;

  float2 center = light_proj_coords.xy;
  float depth = light_proj_coords.z - 0.002;

  float amount = tapAt(center, depth) * 0.2;

  float sz = 2. / 1024;
  float2 rand = rand_2_10(wPos.xy);
  float weigth = 0.1;
  // 8 Random tabs
  for (int i = 0; i < 8; i++) {
	  rand = rand_2_10(wPos.xz + float2(i, 0)) * sz;
	  amount += tapAt(center + rand, depth) * weigth;
  }

  /*amount += tapAt(center + float2(sz, sz), depth);
  amount += tapAt(center + float2(-sz, sz), depth);
  amount += tapAt(center + float2(sz, -sz), depth);
  amount += tapAt(center + float2(-sz, -sz), depth);

  amount += tapAt(center + float2(sz, 0), depth);
  amount += tapAt(center + float2(-sz, 0), depth);
  amount += tapAt(center + float2(0, -sz), depth);
  amount += tapAt(center + float2(0, sz), depth);*/

  //amount *= 1.f / 9.;
  //amount *= 1.f / 5.;

  return amount;
}


//--------------------------------------------------------------------------------------
// GBuffer creation
//--------------------------------------------------------------------------------------
void PSGBuffer(
    in  VS_TEXTURED_OUTPUT input
  , out float4 albedo : SV_Target0
  , out float4 normal : SV_Target1
  , out float4 acc_light : SV_Target2
  , out float  depth : SV_Target3
)
{

  albedo = txDiffuse.Sample(samWrapLinear, input.UV);

  float3   in_normal = normalize(input.wNormal);
  float3   in_tangent = normalize(input.wTangent.xyz);
  float3   in_binormal = cross(in_normal, in_tangent) * input.wTangent.w;
  float3x3 TBN = float3x3( in_tangent
                         , in_binormal
                         , in_normal);

  // Convert the range 0...1 from the texture to range -1 ..1 
  float3 normal_tangent_space = txNormal.Sample(samWrapLinear, input.UV).xyz * 2 - 1.;
	  float3 wnormal_per_pixel = mul(normal_tangent_space, TBN);

	  //wnormal_per_pixel = in_tangent;

  // Save the normal

  bool test = length(in_tangent) < 2;
  float3 m_norm = test ? wnormal_per_pixel : in_normal;
  normal = (float4(m_norm, 1) + 1.) * 0.5;

  
  // Basic diffuse lighting
  float3 L = LightWorldPos.xyz - input.wPos.xyz;
  L = normalize(L);
  float3 N = normalize(wnormal_per_pixel);
  float  diffuse_amount = saturate(dot(N, L));
  
  float  diffuse_amount2 = saturate(dot(in_normal, L));

  depth = dot(input.wPos - cameraWorldPos, cameraWorldFront) / cameraZFar;
  //depth = dot(input.wPos.xyz - cameraWorldPos.xyz, cameraWorldFront.xyz) / cameraZFar;

  // 
  acc_light = getShadowAt(input.wPos);
  //acc_light *= diffuse_amount2;
  //albedo *= (0.2 + acc_light * 0.8);
}

float3 getWorldCoords( float2 screen_coords, float depth ) {
  float3 view_dir = float3(cameraHalfXRes - screen_coords.x
                          , cameraHalfYRes - screen_coords.y
                          , cameraViewD);
  view_dir = view_dir / cameraViewD * cameraZFar * depth;

  float3 world_dir = view_dir.z * cameraWorldFront.xyz
                   + view_dir.y * cameraWorldUp.xyz
                   + view_dir.x * cameraWorldLeft.xyz;
  float3 wPos = cameraWorldPos.xyz + world_dir;
  return wPos;
}

// -------------------------------------------------
float getSpecular(float3 wPos, float3 L, float3 N) {
  float3 E = normalize(cameraWorldPos.xyz - wPos);
  float3 H = normalize(L + E);
  float  spec_amount = pow(saturate(dot(H, N)), 20);    // 20 should come from a texture
  return spec_amount;
}

// -------------------------------------------------
// Point lights
// -------------------------------------------------
float4 PSPointLights(
  in float4 iPosition : SV_Position
  ) :SV_Target0{

  int3 ss_load_coords = uint3(iPosition.xy, 0);
  float depth = txDepth.Load(ss_load_coords).x;
  float3 N = txNormal.Load(ss_load_coords).xyz * 2 - 1.;

  float3 wPos = getWorldCoords(iPosition.xy, depth);

  // Basic diffuse lighting
  float3 L = plight_world_pos.xyz - wPos;
  float  distance_to_light = length(L);
  L = L / distance_to_light;
  float  diffuse_amount = saturate(dot(N, L));

  float spec_amount = getSpecular(wPos, L, N);

  // Attenuation based on distance:   1 - [( r - rmin ) / ( rmax - rmin )]
  float  att_factor = saturate((plight_max_radius - distance_to_light) * plight_inv_delta_radius);

  // Save spec amount in the alpha channel
  float4 output = plight_color * diffuse_amount;
  output.a = spec_amount;
  return output * att_factor;
}


// -------------------------------------------------
// Dir lights
// -------------------------------------------------
float4 PSDirLights(
  in float4 iPosition : SV_Position
  ) :SV_Target0{

  int3 ss_load_coords = uint3(iPosition.xy, 0);
  float depth = txDepth.Load(ss_load_coords).x;
  float3 N = txNormal.Load(ss_load_coords).xyz * 2 - 1.;

	  float3 wPos = getWorldCoords(iPosition.xy, depth);

	 // return float4(wPos.x - int(wPos.x), 0, 0, 1);

  // Basic diffuse lighting
  float3 L = dir_light_world_pos.xyz - wPos;
  float  distance_to_light = length(L);
  L = L / distance_to_light;
  float  diffuse_amount = saturate(dot(N, L));



  // Currently, no attenuation based on distance
  // Attenuation based on shadowmap
  float att_factor = getShadowAt(float4(wPos, 1));

  float spec_amount = getSpecular(wPos, L, N);
  return float4(dir_light_color.xyz * diffuse_amount, spec_amount) * att_factor;
}


// -------------------------------------------------
// Resolve lights
// -------------------------------------------------
float4 PSResolve(
  in float4 iPosition : SV_Position
  ) :SV_Target0{

	int3 ss_load_coords = uint3(iPosition.xy, 0);
	float4 albedo = txDiffuse.Load(ss_load_coords);
	float3 N = txNormal.Load(ss_load_coords).xyz * 2 - 1.;
	float4 diffuse = txAccLight.Load(ss_load_coords);
	float4 env = txEnvironment.Sample(samWrapLinear, N);

	return (albedo * diffuse + diffuse.a) * 0.9 + env* 0.1;
}


// -------------------------------------------------
// Distorsion
// -------------------------------------------------
float4 PSDistorsion(
VS_TEXTURED_OUTPUT vin
, in float4 iPosition : SV_Position

) : SV_Target0{

	float3 wpos = vin.wPos.xyz;
	float4 noise = txNormal.Sample(samWrapLinear, vin.UV * 10 + world_time.xx*0.2) * 2 - 1;
	float4 noise2 = txNormal.Sample(samWrapLinear, float2(1, 1) - vin.UV * 2.32) * 2 - 1;
	noise2 *= 1;

	noise *= 0.9;
	noise2 *= 0.9;

	wpos.x += noise.x * cos(world_time);
	wpos.z += noise.x * sin(world_time + .123f);
	wpos.x += noise2.x * cos(world_time*0.23);
	wpos.z += noise2.x * sin(world_time*1.7 + .123f);

	// ++add noise
	float4 hpos = mul(float4(wpos, 1), ViewProjection);
	hpos.xyz /= hpos.w;   // -1 .. 1
	hpos.x = (hpos.x + 1) * 0.5;
	hpos.y = (1 - hpos.y) * 0.5;
	float4 albedo = txDiffuse.Sample(samClampLinear, hpos.xy);

		// A bit of fresnel
		float3 dir_to_eye = normalize(cameraWorldPos.xyz - vin.wPos.xyz);
		float3 N = normalize(vin.wNormal.xyz);
		float fresnel = 1 - dot(N, dir_to_eye);
	fresnel = pow(fresnel, 4);

	float3 N_reflected = reflect(-dir_to_eye, N);
		float4 env = txEnvironment.Sample(samWrapLinear, N_reflected);
		float4 new_color = float4(albedo.x*0.8, albedo.y*1.0, albedo.z*0.9, 1);

		return env*fresnel + new_color*(1 - fresnel);
}