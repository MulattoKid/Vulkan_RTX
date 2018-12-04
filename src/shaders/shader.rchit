#version 460
#extension GL_NV_ray_tracing : require

layout(location = 0) rayPayloadInNV vec3 resultColor;
hitAttributeNV vec2 hitAttribs;

void main()
{
    //const vec3 barycentrics = vec3(1.0f - hitAttribs.x - hitAttribs.y, hitAttribs.x, hitAttribs.y);
    //resultColor = vec3(barycentrics);
    resultColor = vec3(0.0f, 0.0f, 1.0f);
}
