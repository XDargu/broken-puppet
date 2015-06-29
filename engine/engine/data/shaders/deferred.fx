#include "render/ctes/shader_ctes.h"

Texture2D txDiffuse   : register(t0);
Texture2D txNormal    : register(t1);
Texture2D txDepth     : register(t2);
Texture2D txAccLight  : register(t3);
Texture2D txSpecular    : register(t4);
Texture2D txGloss    : register(t5);

Texture2D txEmissive    : register(t7);

Texture2D txShadowMap : register(t6);

TextureCube  txEnvironment : register(t8);
Texture2D  txLightmap : register(t9);

SamplerState samWrapLinear : register(s0);
SamplerState samClampLinear : register(s1);
SamplerState samBorderLinear : register(s2);
SamplerComparisonState samPCFShadows : register(s3);
SamplerState samCube : TEXUNIT3{
	Texture = txCubemap;
	MIPFILTER = LINEAR;
	MINFILTER = LINEAR;
	MAGFILTER = LINEAR;
};

//--------------------------------------------------------------------------------------
struct VS_TEXTURED_OUTPUT
{
  float4 Pos     : SV_POSITION;
  float2 UV      : TEXCOORD0;
  float4 wPos    : TEXCOORD1;
  float3 wNormal : NORMAL;
  float4 wTangent : TANGENT;
  float2 UVL      : TEXCOORD2;
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
  , float2 UVL : TEXCOORD1
  , float4 Tangent : TANGENT  
  )
{
  VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;
  float4 world_pos = mul(Pos, World);
  output.Pos = mul(world_pos, ViewProjection);
  output.wNormal = mul(Normal, (float3x3)World);
  output.UV = UV * 1;
  output.UVL = UVL;
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
	output.UVL = output.UV;
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
  , out float2  depth : SV_Target3
  , out float4 specular : SV_Target4
  , out float4 gloss : SV_Target5
)
{

  float4 lightmap = txLightmap.Sample(samWrapLinear, input.UVL);
  albedo = txDiffuse.Sample(samWrapLinear, input.UV);
  specular = txSpecular.Sample(samWrapLinear, input.UV);
  gloss = txGloss.Sample(samWrapLinear, input.UV);
  acc_light = float4(0, 0, 0, 0);
  
  float3   in_normal = normalize(input.wNormal);
  float3   in_tangent = normalize(input.wTangent.xyz);
  float3   in_binormal = cross(in_normal, in_tangent) * input.wTangent.w;
  float3x3 TBN = float3x3( in_tangent
                         , -in_binormal
                         , in_normal);

  // Convert the range 0...1 from the texture to range -1 ..1 
  float3 normal_tangent_space = normalize(txNormal.Sample(samWrapLinear, input.UV).xyz * 2 - 1.);
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

  depth.x = dot(input.wPos - cameraWorldPos, cameraWorldFront) / cameraZFar;
  depth.y = t_type.x;
  if (t_type.x == 0.95 || t_type.x == 0.9) {
	  //albedo = float4(1, 0, 0, 1);	  
	  albedo.g += 0.5;
  }

  if (depth.y == 0) {
	  for (int i = 0; i < 4; i++) {
		  if (static_needles[i].w != -1) {
			  //float dist = rand_1_05(input.UV) * 0.0;
			  float dist = 0.5;
			  float rad = distance(input.wPos.xyz, static_needles[i].xyz);
			  if (rad < dist) {
				  albedo += float4(.5, 0, 0, 0) * (1 - (rad / dist));
			  }
		  }		  
	  }
  }
  //depth = dot(input.wPos.xyz - cameraWorldPos.xyz, cameraWorldFront.xyz) / cameraZFar;

  // 
  //acc_light = getShadowAt(input.wPos);

  
	  
  //if (true)
  if (input.UV.x == input.UVL.x && input.UV.y == input.UVL.y)
	  acc_light += float4(0.98, 0.85, 0.8, 0) * 0.15;
  else
	  acc_light += float4(lightmap.xyz * 0.5, 0);
  
  acc_light *= added_ambient_color;
  
  float4 emis = txEmissive.Sample(samWrapLinear, input.UV);
  acc_light += float4(emis.xyz, 0);

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

float2 coord2D(float2 coord)
{
	return float2((coord.x - cameraHalfXRes) / cameraHalfXRes, -(coord.y - cameraHalfYRes) / cameraHalfYRes);
}

float2 perspective_correction(float2 coord)
{
	float camera_aspect_ratio = cameraHalfXRes / cameraHalfYRes;
	coord.x *= camera_aspect_ratio;
	// tan( fov/2 ) = ( yres/2 ) / view_d
	float tan_half_fov = cameraHalfYRes / cameraViewD;

	return coord * tan_half_fov;
}

float3 getViewSpace(float2 screen_coords, float depth) {
	//return getWorldCoords(screen_coords, depth);
	//return float3(perspective_correction(screen_coords), -1) * depth * cameraZFar;
	return float3(perspective_correction(coord2D(screen_coords)), -1) * depth * cameraZFar;
}

float2 LightingFuncGGX_FV(float dotLH, float roughness)
{
	float alpha = roughness*roughness;

	// F
	float F_a, F_b;
	float dotLH5 = pow(1.0f - dotLH, 5);
	F_a = 1.0f;
	F_b = dotLH5;

	// V
	float vis;
	float k = alpha / 2.0f;
	float k2 = k*k;
	float invK2 = 1.0f - k2;
	vis = rcp(dotLH*dotLH*invK2 + k2);

	return float2(F_a*vis, F_b*vis);
}

float LightingFuncGGX_D(float dotNH, float roughness)
{
	float alpha = roughness*roughness;
	float alphaSqr = alpha*alpha;
	float pi = 3.14159f;
	float denom = dotNH * dotNH *(alphaSqr - 1.0) + 1.0f;

	float D = alphaSqr / (pi * denom * denom);
	return D;
}

float G1V(float dotNV, float k)
{
	return 1.0f / (dotNV*(1.0f - k) + k);
}

float LightingFuncGGX_REF(float3 N, float3 V, float3 L, float roughness, float F0)
{
	float alpha = roughness*roughness;

	float3 H = normalize(V + L);

	float dotNL = saturate(dot(N, L));
	float dotNV = saturate(dot(N, V));
	float dotNH = saturate(dot(N, H));
	float dotLH = saturate(dot(L, H));

	float F, D, vis;

	// D
	float alphaSqr = alpha*alpha;
	float pi = 3.14159f;
	float denom = dotNH * dotNH *(alphaSqr - 1.0) + 1.0f;
	D = alphaSqr / (pi * denom * denom);

	// F
	float dotLH5 = pow(1.0f - dotLH, 5);
	F = F0 + (1.0 - F0)*(dotLH5);

	// V
	float k = alpha / 2.0f;
	vis = G1V(dotNL, k)*G1V(dotNV, k);

	float specular = dotNL * D * F * vis;
	return specular;
}

float LightingFuncGGX_OPT3(float3 N, float3 V, float3 L, float roughness, float F0)
{
	float3 H = normalize(V + L);

	float dotNL = saturate(dot(N, L));
	float dotLH = saturate(dot(L, H));
	float dotNH = saturate(dot(N, H));

	float D = LightingFuncGGX_D(dotNH, roughness);
	float2 FV_helper = LightingFuncGGX_FV(dotLH, roughness);
	float FV = F0*FV_helper.x + (1.0f - F0)*FV_helper.y;
	float specular = dotNL * D * FV;

	return specular;
}

// -------------------------------------------------
float getSpecular(float3 wPos, float3 L, float3 N, float3 ss_load_coords) {	
	float3 V = normalize(cameraWorldPos.xyz - wPos);
	float3 R = reflect(normalize(-L), normalize(N));
	float fresnel = dot(N, V);

	float4 gloss = txGloss.Load(ss_load_coords);
	return LightingFuncGGX_OPT3(N, V, L, 1 - gloss, fresnel);

	//float spec_amount = pow(saturate(dot(R, V)), 50);
	float spec_amount = saturate(dot(R, V));
	return spec_amount;
  /*float3 E = normalize(cameraWorldPos.xyz - wPos);
  float3 H = normalize(L + E);
  spec_amount = pow(saturate(dot(H, N)), 50);    // 20 should come from a texture
  return spec_amount;*/
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

  float spec_amount = getSpecular(wPos, L, N, ss_load_coords);

  // Attenuation based on distance:   1 - [( r - rmin ) / ( rmax - rmin )]
  float  att_factor = saturate((plight_max_radius -  distance_to_light) * plight_inv_delta_radius * 0.4);

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

  float angle_cos = dot(L, (float3)-dir_light_direction);
  float max_cos = cos(dir_light_angle * 0.2);

  // Currently, no attenuation based on distance
  // Attenuation based on shadowmap
  float att_factor = 0.0;
  att_factor = saturate(1 - ((max_cos - angle_cos) * 150));
  // Only check for shadows if inside the circle
  if (att_factor > 0) {
	  att_factor *= getShadowAt(float4(wPos, 1));
  }

 // return angle_cos.xxxx;

  float spec_amount = getSpecular(wPos, L, N, ss_load_coords);
  float4 output = float4(dir_light_color.xyz * diffuse_amount, spec_amount) * att_factor;
  return output;
}

// -------------------------------------------------
// Dir lights
// -------------------------------------------------
float4 PSSpotLights(
in float4 iPosition : SV_Position
) :SV_Target0{

	int3 ss_load_coords = uint3(iPosition.xy, 0);
	float depth = txDepth.Load(ss_load_coords).x;
	float3 N = txNormal.Load(ss_load_coords).xyz * 2 - 1.;

	float3 wPos = getWorldCoords(iPosition.xy, depth);

	// return float4(wPos.x - int(wPos.x), 0, 0, 1);

	// Basic diffuse lighting
	float3 L = spot_light_world_pos.xyz - wPos;
	float  distance_to_light = length(L);
	L = L / distance_to_light;
	float  diffuse_amount = saturate(dot(N, L));

	float3 dir = normalize(L);

	float angle_cos = dot(dir, (float3)spot_light_direction);
	float max_cos = cos(0.1);
	
	float att_factor = 0;
	if (angle_cos < max_cos * 0.5) {
		att_factor = getShadowAt(float4(wPos, 1));
	}

	float spec_amount = getSpecular(wPos, L, N, ss_load_coords);
	return float4(spot_light_color.xyz * diffuse_amount, spec_amount) * att_factor;
}

// SSRR
float4 rainbow(float x) {
	float level = x * 2.0;
	float r, g, b;
	if (level <= 0) {
		r = g = b = 0;
	}
	else if (level <= 1) {
		r = lerp(1, 0, level);
		g = lerp(0, 1, level);
		b = 0;
	}
	else if (level > 1) {
		r = 0;
		g = lerp(1, 0, level - 1);
		b = lerp(0, 1, level - 1);
	}
	return float4(r, g, b, 1);
}



float4 ssrrColor(float2 iPosition, matrix viewproj, float4 origColor, float3 worldStartingPos, float3 normal) {
	// SSRR test
	float4 color = float4(0, 0, 0, 0);
		
	int3 ss_load_coords = uint3(iPosition.xy, 0);
	//float4 origColor = txDiffuse.Load(ss_load_coords);
	float depth = txDepth.Load(ss_load_coords).x;
	//float3 normal = normalize(txNormal.Load(ss_load_coords).xyz * 2 - 1.);

	//float3 worldStartingPos = getWorldCoords(iPosition.xy, depth);

	float3 cameraToWorld = worldStartingPos - cameraWorldPos.xyz;
	float cameraToWorldDist = length(cameraToWorld);
	float3 cameraToWorldNorm = normalize(cameraToWorld);
	float3 refl = normalize(reflect(cameraToWorldNorm, normal)); // This is the reflection vector
	//return float4(normal, 1);
	/*float4 sc;
	sc = mul(float4(worldStartingPos, 1), viewproj);
	sc.xyz /= sc.w;
	sc.x = (sc.x + 1) * 0.5;
	sc.y = (1 - sc.y) * 0.5;

	return float4(sc.xy, 0, 0);

	return txDiffuse.Sample(samClampLinear, sc.xy);*/
	//return float4(refl, 1);
	//float2 m_coords = sc.xy;
	

	if (dot(refl, cameraToWorldNorm) < 0) {
		// Ignore reflections going backwards towards the camera, indicate with white
		color = float4(1, 1, 1, 1);
		return origColor;
	}

	float cosAngle = abs(dot(normal, cameraToWorldNorm)); // Will be a value between 0 and 1
	float fact = 1 - cosAngle;
	fact = min(1, 0.8 - fact*fact);

	float3 newPos;
	float4 newScreen;
	float i = 0;
	float3 rayTrace = worldStartingPos;
	float currentWorldDist, rayDist;
	float incr = 0.4;
	do {
		i += 0.05;
		rayTrace += refl*incr;
		incr *= 1.3;
		newScreen = mul(float4(rayTrace, 1), viewproj);
		newScreen.xyz /= newScreen.w;
		newScreen.x = (newScreen.x + 1) * 0.5;
		newScreen.y = (1 - newScreen.y) * 0.5;
		
		float2 coords = newScreen.xy;
		//float2 coords = newScreen.xy / 2.0 + 0.5;
		
		//return float4(coords * float2(cameraHalfXRes, cameraHalfYRes) * 0.5, 0, 0);
		float n_depth = txDepth.Sample(samClampLinear, coords);
		newPos = getWorldCoords(coords * float2(cameraHalfXRes * 2, cameraHalfYRes * 2), n_depth);
		//return txDiffuse.Sample(samClampLinear, coords);
		//newPos = rayTrace;
		
		currentWorldDist = length(newPos.xyz - cameraWorldPos.xyz);
		rayDist = length(rayTrace.xyz - cameraWorldPos.xyz);
		if (newScreen.x > 1 || newScreen.x < -1 || newScreen.y > 1 || newScreen.y < -1 || newScreen.z > 1 || newScreen.z < -1 || i >= 1.0 || cameraToWorldDist > currentWorldDist) {
			break; // This is a failure mode.
		}
	} while (rayDist < currentWorldDist);

	color = txDiffuse.Sample(samClampLinear, newScreen.xy);

	if (dot(refl, cameraToWorldNorm) < 0)
		fact = 1.0; // Ignore reflections going backwards towards the camera
	else if (newScreen.x > 1 || newScreen.x < -1 || newScreen.y > 1 || newScreen.y < -1)
		fact = 1.0; // Falling outside of screen
	else if (cameraToWorldDist > currentWorldDist)
		fact = 1.0;
	
	color = origColor*fact + color*(1 - fact);
	 
	/*if (cameraToWorldDist > currentWorldDist)
		color = float4(1, 1, 0, 1); // Yellow indicates we found a pixel hidden behind another object
	else if (newScreen.x > 1 || newScreen.x < -1 || newScreen.y > 1 || newScreen.y < -1)
		color = float4(0, 0, 0, 1); // Black used for outside of screen
	else if (newScreen.z > 1 && newScreen.z < -1)
		color = float4(1, 1, 1, 1); // White outside of frustum
	else
		color = rainbow(i); // Encode number of iterations as a color. Red, then green and last blue*/
	
	return color;
}

float4 ssrrColor(float2 iPosition, matrix viewproj) {
	int3 ss_load_coords = uint3(iPosition.xy, 0);
	float4 origColor = txDiffuse.Load(ss_load_coords);
	float depth = txDepth.Load(ss_load_coords).x;
	float3 normal = normalize(txNormal.Load(ss_load_coords).xyz * 2 - 1.);

	float3 worldStartingPos = getWorldCoords(iPosition.xy, depth);

	return ssrrColor(iPosition, viewproj, origColor, worldStartingPos, normal);
}

// -------------------------------------------------
// Resolve lights
// -------------------------------------------------
float4 PSResolve(
  in float4 iPosition : SV_Position
  ) :SV_Target0{

	int3 ss_load_coords = uint3(iPosition.xy, 0);
	float4 albedo = txDiffuse.Load(ss_load_coords);
	float4 specular_color = txSpecular.Load(ss_load_coords);
	float4 gloss = txGloss.Load(ss_load_coords);
	float depth = txDepth.Load(ss_load_coords).x;
	float3 N = normalize(txNormal.Load(ss_load_coords).xyz * 2 - 1.);
	float4 diffuse = txAccLight.Load(ss_load_coords);
		
	float3 wPos = getWorldCoords(iPosition.xy, depth);
	float3 I = wPos - cameraWorldPos.xyz;
	I = normalize(I);
	float3 R = reflect(I, N);

	//float3 reflectedColor = txCubemap.Sample(samCube, R);
	float4 env = txEnvironment.Sample(samCube, R);

	float ambient_val = 0;
	float4 ambient_color = float4(0.98, 0.85, 0.8, 0) * 0;

	//float4 specular = float4(diffuse.a * 0.9, diffuse.a * 0.8, diffuse.a * 0.6, 0) * 1.0;
	float spec_intensity = diffuse.a;
	
	float4 specular = specular_color * spec_intensity;// saturate(pow(dot_product, length(gloss))) * length(albedo) * length(specular_color);
		//return specular;

	//return ssrrColor(iPosition.xy, ViewProjection);
		
	//R = reflect(float3(I.x, -I.y, -I.z), normalize(N) * 0.3);
	//R = reflect(I, normalize(N) * 0.3);
	//R = reflect(I, float3(0, 0, 0));
	
		/*env = txEnvironment.Sample(samCube, R);
		if (wPos.y <= 0.01)
			return env;*/

	if (length(N) > 1.73) {
		R = reflect(I, float3(0,0,0));
		env = txEnvironment.Sample(samCube, R);
		//return env;
	}
	
	return (albedo * diffuse + saturate(specular)) * (1 - ambient_val) + albedo * ambient_color * ambient_val;
}


// -------------------------------------------------
// Distorsion
// -------------------------------------------------



float4 PSDistorsion(
VS_TEXTURED_OUTPUT vin
, in float4 iPosition : SV_Position

) : SV_Target0{	


	//vin.UV *= 0.5;
	float3 wpos = vin.wPos.xyz;
	float4 noise = txEmissive.Sample(samWrapLinear, vin.UV * 10 + world_time.xx*0.2) * 2 - 1;
	float4 noise2 = txEmissive.Sample(samWrapLinear, float2(1, 1) - vin.UV * 2.32) * 2 - 1;
	noise2 *= 1;
	//return noise;
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
	fresnel = pow(fresnel, 2);

	//float3 dir_ref_corrected = normalize(float3(cameraWorldPos.x, cameraWorldPos.y, cameraWorldPos.z) - vin.wPos.xyz);
	/*float3 cam_pos = cameraWorldPos.xyz;
	float3 cam_origin = float3(-4.22, 1.19, 5.66);
	float3 pos = vin.wPos.xyz;
	float3 dir_ref_corrected = normalize(cam_pos - pos);
	float3 r = float3(-dir_ref_corrected.x, dir_ref_corrected.y, dir_ref_corrected.z);
	//r += cameraWorldPos.xyz;
	
	float3 N_reflected = reflect(-dir_to_eye, N);
	N_reflected = reflect(r, normalize(N) * 0.3);

	float4 env = txEnvironment.Sample(samWrapLinear, N_reflected);*/
	float4 new_color = float4(albedo.x*0.8, albedo.y*1.0, albedo.z*0.9, 1);

	//float4 refl = float4(ssrrColor(iPosition.xy, ViewProjection).xyz, 1);
	float4 refl = float4(ssrrColor(iPosition.xy, ViewProjection, albedo, wpos, N).xyz, 1);

	//float refl_val = length(txGloss.Load(uint3(iPosition.xy, 0)).xyz);	
	//refl *= float4(0.25, 0.29, 0.21, 1);
	//refl *= refl_val;
	//return refl;
	//refl = float4(1, 0, 0, 1);
	new_color = refl*fresnel + new_color*(1 - fresnel);
	//new_color = float4(new_color.x*0.4, new_color.y*0.5, new_color.z*0.35, 1);
	
	return new_color;
}

// -------------------------------------------------
// Glass
// -------------------------------------------------
float4 PSGlass(
VS_TEXTURED_OUTPUT vin
, in float4 iPosition : SV_Position

) : SV_Target0{

	float3 wpos = vin.wPos.xyz;

	
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
	float4 new_color = float4(albedo.x*0.9, albedo.y*0.9, albedo.z*0.9, 1);

	return env*fresnel + new_color*(1 - fresnel);
}

// Light shafts
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT VSLightShafts(
float4 Pos : POSITION
, float2 UV : TEXCOORD0
, float3 Normal : NORMAL
, float4 Tangent : TANGENT
)
{
	VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;
	output.wNormal = mul(Normal, (float3x3)World);

	float4 world_pos = mul(Pos, World);
	float cos_norm = (1 + cos(world_time)) * 0.5;
	float change = cos_norm;
	world_pos += float4(normalize(output.wNormal).xyz, 0) * change * 0.1 * UV.y;
	output.Pos = mul(world_pos, ViewProjection);
	
	output.UV = UV * 1;
	output.wPos = world_pos;
	// Rotate the tangent and keep the w value
	output.wTangent.xyz = mul(Tangent.xyz, (float3x3)World);
	output.wTangent.w = Tangent.w;
	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSLightShafts(VS_TEXTURED_OUTPUT input
	, in float4 iPosition : SV_Position) : SV_Target0{
	float my_depth = dot(input.wPos - cameraWorldPos, cameraWorldFront) / cameraZFar;

	int3 ss_load_coords = uint3(iPosition.xy, 0);
	float pixel_detph = txDepth.Load(ss_load_coords).x;

	float delta_z = abs(pixel_detph - my_depth);
	delta_z = saturate(delta_z * 1000);

	float3 dir_to_eye = normalize(cameraWorldPos.xyz - input.wPos.xyz);
	float3 N = normalize(input.wNormal.xyz);
	float fresnel = dot(N, dir_to_eye);

	float4 color = txDiffuse.Sample(samClampLinear, input.UV) * float4(1, 0.5, 0.2, 1);
		
	color.a *= txGloss.Sample(samWrapLinear, input.wPos.xz + world_time.xx * 0.1).x;
	color.a *= txGloss.Sample(samWrapLinear, input.wPos.yz - cos(world_time.xx) * 0.05).x;
	//color.a *= delta_z;
	//color.a *= pow(1 - input.UV.y, 1);
	color.a *= 0.6f;
	//color.a *= length(dir_to_eye);
	float change = (sin(world_time) + 1) * 0.5;
	color.a *= pow(fresnel, 2);
	return color;
}