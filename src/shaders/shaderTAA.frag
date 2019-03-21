/*
Copyright (c) 2018-2019 Daniel Fedai Larsen
*/

#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : require

#include "Defines.glsl"
#include "DataLayouts.glsl"

layout(location=0) in vec2 fUV;

layout(set = 0, binding = RS1_PREVIOUS_FRAME_IMAGE_BINDING_LOCATION) uniform sampler2D previousFrameImage;
layout(set = 0, binding = RS1_POSITION_IMAGE_BINDING_LOCATION) uniform sampler2D positionImage;
layout(set = 0, binding = RS1_CURRENT_FRAME_IMAGE_BINDING_LOCATION, input_attachment_index = 0) uniform subpassInput currentFrameImage;
layout(set = 0, binding = RS1_CAMERA_BUFFER_BINDING_LOCATION, std140) uniform cameraBuffer
{
	CameraShader camera;
};

layout(location=0) out vec4 outColor;

void main()
{
	// Remember to not use the w-component as it contains garbage
	vec3 currentFramePosition = texture(positionImage, fUV).xyz;
	vec3 currentFrameColor = subpassLoad(currentFrameImage).rgb;
	
	// Check if this is a special case: see primary.rgen for main ray tracing
	// pass for the cases where this will not pass
	if (currentFramePosition == vec3(0.0f))
	{
		outColor = vec4(currentFrameColor, 1.0f);
		return;
	}
	
	vec4 previousFrameProjected = camera.previousViewProjection * vec4(currentFramePosition, 1.0f);
	// https://www.youtube.com/watch?v=2XXS5UyNjjU @7:40
	vec2 previousFrameUV = previousFrameProjected.xy / previousFrameProjected.w;
	previousFrameUV.y *= -1.0f;
	previousFrameUV *= 0.5f;
	previousFrameUV += 0.5f;
	vec3 previousFrameColor = texture(previousFrameImage, previousFrameUV).rgb;
	
	// Combine current and previous frame's colors
	outColor = vec4(mix(previousFrameColor, currentFrameColor, 0.5f), 1.0f);
	
	//outColor = vec4(currentFrameColor, 1.0f);
	//outColor = vec4(previousFrameColor, 1.0f);
}
