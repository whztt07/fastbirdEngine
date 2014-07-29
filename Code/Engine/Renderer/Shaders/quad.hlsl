//----------------------------------------------------------------------------
// File: Quad.hlsl
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Constant Buffer
//----------------------------------------------------------------------------
#include "Constants.h"

//----------------------------------------------------------------------------
// Vertex Shader
//----------------------------------------------------------------------------
struct a2v
{
	float4 pos		: POSITION;
	float4 color	: COLOR;
};

//----------------------------------------------------------------------------
struct v2p
{
	float4 pos		: SV_Position;
	float4 color	: COLOR;
};

//----------------------------------------------------------------------------
v2p quad_VertexShader( in a2v IN )
{
    v2p OUT;

	OUT.pos = mul(gWorld, IN.pos);
	OUT.color = IN.color;

	return OUT;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 quad_PixelShader( in v2p IN ) : SV_Target
{
	return IN.color;
}