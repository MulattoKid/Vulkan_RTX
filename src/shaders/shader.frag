#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location=0) in vec2 fUV;

layout(set=0, binding=0) uniform sampler2D rayTracingImage;
layout(set=0, binding=1) uniform sampler2D shadowImage;

layout(location=0) out vec4 outColor;

void main()
{
    //outColor = vec4(fUV, 0.0f, 1.0f);
    //outColor = texture(rayTracingImage, fUV).bgra;
    outColor = vec4(texture(shadowImage, fUV).r, 0.0f, 0.0f, 1.0f);
}
