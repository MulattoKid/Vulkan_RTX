#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : require

#include "Defines.glsl"

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
	vec3 occlusion;
    
#if BLUR_3x3
	vec2 horizontalOffsetSmall = vec2(1.0f, 0.0f);
	vec2 horizontalUVs[3] = {
		(fragCoord - horizontalOffsetSmall) * pixelDelta,
		 fragCoord * pixelDelta,
		(fragCoord + horizontalOffsetSmall) * pixelDelta
	};
	occlusion  = texture(shadowImage, horizontalUVs[0]).rgb * FIRST_WEIGHT;
	occlusion += texture(shadowImage, horizontalUVs[1]).rgb * CENTER_WEIGHT;
	occlusion += texture(shadowImage, horizontalUVs[2]).rgb * FIRST_WEIGHT;
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
	occlusion  = texture(shadowImage, horizontalUVs[0]).rgb * FIRST_WEIGHT;
	occlusion += texture(shadowImage, horizontalUVs[1]).rgb * SECOND_WEIGHT;
	occlusion += texture(shadowImage, horizontalUVs[2]).rgb * CENTER_WEIGHT;
	occlusion += texture(shadowImage, horizontalUVs[3]).rgb * SECOND_WEIGHT;
	occlusion += texture(shadowImage, horizontalUVs[4]).rgb * FIRST_WEIGHT;
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
	occlusion  = texture(shadowImage, horizontalUVs[0]).rgb * FIRST_WEIGHT;
	occlusion += texture(shadowImage, horizontalUVs[1]).rgb * SECOND_WEIGHT;
	occlusion += texture(shadowImage, horizontalUVs[2]).rgb * THIRD_WEIGHT;
	occlusion += texture(shadowImage, horizontalUVs[3]).rgb * CENTER_WEIGHT;
	occlusion += texture(shadowImage, horizontalUVs[4]).rgb * THIRD_WEIGHT;
	occlusion += texture(shadowImage, horizontalUVs[5]).rgb * SECOND_WEIGHT;
	occlusion += texture(shadowImage, horizontalUVs[6]).rgb * FIRST_WEIGHT;
#endif

	outColor = vec4(occlusion, 1.0f);
}
