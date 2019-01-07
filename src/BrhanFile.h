#ifndef BRHAN_FILE_H
#define BRHAN_FILE_H

#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"
#include <string>
#include <vector>

struct ModelFromFile
{
	std::string file;
	glm::mat4 translation;
	bool translation_active = false;
	glm::mat4 rotation;
	bool rotation_active = false;
	glm::mat4 scaling;
	bool scaling_active = false;
	
	std::string material = "";
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 reflectance;
	glm::vec3 transmittance;
	bool has_custom_material = false;
};

struct SphereFromFile
{
	glm::vec3 center;
	float radius;
	
	std::string material;
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 reflectance;
	glm::vec3 transmittance;
};

struct BrhanFile
{
	glm::vec3 cameraOrigin;
	unsigned int filmWidth;
	unsigned int filmHeight;
	glm::vec3 cameraTopLeftCorner;
	glm::vec3 cameraHorizontalEnd;
	glm::vec3 cameraVerticalEnd;
	std::vector<ModelFromFile> models;
	std::vector<SphereFromFile> spheres;
	
	BrhanFile(const char* brhanFile);
	void LoadCamera(const std::string& line);
	void AddModel(const std::string& line);
	void AddSphere(const std::string& line);
};

#endif
