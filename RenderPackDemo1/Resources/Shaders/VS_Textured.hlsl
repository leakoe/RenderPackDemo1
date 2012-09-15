#include "Common.hlsl"

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------

struct VS_INPUT
{
	float3 pos	: POSITION;
	float3 norm	: NORMAL;
	float2 tex	: TEXCOORD;
};

struct VS_OUTPUT
{
	float4 pos	: SV_POSITION;
	float3 norm	: NORMAL;
	float2 tex	: TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------

VS_OUTPUT VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	Output.pos = mul( float4(Input.pos, 1), g_mWorldViewProj );
	Output.norm = Input.norm;
	Output.tex = Input.tex;

	return Output;
}

//--------------------------------------------------------------------------------------

