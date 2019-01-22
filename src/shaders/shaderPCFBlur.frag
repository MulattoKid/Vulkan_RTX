#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : require

#include "Defines.glsl"

layout(location=0) in vec2 fUV;

layout(set=0, binding=0) uniform sampler2D rayTracingImage;
layout(set=0, binding=1) uniform sampler2D occlusionImage;

layout(location=0) out vec4 outColor;

void main()
{
	// https://developer.nvidia.com/gpugems/GPUGems/gpugems_ch11.html
	float sum = textureOffset(occlusionImage, fUV, ivec2(-1, -1)).r;
	sum      += textureOffset(occlusionImage, fUV, ivec2( 0, -1)).r;
	sum      += textureOffset(occlusionImage, fUV, ivec2( 1, -1)).r;
	sum      += textureOffset(occlusionImage, fUV, ivec2(-1,  0)).r;
	sum      += textureOffset(occlusionImage, fUV, ivec2( 0,  0)).r;
	sum      += textureOffset(occlusionImage, fUV, ivec2( 1,  0)).r;
	sum      += textureOffset(occlusionImage, fUV, ivec2(-1,  1)).r;
	sum      += textureOffset(occlusionImage, fUV, ivec2( 0,  1)).r;
	sum      += textureOffset(occlusionImage, fUV, ivec2( 1,  1)).r;
	float occlusion = sum / 9.0f;
    vec3 visibility = vec3(1.0f) - occlusion;
    vec3 originalColor = texture(rayTracingImage, fUV).bgr;
    outColor = vec4(originalColor * visibility, 1.0f);
}
