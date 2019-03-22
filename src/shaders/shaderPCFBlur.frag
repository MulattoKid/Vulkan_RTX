/*
Copyright (c) 2018-2019 Daniel Fedai Larsen
*/

#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : require

/*
Overall description:
	This shader blurs the contents in the aoImage given that the value for the current texel
	isn't 0.0.

AO (default=OFF):
	If defined as 1, color information will not be taken into account, and a greyscale image
	will be rendered. This is usefull for inspecting the pure ambient occlusion result.
	
AO_COLOR (default=ON):
	If defined a 1, color information will be taken into account, and a regular RGB image is
	rendered.
	
blurVariable (default=OFF):
	Blurring can be turned ON/OFF with the keyboard key 'b'.
*/

#include "Defines.glsl"

#define AO 1
#define AO_COLOR 0

layout(location=0) in vec2 fUV;

layout(set = 0, binding = RS0_RAY_TRACING_IMAGE_BINDING_LOCATION) uniform sampler2D rayTracingImage;
layout(set = 0, binding = RS0_AO_IMAGE_BINDING_LOCATION) uniform sampler2D aoImage;
layout(set = 0, binding = RS0_BLUR_VARIABLE_BINDING_LOCATION, std140) uniform blurVariableBuffer
{
	uint blurVariable;
};

layout(location=0) out vec4 outColor;

// https://developer.nvidia.com/gpugems/GPUGems/gpugems_ch11.html
void main()
{
	vec3 originalColor = texture(rayTracingImage, fUV).rgb;
	float occlusion = 0.0f;
	
	if (blurVariable == 1)
	{
		occlusion = textureOffset(aoImage, fUV, ivec2(0, 0)).r;
		if (occlusion > 0.0f)
		{
			// Blur 3x3
			occlusion += textureOffset(aoImage, fUV, ivec2(-1, -1)).r;
			occlusion += textureOffset(aoImage, fUV, ivec2( 0, -1)).r;
			occlusion += textureOffset(aoImage, fUV, ivec2( 1, -1)).r;
			
			occlusion += textureOffset(aoImage, fUV, ivec2(-1,  0)).r;
			occlusion += textureOffset(aoImage, fUV, ivec2( 1,  0)).r;
			
			occlusion += textureOffset(aoImage, fUV, ivec2(-1,  1)).r;
			occlusion += textureOffset(aoImage, fUV, ivec2( 0,  1)).r;
			occlusion += textureOffset(aoImage, fUV, ivec2( 1,  1)).r;
			
			occlusion /= 9.0f;
		}
	}
	else
	{
		occlusion = texture(aoImage, fUV).r;
	}
	
	// Out
    vec3 visibility = vec3(1.0f - occlusion);
#if AO
    outColor = vec4(visibility, 1.0f);
#elif AO_COLOR
    outColor = vec4(originalColor * visibility, 1.0f);
#endif
}





























