
#ifdef WIN32

#define float4              XMVECTOR
#define matrix              XMMATRIX
#define cbuffer             struct 
#define SHADER_REGISTER(x)  

#else

// We are in HLSL
#define SHADER_REGISTER(x)  : register(x)

#endif