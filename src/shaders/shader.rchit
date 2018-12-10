#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "UniformDataLayouts.glsl"

layout(set = 1, binding = 0, std430) readonly buffer customIDToAttributeArrayIndexBuffer
{
	uint customIDToAttributeArrayIndex[];
};

layout(set = 1, binding = 1, std430) readonly buffer uvBuffer
{
	UVsPerFace uvsPerFace[];
};
layout(set = 1, binding = 2) uniform sampler2D image;

layout(location = 0) rayPayloadInNV vec3 resultColor;
hitAttributeNV vec2 hitAttribs;

vec2 PointUVFromBarycentric(vec2 faceUVs[3], vec3 barycentric)
{
	return (barycentric.x * faceUVs[0]) + (barycentric.y * faceUVs[1]) + (barycentric.z * faceUVs[2]);
}

void main()
{
	//Get 
    const vec3 barycentric = vec3(1.0f - hitAttribs.x - hitAttribs.y, hitAttribs.x, hitAttribs.y);
    
    //Get face attributes
    const int meshID = gl_InstanceCustomIndexNV;
    const uint attributeArrayIndex = customIDToAttributeArrayIndex[meshID];
	const int faceID = gl_PrimitiveID;
	const vec2 faceUVs[3] = uvsPerFace[attributeArrayIndex + faceID].uvs;
    
    //Calculate attributes for point of intersection
	const vec2 uv = PointUVFromBarycentric(faceUVs, barycentric);
	
	//Output correct color
	resultColor = texture(image, uv).bgr;
}
