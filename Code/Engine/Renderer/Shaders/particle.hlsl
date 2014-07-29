//--------------------------------------------------------------------------------------
// File: particle.hlsl
//--------------------------------------------------------------------------------------
// render as point sprite with geometry

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
#include "Constants.h"

struct a2v
{
	float3 Position : POSITION;
	float4 Direction_Intensity : TEXCOORD0;
	float4 mPivot_Size : TEXCOORD1;
	float4 mRot_Alpha_uv : TEXCOORD2;
	float2 mUVStep : TEXCOORD3;
};

struct v2g
{
	float3 ViewPosition : POSITION;
	float4 ViewDirection_Intensity : TEXCOORD0;
	float4 mPivot_Size : TEXCOORD1;
	float4 mRot_Alpha_uv : TEXCOORD2;
	float2 mUVStep : TEXCOORD3;
};

struct g2p
{
	float4 Position : SV_Position;
	float4 UV_Intensity_Alpha : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
v2g particle_VertexShader(in a2v INPUT)
{
	v2g OUTPUT;
	OUTPUT.ViewPosition = mul(gView, float4(INPUT.Position, 1));
	if (INPUT.Direction_Intensity.x != 1.0f)
	{
		//OUTPUT.ViewDirection_Intensity.xyz = mul((float3x3)gView, INPUT.Direction_Intensity.xyz);
		OUTPUT.ViewDirection_Intensity.xyz = INPUT.Direction_Intensity.xyz;
	}
	else
	{
		OUTPUT.ViewDirection_Intensity.xyz = INPUT.Direction_Intensity.xyz;
	}
	OUTPUT.ViewDirection_Intensity.w = INPUT.Direction_Intensity.w;
	OUTPUT.mPivot_Size = INPUT.mPivot_Size;
	OUTPUT.mRot_Alpha_uv = INPUT.mRot_Alpha_uv;
	OUTPUT.mUVStep = INPUT.mUVStep;

	return OUTPUT;
}

//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
cbuffer cbImmutable
{
	static float2 gUVs[4] = 
	{
		{0.0, 1.0},
		{0.0, 0.0},
		{1.0, 1.0},
		{1.0, 0.0}
	};
}

float2x2 GetMatRot(float rot)
{
	float c = cos(rot);
	float s = sin(rot);
	return float2x2(c, -s, s, c);
}
[maxvertexcount(4)]
void particle_GeometryShader(point v2g INPUT[1], inout TriangleStream<g2p> stream)
{
	g2p OUTPUT;
	for (int i=0; i<4; i++)
	{
		float2 pivot = INPUT[0].mPivot_Size.xy;
		float2 scale = INPUT[0].mPivot_Size.zw;
		float3 u = INPUT[0].ViewDirection_Intensity.xyz * scale.x;
		float3 v = {0.0, 0.0, -scale.y};
		float2x2 matRot = GetMatRot(INPUT[0].mRot_Alpha_uv.x);
		u.xz = mul(matRot, u.xz);
		v.xz = mul(matRot, v.xz);

		float3 viewPos = INPUT[0].ViewPosition + u * (gUVs[i].x-pivot.x) + v * (gUVs[i].y - pivot.y);

		OUTPUT.Position = mul(gProj, float4(viewPos, 1.0));

		float2 uvIndex = INPUT[0].mRot_Alpha_uv.zw;
		float2 uvStep = INPUT[0].mUVStep;
		OUTPUT.UV_Intensity_Alpha.xy = uvIndex * uvStep + gUVs[i] * uvStep;
		OUTPUT.UV_Intensity_Alpha.z = INPUT[0].ViewDirection_Intensity.w;
		OUTPUT.UV_Intensity_Alpha.w = INPUT[0].mRot_Alpha_uv.y;

		stream.Append(OUTPUT);
		if (i%4==3)
			stream.RestartStrip();
	}
}

//---------------------------------------------------------------------------
// PIXEL SHADER
//---------------------------------------------------------------------------
Texture2D gDiffuseTexture : register(t0);
SamplerState gDiffuseSampler : register(s0);
float4 particle_PixelShader(in g2p INPUT) : SV_Target
{
	float4 diffuse = gDiffuseTexture.Sample(gDiffuseSampler, INPUT.UV_Intensity_Alpha.xy);
	diffuse.xyz *=INPUT.UV_Intensity_Alpha.z;
	diffuse.w *= INPUT.UV_Intensity_Alpha.w;

	return diffuse;
}