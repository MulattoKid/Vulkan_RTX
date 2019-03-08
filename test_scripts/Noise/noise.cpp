#include <chrono>
#include <cstdint>
#include <limits>
#include <random>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

typedef unsigned char uchar_t;

constexpr int32_t IMAGE_WIDTH = 1024;
constexpr int32_t IMAGE_HEIGHT = 1024;

constexpr int32_t NUM_SAMPLES = 8 * 8;
constexpr int32_t NUM_CANDIDATES_M = 1;

struct SamplePoint
{
	int32_t x;
	int32_t y;
};

void WhiteNoise(uchar_t* data)
{
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

void BlueNoise(uchar_t* data)
{
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
  	
  	// Print samples
  	printf("{\n");
  	for (int32_t i = 0; i < NUM_SAMPLES; i++)
  	{
  		printf("    {%f, %f},\n", float(samplePoints[i].x) / float(IMAGE_WIDTH), float(samplePoints[i].y) / float(IMAGE_HEIGHT));
  	}
  	printf("}\n");
  	
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

int main(int argc, char** argv)
{
	uchar_t* data = new uchar_t[IMAGE_WIDTH * IMAGE_HEIGHT];
	
	WhiteNoise(data);
	int res =  stbi_write_bmp("white_noise.bmp", IMAGE_WIDTH, IMAGE_HEIGHT, 1, data);
	assert(res != 0);
	
	BlueNoise(data);
	res =  stbi_write_bmp("blue_noise.bmp", IMAGE_WIDTH, IMAGE_HEIGHT, 1, data);
	assert(res != 0);

	return EXIT_SUCCESS;
}
