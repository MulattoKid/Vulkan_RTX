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
