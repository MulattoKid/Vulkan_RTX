/*
Copyright (c) 2018-2019 Daniel Fedai Larsen
*/

#ifndef SHADER_GEOMETRIC_H
#define SHADER_GEOMETRIC_H

// https://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d
mat3 RotationToAlignAToB(vec3 a, vec3 b)
{
	vec3 v = cross(a, b);
	mat3 m = mat3(vec3(0.0f, v[2], -v[1]), vec3(-v[2], 0.0f, v[0]), vec3(v[1], -v[0], 0.0f));
	float s = length(v);
	float c = dot(a, b);
	mat3 rotation = mat3(1.0f) + m + ((m * m) * ((1.0f - c) / (s * s)));
	return rotation;
}

//http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
mat4 Rotate(float angle, vec3 axis)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s,  0.0f,
                oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s,  0.0f,
                oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);
}

#endif
