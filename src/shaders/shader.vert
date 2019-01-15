#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location=0) in vec2 inPosition;
layout(location=1) in vec2 inUV;

layout(location=0) out vec2 fUV;

void main()
{
	fUV = inUV;
    gl_Position = vec4(inPosition, 0.0f, 1.0f);
}
