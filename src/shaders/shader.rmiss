#version 460
#extension GL_NV_ray_tracing : require

layout(location = 0) rayPayloadInNV vec3 resultColor;

void main()
{
    resultColor = vec3(0.0f, 0.0f, 0.0f);
}
