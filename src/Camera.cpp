#include "Camera.h"
#include "glm/geometric.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/trigonometric.hpp"

Camera::Camera(const unsigned int filmWidth, const unsigned int filmHeight, const float verticalFOV, const glm::vec3& origin, const glm::vec3& viewDir)
{
	this->filmWidth = filmWidth;
	this->filmHeight = filmHeight;
	this->verticalFOV = verticalFOV;
	this->aspectRatio = filmWidth / float(filmHeight);
	this->origin = origin;
	this->viewDir = viewDir;
	
	float theta = (verticalFOV * glm::pi<float>()) / 180.0f; //Convert to radians
	float lensHeight = glm::tan(theta);
	float lensWidth = lensHeight * this->aspectRatio;
	float lensHalfWidth = lensWidth / 2.0f;
	float lensHalfHeight = lensHeight / 2.0f;
	
	//Calculate the three vectors that define the camera	
	static const glm::vec3 baseUp(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::normalize(glm::cross(viewDir, baseUp));
	up = glm::normalize(glm::cross(right, viewDir));
	//Calculate top_left_corner
	//1) Start at camera's position
	//2) Go lens_half_width along the camera's left (-right) axis
	//3) Go lens_half_height along the camera's up axis
	//4) Go 1 unit along the camera's view_direction axis
	topLeftCorner = this->origin + (lensHalfWidth * (-right)) + (lensHalfHeight * up) + viewDir;
	//Go width of lense along the camera's right axis
	horizontalEnd = lensWidth * right;
	//Go height of lense along the camera's down (-up) axis
	verticalEnd = lensHeight * (-up);
}

void Camera::Update()
{
	float theta = (verticalFOV * glm::pi<float>()) / 180.0f; //Convert to radians
	float lensHeight = glm::tan(theta);
	float lensWidth = lensHeight * this->aspectRatio;
	float lensHalfWidth = lensWidth / 2.0f;
	float lensHalfHeight = lensHeight / 2.0f;
	
	//Calculate the three vectors that define the camera	
	static const glm::vec3 baseUp(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::normalize(glm::cross(viewDir, baseUp));
	glm::vec3 up = glm::normalize(glm::cross(right, viewDir));
	//Calculate top_left_corner
	//1) Start at camera's position
	//2) Go lens_half_width along the camera's left (-right) axis
	//3) Go lens_half_height along the camera's up axis
	//4) Go 1 unit along the camera's view_direction axis
	topLeftCorner = this->origin + (lensHalfWidth * (-right)) + (lensHalfHeight * up) + viewDir;
	//Go width of lense along the camera's right axis
	horizontalEnd = lensWidth * right;
	//Go height of lense along the camera's down (-up) axis
	verticalEnd = lensHeight * (-up);
}
