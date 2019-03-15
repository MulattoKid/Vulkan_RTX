#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : require

#include "Defines.glsl"

#define AO 0
#define AO_COLOR 1

layout(location=0) in vec2 fUV;

layout(set = 0, binding = 0) uniform sampler2D rayTracingImage;
layout(set = 0, binding = 1) uniform sampler2D aoImage;
layout(set = 0, binding = 2, std140) uniform blurVariableBuffer
{
	uint blurVariable;
};

layout(location=0) out vec4 outColor;

// https://developer.nvidia.com/gpugems/GPUGems/gpugems_ch11.html
void main()
{
	//outColor = vec4(texture(aoImage, fUV).rgb, 1.0f);
	//return;

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
    //outColor = vec4(originalColor, 1.0f);
#endif
}





























