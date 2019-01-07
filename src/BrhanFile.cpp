#include "BrhanFile.h"
#include <fstream>
#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/trigonometric.hpp"
#include "Logger.h"
#include <string>

void BrhanFile::LoadCamera(const std::string& line)
{
	static const std::string positionStr = "position";
	glm::vec3 position(0.0f);
	bool foundPosition = false;
	static const std::string viewDirectionStr = "view_direction";
	glm::vec3 viewDirection(0.0f);
	bool foundViewDirection = false;
	static const std::string verticalFOVStr = "vertical_fov";
	float verticalFOV = 0.0f;
	bool foundVerticalFOV = false;
	static const std::string widthStr = "width";
	bool foundWidth = false;
	static const std::string heightStr = "height";
	bool foundHeight = false;
	
	unsigned int index = 7; //Eat "Camera "
	while (index < line.length())
	{
		if (line.compare(index, positionStr.length(), positionStr) == 0)
		{
			index += 9; //Eat "position["
			for (int i = 0; i < 3; i++)
			{
				unsigned int end = index + 1;
				while (line[end] != ' ' && line[end] != ']') { end++; }
				position[i] = std::stof(line.substr(index, end - index));
				index = end + 1; //+1 to eat space
			}
			foundPosition = true;
		}
		else if (line.compare(index, viewDirectionStr.length(), viewDirectionStr) == 0)
		{
			index += 15; //Eat "view_direction["
			for (int i = 0; i < 3; i++)
			{
				unsigned int end = index + 1;
				while (line[end] != ' ' && line[end] != ']') { end++; }
				viewDirection[i] = std::stof(line.substr(index, end - index));
				index = end + 1; //+1 to eat space
			}
			foundViewDirection = true;
		}
		else if (line.compare(index, verticalFOVStr.length(), verticalFOVStr) == 0)
		{
			index += 13; //Eat "view_direction["
			unsigned int end = index + 1;
			while (line[end] != ']') { end++; }
			verticalFOV = std::stof(line.substr(index, end - index));
			index = end + 1; //+1 to eat space
			foundVerticalFOV = true;
		}
		else if (line.compare(index, widthStr.length(), widthStr) == 0)
		{
			index += 6; //Eat "width["
			unsigned int end = index + 1;
			while (line[end] != ']') { end++; }
			filmWidth = std::stoi(line.substr(index, end - index));
			index = end + 1; //+1 to eat space
			foundWidth = true;
		}
		else if (line.compare(index, heightStr.length(), heightStr) == 0)
		{
			index += 7; //Eat "height["
			unsigned int end = index + 1;
			while (line[end] != ']') { end++; }
			filmHeight = std::stoi(line.substr(index, end - index));
			index = end + 1; //+1 to eat space
			foundHeight = true;
		}
		
		index++;
	}
	
	if (!foundPosition)
  	{
  		LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to locate camera position\n");
  	}
  	if (!foundViewDirection)
  	{
  		LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to locate camera view direction\n");
  	}
  	if (!foundVerticalFOV)
  	{
  		LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to locate camera vertical FOV\n");
  	}
  	if (!foundWidth)
  	{
  		LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to locate camera film width\n");
  	}
  	if (!foundHeight)
  	{
  		LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to locate camera film height\n");
  	}
	
	//Assign camera properties
	cameraOrigin = position;
	float cameraAspectRatio = filmWidth / float(filmHeight);
	float theta = (verticalFOV * glm::pi<float>()) / 180.0f; //Convert to radians
	float lensHeight = glm::tan(theta);
	float lensWidth = lensHeight * cameraAspectRatio;
	float lensHalfWidth = lensWidth / 2.0f;
	float lensHalfHeight = lensHeight / 2.0f;
	//Calculate the three vectors that define the camera	
	glm::vec3 baseUp(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::normalize(glm::cross(viewDirection, baseUp));
	glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, viewDirection));
	//Calculate top_left_corner
	//1) Start at camera's position
	//2) Go lens_half_width along the camera's left (-right) axis
	//3) Go lens_half_height along the camera's up axis
	//4) Go 1 unit along the camera's view_direction axis
	cameraTopLeftCorner = position + (lensHalfWidth * (-cameraRight)) + (lensHalfHeight * cameraUp) + viewDirection;
	//Go width of lense along the camera's right axis
	cameraHorizontalEnd = lensWidth * cameraRight;
	//Go height of lense along the camera's down (-up) axis
	cameraVerticalEnd = lensHeight * (-cameraUp);
}

void BrhanFile::AddModel(const std::string& line)
{
	ModelFromFile model;
	static const std::string fileStr = "file";
	bool foundFile = false;
	static const std::string translateStr = "translate";
	bool foundTranslate = false;
	static const std::string rotateStr = "rotate";
	bool foundRotate = false;
	static const std::string scaleStr = "scale";
	bool foundScale = false;
	
	static const std::string materialStr = "material";
	bool foundMaterial = false;
	static const std::string diffuseStr = "diffuse";
	bool foundDiffuse = false;
	static const std::string specularStr = "specular";
	bool foundSpecular = false;
	static const std::string reflectanceStr = "reflectance";
	bool foundReflectance = false;
	static const std::string transmittanceStr = "transmittance";
	bool foundTransmittance = false;
	
	unsigned int index = 6; //Eat "Model "
	while (index < line.length())
	{
		if (line.compare(index, fileStr.length(), fileStr) == 0)
		{
			index += 5; //Eat "file["
			unsigned int end = index + 1;
			while (line[end] != ']') { end++; }
			model.file = line.substr(index, end - index);
			index = end + 1;
			foundFile = true;
		}
		else if (line.compare(index, translateStr.length(), translateStr) == 0)
		{
			index += 10; //Eat "translate["
			glm::vec3 translationVec(0.0f);
			for (int i = 0; i < 3; i++)
			{
				unsigned int end = index + 1;
				while (line[end] != ' ' && line[end] != ']') { end++; }
				translationVec[i] = std::stof(line.substr(index, end - index));
				index = end + 1; //+1 to eat space
			}
			model.translation = glm::translate(glm::mat4(1.0f), translationVec);
			model.translationActive = true;
			foundTranslate = true;
		}
		else if (line.compare(index, rotateStr.length(), rotateStr) == 0)
		{
			index += 7; //Eat "rotate["
			glm::vec3 rotationVec(0.0f);
			for (int i = 0; i < 3; i++)
			{
				unsigned int end = index + 1;
				while (line[end] != ' ' && line[end] != ']') { end++; }
				rotationVec[i] = std::stof(line.substr(index, end - index));
				index = end + 1; //+1 to eat space
			}
			model.rotation = glm::rotate(glm::mat4(1.0f), glm::radians(rotationVec.x), glm::vec3(1.0f, 0.0f, 0.0f));
			model.rotation = glm::rotate(model.rotation, glm::radians(rotationVec.y), glm::vec3(0.0f, 1.0f, 0.0f));
			model.rotation = glm::rotate(model.rotation, glm::radians(rotationVec.z), glm::vec3(0.0f, 0.0f, 1.0f));
			model.rotationActive = true;
			foundRotate = true;
		}
		else if (line.compare(index, scaleStr.length(), scaleStr) == 0)
		{
			index += 6; //Eat "scale["
			glm::vec3 scalingVec(0.0f);
			for (int i = 0; i < 3; i++)
			{
				unsigned int end = index + 1;
				while (line[end] != ' ' && line[end] != ']') { end++; }
				scalingVec[i] = std::stof(line.substr(index, end - index));
				index = end + 1; //+1 to eat space
			}
			model.scaling = glm::scale(glm::mat4(1.0f), scalingVec);
			model.scalingActive = true;
			foundScale = true;
		}
		else if (line.compare(index, materialStr.length(), materialStr) == 0)
		{
			index += 9; //Eat "material["
			unsigned int end = index + 1;
			while (line[end] != ']') { end++; }
			model.material = line.substr(index, end - index);
			model.hasCustomMaterial = true;
			foundMaterial = true;
		}
		else if (line.compare(index, diffuseStr.length(), diffuseStr) == 0)
		{
			index += 8; //Eat "diffuse["
			for (int i = 0; i < 3; i++)
			{
				unsigned int end = index + 1;
				while (line[end] != ' ' && line[end] != ']') { end++; }
				model.diffuse[i] = std::stof(line.substr(index, end - index));
				index = end + 1; //+1 to eat space
			}
			foundDiffuse = true;
		}
		else if (line.compare(index, specularStr.length(), specularStr) == 0)
		{
			index += 9; //Eat "specular["
			for (int i = 0; i < 3; i++)
			{
				unsigned int end = index + 1;
				while (line[end] != ' ' && line[end] != ']') { end++; }
				model.specular[i] = std::stof(line.substr(index, end - index));
				index = end + 1; //+1 to eat space
			}
			foundSpecular = true;
		}
		else if (line.compare(index, reflectanceStr.length(), reflectanceStr) == 0)
		{
			index += 12; //Eat "reflectance["
			for (int i = 0; i < 3; i++)
			{
				unsigned int end = index + 1;
				while (line[end] != ' ' && line[end] != ']') { end++; }
				model.reflectance[i] = std::stof(line.substr(index, end - index));
				index = end + 1; //+1 to eat space
			}
			foundReflectance = true;
		}
		else if (line.compare(index, transmittanceStr.length(), transmittanceStr) == 0)
		{
			index += 14; //Eat "transmittance["
			for (int i = 0; i < 3; i++)
			{
				unsigned int end = index + 1;
				while (line[end] != ' ' && line[end] != ']') { end++; }
				model.transmittance[i] = std::stof(line.substr(index, end - index));
				index = end + 1; //+1 to eat space
			}
			foundTransmittance = true;
		}
		
		index++;
	}
	
	if (!foundFile)
	{
		LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to find model file\n");
	}
	if (!foundTranslate)
	{
		model.translation = glm::mat4(1.0f);
	}
	if (!foundRotate)
	{
		model.rotation = glm::mat4(1.0f);
	}
	if (!foundScale)
	{
		model.scaling = glm::mat4(1.0f);
	}
	if (foundMaterial)
	{	
		if (model.material == "matte" && !foundDiffuse)
		{
			LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to find diffuse spectrum of model %s on line: '%s'\n", model.file.c_str(), line.c_str());
		}
		if (model.material == "mirror" && !foundSpecular)
		{
			LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to find specular spectrum of model %s on line: '%s'\n", model.file.c_str(), line.c_str());
		}
		/*if (model.material == "plastic" && (!foundDiffuse || !foundSpecular))
		{
			LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to find diffuse or specular spectrum of model %s on line: '%s'\n", model.file.c_str(), line.c_str());
		}
		if ((model.material == "copper" || model.material == "gold" || model.material == "aluminium" || model.material == "salt") && !foundSpecular)
		{
			LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to find specular spectrum of model %s on line: '%s'\n", model.file.c_str(), line.c_str());
		}
		if (model.material == "translucent" && !foundTransmittance)
		{
			LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to find transmittance spectrum of model %s on line: '%s'\n", model.file.c_str(), line.c_str());
		}*/
		if ((model.material == "water" || model.material == "glass") && (!foundReflectance || !foundTransmittance))
		{
			LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to find reflectance or transmittance spectrum of model %s on line: '%s'\n", model.file.c_str(), line.c_str());
		}
	}
	
	models.push_back(model);
}

/*void BrhanFile::AddSphere(const std::string& line)
{
	SphereLoad sphere;
	static const std::string center_str = "center";
	bool found_center = false;
	static const std::string radius_str = "radius";
	bool found_radius = false;
	
	static const std::string material_str = "material";
	bool found_material = false;
	static const std::string diffuse_str = "diffuse";
	bool found_diffuse = false;
	static const std::string specular_str = "specular";
	bool found_specular = false;
	static const std::string reflectance_str = "reflectance";
	bool found_reflectance = false;
	static const std::string transmittance_str = "transmittance";
	bool found_transmittance = false;
	
	unsigned int index = 7; //Eat "Sphere "
	while (index < line.length())
	{
		if (line.compare(index, center_str.length(), center_str) == 0)
		{
			index += 7; //Eat "center["
			for (int i = 0; i < 3; i++)
			{
				unsigned int end = index + 1;
				while (line[end] != ' ' && line[end] != ']') { end++; }
				sphere.center[i] = std::stof(line.substr(index, end - index));
				index = end + 1; //+1 to eat space
			}
			found_center = true;
		}
		else if (line.compare(index, radius_str.length(), radius_str) == 0)
		{
			index += 7; //Eat "radius["
			unsigned int end = index + 1;
			while (line[end] != ']') { end++; }
			sphere.radius = std::stof(line.substr(index, end - index));
			found_radius = true;
		}
		else if (line.compare(index, material_str.length(), material_str) == 0)
		{
			index += 9; //Eat "material["
			unsigned int end = index + 1;
			while (line[end] != ']') { end++; }
			sphere.material = line.substr(index, end - index);
			found_material = true;
		}
		else if (line.compare(index, diffuse_str.length(), diffuse_str) == 0)
		{
			index += 8; //Eat "diffuse["
			for (int i = 0; i < 3; i++)
			{
				unsigned int end = index + 1;
				while (line[end] != ' ' && line[end] != ']') { end++; }
				sphere.diffuse[i] = std::stof(line.substr(index, end - index));
				index = end + 1; //+1 to eat space
			}
			found_diffuse = true;
		}
		else if (line.compare(index, specular_str.length(), specular_str) == 0)
		{
			index += 9; //Eat "specular["
			for (int i = 0; i < 3; i++)
			{
				unsigned int end = index + 1;
				while (line[end] != ' ' && line[end] != ']') { end++; }
				sphere.specular[i] = std::stof(line.substr(index, end - index));
				index = end + 1; //+1 to eat space
			}
			found_specular = true;
		}
		else if (line.compare(index, reflectance_str.length(), reflectance_str) == 0)
		{
			index += 12; //Eat "reflectance["
			for (int i = 0; i < 3; i++)
			{
				unsigned int end = index + 1;
				while (line[end] != ' ' && line[end] != ']') { end++; }
				sphere.reflectance[i] = std::stof(line.substr(index, end - index));
				index = end + 1; //+1 to eat space
			}
			found_reflectance = true;
		}
		else if (line.compare(index, transmittance_str.length(), transmittance_str) == 0)
		{
			index += 14; //Eat "transmittance["
			for (int i = 0; i < 3; i++)
			{
				unsigned int end = index + 1;
				while (line[end] != ' ' && line[end] != ']') { end++; }
				sphere.transmittance[i] = std::stof(line.substr(index, end - index));
				index = end + 1; //+1 to eat space
			}
			found_transmittance = true;
		}
	
		index++;
	}
	
	if (!found_center)
	{
		LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to find center of sphere\n");
	}
	if (!found_radius)
	{
		LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to find radius of sphere\n");
	}
	if (!found_material)
	{
		LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to find material of sphere\n");
	}
	else
	{
		if (sphere.material == "matte" && !found_diffuse)
		{
			LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to find diffuse spectrum of sphere on line: '%s'\n", line.c_str());
		}
		if (sphere.material == "mirror" && !found_specular)
		{
			LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to find specular spectrum of sphere on line: '%s'\n", line.c_str());
		}
		if (sphere.material == "plastic" && (!found_specular || !found_specular))
		{
			LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to find diffuse or specular spectrum of sphere on line: '%s'\n", line.c_str());
		}
		if ((sphere.material == "copper" || sphere.material == "gold" || sphere.material == "aluminium" || sphere.material == "salt") && !found_specular)
		{
			LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to find specular spectrum of sphere on line: '%s'\n", line.c_str());
		}
		if (sphere.material == "translucent" && !found_transmittance)
		{
			LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to find transmittance spectrum of sphere on line: '%s'\n", line.c_str());
		}
		if (sphere.material == "water" && (!found_reflectance || !found_transmittance))
		{
			LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to find reflectance or transmittance spectrum of sphere on line: '%s'\n", line.c_str());
		}
		if (sphere.material == "glass" && (!found_reflectance || !found_transmittance))
		{
			LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to find reflectance or transmittance spectrum of sphere on line: '%s'\n", line.c_str());
		}
	}
	
	spheres.push_back(sphere);
}*/

BrhanFile::BrhanFile(const char* brhanFile)
{
	if (strcmp(brhanFile, "") == 0)
	{
		printf("The only input parameter needed is the path to the scene description file\n");
		exit(EXIT_FAILURE);
	}
	
	std::ifstream file(brhanFile);
  	if (!file.is_open())
  	{
  		LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to open file %s\n", brhanFile);
  	}
  	
  	//Parameters
  	static const std::string cameraStr = "Camera";
  	bool foundCamera = false;
  	static const std::string modelStr = "Model";
  	static const std::string sphereStr = "Sphere";
  	
  	std::string line;
  	while (std::getline(file, line))
  	{
  		if (line.length() == 0) { continue; } //Empty
  		else if (line.compare(0, cameraStr.length(), cameraStr) == 0)
  		{
  			LoadCamera(line);
  			foundCamera = true;
  		}
  		else if (line.compare(0, modelStr.length(), modelStr) == 0)
  		{
  			AddModel(line);
  		}
  		else if (line.compare(0, sphereStr.length(), sphereStr) == 0)
  		{
  			//AddSphere(line);
  		}
  	}
  	
  	file.close();
  	
  	if (!foundCamera)
  	{
  		LOG_ERROR(false, __FILE__, __FUNCTION__, __LINE__, "Failed to load camera from %s\n", brhanFile);
  	}
}
