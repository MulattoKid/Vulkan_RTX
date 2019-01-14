#ifndef SHADER_SAMPLE_H
#define SHADER_SAMPLE_H

#define PI 3.1415926535897932f
#define TWO_PI 6.2831853071795865f
#define ONE_OVER_TWO_PI 0.1591549430918953f

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

float HemispherePdf()
{
	return ONE_OVER_TWO_PI;
}

vec3 SampleHemisphere(vec3 normal, vec2 rng)
{
	float phi = TWO_PI * rng.x;
	float theta = acos(rng.y);
	vec3 sampledDir = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), rng.y);
	
	//This is the sampled direction's hemisphere normal because:
	// x's domain is [-1,1]
	// y's domain is [-1,1]
	// z's domain is [0,1]
	vec3 sampleSpaceNormal = vec3(0.0f, 0.0f, 1.0f);
	mat3 rotation = RotationToAlignAToB(sampleSpaceNormal, normal);
	return normalize(rotation * sampledDir);
}

// p.781
float ConePdf(float cosThetaMax)
{
	return 1.0f / (TWO_PI * (1.0f - cosThetaMax));
}

vec3 SampleCone(vec3 normal, vec2 rng, float cosThetaMax)
{
	float phi = rng.x * 2.0f * PI;
	float cosTheta = (1.0f - rng.y) + rng.y * cosThetaMax;
	float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
	vec3 sampledDir = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
	
	vec3 sampleSpaceNormal = vec3(0.0f, 0.0f, 1.0f);
	mat3 rotation = RotationToAlignAToB(sampleSpaceNormal, normal);
	return normalize(rotation * sampledDir);
}

#endif
