#version 460
#extension GL_NV_ray_tracing : require

layout(location = 0) rayPayloadInNV vec3 ResultColor;
hitAttributeNV vec2 HitAttribs;
//layout(location = 1) hitAttributeNV vec2 HitAttribs;

void main()
{
    const vec3 barycentrics = vec3(1.0f - HitAttribs.x - HitAttribs.y, HitAttribs.x, HitAttribs.y);
    ResultColor = vec3(barycentrics);
}
