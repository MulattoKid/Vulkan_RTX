#ifndef DATA_LAYOUTS_GLSL
#define DATA_LAYOUTS_GLSL

//////////////////////////////////
/////////Input structures/////////
//////////////////////////////////
struct CameraShader
{
	vec4 origin;
	vec4 topLeftCorner;
	vec4 horizontalEnd;
	vec4 verticalEnd;
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
	vec4 otherData;
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
	// .x = ior
	// .y = roughness
	vec4 otherData;
	ivec4 otherData2;
};

struct ShadowRayPayload
{
	vec4 hitDist;
};

#endif
