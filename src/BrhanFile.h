/*
Copyright (c) 2018-2019 Daniel Fedai Larsen
LICENSE: See end of file for license information.
*/

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
	bool translationActive = false;
	glm::mat4 rotation;
	bool rotationActive = false;
	glm::mat4 scaling;
	bool scalingActive = false;
	
	std::string material = "";
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 reflectance;
	glm::vec3 transmittance;
	bool hasCustomMaterial = false;
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

struct SphericalLightFromFile
{
	glm::vec4 centerAndRadius;
	glm::vec4 emittance;
};

struct BrhanFile
{
	unsigned int filmWidth;
	unsigned int filmHeight;
	float cameraVerticalFOV;
	glm::vec3 cameraOrigin;
	glm::vec3 cameraViewDir;
	glm::vec3 cameraTopLeftCorner;
	glm::vec3 cameraHorizontalEnd;
	glm::vec3 cameraVerticalEnd;
	std::vector<ModelFromFile> models;
	std::vector<SphereFromFile> spheres;
	std::vector<SphericalLightFromFile> sphericalLights;
	
	BrhanFile(const char* brhanFile);
	void LoadCamera(const std::string& line);
	void AddModel(const std::string& line);
	void AddSphericalLight(const std::string& line);
	//void AddSphere(const std::string& line);
};

#endif

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
