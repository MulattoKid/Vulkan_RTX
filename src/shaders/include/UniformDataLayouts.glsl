#ifndef UNIFORM_DATA_LAYOUTS
#define UNIFORM_DATA_LAYOUTS

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

#endif
