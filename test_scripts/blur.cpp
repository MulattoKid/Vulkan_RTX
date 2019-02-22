#include <stdio.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char**argv)
{
	unsigned char data[25] = {
		50,  50,  50,  150, 150,
		50,  50,  50,  150, 150,
		50,  50,  150, 150, 150,
		50,  150, 150, 150, 150,
		150, 150, 150, 150, 150,
	};
	unsigned char avgData[25];
	for (int y = 0; y < 5; y++)
	{
		for (int x = 0; x < 5; x++)
		{
			int idx = y * 5 + x;
			avgData[idx] = data[idx];
		}
	}
	for (int y = 1; y < 4; y++)
	{
		for (int x = 1; x < 4; x++)
		{
			int idx = y * 5 + x;
			float sum = 0.0f;
			sum += data[idx - 6];
			sum += data[idx - 5];
			sum += data[idx - 4];
			
			sum += data[idx - 1];
			sum += data[idx];
			sum += data[idx + 1];
			
			sum += data[idx + 4];
			sum += data[idx + 5];
			sum += data[idx + 6];
			
			unsigned char avg = sum / 9.0f;
			avgData[idx] = avg;
			printf("Original: %i Blurred: %i\n", data[idx], avg);
		}
	}
	
	int res =  stbi_write_bmp("original.bmp", 5, 5, 1, data);
	assert(res != 0);
	res =  stbi_write_bmp("avg.bmp", 5, 5, 1, avgData);
	assert(res != 0);

	return 0;
}
