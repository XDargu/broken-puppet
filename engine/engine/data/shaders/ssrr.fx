#include "render/ctes/shader_ctes.h"

Texture2D txDiffuse : register(t0);
Texture2D txNormal    : register(t1);
Texture2D txDepth   : register(t2);
Texture2D txGloss   : register(t5);
SamplerState samWrapLinear : register(s0);
SamplerState samClampLinear : register(s1);

//--------------------------------------------------------------------------------------
struct VS_TEXTURED_OUTPUT
{
  float4 Pos    : SV_POSITION;
  float2 UV     : TEXCOORD0;
};

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
	//return txDiffuse.Load(ss_load_coords);
	float depth = txDepth.Load(ss_load_coords).x;
	//float3 normal = normalize(txNormal.Load(ss_load_coords).xyz * 2 - 1.);
	
	//float3 worldStartingPos = getWorldCoords(iPosition.xy, depth);

	float3 cameraToWorld = worldStartingPos - cameraWorldPos.xyz;
	float cameraToWorldDist = length(cameraToWorld);
	float3 cameraToWorldNorm = normalize(cameraToWorld);
	float3 refl = normalize(reflect(cameraToWorldNorm, normal)); // This is the reflection vector
	//return float4(normal, 1);
	//return float4(worldStartingPos, 1);
	/*float4 sc;
	sc = mul(float4(worldStartingPos, 1), viewproj);
	sc.xyz /= sc.w;
	sc.x = (sc.x + 1) * 0.5;
	sc.y = (1 - sc.y) * 0.5;

	return txDiffuse.Sample(samClampLinear, sc.xy);*/
	//return float4(refl, 1);
	//float2 m_coords = sc.xy;

	if (dot(refl, cameraToWorldNorm) < 0) {
		// Ignore reflections going backwards towards the camera, indicate with white
		return origColor;
	}

	float cosAngle = abs(dot(normal, cameraToWorldNorm)); // Will be a value between 0 and 1
	float fact = 1 - cosAngle;
	fact = min(1, 1.38 - fact*fact);
	
	float3 newPos;
	float4 newScreen;
	float i = 0;
	float3 rayTrace = worldStartingPos;
	float currentWorldDist, rayDist;
	float incr = 1 / ssrr_quality;
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
		float n_depth = txDepth.Sample(samClampLinear, coords).x;
		//return float4(n_depth, 0, 0, 1);
		newPos = getWorldCoords(coords * float2(cameraHalfXRes * 2, cameraHalfYRes * 2), n_depth);
		//return txDiffuse.Sample(samClampLinear, coords);
		//newPos = rayTrace;
		//return float4(newPos, 1);

		currentWorldDist = length(newPos.xyz - cameraWorldPos.xyz);
		rayDist = length(rayTrace.xyz - cameraWorldPos.xyz);
		if (newScreen.x > 1 || newScreen.x < -1 || newScreen.y > 1 || newScreen.y < -1 || newScreen.z > 1 || newScreen.z < -1 || i >= 0.5 || cameraToWorldDist > currentWorldDist) {
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

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT VSSSRR(
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

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSSSRR(in float4 iPosition : SV_Position, VS_TEXTURED_OUTPUT input) : SV_Target
{
	if (ssrr_amount > 0) {
		int3 ss_load_coords = uint3(iPosition.xy, 0);
		float4 color = txDiffuse.Load(ss_load_coords);
		float3 normal = normalize(txNormal.Load(ss_load_coords).xyz * 2 - 1.);
		float depth = txDepth.Load(ss_load_coords).x;
	
		float refl_val = length(txGloss.Load(uint3(iPosition.xy, 0)).xyz) * ssrr_amount;

		float3 wpos = getWorldCoords(iPosition.xy, depth);
		float4 refl = float4(ssrrColor(iPosition.xy, GameViewProjection, color, wpos, normal).xyz, 1);

		return refl * refl_val + color * (1 - refl_val);
	}

	return txDiffuse.Sample(samClampLinear, input.UV);
}
