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
