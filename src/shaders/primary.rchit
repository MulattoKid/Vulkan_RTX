#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "DataLayouts.glsl"
#include "Defines.glsl"

layout(set = 1, binding = 0, std430) readonly buffer customIDToAttributeArrayIndexBuffer
{
	uint customIDToAttributeArrayIndex[];
};
layout(set = 1, binding = 1, std430) readonly buffer perVertexAttributeBuffer
{
	VertexAttributes vertexAttributes[];
};
layout(set = 1, binding = 2, std430) readonly buffer perMeshColor
{
	vec4 defaultMeshColor[];
};
layout(set = 1, binding = 3) uniform sampler2D diffuseTextures[];
layout(set = 1, binding = 4) uniform sampler2D specularTextures[];
layout(set = 1, binding = 5) uniform sampler2D emissiveTextures[];
layout(set = 1, binding = 6) uniform sampler2D roughnessTextures[];

layout(location = PRIMARY_PAYLOAD_LOCATION) rayPayloadInNV PrimaryRayPayload payload;
hitAttributeNV vec2 hitAttribs;

vec3 NormalAtPoint(vec3 faceNormal0, vec3 faceNormal1, vec3 faceNormal2, vec3 barycentric)
{
	return (barycentric.x * faceNormal0) + (barycentric.y * faceNormal1) + (barycentric.z * faceNormal2);
}

vec2 UVAtPoint(vec2 uv0, vec2 uv1, vec2 uv2, vec3 barycentric)
{
	return (barycentric.x * uv0) + (barycentric.y * uv1) + (barycentric.z * uv2);
}

void main()
{
    
    //Get IDs to recover needed attributes
    const int meshID = gl_InstanceCustomIndexNV;
    const uint attributeArrayIndex = customIDToAttributeArrayIndex[meshID];
	const int faceID = gl_PrimitiveID;
	
	//Geometric attributes
	const VertexAttributes v0Attr = vertexAttributes[attributeArrayIndex + (faceID * 3) + 0];
	const VertexAttributes v1Attr = vertexAttributes[attributeArrayIndex + (faceID * 3) + 1];
	const VertexAttributes v2Attr = vertexAttributes[attributeArrayIndex + (faceID * 3) + 2];
	
	//Material attributes
	vec4 meshStaticColor = defaultMeshColor[meshID].bgra;
	
	//Calculate attributes at point of intersection
    const vec3 barycentric = vec3(1.0f - hitAttribs.x - hitAttribs.y, hitAttribs.x, hitAttribs.y);
	const vec3 normal = normalize(NormalAtPoint(v0Attr.normal.xyz, v1Attr.normal.xyz, v2Attr.normal.xyz, barycentric));
	const vec2 uv = UVAtPoint(v0Attr.uv.xy, v1Attr.uv.xy, v2Attr.uv.xy, barycentric);
		
	//Set payload information
	payload.normalAndHitDistance = vec4(normal, gl_HitTNV);
	if (uv.x == 0.0f && uv.y == 0.0f)
	{
		payload.materialColor = meshStaticColor;
	}
	else
	{
		payload.materialColor = texture(diffuseTextures[nonuniformEXT(meshID)], uv).bgra;
	}
}
