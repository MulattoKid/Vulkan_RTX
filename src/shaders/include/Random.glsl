#ifndef SHADER_RANDOM_H
#define SHADER_RANDOM_H

#define PI 3.1415926535897932f

float RngFunc(float inRandom, int seed, int maxSeed)
{
	//return (sin(inRandom) + 1.0f) / 2.0f;
	return (sin(inRandom + ((float(seed) / float(maxSeed)) * PI)) + 1.0f) / 2.0f;
}

vec2 Random2D(vec2 inRandom, int seed, int maxSeed, vec2 coordRelative)
{
	return vec2(RngFunc(inRandom.x + coordRelative.x, seed, maxSeed), RngFunc(inRandom.y + coordRelative.y, seed, maxSeed));
}

#endif
