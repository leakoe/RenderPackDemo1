/*
This pixel shader renders a full screen quad.
*/

struct PSTexQuadInput
{
	float4 pos:SV_POSITION;
	float2 tex:TEXCOORD0;
};

Texture2D g_txColor:register(t0);
Texture2D<int> g_txCounter:register(t1);

SamplerState g_Sampler :register(s0);

float4 PSMain(PSTexQuadInput Input): SV_TARGET
{
	return	g_txColor.Sample(g_Sampler, Input.tex);
}