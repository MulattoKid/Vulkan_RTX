#ifndef CAMERA_H
#define CAMERA_H

#include "glm/vec3.hpp"

struct Camera
{
	unsigned int filmWidth;
	unsigned int filmHeight;
	float verticalFOV;
	float aspectRatio;
	glm::vec3 origin;
	glm::vec3 viewDir;
	glm::vec3 up;
	glm::vec3 topLeftCorner;
	glm::vec3 horizontalEnd;
	glm::vec3 verticalEnd;
	
	Camera() {}
	Camera(const unsigned int filmWidth, const unsigned int filmHeight, const float verticalFOV, const glm::vec3& origin, const glm::vec3& viewDir);
	void Update();
};

#endif
