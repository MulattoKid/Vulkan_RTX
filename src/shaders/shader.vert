#version 450
#extension GL_ARB_separate_shader_objects : enable

/*vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(-0.5, 0.5),
    vec2(0.5, 0.5)
);*/

layout(location=0) in vec2 inPosition;

void main() {
    //gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    gl_Position = vec4(inPosition, 0.0f, 1.0f);
}
