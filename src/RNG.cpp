#include <math.h>
#include "RNG.h"
#include <time.h>

RNG::RNG()
{
	generator = std::default_random_engine(time(NULL));
	distribution = std::uniform_real_distribution<double>(0.0f, 1.0f);
}

float RNG::Uniform1D()
{
	double rng_d = distribution(generator);
	float rng_f = float(rng_d);
	if (rng_f > rng_d)
	{
		rng_f = std::nextafterf(rng_f, -std::numeric_limits<float>::infinity());
	}
	
	return rng_f;
}

void RNG::Uniform1D(float* array)
{
	array[0] = Uniform1D();
}

void RNG::Uniform2D(float* array)
{
	array[0] = Uniform1D();
	array[1] = Uniform1D();
}
