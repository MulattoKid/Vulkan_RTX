#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : require

#include "Defines.glsl"

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
	vec3 occlusion;
    
#if BLUR_3x3
	vec2 verticalOffset = vec2(0.0f, 1.0f);
	vec2 verticalUVs[3] = {
		(fragCoord - verticalOffset) * pixelDelta,
		fragCoord * pixelDelta,
		(fragCoord + verticalOffset) * pixelDelta
	};
	occlusion  = texture(horizontalBlurImage, verticalUVs[0]).rgb * FIRST_WEIGHT;
	occlusion += texture(horizontalBlurImage, verticalUVs[1]).rgb * CENTER_WEIGHT;
	occlusion += texture(horizontalBlurImage, verticalUVs[2]).rgb * FIRST_WEIGHT;
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
	occlusion  = texture(horizontalBlurImage, verticalUVs[0]).rgb * FIRST_WEIGHT;
	occlusion += texture(horizontalBlurImage, verticalUVs[1]).rgb * SECOND_WEIGHT;
	occlusion += texture(horizontalBlurImage, verticalUVs[2]).rgb * CENTER_WEIGHT;
	occlusion += texture(horizontalBlurImage, verticalUVs[3]).rgb * SECOND_WEIGHT;
	occlusion += texture(horizontalBlurImage, verticalUVs[4]).rgb * FIRST_WEIGHT;
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
	occlusion  = texture(horizontalBlurImage, verticalUVs[0]).rgb * FIRST_WEIGHT;
	occlusion += texture(horizontalBlurImage, verticalUVs[1]).rgb * SECOND_WEIGHT;
	occlusion += texture(horizontalBlurImage, verticalUVs[2]).rgb * THIRD_WEIGHT;
	occlusion += texture(horizontalBlurImage, verticalUVs[3]).rgb * CENTER_WEIGHT;
	occlusion += texture(horizontalBlurImage, verticalUVs[4]).rgb * THIRD_WEIGHT;
	occlusion += texture(horizontalBlurImage, verticalUVs[5]).rgb * SECOND_WEIGHT;
	occlusion += texture(horizontalBlurImage, verticalUVs[6]).rgb * FIRST_WEIGHT;
#endif

	// Remember to output BGRA8 as that's the format of the swapchain

#if AO_CONE
    vec3 originalColor = texture(rayTracingImage, fUV).bgr;
    vec3 differenceColor = originalColor - occlusion;
    vec3 changeColor = vec3(1.0f) - differenceColor;
    outColor = vec4(originalColor * changeColor, 1.0f);
	//outColor = texture(rayTracingImage, fUV).bgra;
    
#elif AO_HEMISPHERE
    vec3 originalColor = texture(rayTracingImage, fUV).bgr;
    occlusion = 1.0f - occlusion;
    outColor = vec4(originalColor * occlusion, 1.0f);
	//outColor = texture(rayTracingImage, fUV).bgra;
    //outColor = vec4(occlusion, 1.0f);
#endif
}
