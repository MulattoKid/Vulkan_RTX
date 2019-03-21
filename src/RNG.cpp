/*
Copyright (c) 2018-2019 Daniel Fedai Larsen
LICENSE: See end of file for license information.
*/

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

/*
MIT License

Copyright (c) 2018-2019 Daniel Fedai Larsen

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
