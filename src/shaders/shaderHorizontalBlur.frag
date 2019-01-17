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
layout(set=0, binding=1) uniform sampler2D shadowImage;
layout(set=0, binding=2, std430) readonly buffer pixelDeltaBuffer
{
	vec2 pixelDelta;
};

layout(location=0) out vec4 outColor;

void main()
{
	vec2 fragCoord = gl_FragCoord.xy;
	float shadow;
    
#if BLUR_3x3
	vec2 horizontalOffsetSmall = vec2(1.0f, 0.0f);
	vec2 horizontalUVs[3] = {
		(fragCoord - horizontalOffsetSmall) * pixelDelta,
		 fragCoord * pixelDelta,
		(fragCoord + horizontalOffsetSmall) * pixelDelta
	};
	shadow  = texture(shadowImage, horizontalUVs[0]).r * FIRST_WEIGHT;
	shadow += texture(shadowImage, horizontalUVs[1]).r * CENTER_WEIGHT;
	shadow += texture(shadowImage, horizontalUVs[2]).r * FIRST_WEIGHT;
#elif BLUR_5x5
	vec2 horizontalOffsetSmall  = vec2(1.0f, 0.0f);
	vec2 horizontalOffsetMedium = vec2(2.0f, 0.0f);
	vec2 horizontalUVs[5] = {
		(fragCoord - horizontalOffsetMedium) * pixelDelta,
		(fragCoord - horizontalOffsetSmall)  * pixelDelta,
		 fragCoord * pixelDelta,
		(fragCoord + horizontalOffsetSmall)  * pixelDelta,
		(fragCoord + horizontalOffsetMedium) * pixelDelta
	};
	shadow  = texture(shadowImage, horizontalUVs[0]).r * FIRST_WEIGHT;
	shadow += texture(shadowImage, horizontalUVs[1]).r * SECOND_WEIGHT;
	shadow += texture(shadowImage, horizontalUVs[2]).r * CENTER_WEIGHT;
	shadow += texture(shadowImage, horizontalUVs[3]).r * SECOND_WEIGHT;
	shadow += texture(shadowImage, horizontalUVs[4]).r * FIRST_WEIGHT;
#elif BLUR_7x7
	vec2 horizontalOffsetSmall  = vec2(1.0f, 0.0f);
	vec2 horizontalOffsetMedium = vec2(2.0f, 0.0f);
	vec2 horizontalOffsetLarge  = vec2(3.0f, 0.0f);
	vec2 horizontalUVs[7] = {
		(fragCoord - horizontalOffsetLarge)  * pixelDelta,
		(fragCoord - horizontalOffsetMedium) * pixelDelta,
		(fragCoord - horizontalOffsetSmall)  * pixelDelta,
		 fragCoord * pixelDelta,
		(fragCoord + horizontalOffsetSmall)  * pixelDelta,
		(fragCoord + horizontalOffsetMedium) * pixelDelta,
		(fragCoord + horizontalOffsetLarge)  * pixelDelta
	};
	shadow  = texture(shadowImage, horizontalUVs[0]).r * FIRST_WEIGHT;
	shadow += texture(shadowImage, horizontalUVs[1]).r * SECOND_WEIGHT;
	shadow += texture(shadowImage, horizontalUVs[2]).r * THIRD_WEIGHT;
	shadow += texture(shadowImage, horizontalUVs[3]).r * CENTER_WEIGHT;
	shadow += texture(shadowImage, horizontalUVs[4]).r * THIRD_WEIGHT;
	shadow += texture(shadowImage, horizontalUVs[5]).r * SECOND_WEIGHT;
	shadow += texture(shadowImage, horizontalUVs[6]).r * FIRST_WEIGHT;
#endif

	outColor = vec4(shadow, 0.0f, 0.0f, 0.0f);
}
