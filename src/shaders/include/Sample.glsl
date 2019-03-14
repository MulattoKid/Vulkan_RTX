#ifndef SHADER_SAMPLE_H
#define SHADER_SAMPLE_H

#include "Geometric.glsl"

#define PI 3.1415926535897932f
#define TWO_PI 6.2831853071795865f
#define ONE_OVER_TWO_PI 0.1591549430918953f
#define GOLDEN_ANGLE 2.3999632297286533f

float SameHemisphere(vec3 a, vec3 b)
{
	// Same
	if (dot(a, b) > 0.0f)
	{
		return 1.0f;
	}
	return 0.0f;
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

vec3 SampleHemisphereVaryingZ(vec3 normal, vec2 rng)
{
	float phi = TWO_PI * rng.x;
	float theta = acos(rng.y);
	vec3 sampledDir = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), rng.y + (sin(phi * TWO_PI) * 0.1));
	sampledDir = normalize(sampledDir);
	
	//This is the sampled direction's hemisphere normal because:
	// x's domain is [-1,1]
	// y's domain is [-1,1]
	// z's domain is [0,1]
	vec3 sampleSpaceNormal = vec3(0.0f, 0.0f, 1.0f);
	mat3 rotation = RotationToAlignAToB(sampleSpaceNormal, normal);
	return normalize(rotation * sampledDir);
}

vec3 SampleHemisphereCosine(vec3 normal, vec2 rng)
{
	// Ray Tracing Gems p.211
	float phi = TWO_PI * rng.x;
	float theta = sqrt(rng.y);
	vec3 sampledDir = vec3(theta * cos(phi), theta * sin(phi), sqrt(1 - rng.y));
	
	//This is the sampled direction's hemisphere normal because:
	// x's domain is [-1,1]
	// y's domain is [-1,1]
	// z's domain is [0,1]
	vec3 sampleSpaceNormal = vec3(0.0f, 0.0f, 1.0f);
	mat3 rotation = RotationToAlignAToB(sampleSpaceNormal, normal);
	return normalize(rotation * sampledDir);
}

vec3 SampleHemisphereFibonacciSpiral(vec3 normal, vec2 thetaPhi)
{
	// https://github.com/matt77hias/fibpy/blob/master/src/sampling.py
	float sinTheta = sin(thetaPhi.x);
	float cosTheta = cos(thetaPhi.x);
	float cosPhi = cos(thetaPhi.y);
	float sinPhi = sin(thetaPhi.y);
	vec3 sampledDir = vec3(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta);

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
