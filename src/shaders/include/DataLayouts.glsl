/*
Copyright (c) 2018-2019 Daniel Fedai Larsen
*/

#ifndef DATA_LAYOUTS_GLSL
#define DATA_LAYOUTS_GLSL

/*
This file contains structures that are used as the input "format"
for uniform buffers. Note that std140 has been used.
*/

//////////////////////////////////
/////////Input structures/////////
//////////////////////////////////
struct CameraShader
{
	vec4 origin;
	vec4 topLeftCorner;
	vec4 horizontalEnd;
	vec4 verticalEnd;
	mat4 previousViewProjection;
};

struct SphericalLightSource
{
	vec4 centerAndRadius;
	vec4 emittance;
};

struct OtherData
{
	int numSphericalLightSources;
};

struct MeshAttributes
{
	vec4 diffuseColor;
};

struct VertexAttributes
{
	vec4 normal;
	vec4 uv;
};

//////////////////////////////////
//////Shader-local structures/////
//////////////////////////////////
struct PrimaryRayPayload
{
	vec4 normalAndHitDistance;
	vec4 materialColor;
};

struct ShadowRayPayload
{
	vec4 hitDist;
};

#endif
