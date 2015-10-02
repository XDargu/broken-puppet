#include "render/ctes/shader_ctes.h"

Texture2D txDiffuse : register(t0);
Texture2D txBlurred : register(t1);
Texture2D txDepth   : register(t2);
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
VS_TEXTURED_OUTPUT VSBlur(
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

static float3x3 conv =
{
	1. / 16., 1. / 8., 1. / 16.,
	1. / 8., 1. / 4., 1. / 8.,
	1. / 16., 1. / 8., 1. / 16.
};

static float3x1 conv_linear =
{
	1. / 4., 1. / 2., 1. / 4.
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSBlur(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float depth = txDepth.Sample(samClampLinear, input.UV).x;

	if (blur_amount > 0) {
		float4 color = float4(0, 0, 0, 0);		

		color += txDiffuse.Sample(samClampLinear, input.UV + float2(blur_delta.x * -1, blur_delta.y * -1) * blur_amount * saturate(depth) * 3) * (1./16.);
		color += txDiffuse.Sample(samClampLinear, input.UV + float2(blur_delta.x * -1, blur_delta.y * 0)  * blur_amount * saturate(depth) * 3) * (1./8.);
		color += txDiffuse.Sample(samClampLinear, input.UV + float2(blur_delta.x * -1, blur_delta.y * 1)  * blur_amount * saturate(depth) * 3) * (1./16.);
																																			
		color += txDiffuse.Sample(samClampLinear, input.UV + float2(blur_delta.x * 0, blur_delta.y * -1)  * blur_amount * saturate(depth) * 3) * (1. / 8.);
		color += txDiffuse.Sample(samClampLinear, input.UV + float2(blur_delta.x * 0, blur_delta.y * 0)   * blur_amount * saturate(depth) * 3) * (1./4.);
		color += txDiffuse.Sample(samClampLinear, input.UV + float2(blur_delta.x * 0, blur_delta.y * 1)   * blur_amount * saturate(depth) * 3) * (1./8.);
																																			
		color += txDiffuse.Sample(samClampLinear, input.UV + float2(blur_delta.x * 1, blur_delta.y * -1)  * blur_amount * saturate(depth) * 3) * (1. / 16.);
		color += txDiffuse.Sample(samClampLinear, input.UV + float2(blur_delta.x * 1, blur_delta.y * 0)   * blur_amount * saturate(depth) * 3) * (1./8.);
		color += txDiffuse.Sample(samClampLinear, input.UV + float2(blur_delta.x * 1, blur_delta.y * 1)   * blur_amount * saturate(depth) * 3) * (1./16.);

		// Old loop
		//float2 delta = float2(0, 0);
		/*for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				factor = conv[i][j];

				delta = float2(blur_delta.x * (i - 1), blur_delta.y * (j - 1)) * blur_amount * saturate(depth * 1) * 8;

				color += txDiffuse.Sample(samClampLinear, input.UV + delta * 0.4) * factor;
			}
		}*/

		return color * 1;
	}

	//return float4(input.UV.xy, 0, 0);
	//return float4(depth, 0, 0, 0) * 30;
	return txDiffuse.Sample(samClampLinear, input.UV);
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSBlurX(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float depth = txDepth.Sample(samClampLinear, input.UV).x;

	if (blur_amount > 0) {
		float4 color = float4(0, 0, 0, 0);
		float2 delta = float2(0, 0);
		float factor = 1.f;

		for (int i = 0; i < 3; i++) {
			factor = conv_linear[i];
			delta = float2(blur_delta.x * (i - 1), 0) * blur_amount * saturate(depth) * 8;
			color += txDiffuse.Sample(samClampLinear, input.UV + delta * 0.4) * factor;
		}

		return color * 1;
	}

	return txDiffuse.Sample(samClampLinear, input.UV);
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSBlurY(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float depth = txDepth.Sample(samClampLinear, input.UV).x;

	if (blur_amount > 0) {
		float4 color = float4(0, 0, 0, 0);
			float2 delta = float2(0, 0);
			float factor = 1.f;

		for (int i = 0; i < 3; i++) {
			factor = conv_linear[i];
			delta = float2(0, blur_delta.y * (i - 1)) * blur_amount * saturate(depth) * 8;
			color += txDiffuse.Sample(samClampLinear, input.UV + delta * 0.4) * factor;
		}

		return color * 1;
	}

	return txDiffuse.Sample(samClampLinear, input.UV);

}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSBlurByZ(VS_TEXTURED_OUTPUT input) : SV_Target
{
  float4 original = txDiffuse.Sample(samClampLinear, input.UV);
  float4 blurred = txBlurred.Sample(samClampLinear, input.UV);
  float  depth = txDepth.Sample(samClampLinear, input.UV).x;
  float alfa = 1-saturate(abs( depth - 0.1) * 10);
  
  //return original;
  //return blurred;
  alfa = 1.5;
  return original *(1 - alfa) + blurred * alfa;
  
}

