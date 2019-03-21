/*
Copyright (c) 2018-2019 Daniel Fedai Larsen
*/

#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "DataLayouts.glsl"
#include "Defines.glsl"

layout(location = PRIMARY_PAYLOAD_LOCATION) rayPayloadInNV PrimaryRayPayload payload;

void main()
{
	payload.normalAndHitDistance = vec4(-1.0f);
    payload.materialColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
