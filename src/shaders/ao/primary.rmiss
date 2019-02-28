#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "DataLayouts.glsl"
#include "Defines.glsl"

layout(location = PRIMARY_PAYLOAD_LOCATION) rayPayloadInNV ShadowRayPayload payload;

void main()
{
	payload.hitDist.x = -1.0f;
}
