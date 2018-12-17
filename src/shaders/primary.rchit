#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "DataLayouts.glsl"
#include "Defines.glsl"

layout(set = 1, binding = CUSTOM_ID_TO_ATTRIBUTE_ARRAY_INDEX_BUFFER_BINDING_LOCATION, std430) readonly buffer customIDToAttributeArrayIndexBuffer
{
	uint customIDToAttributeArrayIndex[];
};
layout(set = 1, binding = PER_MESH_ATTRIBUTES_BINDING_LOCATION, std430) readonly buffer perMeshAttributesBuffer
{
	MeshAttributes meshAttributes[];
};
layout(set = 1, binding = PER_VERTEX_ATTRIBUTES_BINDING_LOCATION, std430) readonly buffer perVertexAttributesBuffer
{
	VertexAttributes vertexAttributes[];
};
layout(set = 1, binding = DIFFUSE_TEXTURES_BINDING_LOCATION) uniform sampler2D diffuseTextures[];
layout(set = 1, binding = SPECULAR_TEXTURES_BINDING_LOCATION) uniform sampler2D specularTextures[];
layout(set = 1, binding = EMISSIVE_TEXTURES_BINDING_LOCATION) uniform sampler2D emissiveTextures[];
layout(set = 1, binding = ROUGHNESS_TEXTURES_BINDING_LOCATION) uniform sampler2D roughnessTextures[];

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
	vec4 meshStaticColor = meshAttributes[meshID].diffuseColor.bgra;
	
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
