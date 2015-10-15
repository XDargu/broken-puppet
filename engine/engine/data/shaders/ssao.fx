#include "render/ctes/shader_ctes.h"

Texture2D txDiffuse : register(t0);
Texture2D txNormal    : register(t1);
Texture2D txDepth   : register(t2);
Texture2D txRandom   : register(t9);
SamplerState samWrapLinear : register(s0);
SamplerState samClampLinear : register(s1);

//--------------------------------------------------------------------------------------
struct VS_TEXTURED_OUTPUT
{
  float4 Pos    : SV_POSITION;
  float2 UV     : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT VSSSAO(
  float4 Pos : POSITION
, float2 UV : TEXCOORD0
)
{
  VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;
  output.Pos = mul(Pos, World);
  output.Pos = mul(output.Pos, ViewProjection);
  output.UV = UV;
  return output;
}
/*
float3 getWorldCoords(float2 normalized_screen_coords, float depth) {			// -0.5 .. 0.5
	float3 view_dir = float3(1-normalized_screen_coords.x
		, 1-normalized_screen_coords.y * aspect_ratio
		, cameraViewD / cameraHalfXRes);
	view_dir = view_dir / cameraViewD * cameraZFar * depth;

	float3 world_dir = view_dir.z * cameraWorldFront.xyz
		+ view_dir.y * cameraWorldUp.xyz
		+ view_dir.x * cameraWorldLeft.xyz;
	float3 wPos = cameraWorldPos.xyz + world_dir;
		return wPos;
}
*/

float3 getWorldCoords(float2 screen_coords, float depth) {
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
	float3 world_pos = getWorldCoords(screen_coords, depth);
	float3 pos_vs = mul(world_pos, cameraView);
	return pos_vs;

	//return getWorldCoords(screen_coords, depth);
	//return float3(perspective_correction(screen_coords), -1) * depth * cameraZFar;
//	return float3(perspective_correction(coord2D(screen_coords)), -1) * depth * cameraZFar;
}



float GetAmbientOcclusion(
	in float2 textureCoords
	, in float2 uv
	, in float3 position
	, in float3 normal
	, in float depth
	)
{
	float2 coords = textureCoords + uv;

		//float3 diff = (float3(perspective_correction(coord2D(coords)), -1) * depth * cameraZFar) - position;
		float3 diff = getViewSpace(coords, depth) - position;

	float AmbientOcclusionDepthClamp = 0.5 * 1;
	float AmbientOcclusionScale = 0.5 * 1;
	float AmbientOcclusionBias = 0.1;
	float AmbientOcclusionIntensity = 1;

	//float3 diff = GetPositionInViewSpace(coords, depth) - position;

	float3 v = normalize(diff);
	float  d = length(diff) * AmbientOcclusionScale;
	return max(0.0, dot(normal, v) - AmbientOcclusionBias)*(1.0 / (1.0 + d))*AmbientOcclusionIntensity;
}

static int sample_count = 6;
static float2 poisson16[] = {    // These are the Poisson Disk Samples
		float2(-0.94201624, -0.39906216),
		float2(0.94558609, -0.76890725),
		float2(-0.094184101, -0.92938870),
		float2(0.34495938, 0.29387760),
		float2(-0.91588581, 0.45771432),
		float2(-0.81544232, -0.87912464),
		float2(-0.38277543, 0.27676845),
		float2(0.97484398, 0.75648379),
		float2(0.44323325, -0.97511554),
		float2(0.53742981, -0.47373420),
		float2(-0.26496911, -0.41893023),
		float2(0.79197514, 0.19090188),
		float2(-0.24188840, 0.99706507),
		float2(-0.81409955, 0.91437590),
		float2(0.19984126, 0.78641367),
		float2(0.14383161, -0.14100790)
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSSSAO(
	VS_TEXTURED_OUTPUT input,
	in float4 iPosition : SV_Position
	) : SV_Target
{

	// New code
	int3 screenCoords = uint3(iPosition.xy, 0);

	float depth = txDepth.Load(screenCoords).x;
	float3 wPos = getWorldCoords(screenCoords, depth);

	float3 normal = txNormal.Load(screenCoords).xyz;
	float3 wNormal = normalize(normal * 2.0f - 1.0f);

	/*float3 viewNormal = mul(normal, (float3x3) cameraView);
	viewPos = getViewSpace(input.UV * float2(cameraHalfXRes * 2, cameraHalfYRes * 2), depth);// mul(wPos, cameraView);*/

	//float ssao_radius = 0.003;
	float ssao_radius = 0.0001 / depth;
	float distanceThreshold = 0.2;
	float AmbientOcclusionBias = 0.12;
	float AmbientOcclusionIntensity = 2.5;

	/*float  randomSize = 64.0f;
	float2 randomCoords = txRandom.Sample(samWrapLinear, iPosition.xy / 64).xy * 2.0f - 1.0f;
	randomCoords = normalize(randomCoords);*/
	

	float ao = 0;
	//return float4(input.UV, 0, 1);

	//return float4(viewPos, 1);
	float2 camRes = float2(cameraHalfXRes * 2, cameraHalfYRes * 2);

	for (int i = 0; i < sample_count; ++i) {
		// sample at an offset specified by the current Poisson-Disk sample and scale it by a radius (has to be in Texture-Space)
		float2 sampleTexCoord = input.UV + (poisson16[i] * (ssao_radius));
		float sampleDepth = txDepth.Sample(samClampLinear, sampleTexCoord).x;
		float3 samplePos = getWorldCoords(sampleTexCoord * camRes, sampleDepth);
		float3 sampleDir = normalize(samplePos - wPos);
		//return float4(samplePos, 1);
		// angle between SURFACE-NORMAL and SAMPLE-DIRECTION (vector from SURFACE-POSITION to SAMPLE-POSITION)
		float NdotS = max(dot(wNormal, sampleDir), 0);
		// distance between SURFACE-POSITION and SAMPLE-POSITION
		float VPdistSP = distance(wPos, samplePos);

		// a = distance function
		float a = 0.9 - smoothstep(distanceThreshold, distanceThreshold * 2, VPdistSP);
		// b = dot-Product
		//float b = NdotS;
		
		ao += max(0.0, dot(wNormal, sampleDir) - AmbientOcclusionBias)*a*AmbientOcclusionIntensity;

		//ao += (a * b);
	}

	
	//float3 diffuse = txDiffuse.Sample(samClampLinear, input.UV).xyz;
	//return float4(diffuse, 0) * (1 - (ao / sample_count));
	return (1 - (ao / sample_count));
	/*float2 uv0 = input.UV;

	int3 screenCoords = uint3(iPosition.xy, 0);

	float3 diffuse = txDiffuse.Sample(samClampLinear, input.UV).xyz;
	float3 normal = txNormal.Sample(samClampLinear, input.UV).xyz;
	//normal *= cameraViewD;
	normal = normalize(normal * 2.0f - 1.0f);
	float depth = txDepth.Sample(samClampLinear, input.UV).x;

	float3 world_pos = getWorldCoords(screenCoords, depth);

	float3 normal_vs = mul(normal, (float3x3) cameraView);
	float3 pos_vs = mul(world_pos, cameraView);

	float3 positionViewSpace = getViewSpace(iPosition.xy, depth);
	//positionViewSpace = float3(perspective_correction(coord2D(screenCoords)), -1) * depth * cameraZFar;
	//return float4(positionViewSpace, 0);
	//return float4(normal, 0);
	//return float4(normal_vs, 0);
	//return float4(pos_vs.y - (int)pos_vs.y, 0, 0, 0);


	float2 resolution = 1;// 1 / ssao_delta;
	float2 screenTexture = float2(iPosition.x * resolution.x, iPosition.y * resolution.y);

	//return float4(screenTexture.x, screenTexture.y, 0, 0) * 0.001;
	
	float  randomSize = 64.0f;
	float2 randomCoords = txRandom.Sample(samWrapLinear, screenTexture / randomSize).xy * 2.0f - 1.0f;
	randomCoords = normalize(randomCoords);
	//return float4(randomCoords, 0, 0);

	//return float4(randomCoords.x, randomCoords.y, 0, 0) * 1;
	
	float ssao_radius = 0.5 / depth;

	// SSAO
	float ambientOcclusion = 0.0f;
	int iterations = 4;
	const float2 vec[4] = { float2(1.0f, 0.0f), float2(-1.0f, 0.0f), float2(0.0f, 1.0f), float2(0.0f, -1.0f) };

	for (int i = 0; i < iterations; ++i)
	{
		float2 coord1 = reflect(vec[i], randomCoords) * ssao_radius;
		float2 coord2 = float2(coord1.x * 0.707f - coord1.y * 0.707f,
		                       coord1.x * 0.707f + coord1.y * 0.707f);
		ambientOcclusion += GetAmbientOcclusion(iPosition.xy, coord1 * 0.25f, positionViewSpace, normal_vs, depth);
		ambientOcclusion += GetAmbientOcclusion(iPosition.xy, coord2 * 0.5f, positionViewSpace, normal_vs, depth);
		ambientOcclusion += GetAmbientOcclusion(iPosition.xy, coord1 * 0.75f, positionViewSpace, normal_vs, depth);
		ambientOcclusion += GetAmbientOcclusion(iPosition.xy, coord2 * 1.0f, positionViewSpace, normal_vs, depth);
	}

	ambientOcclusion /= (float)iterations * 4.0f;
	
	//return float4(diffuse, 0);// *(1 - ambientOcclusion);
	return 1.0 - ambientOcclusion;*/
}

float4 PSSSAOBlur(VS_TEXTURED_OUTPUT input, in float4 iPosition : SV_Position) : SV_Target
{
	float4 original = txDiffuse.Sample(samClampLinear, input.UV);
	float4 ao = txRandom.Sample(samClampLinear, input.UV);
	float4 depth = txDepth.Sample(samClampLinear, input.UV);
	//return original;
	//return txLuminance.Sample(samClampLinear, input.UV);
	// Blur the glow image
	float4 luminance = float4(0, 0, 0, 0);
	

	luminance += txRandom.Sample(samClampLinear, input.UV + float2(ssao_delta.x * -1, ssao_delta.y * -1) * 3) * (1. / 16.);
	luminance += txRandom.Sample(samClampLinear, input.UV + float2(ssao_delta.x * -1, ssao_delta.y * 0)  * 3) * (1. / 8.);
	luminance += txRandom.Sample(samClampLinear, input.UV + float2(ssao_delta.x * -1, ssao_delta.y * 1)  * 3) * (1. / 16.);

	luminance += txRandom.Sample(samClampLinear, input.UV + float2(ssao_delta.x * 0, ssao_delta.y * -1)  * 3) * (1. / 8.);
	luminance += txRandom.Sample(samClampLinear, input.UV + float2(ssao_delta.x * 0, ssao_delta.y * 0)   * 3) * (1. / 4.);
	luminance += txRandom.Sample(samClampLinear, input.UV + float2(ssao_delta.x * 0, ssao_delta.y * 1)   * 3) * (1. / 8.);

	luminance += txRandom.Sample(samClampLinear, input.UV + float2(ssao_delta.x * 1, ssao_delta.y * -1)  * 3) * (1. / 16.);
	luminance += txRandom.Sample(samClampLinear, input.UV + float2(ssao_delta.x * 1, ssao_delta.y * 0)   * 3) * (1. / 8.);
	luminance += txRandom.Sample(samClampLinear, input.UV + float2(ssao_delta.x * 1, ssao_delta.y * 1)   * 3) * (1. / 16.);

	// Old loop
	/*
	//float2 delta = float2(0, 0);
	//float factor = 1.f;
	for (int i = 0; i < 5; i++) {
	for (int j = 0; j < 5; j++) {
	//factor = conv[i][j];
	factor = 1.0 / 25.0;
	delta = float2(glow_delta.x * (i - 1), glow_delta.y * (j - 1)) * 20;
	luminance += txLuminance.SampleLevel(samClampLinear, input.UV + delta * 0.3, 9) * factor;
	}
	}*/

	return (luminance * original);
	//return original;
	//return blurred;
	//return luminance;
	if (luminance.r > 0.1) {
		float alfa = 0.8;
		return original *(1 - alfa) + luminance * alfa;
	}
	return original;

}
