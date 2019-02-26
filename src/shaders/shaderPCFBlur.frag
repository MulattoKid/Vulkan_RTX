#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : require

#include "Defines.glsl"

layout(location=0) in vec2 fUV;

layout(set=0, binding=0) uniform sampler2D rayTracingImage;
layout(set=0, binding=1) uniform sampler2D occlusionImage;
layout(set=0, binding=2, std140) uniform blurVariableBuffer
{
	uint blurVariable;
};

layout(location=0) out vec4 outColor;

// https://developer.nvidia.com/gpugems/GPUGems/gpugems_ch11.html
void main()
{
	vec3 originalColor = texture(rayTracingImage, fUV).rgb;
	float occlusion = DEFAULT_OCCLUSION;
	
	if (blurVariable == 1)
	{
		// Current best
		/*float sum  = textureOffset(occlusionImage, fUV, ivec2( 0,  0)).r;
		if (sum == 0.0f)
		{
			outColor = vec4(originalColor, 1.0f);
			//outColor = vec4(1.0f);
			return;
		}
		sum       += textureOffset(occlusionImage, fUV, ivec2(-4, -4)).r;
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
		occlusion = sum / 25.0f;*/
	
		// Sample 3x3 first and check the average value against the center texel's value
		// If the delta is large, sample 5x5 and average
		/*float centerOcclusion  = textureOffset(occlusionImage, fUV, ivec2(0, 0)).r;
		float borderOcclusion  = textureOffset(occlusionImage, fUV, ivec2(-1, -1)).r;
		borderOcclusion       += textureOffset(occlusionImage, fUV, ivec2(-1,  0)).r;
		borderOcclusion       += textureOffset(occlusionImage, fUV, ivec2(-1,  1)).r;
		borderOcclusion       += textureOffset(occlusionImage, fUV, ivec2( 0, -1)).r;
		borderOcclusion       += textureOffset(occlusionImage, fUV, ivec2( 0,  1)).r;
		borderOcclusion       += textureOffset(occlusionImage, fUV, ivec2( 1, -1)).r;
		borderOcclusion       += textureOffset(occlusionImage, fUV, ivec2( 1,  0)).r;
		borderOcclusion       += textureOffset(occlusionImage, fUV, ivec2( 1,  1)).r;
		float avgBorderOcclusion = borderOcclusion / 9.0f;
		occlusion = centerOcclusion;
		if (abs(centerOcclusion - avgBorderOcclusion) > 0.05f)
		{
			occlusion  = centerOcclusion + borderOcclusion;
			occlusion += textureOffset(occlusionImage, fUV, ivec2(-2, -2)).r;
			occlusion += textureOffset(occlusionImage, fUV, ivec2(-2, -1)).r;
			occlusion += textureOffset(occlusionImage, fUV, ivec2(-2,  0)).r;
			occlusion += textureOffset(occlusionImage, fUV, ivec2(-2,  1)).r;
			occlusion += textureOffset(occlusionImage, fUV, ivec2(-2,  2)).r;
			
			occlusion += textureOffset(occlusionImage, fUV, ivec2(-1, -2)).r;
			occlusion += textureOffset(occlusionImage, fUV, ivec2(-1,  2)).r;
			
			occlusion += textureOffset(occlusionImage, fUV, ivec2( 0, -2)).r;
			occlusion += textureOffset(occlusionImage, fUV, ivec2( 0,  2)).r;
			
			occlusion += textureOffset(occlusionImage, fUV, ivec2( 1, -2)).r;
			occlusion += textureOffset(occlusionImage, fUV, ivec2( 1,  2)).r;
			
			occlusion += textureOffset(occlusionImage, fUV, ivec2( 2, -2)).r;
			occlusion += textureOffset(occlusionImage, fUV, ivec2( 2, -1)).r;
			occlusion += textureOffset(occlusionImage, fUV, ivec2( 2,  0)).r;
			occlusion += textureOffset(occlusionImage, fUV, ivec2( 2,  1)).r;
			occlusion += textureOffset(occlusionImage, fUV, ivec2( 2,  2)).r;
			
			occlusion /= 25.0f;
		}*/
		
		// Blur 5x5
		occlusion  = textureOffset(occlusionImage, fUV, ivec2(-2, -2)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-2, -1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-2,  0)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-2,  1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-2,  2)).r;
		
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-1, -2)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-1, -1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-1,  0)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-1,  1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-1,  2)).r;
		
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 0, -2)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 0, -1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 0,  0)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 0,  1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 0,  2)).r;
		
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 1, -2)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 1, -1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 1,  0)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 1,  1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 1,  2)).r;
		
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 2, -2)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 2, -1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 2,  0)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 2,  1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 2,  2)).r;
		
		occlusion /= 25.0f;
		
		// Blur skipping 7x7
		/*occlusion  = textureOffset(occlusionImage, fUV, ivec2(-3, -3)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-3, -1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-3,  1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-3,  3)).r;
		
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-2, -2)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-2,  0)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-2,  2)).r;
		
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-1, -3)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-1, -1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-1,  1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-1,  3)).r;
		
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 0, -2)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 0,  0)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 0,  2)).r;
		
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 1, -3)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 1, -1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 1,  1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 1,  3)).r;
		
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 2, -2)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 2,  0)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 2,  2)).r;
		
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 3, -3)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 3, -1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 3,  1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 3,  3)).r;
		
		occlusion /= 25.0f;*/
		
		// Blur skipping 9x9
		/*occlusion  = textureOffset(occlusionImage, fUV, ivec2(-4, -4)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-4, -2)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-4,  0)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-4,  2)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-4,  4)).r;
		
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-3, -3)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-3, -1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-3,  1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-3,  3)).r;
		
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-2, -4)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-2, -2)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-2,  0)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-2,  2)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-2,  4)).r;
		
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-1, -3)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-1, -1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-1,  1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2(-1,  3)).r;
		
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 0, -4)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 0, -2)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 0,  0)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 0,  2)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 0,  4)).r;
		
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 1, -3)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 1, -1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 1,  1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 1,  3)).r;
		
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 2, -4)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 2, -2)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 2,  0)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 2,  2)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 2,  4)).r;
		
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 3, -3)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 3, -1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 3,  1)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 3,  3)).r;
		
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 4, -4)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 4, -2)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 4,  0)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 4,  2)).r;
		occlusion += textureOffset(occlusionImage, fUV, ivec2( 4,  4)).r;
		
		occlusion /= 41.0f;*/
	}
	else
	{
		occlusion = texture(occlusionImage, fUV).r;
	}
	
	// Out
    vec3 visibility = vec3(1.0f - occlusion);
    outColor = vec4(originalColor * visibility, 1.0f);
    outColor = vec4(visibility, 1.0f);
}





























