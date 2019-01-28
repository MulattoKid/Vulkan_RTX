#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : require

#include "Defines.glsl"

layout(location=0) in vec2 fUV;

layout(set=0, binding=0) uniform sampler2D rayTracingImage;
layout(set=0, binding=1) uniform sampler2D occlusionImage;

layout(location=0) out vec4 outColor;

#define TOP_LEFT 0.150342f
#define TOP_CENTER 0.094907f
#define TOP_RIGHT 0.023792f
#define CENTER_LEFT 0.094907f
#define CENTER_CENTER 0.059912f
#define CENTER_RIGHT 0.015019f
#define BOTTOM_LEFT 0.023792f
#define BOTTOM_CENTER 0.015019f
#define BOTTOM_RIGHT 0.003765f

// https://developer.nvidia.com/gpugems/GPUGems/gpugems_ch11.html
void main()
{
    vec3 originalColor = texture(rayTracingImage, fUV).bgr;
    
#if AO_CONE
	vec3 sum  = textureOffset(occlusionImage, fUV, ivec2(-1, -1)).rgb;
	sum      += textureOffset(occlusionImage, fUV, ivec2( 0, -1)).rgb;
	sum      += textureOffset(occlusionImage, fUV, ivec2( 1, -1)).rgb;
	sum      += textureOffset(occlusionImage, fUV, ivec2(-1,  0)).rgb;
	sum      += textureOffset(occlusionImage, fUV, ivec2( 0,  0)).rgb;
	sum      += textureOffset(occlusionImage, fUV, ivec2( 1,  0)).rgb;
	sum      += textureOffset(occlusionImage, fUV, ivec2(-1,  1)).rgb;
	sum      += textureOffset(occlusionImage, fUV, ivec2( 0,  1)).rgb;
	sum      += textureOffset(occlusionImage, fUV, ivec2( 1,  1)).rgb;
	vec3 occlusion = sum / 9.0f;
    vec3 visibility = vec3(1.0f) - occlusion;
#elif AO_HEMISPHERE
#ifdef GAUSS
	float sum  = textureOffset(occlusionImage, fUV, ivec2(-4, -4)).r * TOP_LEFT;
	sum       += textureOffset(occlusionImage, fUV, ivec2(-2, -4)).r * TOP_CENTER;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 0, -4)).r * TOP_RIGHT;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 2, -4)).r * TOP_CENTER;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 4, -4)).r * TOP_CENTER;
	
	sum       += textureOffset(occlusionImage, fUV, ivec2(-4, -2)).r * CENTER_LEFT;
	sum       += textureOffset(occlusionImage, fUV, ivec2(-2, -2)).r * CENTER_CENTER;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 0, -2)).r * CENTER_RIGHT;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 2, -2)).r * CENTER_CENTER;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 4, -2)).r * CENTER_LEFT;
	
	sum       += textureOffset(occlusionImage, fUV, ivec2(-4,  0)).r * BOTTOM_LEFT;
	sum       += textureOffset(occlusionImage, fUV, ivec2(-2,  0)).r * BOTTOM_CENTER;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 0,  0)).r * BOTTOM_RIGHT;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 2,  0)).r * BOTTOM_CENTER;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 4,  0)).r * BOTTOM_LEFT;
	
	sum       += textureOffset(occlusionImage, fUV, ivec2(-4,  2)).r * CENTER_LEFT;
	sum       += textureOffset(occlusionImage, fUV, ivec2(-2,  2)).r * CENTER_CENTER;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 0,  2)).r * CENTER_RIGHT;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 2,  2)).r * CENTER_CENTER;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 4,  2)).r * CENTER_LEFT;
	
	sum       += textureOffset(occlusionImage, fUV, ivec2(-4,  4)).r * TOP_LEFT;
	sum       += textureOffset(occlusionImage, fUV, ivec2(-2,  4)).r * TOP_CENTER;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 0,  4)).r * TOP_RIGHT;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 2,  4)).r * TOP_CENTER;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 4,  4)).r * TOP_LEFT;
	float occlusion = sum;
#else
	float sum  = textureOffset(occlusionImage, fUV, ivec2(-4, -4)).r;
	sum       += textureOffset(occlusionImage, fUV, ivec2(-2, -4)).r;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 0, -4)).r;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 2, -4)).r;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 4, -4)).r;
	
	sum       += textureOffset(occlusionImage, fUV, ivec2(-4, -2)).r;
	sum       += textureOffset(occlusionImage, fUV, ivec2(-2, -2)).r;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 0, -2)).r;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 2, -2)).r;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 4, -2)).r;
	
	sum       += textureOffset(occlusionImage, fUV, ivec2(-4,  0)).r;
	sum       += textureOffset(occlusionImage, fUV, ivec2(-2,  0)).r;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 0,  0)).r;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 2,  0)).r;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 4,  0)).r;
	
	sum       += textureOffset(occlusionImage, fUV, ivec2(-4,  2)).r;
	sum       += textureOffset(occlusionImage, fUV, ivec2(-2,  2)).r;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 0,  2)).r;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 2,  2)).r;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 4,  2)).r;
	
	sum       += textureOffset(occlusionImage, fUV, ivec2(-4,  4)).r;
	sum       += textureOffset(occlusionImage, fUV, ivec2(-2,  4)).r;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 0,  4)).r;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 2,  4)).r;
	sum       += textureOffset(occlusionImage, fUV, ivec2( 4,  4)).r;
	float occlusion = sum / 25.0f;
#endif
	
	if (sum == 0.0f)
	{
    	outColor = vec4(originalColor, 1.0f);
    	return;
	}
    vec3 visibility = vec3(1.0f - occlusion);
	
#endif
	
    outColor = vec4(originalColor * visibility, 1.0f);
}
