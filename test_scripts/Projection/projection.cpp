#include "../../src/glm/vec3.hpp"
#include "../../src/glm/vec4.hpp"
#include "../../src/glm/gtc/matrix_transform.hpp"
#include <stdio.h>

int main(int argc, char** argv)
{
	// Base data
	float camFOV = 45.0f;
	float camFOVRad = glm::radians(camFOV);
	float camWidth = 1920.0f;
	float camHeight = 1080.0f;
	float camAspectRatio = camWidth / camHeight;
	glm::vec3 camOrigin = glm::vec3(0.0f, 1.1f, 0.3f);
	glm::vec3 camViewDir = glm::normalize(glm::vec3(0.0f, -0.3f, -1.0f));
	printf("View dir: %f %f %f\n", camViewDir.x, camViewDir.y, camViewDir.z);
	glm::vec2 uv = glm::vec2(0.34f, 0.58f);
	printf("Original UV: %f %f\n", uv.x, uv.y);
	
	// Ray tracing
	float lensHalfHeight = glm::tan(camFOVRad / 2.0f);
	float lensHalfWidth = lensHalfHeight * camAspectRatio;
	glm::vec3 baseUp(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::normalize(glm::cross(camViewDir, baseUp));
	glm::vec3 up = glm::normalize(glm::cross(right, camViewDir));
	glm::vec3 topLeftCorner = camOrigin + (lensHalfWidth * (-right)) + (lensHalfHeight * up) + camViewDir;
	glm::vec3 horizontalEnd = 2.0f * lensHalfWidth * right;
	glm::vec3 verticalEnd = 2.0f * lensHalfHeight * (-up);
    glm::vec3 rayDir = glm::normalize(topLeftCorner + (horizontalEnd * uv.x) + (verticalEnd * uv.y) - camOrigin);
    printf("Ray dir: %f %f %f\n", rayDir.x, rayDir.y, rayDir.z);
	
	// Rasterization
	glm::mat4x4 view = glm::lookAt(camOrigin, camOrigin + camViewDir, baseUp);
	glm::mat4x4 projection = glm::perspective(camFOVRad, camAspectRatio, 0.01f, 100.0f);
	glm::mat4x4 vp = projection * view;
	glm::vec3 worldPos = camOrigin + rayDir * 3.29f;
	glm::vec4 projectedPos = vp * glm::vec4(worldPos, 1.0f);
	projectedPos /= projectedPos.w;
	glm::vec2 reconstructedUV = glm::vec2(
									0.5f * projectedPos.x + 0.5f,
									0.5f * (projectedPos.y * -1.0f) + 0.5f
								);
	printf("Reconstructed UV: %f %f\n", reconstructedUV.x, reconstructedUV.y);

	return EXIT_SUCCESS;
}
