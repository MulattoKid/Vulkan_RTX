#ifndef RNG_H
#define RNG_H

#include <random>
#include <time.h>

struct RNG
{
	std::default_random_engine generator;
	std::uniform_real_distribution<double> distribution;

	RNG();
	float Uniform1D();
	void Uniform1D(float* array);
	void Uniform2D(float* array);
};

#endif
