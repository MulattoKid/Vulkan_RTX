/*
Copyright (c) 2018-2019 Daniel Fedai Larsen
LICENSE: See end of file for license information.
*/

#include "../../src/glm/vec2.hpp"
#include "../../src/glm/vec3.hpp"
#include "../../src/glm/vec4.hpp"
#include "../../src/glm/mat4x4.hpp"
#include "../../src/glm/gtc/matrix_transform.hpp"
#include <stdio.h>

int main()
{
    int sampleWidth = 8, sampleHeight = 8;
    int textureWidth = 64, textureHeight = 64;
    int pixelX = 245, pixelY = 985;
    
    glm::vec2 textureUV(pixelX / float(textureWidth), pixelY / float(textureHeight));
    printf("TextureUV: %f, %f\n", textureUV.x, textureUV.y);
    float rotation = 0.125f * 6.2831853071795865f;
    printf("Rotation: %f\n", rotation);
    
	int sampleX = 0, sampleY = 0;
	printf("Sample: %i, %i\n", sampleX, sampleY);
	
	int sampleIdx = sampleY * sampleWidth + sampleX;
	printf("SampleIdx: %i\n", sampleIdx);
	
	int sampleCenterX = sampleWidth / 2;
	int sampleCenterY = sampleHeight / 2;
	printf("SampleCenter: %i, %i\n", sampleCenterX, sampleCenterY);
	
	int sampleXRelative = sampleX - sampleCenterX;
	int sampleYRelative = sampleY - sampleCenterY;
	glm::vec4 sampleRelative(sampleXRelative, 0.0f, sampleYRelative, 1.0f);
	printf("RelativeSample: %f, %f, %f, %f\n", sampleRelative.x, sampleRelative.y, sampleRelative.z, sampleRelative.w);
	
	glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f, -1.0f, 0.0f));
	glm::vec4 rotatedSampleRelative = rotate * sampleRelative;
	printf("RotatedRelativeSample: %f, %f, %f, %f\n", rotatedSampleRelative.x, rotatedSampleRelative.y, rotatedSampleRelative.z, rotatedSampleRelative.w);
	
	int rotatedSampleRelativeX = std::round(rotatedSampleRelative.x);
	int rotatedSampleRelativeY = std::round(rotatedSampleRelative.z);
	printf("RotatedSampleRelative: %i, %i\n", rotatedSampleRelativeX, rotatedSampleRelativeY);
	
	int rotatedSampleX = rotatedSampleRelativeX + sampleCenterX;
	int rotatedSampleY = rotatedSampleRelativeY + sampleCenterY;
	printf("RotatedSample: %i, %i\n", rotatedSampleX, rotatedSampleY);
	
	int wrapRotatedSampleX = rotatedSampleX;
	if (wrapRotatedSampleX < 0)
	{
		wrapRotatedSampleX = sampleWidth + wrapRotatedSampleX;
	}
	else if (wrapRotatedSampleX >= sampleWidth)
	{
		wrapRotatedSampleX = sampleWidth - (sampleWidth - wrapRotatedSampleX) - 1;
	}
	int wrapRotatedSampleY = rotatedSampleY;
	if (wrapRotatedSampleY < 0)
	{
		wrapRotatedSampleY = sampleHeight + wrapRotatedSampleY;
	}
	else if (wrapRotatedSampleY >= sampleHeight)
	{
		wrapRotatedSampleY = sampleHeight - (sampleHeight - wrapRotatedSampleY) - 1;
	}
	printf("WrappedRotatedSample: %i, %i\n", wrapRotatedSampleX, wrapRotatedSampleY);

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
