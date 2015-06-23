#include "render/ctes/shader_ctes.h"

Texture2D txDiffuse   : register(t0);
Texture2D txNormal    : register(t1);
Texture2D txSpecular  : register(t2);
Texture2D txGloss  : register(t3);
Texture2D txEmissive  : register(t4);
Texture2D txAO  : register(t5);
Texture2D txShadowMap : register(t6);
TextureCube EnvMap : register(t7);
SamplerState samWrapLinear : register(s0);
SamplerState samClampLinear : register(s1);
SamplerState samBorderLinear : register(s2);
SamplerComparisonState samPCFShadows : register(s3);
SamplerState EnvMapSampler : TEXUNIT3{
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
};

//--------------------------------------------------------------------------------------
void VSGenShadows(
        float4 iPos : POSITION
  , out float4 oPos : SV_POSITION
  ) {
  float4 world_pos = mul(iPos, World);
  oPos = mul(world_pos, ViewProjection);
}

void VSGenShadowsSkel(
	float4 ipos     : POSITION
	, float2 iuv : TEXCOORD0
	, float3 inormal : NORMAL
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
  output.UV = UV;
  output.wPos = world_pos;
  // Rotate the tangent and keep the w value
  output.wTangent.xyz = mul(Tangent.xyz, (float3x3)World);
  output.wTangent.w = Tangent.w;
  return output;
}


// PBR TEST

float PI = 3.14159;

// http://graphicrants.blogspot.com.au/2013/08/specular-brdf-reference.html
float GGX(float NdotV, float a)
{
	float k = a / 2;
	return NdotV / (NdotV * (1.0f - k) + k);
}

// http://graphicrants.blogspot.com.au/2013/08/specular-brdf-reference.html
float G_Smith(float a, float nDotV, float nDotL)
{
	return GGX(nDotL, a * a) * GGX(nDotV, a * a);
}

float radicalInverse_VdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float2 Hammersley(uint i, uint N) {
	return float2(float(i) / float(N), radicalInverse_VdC(i));
}


float3 ImportanceSampleGGX(float2 Xi, float Roughness, float3 N)
{
	float a = Roughness * Roughness; // DISNEY'S ROUGHNESS [see Burley'12 siggraph]

	// Compute distribution direction
	float Phi = 2 * PI * Xi.x;
	float CosTheta = sqrt((1 - Xi.y) / (1 + (a*a - 1) * Xi.y));
	float SinTheta = sqrt(1 - CosTheta * CosTheta);

	// Convert to spherical direction
	float3 H;
	H.x = SinTheta * cos(Phi);
	H.y = SinTheta * sin(Phi);
	H.z = CosTheta;

	float3 UpVector = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 TangentX = normalize(cross(UpVector, N));
	float3 TangentY = cross(N, TangentX);

	// Tangent to world space
	return TangentX * H.x + TangentY * H.y + N * H.z;
}

float3 SpecularIBL(float3 SpecularColor, float Roughness, float3 N, float3 V)
{
	float3 SpecularLighting = 0;
	const uint NumSamples = 1024;
	for (uint i = 0; i < NumSamples; i++)
	{
		float2 Xi = Hammersley(i, NumSamples);
		float3 H = ImportanceSampleGGX(Xi, Roughness, N);
		float3 L = 2 * dot(V, H) * H - V;
		float NoV = saturate(dot(N, V));
		float NoL = saturate(dot(N, L));
		float NoH = saturate(dot(N, H));
		float VoH = saturate(dot(V, H));
		if (NoL > 0)
		{
			float3 SampleColor = EnvMap.SampleLevel(EnvMapSampler, L, 0).rgb;
			float G = G_Smith(Roughness, NoV, NoL);
			float Fc = pow(1 - VoH, 5);
			float3 F = (1 - Fc) * SpecularColor + Fc;
				// Incident light = SampleColor * NoL
				// Microfacet specular = D*G*F / (4*NoL*NoV)
				// pdf = D * NoH / (4 * VoH)
				SpecularLighting += SampleColor * F * G * VoH / (NoH * NoV);
		}
	}
	return SpecularLighting / NumSamples;
}

float3 PrefilterEnvMap(float Roughness, float3 R)
{
	float3 TotalWeight = float3(0, 0, 0);
	float3 N = R;
	float3 V = R;
	float3 PrefilteredColor = 0;
	const uint NumSamples = 16;
	for (uint i = 0; i < NumSamples; i++)
	{
		float2 Xi = Hammersley(i, NumSamples);
			float3 H = ImportanceSampleGGX(Xi, Roughness, N);
			float3 L = 2 * dot(V, H) * H - V;
			float NoL = saturate(dot(N, L));
		if (NoL > 0)
		{
			PrefilteredColor += EnvMap.SampleLevel(EnvMapSampler, L, 0).rgb * NoL;
			TotalWeight += NoL;
		}
	}
	return PrefilteredColor / TotalWeight;
}

float2 IntegrateBRDF(float Roughness, float NoV, float3 N)
{
	//float3 N = float3(0, 0, 1);

	float3 V;
	V.x = sqrt(1.0f - NoV * NoV); // sin
	V.y = 0;
	V.z = NoV; // cos
	float A = 0;
	float B = 0;
	const uint NumSamples = 16;
	for (uint i = 0; i < NumSamples; i++)
	{
		float2 Xi = Hammersley(i, NumSamples);
		float3 H = ImportanceSampleGGX(Xi, Roughness, N);
		float3 L = 2 * dot(V, H) * H - V;
		float NoL = saturate(L.z);
		float NoH = saturate(H.z);
		float VoH = saturate(dot(V, H));
		if (NoL > 0)
		{
			float G = G_Smith(Roughness, NoV, NoL);
			float G_Vis = G * VoH / (NoH * NoV);
			float Fc = pow(1 - VoH, 5);
			A += (1 - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}
	return float2(A, B) / NumSamples;
}

float3 ApproximateSpecularIBL(float3 SpecularColor, float Roughness, float3 N, float3 V)
{
	float NoV = saturate(dot(N, V));
	float3 R = 2 * dot(V, N) * N - V;

	float3 PrefilteredColor = PrefilterEnvMap(Roughness, R);
	//float3 PrefilteredColor = float3(1, 1, 1);
	float2 EnvBRDF = IntegrateBRDF(Roughness, NoV, N);

	return PrefilteredColor * (SpecularColor * EnvBRDF.x + EnvBRDF.y);
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
  float3 E = normalize(CameraWorldPos.xyz - input.wPos.xyz);
  //float3 H = normalize(E + L);
  //float  cos_beta = saturate( dot(H, N) );
  float3 ER = reflect(-E, N);
  float  cos_beta = saturate(dot(ER, L));
  float  spec_amount = pow(cos_beta, 20.);

  float4 albedo = txDiffuse.Sample(samWrapLinear, input.UV);
  return albedo * diffuse_amount + spec_amount;
}


//--------------------------------------------------------------------------------------
float getShadowAtCoords(float4 wPos) {
	float4 light_proj_coords = mul(wPos, LightViewProjectionOffset);
		light_proj_coords.xyz /= light_proj_coords.w;
	if (light_proj_coords.z <= 1e-3)
		return 0;
	float amount = txShadowMap.SampleCmpLevelZero(samPCFShadows
		, light_proj_coords.xy, light_proj_coords.z - 0.01);
	return amount;
}

float getShadowAt(float4 wPos) {

	float4 amount = getShadowAtCoords(wPos);

		float sz = 1.0 / 128
		;

	amount += getShadowAtCoords(wPos + float4(sz, sz, 0, 0));
	amount += getShadowAtCoords(wPos + float4(-sz, sz, 0, 0));
	amount += getShadowAtCoords(wPos + float4(sz, -sz, 0, 0));
	amount += getShadowAtCoords(wPos + float4(-sz, -sz, 0, 0));

	amount *= 1.0 / 5;

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
  float3 normal_tangent_space =  txNormal.Sample(samWrapLinear, input.UV) * 2 - 1.;
  float3 wnormal_per_pixel = mul(normal_tangent_space, TBN);

  // Save the normal
  normal = float4(wnormal_per_pixel, 1);
  
  // Basic diffuse lighting
  float3 L = LightWorldPos.xyz - input.wPos.xyz;	  
  L = normalize(L);
  float3 N = normalize(wnormal_per_pixel);
  float  diffuse_amount = saturate(dot(N, L));

  // Speculares
  float3 E = normalize(CameraWorldPos.xyz - input.wPos.xyz);
  //float3 H = normalize(E + L);
  //float  cos_beta = saturate( dot(H, N) );
  float3 ER = reflect(-E, N);
  float cos_beta = saturate(dot(ER, L));
  float spec_amount = pow(cos_beta, 2.);
  float spec_intensity = txSpecular.Sample(samWrapLinear, input.UV);
  float spec_final = saturate(spec_amount * spec_intensity);

  /*float3 reflectedColor = txCubemap.Sample(samCube, ER);
  float4 cube_reflection = float4(reflectedColor, 1);*/

  float3 emissiveColor = txEmissive.Sample(samWrapLinear, input.UV);
  float4 emissive = float4(emissiveColor, 1);
  // 
  acc_light = getShadowAt(input.wPos);

  //albedo *= (0.3 + acc_light * 0.7);
  //albedo = (diffuse_amount * albedo + spec_final) * acc_light + emissive;  

  //ApproximateSpecularIBL(float3 SpecularColor, float Roughness, float3 N, float3 V)
  float3 SpecularColor = txSpecular.Sample(samWrapLinear, input.UV).rgb;
	  SpecularColor = pow(length(SpecularColor), 2) * SpecularColor;
  float Roughness = 1 - txGloss.Sample(samWrapLinear, input.UV);
  //SpecularColor = float3(0.2, 0.2, 0.2);
  //Roughness = 1;
  float3 res = ApproximateSpecularIBL(SpecularColor, Roughness, N, E);

  //albedo = albedo * (diffuse_amount + float4(res, 1));

  // Fresnel
  float angle = 1.0f - saturate(dot(N, E));
  float fresnel = angle * angle;
  fresnel = fresnel * fresnel;
  fresnel = fresnel * angle;
  float refractionStrength = 0.1;
  float4 fresnelFromAlbedo = saturate(fresnel * (1.0f - saturate(albedo)) + albedo - refractionStrength);
	  float4 fresnel4 = float4(fresnel, fresnel, fresnel, fresnel);

	  float4 ao = txAO.Sample(samWrapLinear, input.UV);

	  float4 diffuse_brdf = /*(1 - fresnel) */ albedo / 3.14159;
	  albedo = albedo / 3.14159 * diffuse_amount + emissive;
	  //albedo = (saturate(albedo * diffuse_amount * ao) + saturate(float4(res, 1))) * saturate(acc_light) + emissive;

	  
  //albedo *= saturate(acc_light);
  
  
  //albedo = float4(1, 0, 0, 1);
}



