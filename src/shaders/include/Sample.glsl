#ifndef SHADER_SAMPLE_H
#define SHADER_SAMPLE_H

#define TWO_PI 6.2831853071795865

float Rand(vec2 co, float rnd)
{
    return fract(sin(dot(co, vec2(12.9898f, 78.233f)) * rnd) * 43758.5453f);
}

vec2 Rand2D(vec2 co, vec2 rnd)
{
    return vec2(fract(sin(dot(co, vec2(12.9898,78.233)) * rnd.x) * 43758.5453),
    			fract(sin(dot(co, vec2(12.9898,78.233)) * rnd.y) * 43758.5453));
}

// https://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d
mat3 RotationToAlignAToB(vec3 a, vec3 b)
{
	vec3 v = cross(a, b);
	//mat3 m = mat3(vec3(0.0f, v[2], -v[1]), vec3(-v[2], 0.0f, v[0]), vec3(v[1], -v[0], 0.0f));
	mat3 m = mat3(vec3(0.0f, -v[2], v[1]), vec3(v[2], 0.0f, -v[0]), vec3(-v[1], v[0], 0.0f));
	float s = length(v);
	float c = dot(a, b);
	mat3 rotation = mat3(1.0f) + m + ((m * m) * ((1.0f - c) / (s * s)));
	return rotation;
}

vec3 SampleNormalHemisphere(vec3 normal, vec2 rng)
{
	float theta = acos(rng.x);
	float phi = TWO_PI * rng.y;
	vec3 samplePoint = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), rng.x);
	
	//This is the sampled hemisphere's normal because:
	// x's domain is [-1,1]
	// y's domain is [-1,1]
	// z's domain is [0,1]
	vec3 normalSampleSpace = vec3(0.0f, 0.0f, 1.0f);
	mat3 rotation = RotationToAlignAToB(normalSampleSpace, normal);
	return normalize(rotation * samplePoint);
}

#endif
