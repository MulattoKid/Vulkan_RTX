#version 450
#extension GL_ARB_separate_shader_objects : enable

#define BLUR_3x3 1
#define BLUR_5x5 0
#define BLUR_7x7 0

#define SIGMA_07 1
#define SIGMA_1 0
#define SIGMA_4 0

// http://dev.theomader.com/gaussian-kernel-calculator/
#if BLUR_3x3
	#if SIGMA_07
		#define FIRST_WEIGHT 0.228814f
		#define CENTER_WEIGHT 0.542373f
	#elif SIGMA_1
		#define FIRST_WEIGHT 0.27901f
		#define CENTER_WEIGHT 0.44198f
	#elif SIGMA_4
		#define FIRST_WEIGHT 0.329861f
		#define CENTER_WEIGHT 0.340277f
	#endif
#elif BLUR_5x5
	
#elif BLUR_7x7
	
#endif

layout(location=0) in vec2 fUV;

layout(set=0, binding=0) uniform sampler2D rayTracingImage;
layout(set=0, binding=1) uniform sampler2D shadowImage;
layout(set=0, binding=2, std430) readonly buffer pixelDeltaBuffer
{
	vec2 pixelDelta;
};

layout(location=0) out vec4 outColor;

void main()
{
	vec2 fragCoord = gl_FragCoord.xy;
    //outColor = vec4(fUV, 0.0f, 1.0f);
    //outColor = texture(rayTracingImage, fUV).bgra;
    outColor = vec4(texture(shadowImage, fUV).r, 0.0f, 0.0f, 1.0f);
    
#if BLUR_3x3
	vec2 horizontalOffset = vec2(1.0f, 0.0f);
	vec2 horizontalUVs[3] = {
		(fragCoord - horizontalOffset) * pixelDelta,
		fragCoord * pixelDelta,
		(fragCoord + horizontalOffset) * pixelDelta
	};
	float left = texture(shadowImage, horizontalUVs[0]).r * FIRST_WEIGHT;
	float center = texture(shadowImage, horizontalUVs[1]).r * CENTER_WEIGHT;
	float right = texture(shadowImage, horizontalUVs[2]).r * FIRST_WEIGHT;
	outColor = vec4(left + center + right, 0.0f, 0.0f, 1.0f);
#elif BLUR_5x5
	
#elif BLUR_7x7
	
#endif
	
	outColor = vec4(1.0f, 0.0f, 1.0f, 0.0f);
}
