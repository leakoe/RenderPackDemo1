//--------------------------------------------------------------------------------------
//Common unifrom variables
//--------------------------------------------------------------------------------------

//per object
cbuffer cbObject	 : register( b0 )
{
	matrix		g_mWorld;
	matrix		g_mWorldInv;
	matrix		g_mWorldIT;
	matrix		g_mWorldView;
	matrix		g_mWorldViewIT;
	matrix		g_mWorldViewProj;
};

//per camera + global settings
cbuffer cbCamera	 : register( b1 )
{
	//camera
	matrix		g_mView;
	matrix		g_mProj;
	matrix		g_mProjInv;
	matrix		g_mViewProj;
	matrix		g_mViewProjInv;
	float4		g_vEyePos;
	
	//global stuff
	float4		g_vScreenDimensions; //suggested usage: xy: width height, zw: pixel size
	float		g_fTime;
};

//per light
cbuffer cbLight		: register( b2 )
{
	matrix		g_mLightViewProj;
	float4		g_vLightPos;
	float4		g_vLightColor;
};

//per material
cbuffer cbMaterial	: register( b3 )
{
	float4		g_vDiffuse;
	
	float		g_fTransparency;
	float		g_fShininess;
	float		g_fShineStrength;
	float		g_fReflectivity;
	float		g_fIOR;
	//for Frensel approximation
	float		g_fR0;
	float		g_fR0Inv;
};


//

//---------------------------------------------------------------------------------------
// Commonly used methods
//---------------------------------------------------------------------------------------

float PhongBlinn(float3 N, float3 V, float3 L, float shininess)
{
	float3 H = normalize(L + V);
	return pow( saturate( dot(N, H) ), shininess);
}

bool ShadowMapCompare(float4 wpos, Texture2D txShadowMap, SamplerState samplerState)
{
	//sample the shadow map
	float4 lightScreenPos = mul(wpos, g_mLightViewProj);
	
	if(lightScreenPos.z < 0)
		return true;
	
	lightScreenPos /= lightScreenPos.w;
	
	float myDepth = lightScreenPos.z;
	float lightDepth = txShadowMap.Sample( samplerState, lightScreenPos.xy).x;	
	
	return myDepth < lightDepth || lightDepth == 0;
}
