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
cbuffer m_rCBAUs : register(b6) {
	//int  g_pAU_nVerts[6];
	float1 g_pAU_Weights[6];
}

cbuffer m_rCBAU1s: register(b5) {
	float4 g_pAnimationUnits[668];
	//float g_pAU_Weights[6];
}

//--------------------------------------------------------------------------------------
// Vertex Shader, g_pAU_nVerts, g_nAU_weights, g_pAnimationUnits xyz, idx
//--------------------------------------------------------------------------------------

VS_OUTPUT VSMain( VS_INPUT Input, uint vidx	: SV_VertexID )
{
	VS_OUTPUT Output;

	float4 vpos = float4(g_Positions[vidx], 1.0f);
	for(uint auidx = 0; auidx < 6; auidx++) {
		//float tmp_weight =  ;
		/*if(auidx == 0) {
			tmp_weight = 0.f;
		}
		 else if (auidx == 1) {
			tmp_weight = 0.5f;
		} */
		// else if (auidx == 2) {
		//	tmp_weight = 0.5f;
		//} 
		// else if (auidx == 3) {
		//	tmp_weight = 0.5f;
		//}	
		// else if (auidx == 4) {
		//	tmp_weight = 0.5f;
		//}
		// else if (auidx == 5) {
		//	tmp_weight = 0.5f;
		//}
		
			vpos.xyz += g_pAU_Weights[auidx]*g_pAnimationUnits[auidx*113+vidx].xyz;
			if(auidx > 1 ) {
				vpos.xyz += ( g_pAU_Weights[auidx]*0.3)*g_pAnimationUnits[auidx*113+vidx].xyz;
			}

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

