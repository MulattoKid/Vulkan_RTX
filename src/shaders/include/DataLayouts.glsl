#ifndef DATA_LAYOUTS_GLSL
#define DATA_LAYOUTS_GLSL

//Only for unfirom/buffer structs and inter-shader structs

struct PrimaryRayPayload
{
	vec4 normalAndHitDistance;
	vec4 materialColor;
};

struct ShadowRayPayload
{
	vec4 hitDist;
};

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
	vec4 roughness;
};

struct VertexAttributes
{
	vec4 normal;
	vec4 uv;
};

#endif
