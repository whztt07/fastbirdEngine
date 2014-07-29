//--------------------------------------------------------------------------------------
// File: locator.hlsl
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "Constants.h"

//
//Texture2D  gDiffuseTexture : register(t0);
//
//SamplerState gDiffuseSampler : register(s0);

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct a2v
{
	float4 Position : POSITION;
};

struct v2p 
{
	float4 Position   : SV_Position;
};

v2p locator_VertexShader( in a2v INPUT )
{
    v2p OUTPUT;

	OUTPUT.Position = mul( gWorldViewProj, INPUT.Position );

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 locator_PixelShader( in v2p INPUT ) : SV_Target
{		
	// Specular Light
    return gAmbientColor;
}