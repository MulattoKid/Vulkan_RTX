#ifndef DATA_LAYOUTS_GLSL
#define DATA_LAYOUTS_GLSL

//Only for unfirom/buffer structs and inter-shader structs

struct Camera
{
	vec4 origin;
	vec4 topLeftCorner;
	vec4 horizontalEnd;
	vec4 verticalEnd;
};

struct VertexAttributes
{
	vec4 normal;
	vec4 uv;
};

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
