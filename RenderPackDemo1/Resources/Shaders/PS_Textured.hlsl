//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

Texture2D 	texDiffuse		: register(t0);

SamplerState g_samLinear 	: register( s0 );

struct VS_OUTPUT
{
	float4 pos	: SV_POSITION;
	float3 norm	: NORMAL;
	float2 tex	: TEXCOORD;
};


float4 PSMain(VS_OUTPUT Input) : SV_TARGET
{	
	
	//return float4(Input.tex.x, Input.tex.y, 0.5f, 1.0f);
	
	float4 color = texDiffuse.Sample(g_samLinear, Input.tex);
	
	// simulate some kind of diffuse lighting
	float cosTheta = saturate(Input.norm.y); // the light is coming directly from above
	
	return color * (0.7f * cosTheta + 0.05f);
}


	