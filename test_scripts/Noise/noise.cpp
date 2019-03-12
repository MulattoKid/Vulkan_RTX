#include <chrono>
#include <cstdint>
#include <limits>
#include <random>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define PRINT 1

typedef unsigned char uchar_t;

constexpr int32_t IMAGE_WIDTH = 1024;
constexpr int32_t IMAGE_HEIGHT = 1024;

constexpr int32_t NUM_DIMENSIONS = 2;
constexpr int32_t NUM_SAMPLES = 64;
constexpr int32_t NUM_CANDIDATES_M = 1;
constexpr int32_t NUM_FRAMES = 3;

constexpr float TWO_PI = 6.2831853071795865f;
constexpr float GOLDEN_ANGLE = 2.3999632297286533f;

struct SamplePoint
{
	int32_t x;
	int32_t y;
};

void WhiteNoise(uchar_t* data)
{
	printf("White noise...\n");
	SamplePoint samplePoints[NUM_SAMPLES];
	std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
  	std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
  	
  	for (int32_t s = 0; s < NUM_SAMPLES; s++)
  	{
  		samplePoints[s].x = int32_t(IMAGE_WIDTH * distribution(generator));
  		samplePoints[s].y = int32_t(IMAGE_HEIGHT * distribution(generator));
  	}
  	
  	// Just for outlining the picture border
  	for (int y = 0; y < IMAGE_HEIGHT; y++)
  	{
  		for (int x = 0; x < IMAGE_WIDTH; x++)
  		{
  			data[y * IMAGE_WIDTH + x] = 64;
  		}
  	}
  	// Color samples
  	for (int32_t i = 0; i < NUM_SAMPLES; i++)
  	{
  		int32_t dataIdx = samplePoints[i].y * IMAGE_WIDTH + samplePoints[i].x;
  		data[dataIdx] = 255;
  	}
}

// https://blog.demofox.org/2017/10/20/generating-blue-noise-sample-points-with-mitchells-best-candidate-algorithm/
void BlueNoise(uchar_t* data)
{
	printf("Blue noise...\n");
	SamplePoint samplePoints[NUM_SAMPLES];
	std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
  	std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
  	
  	// Generate first sample
  	samplePoints[0].x = int32_t(IMAGE_WIDTH * distribution(generator));
  	samplePoints[0].y = int32_t(IMAGE_HEIGHT * distribution(generator));
  	
  	// Generate remaining samples
  	for (int32_t s = 1; s < NUM_SAMPLES; s++)
  	{
  		const int32_t numCandidates = s * NUM_CANDIDATES_M + 1;
  		
  		int32_t furthestDistanceGlobal = std::numeric_limits<int32_t>::min();
  		int32_t x, y, closestDistanceLocal;
  		for (int32_t c = 0; c < numCandidates; c++)
  		{
		  	x = int32_t(IMAGE_WIDTH * distribution(generator));
		  	y = int32_t(IMAGE_HEIGHT * distribution(generator));
		  	
	  		closestDistanceLocal = std::numeric_limits<int32_t>::max();
	  		int32_t distanceX, distanceY, distance;
		  	for (int32_t i = 0; i < s; i++)
		  	{
		  		// Takes wrapping-distance into account
		  		distanceX = std::min(IMAGE_WIDTH - std::abs(samplePoints[i].x - x),
		  									 std::abs(samplePoints[i].x - x));
			  	distanceY = std::min(IMAGE_HEIGHT - std::abs(samplePoints[i].y - y),
			  								 std::abs(samplePoints[i].y - y));
			  	// Avoid sqrt as the actual distance is irrelevant.
			  	// I only care about the largest distance and sqrt doesn't change that.
			  	distance = distanceX * distanceX + distanceY * distanceY;
			  	if (distance < closestDistanceLocal)
			  	{
			  		closestDistanceLocal = distance;
			  	}
		  	}
		  	
		  	if (closestDistanceLocal > furthestDistanceGlobal)
		  	{
		  		samplePoints[s].x = x;
		  		samplePoints[s].y = y;
		  		furthestDistanceGlobal = closestDistanceLocal;
		  	}
  		}
  	}
  	
#if PRINT
  	// Print samples
  	printf("BLUE NOISE =\n{\n");
  	for (int32_t i = 0; i < NUM_SAMPLES; i++)
  	{
  		printf("    {%f, %f},\n", float(samplePoints[i].x) / float(IMAGE_WIDTH), float(samplePoints[i].y) / float(IMAGE_HEIGHT));
  	}
  	printf("}\n");
#endif
  	
  	// Just for outlining the picture border
  	for (int y = 0; y < IMAGE_HEIGHT; y++)
  	{
  		for (int x = 0; x < IMAGE_WIDTH; x++)
  		{
  			data[y * IMAGE_WIDTH + x] = 64;
  		}
  	}
  	// Color samples
  	for (int32_t i = 0; i < NUM_SAMPLES; i++)
  	{
  		int32_t dataIdx = samplePoints[i].y * IMAGE_WIDTH + samplePoints[i].x;
  		data[dataIdx] = 255;
  	}
}

void BlueNoiseAnimate(uchar_t* data)
{
	printf("Blue noise animation...\n");
	SamplePoint samplePoints[NUM_SAMPLES * NUM_FRAMES];
	std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
  	std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
  	
  	// Generate first sample
  	samplePoints[0].x = int32_t(IMAGE_WIDTH * distribution(generator));
  	samplePoints[0].y = int32_t(IMAGE_HEIGHT * distribution(generator));
  	
  	// Generate remaining samples for frame 0
  	for (int32_t s = 1; s < NUM_SAMPLES; s++)
  	{
  		const int32_t numCandidates = s * NUM_CANDIDATES_M + 1;
  		
  		int32_t furthestDistanceGlobal = std::numeric_limits<int32_t>::min();
  		int32_t x, y, closestDistanceLocal;
  		for (int32_t c = 0; c < numCandidates; c++)
  		{
		  	x = int32_t(IMAGE_WIDTH * distribution(generator));
		  	y = int32_t(IMAGE_HEIGHT * distribution(generator));
		  	
	  		closestDistanceLocal = std::numeric_limits<int32_t>::max();
	  		int32_t distanceX, distanceY, distance;
		  	for (int32_t i = 0; i < s; i++)
		  	{
		  		// Takes wrapping-distance into account
		  		distanceX = std::min(IMAGE_WIDTH - std::abs(samplePoints[i].x - x),
		  									 std::abs(samplePoints[i].x - x));
			  	distanceY = std::min(IMAGE_HEIGHT - std::abs(samplePoints[i].y - y),
			  								 std::abs(samplePoints[i].y - y));
			  	// Avoid sqrt as the actual distance is irrelevant.
			  	// I only care about the largest distance and sqrt doesn't change that.
			  	distance = distanceX * distanceX + distanceY * distanceY;
			  	if (distance < closestDistanceLocal)
			  	{
			  		closestDistanceLocal = distance;
			  	}
		  	}
		  	
		  	if (closestDistanceLocal > furthestDistanceGlobal)
		  	{
		  		samplePoints[s].x = x;
		  		samplePoints[s].y = y;
		  		furthestDistanceGlobal = closestDistanceLocal;
		  		
  				if (s >= NUM_SAMPLES / 2)
  				{
  					int32_t offset = (NUM_SAMPLES / 2);
			  		samplePoints[offset + s].x = x;
			  		samplePoints[offset + s].y = y;
  				}
		  	}
  		}
  	}
  	
  	// Generate samples for remaining frames
  	for (int32_t f = 1; f < NUM_FRAMES; f++)
  	{
	  	int32_t frameOffset = f * NUM_SAMPLES;
  		for (int32_t s = NUM_SAMPLES / 2; s < NUM_SAMPLES; s++)
	  	{
	  		const int32_t numCandidates = s * NUM_CANDIDATES_M + 1;
	  		
	  		int32_t furthestDistanceGlobal = std::numeric_limits<int32_t>::min();
	  		int32_t x, y, closestDistanceLocal;
	  		for (int32_t c = 0; c < numCandidates; c++)
	  		{
			  	x = int32_t(IMAGE_WIDTH * distribution(generator));
			  	y = int32_t(IMAGE_HEIGHT * distribution(generator));
			  	
		  		closestDistanceLocal = std::numeric_limits<int32_t>::max();
		  		int32_t distanceX, distanceY, distance;
			  	for (int32_t i = 0; i < s; i++)
			  	{
			  		// Takes wrapping-distance into account
			  		distanceX = std::min(IMAGE_WIDTH - std::abs(samplePoints[frameOffset + i].x - x),
			  									 std::abs(samplePoints[frameOffset + i].x - x));
				  	distanceY = std::min(IMAGE_HEIGHT - std::abs(samplePoints[frameOffset + i].y - y),
				  								 std::abs(samplePoints[frameOffset + i].y - y));
				  	// Avoid sqrt as the actual distance is irrelevant.
				  	// I only care about the largest distance and sqrt doesn't change that.
				  	distance = distanceX * distanceX + distanceY * distanceY;
				  	if (distance < closestDistanceLocal)
				  	{
				  		closestDistanceLocal = distance;
				  	}
			  	}
			  	
			  	if (closestDistanceLocal > furthestDistanceGlobal)
			  	{
			  		samplePoints[frameOffset + s].x = x;
			  		samplePoints[frameOffset + s].y = y;
			  		furthestDistanceGlobal = closestDistanceLocal;
			  		
	  				if ((s >= NUM_SAMPLES / 2) && (f + 1 < NUM_FRAMES))
	  				{
	  					int32_t offset = (NUM_SAMPLES / 2) - 1;
				  		samplePoints[frameOffset + offset + s].x = x;
				  		samplePoints[frameOffset + offset + s].y = y;
	  				}
			  	}
	  		}
	  	}
  	}
  	
#if PRINT
  	// Print samples
  	printf("BLUE NOISE ANIMATE =\n{\n");
  	for (int32_t f = 0; f < NUM_FRAMES; f++)
  	{
  		int32_t frameOffset = NUM_SAMPLES * f;
  		printf("    {\n");
  		for (int32_t s = 0; s < NUM_SAMPLES; s++)
	  	{
	  		printf("        {%f, %f},\n", float(samplePoints[frameOffset + s].x) / float(IMAGE_WIDTH), float(samplePoints[frameOffset + s].y) / float(IMAGE_HEIGHT));
	  	}
	  	printf("    }\n");
  	}
	printf("}\n");
#endif
  	
  	// Just for outlining the picture border
  	for (int32_t f = 0; f < NUM_FRAMES; f++)
  	{
	  	int32_t frameOffset = IMAGE_WIDTH * IMAGE_HEIGHT * f;
  		for (int y = 0; y < IMAGE_HEIGHT; y++)
	  	{
	  		for (int x = 0; x < IMAGE_WIDTH; x++)
	  		{
	  			data[frameOffset + (y * IMAGE_WIDTH + x)] = 64;
	  		}
	  	}
  	}
  	// Color samples
  	for (int32_t f = 0; f < NUM_FRAMES; f++)
  	{
  		int32_t frameOffsetSamples = NUM_SAMPLES * f;
	  	int32_t frameOffsetData = IMAGE_WIDTH * IMAGE_HEIGHT * f;
  		for (int32_t s = 0; s < NUM_SAMPLES; s++)
	  	{
	  		int32_t sampleIdx = frameOffsetSamples + s;
	  		int32_t dataIdx = frameOffsetData + (samplePoints[sampleIdx].y * IMAGE_WIDTH + samplePoints[sampleIdx].x);
	  		data[dataIdx] = 255;
	  	}
  	}
}

// http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/#GeneralizingGoldenRatio
void GoldenRatio(uchar_t* data)
{
	printf("Golden ratio...\n");
	SamplePoint samplePoints[NUM_SAMPLES];
	// Note: golden ratio and alphas for two dimensions
	float goldenRatio = 1.32471795724474602596f;
	float alphas[NUM_DIMENSIONS] = {
		0.75487765f, 0.56984026f
	};
	
	// Generate samples
	for (int32_t s = 0; s < NUM_SAMPLES; s++)
	{
		float x = 0.5f + alphas[0] * float(s + 1);
		x -= float(int32_t(x));
		float y = 0.5f + alphas[1] * float(s + 1);
		y -= float(int32_t(y));

		samplePoints[s].x = int32_t(x * IMAGE_WIDTH);
		samplePoints[s].y = int32_t(y * IMAGE_HEIGHT);
	}
	
#if PRINT
	// Print samples
  	printf("GOLDEN RATIO =\n{\n");
  	for (int32_t i = 0; i < NUM_SAMPLES; i++)
  	{
  		printf("    {%f, %f},\n", float(samplePoints[i].x) / float(IMAGE_WIDTH), float(samplePoints[i].y) / float(IMAGE_HEIGHT));
  	}
  	printf("}\n");
#endif
  	
  	// Just for outlining the picture border
  	for (int y = 0; y < IMAGE_HEIGHT; y++)
  	{
  		for (int x = 0; x < IMAGE_WIDTH; x++)
  		{
  			data[y * IMAGE_WIDTH + x] = 64;
  		}
  	}
  	// Color samples
  	for (int32_t i = 0; i < NUM_SAMPLES; i++)
  	{
  		int32_t dataIdx = samplePoints[i].y * IMAGE_WIDTH + samplePoints[i].x;
  		data[dataIdx] = 255;
  	}
}

void FibonacciSpiral2D(uchar_t* data)
{
	printf("Fibonacci spiral 2D...\n");
	SamplePoint samplePoints[NUM_SAMPLES];
	float goldenRatio = 1.6180339887498948482f;
	float centerX = IMAGE_WIDTH / 2;
	float centerY = IMAGE_HEIGHT / 2;
	
	for (int32_t s = 0; s < NUM_SAMPLES; s++)
	{
		float a = float(s) / float(NUM_SAMPLES);
		float b = float(s) / goldenRatio;
		float r = std::sqrt(a);
		float theta = TWO_PI * b;
		
		int32_t x = ((r * std::cos(theta) + 1.0f) / 2.0f) * IMAGE_WIDTH;
		int32_t y = ((r * std::sin(theta) + 1.0f) / 2.0f) * IMAGE_HEIGHT;
		samplePoints[s].x = x;
		samplePoints[s].y = y;
	}
	
#if PRINT
	// Print samples
  	printf("FIBONACCI SPIRAL 2D =\n{\n");
  	for (int32_t i = 0; i < NUM_SAMPLES; i++)
  	{
  		printf("    {%f, %f},\n", float(samplePoints[i].x) / float(IMAGE_WIDTH), float(samplePoints[i].y) / float(IMAGE_HEIGHT));
  	}
  	printf("}\n");
#endif
	
	// Just for outlining the picture border
  	for (int y = 0; y < IMAGE_HEIGHT; y++)
  	{
  		for (int x = 0; x < IMAGE_WIDTH; x++)
  		{
  			data[y * IMAGE_WIDTH + x] = 64;
  		}
  	}
  	// Color samples
  	for (int32_t i = 0; i < NUM_SAMPLES; i++)
  	{
  		int32_t dataIdx = samplePoints[i].y * IMAGE_WIDTH + samplePoints[i].x;
  		data[dataIdx] = 255;
  	}
}

void FibonacciSpiral3D(uchar_t* data)
{
	printf("Fibonacci spiral 3D...\n");
	
#if PRINT
  	printf(" FIBONACCI SPIRAL 3D =\n{\n");
#endif
	for (int32_t s = 0; s < NUM_SAMPLES; s++)
	{
		float sinTheta = sqrt((s + 0.5f) / (NUM_SAMPLES - 0.5f));
		float cosTheta = sqrt(1.0f - sinTheta * sinTheta);
		float phi = GOLDEN_ANGLE * (s + 1.0f);
		float theta = std::acos(cosTheta);
#if PRINT
		printf("    {%f, %f},\n", theta, phi);
#endif
	}
#if PRINT	
  	printf("}\n");
#endif
}

int main(int argc, char** argv)
{
	// Includes enough memory for animation, but that is only
	// used by BlueNoiseAnimate
	uchar_t* data = new uchar_t[IMAGE_WIDTH * IMAGE_HEIGHT * NUM_FRAMES];
	
	WhiteNoise(data);
	int res =  stbi_write_bmp("white_noise.bmp", IMAGE_WIDTH, IMAGE_HEIGHT, 1, data);
	assert(res != 0);
	
	BlueNoise(data);
	res =  stbi_write_bmp("blue_noise.bmp", IMAGE_WIDTH, IMAGE_HEIGHT, 1, data);
	assert(res != 0);
	
	BlueNoiseAnimate(data);
	for (int32_t i = 0; i < NUM_FRAMES; i++)
	{
		std::string frameName = "blue_noise_animate_f" + std::to_string(i) + ".bmp";
		int32_t frameOffset = IMAGE_WIDTH * IMAGE_HEIGHT * i;
		res =  stbi_write_bmp(frameName.c_str(), IMAGE_WIDTH, IMAGE_HEIGHT, 1, data + frameOffset);
		assert(res != 0);
	}
	
	GoldenRatio(data);
	res =  stbi_write_bmp("golden_ratio.bmp", IMAGE_WIDTH, IMAGE_HEIGHT, 1, data);
	assert(res != 0);
	
	FibonacciSpiral2D(data);
	res =  stbi_write_bmp("fibonacci_spiral_2D.bmp", IMAGE_WIDTH, IMAGE_HEIGHT, 1, data);
	assert(res != 0);
	
	FibonacciSpiral3D(data);
	
	

	return EXIT_SUCCESS;
}
