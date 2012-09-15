/*
Vertex Shader for rendering a full-screen quad
*/

struct VSQuadInput
{
	float2 pos:Position;
};

struct PSTexQuadInput
{
	float4 pos:SV_POSITION;
	float2 tex:TEXCOORD0;
};

PSTexQuadInput VSMain(VSQuadInput Input)
{
	PSTexQuadInput Output;

	Output.pos.x = Input.pos.x;
	Output.pos.y = Input.pos.y;
	Output.pos.z = 0;
	Output.pos.w = 1;

	Output.tex = 0.5*Input.pos.xy + float2(0.5,0.5);
	Output.tex.y = 1.0-Output.tex.y;

	return Output;
}