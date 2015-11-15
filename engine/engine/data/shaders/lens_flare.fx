#include "render/ctes/shader_ctes.h"

Texture2D txDiffuse : register(t0);
Texture2D txLuminance : register(t1);
Texture2D txLensFlareMask : register(t9);
Texture2D txLensDirt : register(t10);

SamplerState samWrapLinear : register(s0);
SamplerState samClampLinear : register(s1);

//--------------------------------------------------------------------------------------
struct VS_TEXTURED_OUTPUT
{
  float4 Pos    : SV_POSITION;
  float2 UV     : TEXCOORD0;
  float2 UV1     : TEXCOORD1;
  float2 UV2     : TEXCOORD2;
  float2 UV3     : TEXCOORD3;
  float2 UV4     : TEXCOORD4;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT VSLensFlare(
  float4 Pos : POSITION
, float2 UV : TEXCOORD0
)
{
  VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;
  output.Pos = mul(Pos, World);
  output.Pos = mul(output.Pos, ViewProjection);
  output.UV = UV;
  output.UV1 = (UV - float2(0.5, 0.5))*-1.2f + 0.5f;
  output.UV2 = (UV - float2(0.5, 0.5))*2.0f + 0.5f;
  output.UV3 = (UV - float2(0.5, 0.5))*-0.3f + 0.5f;
  output.UV4 = (UV - float2(0.5, 0.5))*0.6f + 0.5f;
  return output;
}


float4 textureDistorted(
    in Texture2D tex,
    in float2 texcoord,
    in float2 direction, // direction of distortion
    in float3 distortion // per-channel distortion factor
    ) {

    float colorR = tex.Sample(samClampLinear, texcoord + direction*distortion.x).r;
    float colorG = tex.Sample(samClampLinear, texcoord + direction*distortion.y).g;
    float colorB = tex.Sample(samClampLinear, texcoord + direction*distortion.z).b;
    return float4(colorR,colorG,colorB,0);
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSLensFlare(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 color = float4(0, 0, 0, 1);
  
    float mask1 = txLensFlareMask.Sample(samClampLinear, input.UV1).x;
    float mask2 = txLensFlareMask.Sample(samClampLinear, input.UV2).x;
    float mask3 = txLensFlareMask.Sample(samClampLinear, input.UV3).x;
    float mask4 = txLensFlareMask.Sample(samClampLinear, input.UV4).x;

    float2 center1 = abs((input.UV1 - float2(0.5, 0.5)) * 2);
    float dist1 = length(center1);
    float weight1 = pow(1.0 - dist1, 6.0f);

    float2 center2 = abs((input.UV2 - float2(0.5, 0.5)) * 2);
    float dist2 = length(center2);
    float weight2 = pow(1.0 - dist2, 6.0f);

    float2 center3 = abs((input.UV3 - float2(0.5, 0.5)) * 2);
    float dist3 = length(center3);
    float weight3 = pow(1.0 - dist3, 6.0f);

    float2 center4 = abs((input.UV4 - float2(0.5, 0.5)) * 2);
    float dist4 = length(center4);
    float weight4 = pow(1.0 - dist4, 6.0f);


    float uDistortion = 5.0f;
	float2 screenResGlobal = float2(cameraHalfXRes * 2, cameraHalfYRes * 2);
    float2 texelSize = 1.0 / screenResGlobal.xy;

    float3 distortion = float3(-texelSize.x * uDistortion, 0.0, texelSize.x * uDistortion);
    float2 direction = normalize(input.UV1 - input.UV);
	float4 color1 = txLuminance.Sample(samClampLinear, input.UV1);
	color1 = textureDistorted(txLuminance, input.UV1, direction, distortion);
	
    direction = normalize(input.UV2 - input.UV);
	float4 color2 = txLuminance.Sample(samClampLinear, input.UV2);
		color2 = textureDistorted(txLuminance, input.UV2, direction, distortion);

   
    
    direction = normalize(input.UV3 - input.UV);
	float4 color3 = txLuminance.Sample(samClampLinear, input.UV3)*mask3;
		color3 = textureDistorted(txLuminance, input.UV3, direction, distortion);
   
    
    direction = normalize(input.UV4 - input.UV);
	float4 color4 = txLuminance.Sample(samClampLinear, input.UV4);
		color4 = textureDistorted(txLuminance, input.UV4, direction, distortion);

    color1 *= mask1*weight1;
    color2 *= mask2*weight2;
    color3 *= mask3*weight3;
    color4 *= mask4*weight4;
    color = color1 + color2 + color3 + color4;
		
    float2 haloVec = normalize((float2(0.5, 0.5) - input.UV) * 0.5f) * 0.6f;
    float maskHalo = txLensFlareMask.Sample(samClampLinear, input.UV + haloVec).x;

    float weight = length(float2(0.5, 0.5) - (input.UV + haloVec)) / length(float2(0.5, 0.5));
    weight = pow(1.0 - weight, 5.0f);
	float4 colorHalo = textureDistorted(txDiffuse, input.UV + haloVec, direction, distortion)*weight*maskHalo;
    color += colorHalo;	

	float3 dirt = txLensDirt.Sample(samClampLinear, input.UV).xyz*5.0f;

	color *= float4(dirt, 1);
	color = float4(color.xyz*0.1f, 1);
	
	return txDiffuse.Sample(samClampLinear, input.UV);// +color;
    


}