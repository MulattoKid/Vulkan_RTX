#ifndef UNIFORM_DATA_LAYOUTS
#define UNIFORM_DATA_LAYOUTS

//std140
struct Camera
{
	vec4 origin;
	vec4 topLeftCorner;
	vec4 horizontalEnd;
	vec4 verticalEnd;
};

//std430
struct UVsPerFace
{
	vec2 uvs[3];
};

#endif
