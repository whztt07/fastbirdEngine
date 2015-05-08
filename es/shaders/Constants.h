#include "ShaderCommon.h"
#ifndef __SHADER_CONSTANTS_H_
#define __SHADER_CONSTANTS_H_
#ifdef CPP
#pragma once
namespace fastbird {
#endif

cbuffer FRAME_CONSTANTS
	#ifndef CPP
		: register(b0)
	#endif
{
	float4x4 gView;
	float4x4 gInvView;
	float4x4 gViewProj;
	float4x4 gInvViewProj;
	float4x4 gLightViewProj;
	float4x4 gCamTransform;
	float4 gDirectionalLightDir_Intensity[2];
	float4 gDirectionalLightDiffuse[2];
	float4 gDirectionalLightSpecular[2];
	float4 gMousePos; // x, y : current pos   z, w: down pos
	float4 gDeltaTime;
	float gTime;
	float3 gFrameConstDummy;
};

cbuffer OBJECT_CONSTANTS
	#ifndef CPP
		: register (b1)
	#endif
{
	float4x4 gWorld;
	float4x4 gWorldView;
	float4x4 gWorldViewProj;
};

cbuffer MATERIAL_CONSTANTS
	#ifndef CPP
		: register (b2)
	#endif
{
	float4 gAmbientColor;
	float4 gDiffuseColor;
	float4 gSpecularColor;
	float4 gEmissiveColor;
};

cbuffer MATERIAL_PARAMETERS
#ifndef CPP
	: register (b3)
#endif
{
	float4 gMaterialParam[5];
};

cbuffer RARE_CONSTANTS
#ifndef CPP
	: register (b4)
#endif
{
	float4x4 gProj;
	float4x4 gInvProj;
	float2 gNearFar;
	float2 gScreenSize;
	float4 gFogColor;
	float gTangentTheta;
	float gScreenRatio;
	float gMiddleGray;
	float gStarPower;	
	float gBloomPower;
	float3 gEmpty;
};

cbuffer BIG_BUFFER
#ifndef CPP
	: register (b5)
#endif
{
	float4 gSampleOffsets[16];
	float4 gSampleWeights[16];
};

cbuffer IMMUTABLE_CONSTANTS
#ifndef CPP
	: register (b6)
#endif
{
	float4 gIrradConstsnts[9];
	// will be casted float2[16]
	float4 gHammersley[8]; // reference http://www.cse.cuhk.edu.hk/~ttwong/papers/udpoint/udpoint.pdf
};

cbuffer POINT_LIGHT_CONSTANTS
#ifndef CPP
	: register (b7)
#endif
{
	float4 gPointLightPos[MAX_POINT_LIGHT];
	float4 gPointLightColor[MAX_POINT_LIGHT];
	
#ifdef CPP	
	POINT_LIGHT_CONSTANTS()
	{
		memset(this, 0, sizeof(POINT_LIGHT_CONSTANTS));
	}
#endif
	};

#ifdef CPP
} // namespace
#endif

#endif //__SHADER_CONSTANTS_H_