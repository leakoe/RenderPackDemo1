#include "Common.hlsl"
struct VS_INPUT
{
	float3 pos	: POSITION;
	
};

struct VS_OUTPUT
{
	float4 pos	: SV_POSITION;

};

StructuredBuffer<float3> 	g_Positions		: register(t0);
//per AU
//Buffer cbAU : register(b4) ;
//per AU
cbuffer m_rCBAUs : register(b4) {
	int g_nAUs;
	int g_nAllVerts;
	float4 g_pAnimationUnits[6*113];
	int1  g_pAU_nVerts[6];
	FLOAT g_pAU_Weights[6];
}

//--------------------------------------------------------------------------------------
// Vertex Shader, g_pAU_nVerts, g_nAU_weights, g_pAnimationUnits xyz, idx
//--------------------------------------------------------------------------------------

VS_OUTPUT VSMain( VS_INPUT Input, uint vidx	: SV_VertexID )
{
	VS_OUTPUT Output;

	float4 vpos = float4(g_Positions[vidx], 1.0f);
	for(uint auidx = 0; auidx < 6; auidx++) {
		float tmp_weight = 1.0;
		if(auidx == 0) {
			tmp_weight = 0.5f;
		} else if (auidx == 1) {
			tmp_weight = 0.5f;
		} else if (auidx == 2) {
			tmp_weight = 0.5f;
		} else if (auidx == 3) {
			tmp_weight = 0.5f;
		} else if (auidx == 4) {
			tmp_weight = 0.5f;
		} else if (auidx == 5) {
			tmp_weight = 0.5f;
		}
		//float tmp_weight = g_pAU_Weights[auidx];
		//if(tmp_weight != 0) {
					vpos.x += tmp_weight*g_pAnimationUnits[auidx*113+vidx].x;
					vpos.y += tmp_weight*g_pAnimationUnits[auidx*113+vidx].y;
					vpos.z += tmp_weight*g_pAnimationUnits[auidx*113+vidx].z;
			
		//}			
	}
	Output.pos = mul( vpos, g_mWorldViewProj );
	
	return Output;
}
//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
//
//VS_OUTPUT VSMain( uint idx: SV_VertexID )
//{
//	pos = 0;
//	for(int auidx = 0; auidx < 10; auidx++) {
//		aupos = texAUs.Load(idx, auidx, 0);
//		pos += aupos *auweight[auidx];
//	}
//	
//	
//	return Output;
//}

//--------------------------------------------------------------------------------------

