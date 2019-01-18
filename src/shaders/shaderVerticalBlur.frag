#version 450
#extension GL_ARB_separate_shader_objects : enable

#define BLUR_3x3 0
#define BLUR_5x5 0
#define BLUR_7x7 1

#define SIGMA_07 0
#define SIGMA_1 0
#define SIGMA_4 1

// http://dev.theomader.com/gaussian-kernel-calculator/
#if BLUR_3x3
	#if SIGMA_07
		#define FIRST_WEIGHT  0.228814f
		#define CENTER_WEIGHT 0.542373f
	#elif SIGMA_1
		#define FIRST_WEIGHT  0.27901f
		#define CENTER_WEIGHT 0.44198f
	#elif SIGMA_4
		#define FIRST_WEIGHT  0.329861f
		#define CENTER_WEIGHT 0.340277f
	#endif
#elif BLUR_5x5
	#if SIGMA_07
		#define FIRST_WEIGHT  0.015890f
		#define SECOND_WEIGHT 0.221542f
		#define CENTER_WEIGHT 0.525136f
	#elif SIGMA_1
		#define FIRST_WEIGHT  0.061360f
		#define SECOND_WEIGHT 0.244770f
		#define CENTER_WEIGHT 0.387740f
	#elif SIGMA_4
		#define FIRST_WEIGHT  0.187691f
		#define SECOND_WEIGHT 0.206038f
		#define CENTER_WEIGHT 0.212543f
	#endif
#elif BLUR_7x7
	#if SIGMA_07
		#define FIRST_WEIGHT  0.000177f
		#define SECOND_WEIGHT 0.015885f
		#define THIRD_WEIGHT  0.221463f
		#define CENTER_WEIGHT 0.524950f
	#elif SIGMA_1
		#define FIRST_WEIGHT  0.005980f
		#define SECOND_WEIGHT 0.060626f
		#define THIRD_WEIGHT  0.241843f
		#define CENTER_WEIGHT 0.383103f
	#elif SIGMA_4
		#define FIRST_WEIGHT  0.121597f
		#define SECOND_WEIGHT 0.142046f
		#define THIRD_WEIGHT  0.155931f
		#define CENTER_WEIGHT 0.160854f
	#endif
#endif

layout(location=0) in vec2 fUV;

layout(set=0, binding=0) uniform sampler2D rayTracingImage;
layout(set=0, binding=1) uniform sampler2D horizontalBlurImage;
layout(set=0, binding=2, std430) readonly buffer pixelDeltaBuffer
{
	vec2 pixelDelta;
};

layout(location=0) out vec4 outColor;

void main()
{
	vec2 fragCoord = gl_FragCoord.xy;
	vec3 shadow;
    
#if BLUR_3x3
	vec2 verticalOffset = vec2(0.0f, 1.0f);
	vec2 verticalUVs[3] = {
		(fragCoord - verticalOffset) * pixelDelta,
		fragCoord * pixelDelta,
		(fragCoord + verticalOffset) * pixelDelta
	};
	shadow  = texture(horizontalBlurImage, verticalUVs[0]).rgb * FIRST_WEIGHT;
	shadow += texture(horizontalBlurImage, verticalUVs[1]).rgb * CENTER_WEIGHT;
	shadow += texture(horizontalBlurImage, verticalUVs[2]).rgb * FIRST_WEIGHT;
#elif BLUR_5x5
	vec2 verticalOffsetSmall  = vec2(1.0f, 0.0f);
	vec2 verticalOffsetMedium = vec2(2.0f, 0.0f);
	vec2 verticalUVs[5] = {
		(fragCoord - verticalOffsetMedium) * pixelDelta,
		(fragCoord - verticalOffsetSmall)  * pixelDelta,
		 fragCoord * pixelDelta,
		(fragCoord + verticalOffsetSmall)  * pixelDelta,
		(fragCoord + verticalOffsetMedium) * pixelDelta
	};
	shadow  = texture(horizontalBlurImage, verticalUVs[0]).rgb * FIRST_WEIGHT;
	shadow += texture(horizontalBlurImage, verticalUVs[1]).rgb * SECOND_WEIGHT;
	shadow += texture(horizontalBlurImage, verticalUVs[2]).rgb * CENTER_WEIGHT;
	shadow += texture(horizontalBlurImage, verticalUVs[3]).rgb * SECOND_WEIGHT;
	shadow += texture(horizontalBlurImage, verticalUVs[4]).rgb * FIRST_WEIGHT;
#elif BLUR_7x7
	vec2 verticalOffsetSmall  = vec2(1.0f, 0.0f);
	vec2 verticalOffsetMedium = vec2(2.0f, 0.0f);
	vec2 verticalOffsetLarge  = vec2(3.0f, 0.0f);
	vec2 verticalUVs[7] = {
		(fragCoord - verticalOffsetLarge)  * pixelDelta,
		(fragCoord - verticalOffsetMedium) * pixelDelta,
		(fragCoord - verticalOffsetSmall)  * pixelDelta,
		 fragCoord * pixelDelta,
		(fragCoord + verticalOffsetSmall)  * pixelDelta,
		(fragCoord + verticalOffsetMedium) * pixelDelta,
		(fragCoord + verticalOffsetLarge)  * pixelDelta
	};
	shadow  = texture(horizontalBlurImage, verticalUVs[0]).rgb * FIRST_WEIGHT;
	shadow += texture(horizontalBlurImage, verticalUVs[1]).rgb * SECOND_WEIGHT;
	shadow += texture(horizontalBlurImage, verticalUVs[2]).rgb * THIRD_WEIGHT;
	shadow += texture(horizontalBlurImage, verticalUVs[3]).rgb * CENTER_WEIGHT;
	shadow += texture(horizontalBlurImage, verticalUVs[4]).rgb * THIRD_WEIGHT;
	shadow += texture(horizontalBlurImage, verticalUVs[5]).rgb * SECOND_WEIGHT;
	shadow += texture(horizontalBlurImage, verticalUVs[6]).rgb * FIRST_WEIGHT;
#endif

	// Remember to output BGRA8 as that's the format of the swapchain
	outColor = texture(rayTracingImage, fUV).bgra;
    //outColor = vec4(shadow, 1.0f);
    vec3 originalColor = texture(rayTracingImage, fUV).bgr;
    vec3 differenceColor = originalColor - shadow;
    vec3 changeColor = vec3(1.0f) - differenceColor;
    //outColor = vec4(originalColor * changeColor, 1.0f);
}
