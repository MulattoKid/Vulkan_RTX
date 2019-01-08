#ifndef DATA_LAYOUTS_GLSL
#define DATA_LAYOUTS_GLSL

// Input structures

struct Camera
{
	vec4 origin;
	vec4 topLeftCorner;
	vec4 horizontalEnd;
	vec4 verticalEnd;
};

struct MeshAttributes
{
	vec4 diffuseColor;
	vec4 specularColor;
	vec4 emissiveColor;
	// .x = material type
	// .y = ior
	// .z = roughness
	vec4 otherData;
};

struct VertexAttributes
{
	vec4 normal;
	vec4 uv;
};


// Shader-local structures

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
