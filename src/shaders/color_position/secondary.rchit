#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "DataLayouts.glsl"
#include "Defines.glsl"

layout(location = SECONDARY_PAYLOAD_LOCATION) rayPayloadInNV ShadowRayPayload payload;

void main()
{
	payload.hitDist.x = gl_HitTNV;
}
