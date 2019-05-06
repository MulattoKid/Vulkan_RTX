/*
Copyright (c) 2018-2019 Daniel Fedai Larsen
LICENSE: See end of file for license information.
*/

#include <algorithm>
#include <cassert>
#include <cstdio>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(int argc, char** argv)
{
	assert(argc == 3);
	printf("Image0: %s\n", argv[1]);
	printf("Image1: %s\n", argv[2]);
	
	int img0X, img0Y, img0Comp;
	unsigned char* img0 = stbi_load(argv[1], &img0X, &img0Y, &img0Comp, 1);
	assert(img0 != NULL);
	int img1X, img1Y, img1Comp;
	unsigned char* img1 = stbi_load(argv[2], &img1X, &img1Y, &img1Comp, 1);
	assert(img1 != NULL);
	assert(img0X == img1X && img0Y == img1Y);
	
	// Tanimoto
	unsigned long long int tanimotoInnerProduct = 0;
	double tanimotoDistance0 = 0.0;
	double tanimotoDistance1 = 0.0;
	for (int y = 0; y < img0Y; y++)
	{
		for (int x = 0; x < img0X; x++)
		{
			int idx = y * img0X + x;
			tanimotoInnerProduct += img0[idx] * img1[idx];
			tanimotoDistance0 += std::pow(std::abs(img0[idx]), 2);
			tanimotoDistance1 += std::pow(std::abs(img1[idx]), 2);
		}
	}
	double tanimotoSimilarity = double(tanimotoInnerProduct) / (tanimotoDistance0 + tanimotoDistance1 - tanimotoInnerProduct);
	printf("-----------------------------------------\n");
	printf("Tanimoto inner product: %llu\n", tanimotoInnerProduct);
	printf("Tanimoto distance 0: %f\n", tanimotoDistance0);
	printf("Tanimoto distance 1: %f\n", tanimotoDistance1);
	printf("Tanimoto similarity: %f\n", tanimotoSimilarity);
	
	// Minimum ratio
	double minimumRatioSum = 0.0;
	for (int y = 0; y < img0Y; y++)
	{
		for (int x = 0; x < img0X; x++)
		{
			int idx = y * img0X + x;
			if (img0[idx] != 0 && img1[idx] != 0)
			{
				double a = double(img0[idx]) / double(img1[idx]);
				double b = double(img1[idx]) / double(img0[idx]);
				minimumRatioSum += std::min(a, b);
			}
		}
	}
	double minimumRatioSimilarity = minimumRatioSum / double(img0X * img0Y);
	printf("-----------------------------------------\n");
	printf("Minimum ratio sum: %f\n", minimumRatioSum);
	printf("Minimum ratio similarity: %f\n", minimumRatioSimilarity);
	
	stbi_image_free(img1);
	stbi_image_free(img0);
	
	return 0;
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
