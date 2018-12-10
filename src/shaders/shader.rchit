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

layout(location = 0) rayPayloadInNV vec3 resultColor;
hitAttributeNV vec2 hitAttribs;

void main()
{
    //const vec3 barycentrics = vec3(1.0f - hitAttribs.x - hitAttribs.y, hitAttribs.x, hitAttribs.y);
    //resultColor = vec3(barycentrics);
    
    int meshID = gl_InstanceCustomIndexNV;
    uint attributeArrayIndex = customIDToAttributeArrayIndex[meshID];
	int faceID = gl_PrimitiveID;
    resultColor = vec3(uvsPerFace[attributeArrayIndex + faceID].uvs[0], 0.0f);
    //resultColor = vec3(0.0f, 1.0f, 0.0f);
    //resultColor = vec3(uvsPerFace[0].uvs[0], 0.0f);
	//resultColor = vec3(attributeArrayIndex, 0.0f, 0.0f);
    
    //resultColor = vec3(1.0f, 0.0f, 1.0f) + attributeArrayIndex;
}
