#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "DataLayouts.glsl"

layout(location = 1) rayPayloadInNV ShadowRayPayload payload;

void main()
{
	payload.hitDist.x = 1.0f;//gl_HitTNV;
}
