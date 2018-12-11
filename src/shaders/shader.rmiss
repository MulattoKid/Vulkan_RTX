#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "DataLayouts.glsl"

layout(location = 0) rayPayloadInNV PrimaryRayPayload payload;

void main()
{
    payload.materialColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
