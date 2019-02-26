#include "BrhanFile.h"
#include "glm/gtc/constants.hpp"
#include "glm/geometric.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/trigonometric.hpp"
#include "glm/vec3.hpp"
#include "shaders/include/Defines.glsl"
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "VulkanApp.h"

VulkanApp* vulkanApp;
uint32_t blurVariable;

void MouseCallback(GLFWwindow * window, double xpos, double ypos)
{
	const double horizontalRotationSpeed = glm::two_pi<double>() / 2.0;
	const double verticalRotationSpeed = glm::two_pi<double>() / 2.0;
	// Center xpos and ypos as they are relative to the top-left window corner
	// Range goes from [0,1] to [-0.5, 0.5]
	xpos = xpos / double(vulkanApp->camera.filmWidth) - 0.5;
	ypos = ypos / double(vulkanApp->camera.filmHeight) - 0.5;
	
	glm::vec3 cameraRight = glm::normalize(glm::cross(vulkanApp->camera.viewDir, vulkanApp->camera.up));
	// Get rotation for camera's view direction around Y-axis according to difference
	// in x-position
	double diff = xpos - vulkanApp->lastMouseX;
	glm::mat4x4 rot = glm::rotate(glm::mat4(1.0f), float(diff * horizontalRotationSpeed), -vulkanApp->camera.up);
	// Get rotation for camera's view direction around X-axis according to difference
	// in y-position
	diff = ypos - vulkanApp->lastMouseY;
	rot = glm::rotate(rot, float(diff * verticalRotationSpeed), cameraRight);
	
	// Update camera's view direction
	vulkanApp->camera.viewDir = glm::normalize(glm::vec3(rot * glm::vec4(vulkanApp->camera.viewDir, 1.0f)));
	
	vulkanApp->lastMouseX = xpos;
	vulkanApp->lastMouseY = ypos;
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	glm::vec3 cameraRight = glm::normalize(glm::cross(vulkanApp->camera.viewDir, vulkanApp->camera.up));
	glm::vec3 cameraOriginDelta(0.0f);
	glm::vec3 cameraViewDirDelta(0.0f);
	switch (key)
	{
		case GLFW_KEY_W:
			switch (action)
			{
				case GLFW_PRESS:
    				cameraOriginDelta += 0.1f * vulkanApp->camera.viewDir;
					break;
				case GLFW_REPEAT:
    				cameraOriginDelta += 0.1f * vulkanApp->camera.viewDir;
					break;
				default:
					break;
			}
			break;
		case GLFW_KEY_S:
			switch (action)
			{
				case GLFW_PRESS:
    				cameraOriginDelta -= 0.1f * vulkanApp->camera.viewDir;
					break;
				case GLFW_REPEAT:
    				cameraOriginDelta -= 0.1f * vulkanApp->camera.viewDir;
					break;
				default:
					break;
			}
			break;
		case GLFW_KEY_D:
			switch (action)
			{
				case GLFW_PRESS:
					cameraOriginDelta += 0.1f * cameraRight;
					break;
				case GLFW_REPEAT:
					cameraOriginDelta += 0.1f * cameraRight;
					break;
				default:
					break;
			}
			break;
		case GLFW_KEY_A:
			switch (action)
			{
				case GLFW_PRESS:
					cameraOriginDelta -= 0.1f * cameraRight;
					break;
				case GLFW_REPEAT:
					cameraOriginDelta -= 0.1f * cameraRight;
					break;
				default:
					break;
			}
			break;
		case GLFW_KEY_B:
			switch (action)
			{
				case GLFW_PRESS:
					blurVariable ^= 1;
					break;
				default:
					break;
			}
		default:
			break;
	}
    
    vulkanApp->camera.origin += cameraOriginDelta;
}

void Raytrace(const char* brhanFile)
{
	BrhanFile sceneFile(brhanFile);

	std::vector<const char*> validationLayerNames = {
		"VK_LAYER_LUNARG_standard_validation"
	};
	std::vector<const char*> extensionNames = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_NV_RAY_TRACING_EXTENSION_NAME
	};
	VulkanAppCreateInfo vkAppInfo = {};
	vkAppInfo.graphicsApp = VK_TRUE;
	vkAppInfo.windowWidth = sceneFile.filmWidth;
	vkAppInfo.windowHeight = sceneFile.filmHeight;
	vkAppInfo.windowName = "Vulkan RTX";
	vkAppInfo.appName = "Vulkan RTX";
	vkAppInfo.engineName = "VRTX";
	vkAppInfo.validationLayerCount = validationLayerNames.size();
	vkAppInfo.validationLayerNames = validationLayerNames.data();
	vkAppInfo.extensionCount = extensionNames.size();
	vkAppInfo.extensionNames = extensionNames.data();
	vkAppInfo.maxFramesInFlight = 2;
	vulkanApp = new VulkanApp(&vkAppInfo);
	VulkanApp& vkApp = *vulkanApp;
	
	////////////////////////////
	///////////MOUSE////////////
	////////////////////////////
	vkApp.lastMouseX = 0.0;
	vkApp.lastMouseY = 0.0;
	
	////////////////////////////
	///////////CAMERA///////////
	////////////////////////////
	vkApp.camera = Camera(sceneFile.filmWidth, sceneFile.filmHeight, sceneFile.cameraVerticalFOV, sceneFile.cameraOrigin, sceneFile.cameraViewDir);
	std::vector<float> cameraData = {
		vkApp.camera.origin.x, vkApp.camera.origin.y, vkApp.camera.origin.z, 0.0f,
		vkApp.camera.topLeftCorner.x, vkApp.camera.topLeftCorner.y, vkApp.camera.topLeftCorner.z, 0.0f,
		vkApp.camera.horizontalEnd.x, vkApp.camera.horizontalEnd.y, vkApp.camera.horizontalEnd.z, 0.0f,
		vkApp.camera.verticalEnd.x, vkApp.camera.verticalEnd.y, vkApp.camera.verticalEnd.z, 0.0f
	};
	VkDeviceSize cameraBufferSize = cameraData.size() * sizeof(float);
	VkBuffer cameraBuffer;
	VkDeviceMemory cameraBufferMemory;
	vkApp.CreateHostVisibleBuffer(cameraBufferSize, (void*)(cameraData.data()), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &cameraBuffer, &cameraBufferMemory);
	
	////////////////////////////
	///////////LIGHTS///////////
	////////////////////////////
	// See shaders/include/Datalayouts.glsl for structure layout
	assert(sceneFile.sphericalLights.size() > 0);
	const int numFloatsPerLight = 8;
	std::vector<float> lights;
	for (SphericalLightFromFile sL : sceneFile.sphericalLights)
	{
		lights.push_back(sL.centerAndRadius.x);
		lights.push_back(sL.centerAndRadius.y);
		lights.push_back(sL.centerAndRadius.z);
		lights.push_back(sL.centerAndRadius.w);
		lights.push_back(sL.emittance.x);
		lights.push_back(sL.emittance.y);
		lights.push_back(sL.emittance.z);
		lights.push_back(sL.emittance.w);
	}
	VkDeviceSize lightsBufferSize = lights.size() * sizeof(float);
	VkBuffer lightsBuffer;
	VkDeviceMemory lightsBufferMemory;
	vkApp.CreateHostVisibleBuffer(lightsBufferSize, (void*)(lights.data()), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &lightsBuffer, &lightsBufferMemory);
	
	////////////////////////////
	/////////OTHER DATA/////////
	////////////////////////////
	// See shaders/include/Datalayouts.glsl for structure layout
	size_t otherDataNumBytes = sizeof(int);
	char* otherData = new char[otherDataNumBytes];
	*(int*)(otherData) = int(lights.size() / numFloatsPerLight);
	VkDeviceSize otherDataBufferSize = otherDataNumBytes;
	VkBuffer otherDataBuffer;
	VkDeviceMemory otherDataBufferMemory;
	vkApp.CreateDeviceBuffer(otherDataBufferSize, (void*)(otherData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &otherDataBuffer, &otherDataBufferMemory);
	delete[] otherData;
	
	////////////////////////////
	//////////GEOMETRY//////////
	////////////////////////////
	std::vector<Mesh> meshes;
	for (const ModelFromFile& mff : sceneFile.models)
	{
		vkApp.LoadMesh(mff, &meshes);
	}
	std::vector<std::vector<float>> geometryData;
	for (Mesh& mesh : meshes)
	{
		geometryData.push_back(mesh.vertices);
	}
	
	std::vector<float> perMeshAttributeData;
	std::vector<float> perVertexAttributeData;
	std::vector<uint32_t> customIDToAttributeArrayIndex;
	vkApp.BuildColorAndAttributeData(meshes, &perMeshAttributeData, &perVertexAttributeData, &customIDToAttributeArrayIndex);
	
	VkDeviceSize perMeshAttributeBufferSize = perMeshAttributeData.size() * sizeof(float);
	VkBuffer perMeshAttributeBuffer;
	VkDeviceMemory perMeshAttributeBufferMemory;
	vkApp.CreateDeviceBuffer(perMeshAttributeBufferSize, (void*)(perMeshAttributeData.data()), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &perMeshAttributeBuffer, &perMeshAttributeBufferMemory);
	perMeshAttributeData.resize(0);
	
	VkDeviceSize perVertexAttributeBufferSize = perVertexAttributeData.size() * sizeof(float);
	VkBuffer perVertexAttributeBuffer;
	VkDeviceMemory perVertexAttributeBufferMemory;
	vkApp.CreateDeviceBuffer(perVertexAttributeBufferSize, (void*)(perVertexAttributeData.data()), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &perVertexAttributeBuffer, &perVertexAttributeBufferMemory);
	perVertexAttributeData.resize(0);
	
	VkDeviceSize customIDToAttributeArrayIndexBufferSize = customIDToAttributeArrayIndex.size() * sizeof(uint32_t);
	VkBuffer customIDToAttributeArrayIndexBuffer;
	VkDeviceMemory customIDToAttributeArrayIndexBufferMemory;
	vkApp.CreateDeviceBuffer(customIDToAttributeArrayIndexBufferSize, (void*)(customIDToAttributeArrayIndex.data()), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &customIDToAttributeArrayIndexBuffer, &customIDToAttributeArrayIndexBufferMemory);
	customIDToAttributeArrayIndex.resize(0);
	
	////////////////////////////
	//////////TEXTURE///////////
	////////////////////////////
	VkSampler defaultSampler;
	vkApp.CreateDefaultSampler(&defaultSampler, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
	VulkanTexture dummyTexture;
	vkApp.CreateDummyImage(&dummyTexture.image, &dummyTexture.imageMemory, &dummyTexture.imageView);
	
	////////////////////////////
	///ACCELERATION STRUCTURE///
	////////////////////////////
	VulkanAccelerationStructure accStruct;
	vkApp.CreateVulkanAccelerationStructure(geometryData, &accStruct);
	vkApp.BuildAccelerationStructure(accStruct);
	geometryData.resize(0);
	
	////////////////////////////
	////RAY TRACING PIPELINE////
	////////////////////////////
	const uint32_t numShaders = 5;
	std::vector<VkShaderModule> shaderModules(numShaders);
	const uint32_t rayGenShaderIndex = 0;
	const uint32_t primaryCHitShaderIndex = 1;
	const uint32_t secondaryCHitShaderIndex = 2;
	const uint32_t primaryMissShaderIndex = 3;
	const uint32_t secondaryMissShaderIndex = 4;
	vkApp.CreateShaderModule("src/shaders/out/primary_rgen.spv", &shaderModules[rayGenShaderIndex]);
	vkApp.CreateShaderModule("src/shaders/out/primary_rchit.spv", &shaderModules[primaryCHitShaderIndex]);
	vkApp.CreateShaderModule("src/shaders/out/secondary_rchit.spv", &shaderModules[secondaryCHitShaderIndex]);
	vkApp.CreateShaderModule("src/shaders/out/primary_rmiss.spv", &shaderModules[primaryMissShaderIndex]);
	vkApp.CreateShaderModule("src/shaders/out/secondary_rmiss.spv", &shaderModules[secondaryMissShaderIndex]);
	
	std::vector<VkPipelineShaderStageCreateInfo> rayTracingShaderStageInfos(numShaders);
	rayTracingShaderStageInfos[rayGenShaderIndex].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	rayTracingShaderStageInfos[rayGenShaderIndex].pNext = NULL;
	rayTracingShaderStageInfos[rayGenShaderIndex].flags = 0;
	rayTracingShaderStageInfos[rayGenShaderIndex].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
	rayTracingShaderStageInfos[rayGenShaderIndex].module = shaderModules[rayGenShaderIndex];
	rayTracingShaderStageInfos[rayGenShaderIndex].pName = "main";
	rayTracingShaderStageInfos[rayGenShaderIndex].pSpecializationInfo = NULL;
	rayTracingShaderStageInfos[primaryCHitShaderIndex].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	rayTracingShaderStageInfos[primaryCHitShaderIndex].pNext = NULL;
	rayTracingShaderStageInfos[primaryCHitShaderIndex].flags = 0;
	rayTracingShaderStageInfos[primaryCHitShaderIndex].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	rayTracingShaderStageInfos[primaryCHitShaderIndex].module = shaderModules[primaryCHitShaderIndex];
	rayTracingShaderStageInfos[primaryCHitShaderIndex].pName = "main";
	rayTracingShaderStageInfos[primaryCHitShaderIndex].pSpecializationInfo = NULL;
	rayTracingShaderStageInfos[secondaryCHitShaderIndex].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	rayTracingShaderStageInfos[secondaryCHitShaderIndex].pNext = NULL;
	rayTracingShaderStageInfos[secondaryCHitShaderIndex].flags = 0;
	rayTracingShaderStageInfos[secondaryCHitShaderIndex].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	rayTracingShaderStageInfos[secondaryCHitShaderIndex].module = shaderModules[secondaryCHitShaderIndex];
	rayTracingShaderStageInfos[secondaryCHitShaderIndex].pName = "main";
	rayTracingShaderStageInfos[secondaryCHitShaderIndex].pSpecializationInfo = NULL;
	rayTracingShaderStageInfos[primaryMissShaderIndex].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	rayTracingShaderStageInfos[primaryMissShaderIndex].pNext = NULL;
	rayTracingShaderStageInfos[primaryMissShaderIndex].flags = 0;
	rayTracingShaderStageInfos[primaryMissShaderIndex].stage = VK_SHADER_STAGE_MISS_BIT_NV;
	rayTracingShaderStageInfos[primaryMissShaderIndex].module = shaderModules[primaryMissShaderIndex];
	rayTracingShaderStageInfos[primaryMissShaderIndex].pName = "main";
	rayTracingShaderStageInfos[primaryMissShaderIndex].pSpecializationInfo = NULL;
	rayTracingShaderStageInfos[secondaryMissShaderIndex].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	rayTracingShaderStageInfos[secondaryMissShaderIndex].pNext = NULL;
	rayTracingShaderStageInfos[secondaryMissShaderIndex].flags = 0;
	rayTracingShaderStageInfos[secondaryMissShaderIndex].stage = VK_SHADER_STAGE_MISS_BIT_NV;
	rayTracingShaderStageInfos[secondaryMissShaderIndex].module = shaderModules[secondaryMissShaderIndex];
	rayTracingShaderStageInfos[secondaryMissShaderIndex].pName = "main";
	rayTracingShaderStageInfos[secondaryMissShaderIndex].pSpecializationInfo = NULL;
	
	std::vector<VkRayTracingShaderGroupCreateInfoNV> rayTracingGroupInfos(numShaders);
	rayTracingGroupInfos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
	rayTracingGroupInfos[0].pNext = NULL;
	rayTracingGroupInfos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
	rayTracingGroupInfos[0].generalShader = rayGenShaderIndex;
	rayTracingGroupInfos[0].closestHitShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[0].anyHitShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[0].intersectionShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
	rayTracingGroupInfos[1].pNext = NULL;
	rayTracingGroupInfos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
	rayTracingGroupInfos[1].generalShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[1].closestHitShader = primaryCHitShaderIndex;
	rayTracingGroupInfos[1].anyHitShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[1].intersectionShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[2].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
	rayTracingGroupInfos[2].pNext = NULL;
	rayTracingGroupInfos[2].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
	rayTracingGroupInfos[2].generalShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[2].closestHitShader = secondaryCHitShaderIndex;
	rayTracingGroupInfos[2].anyHitShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[2].intersectionShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[3].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
	rayTracingGroupInfos[3].pNext = NULL;
	rayTracingGroupInfos[3].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
	rayTracingGroupInfos[3].generalShader = primaryMissShaderIndex;
	rayTracingGroupInfos[3].closestHitShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[3].anyHitShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[3].intersectionShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[4].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
	rayTracingGroupInfos[4].pNext = NULL;
	rayTracingGroupInfos[4].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
	rayTracingGroupInfos[4].generalShader = secondaryMissShaderIndex;
	rayTracingGroupInfos[4].closestHitShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[4].anyHitShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[4].intersectionShader = VK_SHADER_UNUSED_NV;
	
	const uint32_t numDescriptorSets = 2;
	std::vector<VkDescriptorSetLayoutCreateInfo> descriptorSetLayoutInfos(numDescriptorSets);
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings0(DESCRIPTOR_SET_0_NUM_BINDINGS);
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings1(DESCRIPTOR_SET_1_NUM_BINDINGS);
	std::vector<VkDescriptorSetLayout> rayTracingPipelineDescriptorSetLayouts(numDescriptorSets);
	
	//Desriptor set 0
	VkDescriptorSetLayoutBinding& accelerationStructureDescriptorSetLayoutBinding = descriptorSetLayoutBindings0[ACCELERATION_STRUCTURE_NV_BINDING_LOCATION];
	accelerationStructureDescriptorSetLayoutBinding.binding = ACCELERATION_STRUCTURE_NV_BINDING_LOCATION;
	accelerationStructureDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
	accelerationStructureDescriptorSetLayoutBinding.descriptorCount = 1;
	accelerationStructureDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;
	accelerationStructureDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
	VkDescriptorSetLayoutBinding& rayTracingImageDescriptorSetLayoutBinding = descriptorSetLayoutBindings0[RESULT_IMAGE_BINDING_LOCATION];
	rayTracingImageDescriptorSetLayoutBinding.binding = RESULT_IMAGE_BINDING_LOCATION;
	rayTracingImageDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	rayTracingImageDescriptorSetLayoutBinding.descriptorCount = 1;
	rayTracingImageDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;
	rayTracingImageDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
	VkDescriptorSetLayoutBinding& rayTracingOcclusionImageDescriptorSetLayoutBinding = descriptorSetLayoutBindings0[OCCLUSION_IMAGE_BINDING_LOCATION];
	rayTracingOcclusionImageDescriptorSetLayoutBinding.binding = OCCLUSION_IMAGE_BINDING_LOCATION;
	rayTracingOcclusionImageDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	rayTracingOcclusionImageDescriptorSetLayoutBinding.descriptorCount = 1;
	rayTracingOcclusionImageDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;
	rayTracingOcclusionImageDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
	VkDescriptorSetLayoutBinding& cameraDescriptorSetLayoutBinding = descriptorSetLayoutBindings0[CAMERA_BUFFER_BINDING_LOCATION];
	cameraDescriptorSetLayoutBinding.binding = CAMERA_BUFFER_BINDING_LOCATION;
	cameraDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	cameraDescriptorSetLayoutBinding.descriptorCount = 1;
	cameraDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;
	cameraDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
	VkDescriptorSetLayoutBinding& lightsDescriptorSetLayoutBinding = descriptorSetLayoutBindings0[LIGHTS_BUFFER_BINDING_LOCATION];
	lightsDescriptorSetLayoutBinding.binding = LIGHTS_BUFFER_BINDING_LOCATION;
	lightsDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	lightsDescriptorSetLayoutBinding.descriptorCount = 1;
	lightsDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;
	lightsDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
	VkDescriptorSetLayoutBinding& otherDataDescriptorSetLayoutBinding = descriptorSetLayoutBindings0[OTHER_DATA_BUFFER_BINDING_LOCATION];
	otherDataDescriptorSetLayoutBinding.binding = OTHER_DATA_BUFFER_BINDING_LOCATION;
	otherDataDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	otherDataDescriptorSetLayoutBinding.descriptorCount = 1;
	otherDataDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;
	otherDataDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
	VkDescriptorSetLayoutCreateInfo& descriptorSetLayoutInfo0 = descriptorSetLayoutInfos[0];
	descriptorSetLayoutInfo0.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo0.pNext = NULL;
	descriptorSetLayoutInfo0.flags = 0;
	descriptorSetLayoutInfo0.bindingCount = descriptorSetLayoutBindings0.size();
	descriptorSetLayoutInfo0.pBindings = descriptorSetLayoutBindings0.data();
	CHECK_VK_RESULT(vkCreateDescriptorSetLayout(vkApp.vkDevice, &descriptorSetLayoutInfo0, NULL, &rayTracingPipelineDescriptorSetLayouts[0]))
	
	//Descriptor set 1
	VkDescriptorSetLayoutBinding& customIDToAttributeArrayIndexDescriptorSetLayoutBinding = descriptorSetLayoutBindings1[CUSTOM_ID_TO_ATTRIBUTE_ARRAY_INDEX_BUFFER_BINDING_LOCATION];
	customIDToAttributeArrayIndexDescriptorSetLayoutBinding.binding = CUSTOM_ID_TO_ATTRIBUTE_ARRAY_INDEX_BUFFER_BINDING_LOCATION;
	customIDToAttributeArrayIndexDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	customIDToAttributeArrayIndexDescriptorSetLayoutBinding.descriptorCount = 1;
	customIDToAttributeArrayIndexDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	customIDToAttributeArrayIndexDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
	VkDescriptorSetLayoutBinding& perMeshAttributesDescriptorSetLayoutBinding = descriptorSetLayoutBindings1[PER_MESH_ATTRIBUTES_BINDING_LOCATION];
	perMeshAttributesDescriptorSetLayoutBinding.binding = PER_MESH_ATTRIBUTES_BINDING_LOCATION;
	perMeshAttributesDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	perMeshAttributesDescriptorSetLayoutBinding.descriptorCount = 1;
	perMeshAttributesDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	perMeshAttributesDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
	VkDescriptorSetLayoutBinding& perVertexAttributesDescriptorSetLayoutBinding = descriptorSetLayoutBindings1[PER_VERTEX_ATTRIBUTES_BINDING_LOCATION];
	perVertexAttributesDescriptorSetLayoutBinding.binding = PER_VERTEX_ATTRIBUTES_BINDING_LOCATION;
	perVertexAttributesDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	perVertexAttributesDescriptorSetLayoutBinding.descriptorCount = 1;
	perVertexAttributesDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	perVertexAttributesDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
	VkDescriptorSetLayoutBinding& diffuseImageDescriptorSetLayoutBinding = descriptorSetLayoutBindings1[DIFFUSE_TEXTURES_BINDING_LOCATION];
	diffuseImageDescriptorSetLayoutBinding.binding = DIFFUSE_TEXTURES_BINDING_LOCATION;
	diffuseImageDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	diffuseImageDescriptorSetLayoutBinding.descriptorCount = meshes.size();
	diffuseImageDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	diffuseImageDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
	VkDescriptorSetLayoutBinding& specularImageDescriptorSetLayoutBinding = descriptorSetLayoutBindings1[SPECULAR_TEXTURES_BINDING_LOCATION];
	specularImageDescriptorSetLayoutBinding.binding = SPECULAR_TEXTURES_BINDING_LOCATION;
	specularImageDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	specularImageDescriptorSetLayoutBinding.descriptorCount = meshes.size();
	specularImageDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	specularImageDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
	VkDescriptorSetLayoutCreateInfo& descriptorSetLayoutInfo1 = descriptorSetLayoutInfos[1];
	descriptorSetLayoutInfo1.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo1.pNext = NULL;
	descriptorSetLayoutInfo1.flags = 0;
	descriptorSetLayoutInfo1.bindingCount = descriptorSetLayoutBindings1.size();
	descriptorSetLayoutInfo1.pBindings = descriptorSetLayoutBindings1.data();
	CHECK_VK_RESULT(vkCreateDescriptorSetLayout(vkApp.vkDevice, &descriptorSetLayoutInfo1, NULL, &rayTracingPipelineDescriptorSetLayouts[1]))
	
	//Pipeline
	VkPipelineLayoutCreateInfo rayTracingPipelineLayoutInfo = {};
	rayTracingPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	rayTracingPipelineLayoutInfo.pNext = NULL;
	rayTracingPipelineLayoutInfo.flags = 0;
	rayTracingPipelineLayoutInfo.setLayoutCount = rayTracingPipelineDescriptorSetLayouts.size();
	rayTracingPipelineLayoutInfo.pSetLayouts = rayTracingPipelineDescriptorSetLayouts.data();
	rayTracingPipelineLayoutInfo.pushConstantRangeCount = 0;
	rayTracingPipelineLayoutInfo.pPushConstantRanges = NULL;
	VkPipelineLayout rayTracingPipelineLayout;
	CHECK_VK_RESULT(vkCreatePipelineLayout(vkApp.vkDevice, &rayTracingPipelineLayoutInfo, NULL, &rayTracingPipelineLayout))
	
	VkRayTracingPipelineCreateInfoNV rayTracingPipelineInfo = {};
	rayTracingPipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
	rayTracingPipelineInfo.pNext = NULL;
	rayTracingPipelineInfo.flags = 0;
	rayTracingPipelineInfo.stageCount = rayTracingShaderStageInfos.size();
	rayTracingPipelineInfo.pStages = rayTracingShaderStageInfos.data();
	rayTracingPipelineInfo.groupCount = rayTracingGroupInfos.size();
	rayTracingPipelineInfo.pGroups = rayTracingGroupInfos.data();
	rayTracingPipelineInfo.maxRecursionDepth = 1;
	rayTracingPipelineInfo.layout = rayTracingPipelineLayout;
	rayTracingPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	rayTracingPipelineInfo.basePipelineIndex = -1;
	VkPipeline rayTracingPipeline;
	CHECK_VK_RESULT(vkCreateRayTracingPipelinesNV(vkApp.vkDevice, VK_NULL_HANDLE, 1, &rayTracingPipelineInfo, NULL, &rayTracingPipeline))
	
	////////////////////////////
	////SHADER BINDING TABLE////
	////////////////////////////
	VkPhysicalDeviceRayTracingPropertiesNV physicalDeviceRayTracingProperties = {};
	physicalDeviceRayTracingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV;
	physicalDeviceRayTracingProperties.pNext = NULL;
	VkPhysicalDeviceProperties2 physicalDeviceProperties2 = {};
	physicalDeviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	physicalDeviceProperties2.pNext = &physicalDeviceRayTracingProperties;
	vkGetPhysicalDeviceProperties2(vkApp.vkPhysicalDevice, &physicalDeviceProperties2);
	VkBuffer shaderBindingTableBuffer;
	VkDeviceMemory shaderBindindTableBufferMemory;
	VkDeviceSize shaderBindingTableBufferSize = physicalDeviceRayTracingProperties.shaderGroupHandleSize * rayTracingGroupInfos.size();
	std::vector<char> shaderGroupHandles(shaderBindingTableBufferSize);
	CHECK_VK_RESULT(vkGetRayTracingShaderGroupHandlesNV(vkApp.vkDevice, rayTracingPipeline, 0, rayTracingGroupInfos.size(), shaderBindingTableBufferSize, (void*)(shaderGroupHandles.data())))
	vkApp.CreateDeviceBuffer(shaderBindingTableBufferSize, (void*)(shaderGroupHandles.data()), VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, &shaderBindingTableBuffer, &shaderBindindTableBufferMemory);
	
	//Ray tracing image
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext = NULL;
	imageInfo.flags = 0;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageInfo.extent.width = vkApp.vkSurfaceExtent.width;
	imageInfo.extent.height = vkApp.vkSurfaceExtent.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	/* IGNORED
	imageInfo.queueFamilyIndexCount = 
    imageInfo.pQueueFamilyIndices = */
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImage rayTracingImage;
	CHECK_VK_RESULT(vkCreateImage(vkApp.vkDevice, &imageInfo, NULL, &rayTracingImage))
	
	VkMemoryRequirements rayTracingImageMemoryRequirements;
	vkGetImageMemoryRequirements(vkApp.vkDevice, rayTracingImage, &rayTracingImageMemoryRequirements);
	VkMemoryAllocateInfo rayTracingImageAllocateInfo = {};
	rayTracingImageAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	rayTracingImageAllocateInfo.allocationSize = rayTracingImageMemoryRequirements.size;
	rayTracingImageAllocateInfo.memoryTypeIndex = vkApp.FindMemoryType(rayTracingImageMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VkDeviceMemory rayTracingImageMemory;
	CHECK_VK_RESULT(vkAllocateMemory(vkApp.vkDevice, &rayTracingImageAllocateInfo, NULL, &rayTracingImageMemory))
	vkBindImageMemory(vkApp.vkDevice, rayTracingImage, rayTracingImageMemory, 0);
	
	VkImageViewCreateInfo rayTracingImageViewCreateInfo;
    rayTracingImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    rayTracingImageViewCreateInfo.pNext = NULL;
    rayTracingImageViewCreateInfo.flags = 0;
    rayTracingImageViewCreateInfo.image = rayTracingImage;
    rayTracingImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    rayTracingImageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    rayTracingImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	rayTracingImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	rayTracingImageViewCreateInfo.subresourceRange.levelCount = 1;
	rayTracingImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	rayTracingImageViewCreateInfo.subresourceRange.layerCount = 1;
    rayTracingImageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	VkImageView rayTracingImageView;
    CHECK_VK_RESULT(vkCreateImageView(vkApp.vkDevice, &rayTracingImageViewCreateInfo, NULL, &rayTracingImageView))
    
    //Transition ray tracing image layout
	vkApp.TransitionImageLayoutSingle(rayTracingImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0);
    
    //Ray tracing OCCLUSION image
	imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	VkImage rayTracingOcclusionImage;
	CHECK_VK_RESULT(vkCreateImage(vkApp.vkDevice, &imageInfo, NULL, &rayTracingOcclusionImage))
	
	VkMemoryRequirements rayTracingOcclusionImageMemoryRequirements;
	vkGetImageMemoryRequirements(vkApp.vkDevice, rayTracingOcclusionImage, &rayTracingOcclusionImageMemoryRequirements);
	VkMemoryAllocateInfo rayTracingOcclusionImageAllocateInfo = {};
	rayTracingOcclusionImageAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	rayTracingOcclusionImageAllocateInfo.allocationSize = rayTracingOcclusionImageMemoryRequirements.size;
	rayTracingOcclusionImageAllocateInfo.memoryTypeIndex = vkApp.FindMemoryType(rayTracingOcclusionImageMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VkDeviceMemory rayTracingOcclusionImageMemory;
	CHECK_VK_RESULT(vkAllocateMemory(vkApp.vkDevice, &rayTracingOcclusionImageAllocateInfo, NULL, &rayTracingOcclusionImageMemory))
	vkBindImageMemory(vkApp.vkDevice, rayTracingOcclusionImage, rayTracingOcclusionImageMemory, 0);
    
    VkImageViewCreateInfo rayTracingOcclusionImageViewCreateInfo;
    rayTracingOcclusionImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    rayTracingOcclusionImageViewCreateInfo.pNext = NULL;
    rayTracingOcclusionImageViewCreateInfo.flags = 0;
    rayTracingOcclusionImageViewCreateInfo.image = rayTracingOcclusionImage;
    rayTracingOcclusionImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    rayTracingOcclusionImageViewCreateInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    rayTracingOcclusionImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	rayTracingOcclusionImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	rayTracingOcclusionImageViewCreateInfo.subresourceRange.levelCount = 1;
	rayTracingOcclusionImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	rayTracingOcclusionImageViewCreateInfo.subresourceRange.layerCount = 1;
    rayTracingOcclusionImageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	VkImageView rayTracingOcclusionImageView;
    CHECK_VK_RESULT(vkCreateImageView(vkApp.vkDevice, &rayTracingOcclusionImageViewCreateInfo, NULL, &rayTracingOcclusionImageView))
    
    //Transition ray tracing OCCLUSION image layout
	vkApp.TransitionImageLayoutSingle(rayTracingOcclusionImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0);
	
	//Descriptor pool
	const uint32_t numTexturesPerMesh = 4;
	std::vector<VkDescriptorPoolSize> poolSizes = {
		{ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, uint32_t(meshes.size() * numTexturesPerMesh) }
	};
	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.pNext = NULL;
	descriptorPoolInfo.flags = 0;
	descriptorPoolInfo.maxSets = numDescriptorSets;
	descriptorPoolInfo.poolSizeCount = poolSizes.size();
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	VkDescriptorPool descriptorPool;
	CHECK_VK_RESULT(vkCreateDescriptorPool(vkApp.vkDevice, &descriptorPoolInfo, NULL, &descriptorPool))
	
	//Descriptor sets
	std::vector<VkDescriptorSet> descriptorSets(numDescriptorSets);
	
	//Descriptor set 0
	VkDescriptorSet& descriptorSet0 = descriptorSets[0];
    std::vector<VkWriteDescriptorSet> descriptorSet0Writes(DESCRIPTOR_SET_0_NUM_BINDINGS);
    
	VkDescriptorSetAllocateInfo descriptorSet0AllocateInfo = {};
	descriptorSet0AllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSet0AllocateInfo.pNext = NULL;
	descriptorSet0AllocateInfo.descriptorPool = descriptorPool;
	descriptorSet0AllocateInfo.descriptorSetCount = 1;
	descriptorSet0AllocateInfo.pSetLayouts = &rayTracingPipelineDescriptorSetLayouts[0];
	CHECK_VK_RESULT(vkAllocateDescriptorSets(vkApp.vkDevice, &descriptorSet0AllocateInfo, &descriptorSet0))
	
	VkWriteDescriptorSetAccelerationStructureNV descriptorAccelerationStructureInfo = {};
    descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
    descriptorAccelerationStructureInfo.pNext = NULL;
    descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
    descriptorAccelerationStructureInfo.pAccelerationStructures = &accStruct.topAccStruct.accelerationStructure;

    VkWriteDescriptorSet& accelerationStructureWrite = descriptorSet0Writes[ACCELERATION_STRUCTURE_NV_BINDING_LOCATION];
    accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    accelerationStructureWrite.pNext = &descriptorAccelerationStructureInfo; // Notice that pNext is assigned here!
    accelerationStructureWrite.dstSet = descriptorSet0;
    accelerationStructureWrite.dstBinding = ACCELERATION_STRUCTURE_NV_BINDING_LOCATION;
    accelerationStructureWrite.dstArrayElement = 0;
    accelerationStructureWrite.descriptorCount = 1;
    accelerationStructureWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
    accelerationStructureWrite.pImageInfo = NULL;
    accelerationStructureWrite.pBufferInfo = NULL;
    accelerationStructureWrite.pTexelBufferView = NULL;
    
    VkDescriptorImageInfo descriptorRayTracingImageInfo = {};
    descriptorRayTracingImageInfo.sampler = VK_NULL_HANDLE;
    descriptorRayTracingImageInfo.imageView = rayTracingImageView;
    descriptorRayTracingImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    
    VkWriteDescriptorSet& rayTracingImageWrite = descriptorSet0Writes[RESULT_IMAGE_BINDING_LOCATION];
    rayTracingImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    rayTracingImageWrite.pNext = NULL;
    rayTracingImageWrite.dstSet = descriptorSet0;
    rayTracingImageWrite.dstBinding = RESULT_IMAGE_BINDING_LOCATION;
    rayTracingImageWrite.dstArrayElement = 0;
    rayTracingImageWrite.descriptorCount = 1;
    rayTracingImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    rayTracingImageWrite.pImageInfo = &descriptorRayTracingImageInfo;
    rayTracingImageWrite.pBufferInfo = NULL;
    rayTracingImageWrite.pTexelBufferView = NULL;
    
    VkDescriptorImageInfo descriptorRayTracingOcclusionImageInfo = {};
    descriptorRayTracingOcclusionImageInfo.sampler = VK_NULL_HANDLE;
    descriptorRayTracingOcclusionImageInfo.imageView = rayTracingOcclusionImageView;
    descriptorRayTracingOcclusionImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    
    VkWriteDescriptorSet& rayTracingOcclusionImageWrite = descriptorSet0Writes[OCCLUSION_IMAGE_BINDING_LOCATION];
    rayTracingOcclusionImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    rayTracingOcclusionImageWrite.pNext = NULL;
    rayTracingOcclusionImageWrite.dstSet = descriptorSet0;
    rayTracingOcclusionImageWrite.dstBinding = OCCLUSION_IMAGE_BINDING_LOCATION;
    rayTracingOcclusionImageWrite.dstArrayElement = 0;
    rayTracingOcclusionImageWrite.descriptorCount = 1;
    rayTracingOcclusionImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    rayTracingOcclusionImageWrite.pImageInfo = &descriptorRayTracingOcclusionImageInfo;
    rayTracingOcclusionImageWrite.pBufferInfo = NULL;
    rayTracingOcclusionImageWrite.pTexelBufferView = NULL;
    
    VkDescriptorBufferInfo descriptorCameraInfo = {};
    descriptorCameraInfo.buffer = cameraBuffer;
    descriptorCameraInfo.offset = 0;
    descriptorCameraInfo.range = cameraBufferSize;
    
    VkWriteDescriptorSet& cameraWrite = descriptorSet0Writes[CAMERA_BUFFER_BINDING_LOCATION];
    cameraWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    cameraWrite.pNext = NULL;
    cameraWrite.dstSet = descriptorSet0;
    cameraWrite.dstBinding = CAMERA_BUFFER_BINDING_LOCATION;
    cameraWrite.dstArrayElement = 0;
    cameraWrite.descriptorCount = 1;
    cameraWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    cameraWrite.pImageInfo = NULL;
    cameraWrite.pBufferInfo = &descriptorCameraInfo;
    cameraWrite.pTexelBufferView = NULL;
    
    VkDescriptorBufferInfo descriptorLightsInfo = {};
    descriptorLightsInfo.buffer = lightsBuffer;
    descriptorLightsInfo.offset = 0;
    descriptorLightsInfo.range = lightsBufferSize;
    
    VkWriteDescriptorSet& lightsWrite = descriptorSet0Writes[LIGHTS_BUFFER_BINDING_LOCATION];
    lightsWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    lightsWrite.pNext = NULL;
    lightsWrite.dstSet = descriptorSet0;
    lightsWrite.dstBinding = LIGHTS_BUFFER_BINDING_LOCATION;
    lightsWrite.dstArrayElement = 0;
    lightsWrite.descriptorCount = 1;
    lightsWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    lightsWrite.pImageInfo = NULL;
    lightsWrite.pBufferInfo = &descriptorLightsInfo;
    lightsWrite.pTexelBufferView = NULL;
    
    VkDescriptorBufferInfo descriptorOtherDataInfo = {};
    descriptorOtherDataInfo.buffer = otherDataBuffer;
    descriptorOtherDataInfo.offset = 0;
    descriptorOtherDataInfo.range = otherDataBufferSize;
    
    VkWriteDescriptorSet& otherDataWrite = descriptorSet0Writes[OTHER_DATA_BUFFER_BINDING_LOCATION];
    otherDataWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    otherDataWrite.pNext = NULL;
    otherDataWrite.dstSet = descriptorSet0;
    otherDataWrite.dstBinding = OTHER_DATA_BUFFER_BINDING_LOCATION;
    otherDataWrite.dstArrayElement = 0;
    otherDataWrite.descriptorCount = 1;
    otherDataWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    otherDataWrite.pImageInfo = NULL;
    otherDataWrite.pBufferInfo = &descriptorOtherDataInfo;
    otherDataWrite.pTexelBufferView = NULL;
    
    vkUpdateDescriptorSets(vkApp.vkDevice, descriptorSet0Writes.size(), descriptorSet0Writes.data(), 0, NULL);
    
    //Descriptor set 1
	VkDescriptorSet& descriptorSet1 = descriptorSets[1];
    std::vector<VkWriteDescriptorSet> descriptorSet1Writes(DESCRIPTOR_SET_1_NUM_BINDINGS);
    
	VkDescriptorSetAllocateInfo descriptorSet1AllocateInfo = {};
	descriptorSet1AllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSet1AllocateInfo.pNext = NULL;
	descriptorSet1AllocateInfo.descriptorPool = descriptorPool;
	descriptorSet1AllocateInfo.descriptorSetCount = 1;
	descriptorSet1AllocateInfo.pSetLayouts = &rayTracingPipelineDescriptorSetLayouts[1];
	CHECK_VK_RESULT(vkAllocateDescriptorSets(vkApp.vkDevice, &descriptorSet1AllocateInfo, &descriptorSet1))
    
    VkDescriptorBufferInfo descriptorCustomIDToAttributeArrayIndexInfo = {};
    descriptorCustomIDToAttributeArrayIndexInfo.buffer = customIDToAttributeArrayIndexBuffer;
    descriptorCustomIDToAttributeArrayIndexInfo.offset = 0;
    descriptorCustomIDToAttributeArrayIndexInfo.range = customIDToAttributeArrayIndexBufferSize;
    
    VkWriteDescriptorSet& customIDToAttributeArrayIndexWrite = descriptorSet1Writes[CUSTOM_ID_TO_ATTRIBUTE_ARRAY_INDEX_BUFFER_BINDING_LOCATION];
    customIDToAttributeArrayIndexWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    customIDToAttributeArrayIndexWrite.pNext = NULL;
    customIDToAttributeArrayIndexWrite.dstSet = descriptorSet1;
    customIDToAttributeArrayIndexWrite.dstBinding = CUSTOM_ID_TO_ATTRIBUTE_ARRAY_INDEX_BUFFER_BINDING_LOCATION;
    customIDToAttributeArrayIndexWrite.dstArrayElement = 0;
    customIDToAttributeArrayIndexWrite.descriptorCount = 1;
    customIDToAttributeArrayIndexWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    customIDToAttributeArrayIndexWrite.pImageInfo = NULL;
    customIDToAttributeArrayIndexWrite.pBufferInfo = &descriptorCustomIDToAttributeArrayIndexInfo;
    customIDToAttributeArrayIndexWrite.pTexelBufferView = NULL;
    
    VkDescriptorBufferInfo descriptorPerMeshAttributesInfo = {};
    descriptorPerMeshAttributesInfo.buffer = perMeshAttributeBuffer;
    descriptorPerMeshAttributesInfo.offset = 0;
    descriptorPerMeshAttributesInfo.range = perMeshAttributeBufferSize;
    
    VkWriteDescriptorSet& perMeshAttributesWrite = descriptorSet1Writes[PER_MESH_ATTRIBUTES_BINDING_LOCATION];
    perMeshAttributesWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    perMeshAttributesWrite.pNext = NULL;
    perMeshAttributesWrite.dstSet = descriptorSet1;
    perMeshAttributesWrite.dstBinding = PER_MESH_ATTRIBUTES_BINDING_LOCATION;
    perMeshAttributesWrite.dstArrayElement = 0;
    perMeshAttributesWrite.descriptorCount = 1;
    perMeshAttributesWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    perMeshAttributesWrite.pImageInfo = NULL;
    perMeshAttributesWrite.pBufferInfo = &descriptorPerMeshAttributesInfo;
    perMeshAttributesWrite.pTexelBufferView = NULL;
    
    VkDescriptorBufferInfo descriptorperVertexAttributesInfo = {};
    descriptorperVertexAttributesInfo.buffer = perVertexAttributeBuffer;
    descriptorperVertexAttributesInfo.offset = 0;
    descriptorperVertexAttributesInfo.range = perVertexAttributeBufferSize;
    
    VkWriteDescriptorSet& perVerexAttributesWrite = descriptorSet1Writes[PER_VERTEX_ATTRIBUTES_BINDING_LOCATION];
    perVerexAttributesWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    perVerexAttributesWrite.pNext = NULL;
    perVerexAttributesWrite.dstSet = descriptorSet1;
    perVerexAttributesWrite.dstBinding = PER_VERTEX_ATTRIBUTES_BINDING_LOCATION;
    perVerexAttributesWrite.dstArrayElement = 0;
    perVerexAttributesWrite.descriptorCount = 1;
    perVerexAttributesWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    perVerexAttributesWrite.pImageInfo = NULL;
    perVerexAttributesWrite.pBufferInfo = &descriptorperVertexAttributesInfo;
    perVerexAttributesWrite.pTexelBufferView = NULL;
    
    std::vector<VkDescriptorImageInfo> diffuseImageInfos(meshes.size());
    std::vector<VkDescriptorImageInfo> specularImageInfos(meshes.size());
    std::vector<VkDescriptorImageInfo> emissiveImageInfos(meshes.size());
    std::vector<VkDescriptorImageInfo> roughnessImageInfos(meshes.size());
    for (size_t i = 0; i < meshes.size(); i++)
    {
    	VkDescriptorImageInfo& diffuseImageInfo = diffuseImageInfos[i];
    	VkDescriptorImageInfo& specularImageInfo = specularImageInfos[i];
    	
		diffuseImageInfo.sampler = defaultSampler;
		if (meshes[i].material.diffuseTexture == NULL)
		{
			diffuseImageInfo.imageView = dummyTexture.imageView;
		}
		else
		{
			diffuseImageInfo.imageView = meshes[i].material.diffuseTexture->imageView;
		}
		diffuseImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		
		specularImageInfo.sampler = defaultSampler;
		if (meshes[i].material.specularTexture == NULL)
		{
			specularImageInfo.imageView = dummyTexture.imageView;
		}
		else
		{
			specularImageInfo.imageView = meshes[i].material.specularTexture->imageView;
		}
		specularImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    meshes.resize(0);
    
    VkWriteDescriptorSet& diffuseImageWrite = descriptorSet1Writes[DIFFUSE_TEXTURES_BINDING_LOCATION];
    diffuseImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    diffuseImageWrite.pNext = NULL;
    diffuseImageWrite.dstSet = descriptorSet1;
    diffuseImageWrite.dstBinding = DIFFUSE_TEXTURES_BINDING_LOCATION;
    diffuseImageWrite.dstArrayElement = 0;
    diffuseImageWrite.descriptorCount = diffuseImageInfos.size();
    diffuseImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    diffuseImageWrite.pImageInfo = diffuseImageInfos.data();
    diffuseImageWrite.pBufferInfo = NULL;
    diffuseImageWrite.pTexelBufferView = NULL;
    
    VkWriteDescriptorSet& specularImageWrite = descriptorSet1Writes[SPECULAR_TEXTURES_BINDING_LOCATION];
    specularImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    specularImageWrite.pNext = NULL;
    specularImageWrite.dstSet = descriptorSet1;
    specularImageWrite.dstBinding = SPECULAR_TEXTURES_BINDING_LOCATION;
    specularImageWrite.dstArrayElement = 0;
    specularImageWrite.descriptorCount = specularImageInfos.size();
    specularImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    specularImageWrite.pImageInfo = specularImageInfos.data();
    specularImageWrite.pBufferInfo = NULL;
    specularImageWrite.pTexelBufferView = NULL;
    
    vkUpdateDescriptorSets(vkApp.vkDevice, descriptorSet1Writes.size(), descriptorSet1Writes.data(), 0, NULL);
    
    ////////////////////////////
	/////GRAPHICS PIPELINE//////
	////////////////////////////
	VkShaderModule vertexShaderModule, fragmentShaderModuleRenderPass0;
	vkApp.CreateShaderModule("src/shaders/out/vert.spv", &vertexShaderModule);
	vkApp.CreateShaderModule("src/shaders/out/fragPCFBlur.spv", &fragmentShaderModuleRenderPass0);

	// Vertex buffer
	// UV coordinates are a bit weird as the image has to be flipped to
	// be shown correctly when sampled (it's upside-down)
	std::vector<float> vertexData = {
		-1.0f, -1.0f,	0.0f, 0.0f,
		-1.0f,  1.0f,	0.0f, 1.0f,
		 1.0f,  1.0f,	1.0f, 1.0f,
		 1.0f, -1.0f,	1.0f, 0.0f
	};
	VkDeviceSize vertexBufferSize = vertexData.size() * sizeof(float);
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	vkApp.CreateDeviceBuffer(vertexBufferSize, (void*)(vertexData.data()), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &vertexBuffer, &vertexBufferMemory);
	// Index buffer
	std::vector<uint32_t> indexData = {
		0, 1, 2,
		2, 3, 0
	};
	VkDeviceSize indexBufferSize = indexData.size() * sizeof(uint32_t);
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	vkApp.CreateDeviceBuffer(indexBufferSize, (void*)(indexData.data()), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, &indexBuffer, &indexBufferMemory);
	// Blur buffer
	blurVariable = 0;
	VkDeviceSize blurBufferSize = sizeof(uint32_t);
	VkBuffer blurBuffer;
	VkDeviceMemory blurBufferMemory;
	vkApp.CreateHostVisibleBuffer(blurBufferSize, (void*)(&blurVariable), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &blurBuffer, &blurBufferMemory);

	std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfosRenderPass0(2);
	shaderStageInfosRenderPass0[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfosRenderPass0[0].pNext = NULL;
	shaderStageInfosRenderPass0[0].flags = 0;
	shaderStageInfosRenderPass0[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageInfosRenderPass0[0].module = vertexShaderModule;
	shaderStageInfosRenderPass0[0].pName = "main";
	shaderStageInfosRenderPass0[0].pSpecializationInfo = NULL;
	shaderStageInfosRenderPass0[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfosRenderPass0[1].pNext = NULL;
	shaderStageInfosRenderPass0[1].flags = 0;
	shaderStageInfosRenderPass0[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageInfosRenderPass0[1].module = fragmentShaderModuleRenderPass0;
	shaderStageInfosRenderPass0[1].pName = "main";
	shaderStageInfosRenderPass0[1].pSpecializationInfo = NULL;
	
	VkVertexInputBindingDescription inputDesc = {};
	inputDesc.binding = 0;
	inputDesc.stride = 4 * sizeof(float);
	inputDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::vector<VkVertexInputAttributeDescription> attributeDescs(2);
	VkVertexInputAttributeDescription& vertexDesc = attributeDescs[0];
	vertexDesc.location = 0;
	vertexDesc.binding = 0;
	vertexDesc.format = VK_FORMAT_R32G32_SFLOAT;
	vertexDesc.offset = 0;
	VkVertexInputAttributeDescription& uvDesc = attributeDescs[1];
	uvDesc.location = 1;
	uvDesc.binding = 0;
	uvDesc.format = VK_FORMAT_R32G32_SFLOAT;
	uvDesc.offset = 2 * sizeof(float);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.pNext = NULL;
	vertexInputInfo.flags = 0;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &inputDesc;
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescs.size();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescs.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.pNext = NULL;
	inputAssemblyInfo.flags = 0;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = vkApp.GetDefaultViewport();
	VkRect2D scissor = vkApp.GetDefaultScissor();
	VkPipelineViewportStateCreateInfo viewportInfo = {};
	viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportInfo.pNext = NULL;
	viewportInfo.flags = 0;
	viewportInfo.viewportCount = 1;
	viewportInfo.pViewports = &viewport;
	viewportInfo.scissorCount = 1;
	viewportInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
	rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationInfo.pNext = NULL;
	rasterizationInfo.flags = 0;
	rasterizationInfo.depthClampEnable = VK_FALSE;
	rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationInfo.depthBiasEnable = VK_FALSE;
	/* IGNORED
	rasterization_info.depthBiasConstantFactor
	rasterization_info.depthBiasClamp
	rasterization_info.depthBiasSlopeFactor*/
	rasterizationInfo.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo multisampleInfo = {};
	multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleInfo.pNext = NULL;
	multisampleInfo.flags = 0;
	multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleInfo.sampleShadingEnable = VK_FALSE;
	/* IGNORED
	multisample_info.minSampleShading
	multisample_info.pSampleMask*/
	multisampleInfo.alphaToCoverageEnable = VK_FALSE;
	multisampleInfo.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.blendEnable = VK_FALSE;
	/* IGNORED
	color_blend_attachment.srcColorBlendFactor 
	color_blend_attachment.dstColorBlendFactor
	color_blend_attachment.colorBlendOp
	color_blend_attachment.srcAlphaBlendFactor
	color_blend_attachment.dstAlphaBlendFactor
	color_blend_attachment.alphaBlendOp*/
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlendInfo = {};
	colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendInfo.pNext = NULL;
	colorBlendInfo.flags = 0;
	colorBlendInfo.logicOpEnable = VK_FALSE;
	/* INGORED
	color_blend_info.logicOp*/
	colorBlendInfo.attachmentCount = 1;
	colorBlendInfo.pAttachments = &colorBlendAttachment;
	colorBlendInfo.blendConstants[0] = 0.0f;
	colorBlendInfo.blendConstants[1] = 0.0f;
	colorBlendInfo.blendConstants[2] = 0.0f;
	colorBlendInfo.blendConstants[3] = 0.0f;

	// Descriptors setup
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindingsRenderPass0(3);
	VkDescriptorSetLayoutBinding& rayTracingImageBinding = descriptorSetLayoutBindingsRenderPass0[0];
	rayTracingImageBinding.binding = 0;
	rayTracingImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	rayTracingImageBinding.descriptorCount = 1;
	rayTracingImageBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	rayTracingImageBinding.pImmutableSamplers = NULL;
	VkDescriptorSetLayoutBinding& rayTracingOcclusionImageBinding = descriptorSetLayoutBindingsRenderPass0[1];
	rayTracingOcclusionImageBinding.binding = 1;
	rayTracingOcclusionImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	rayTracingOcclusionImageBinding.descriptorCount = 1;
	rayTracingOcclusionImageBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	rayTracingOcclusionImageBinding.pImmutableSamplers = NULL;
	VkDescriptorSetLayoutBinding& blurUniformBinding = descriptorSetLayoutBindingsRenderPass0[2];
	blurUniformBinding.binding = 2;
	blurUniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	blurUniformBinding.descriptorCount = 1;
	blurUniformBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	blurUniformBinding.pImmutableSamplers = NULL;

	VkDescriptorSetLayoutCreateInfo descriptorSetInfoGraphics = {};
	descriptorSetInfoGraphics.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetInfoGraphics.pNext = NULL;
	descriptorSetInfoGraphics.flags = 0;
	descriptorSetInfoGraphics.bindingCount = descriptorSetLayoutBindingsRenderPass0.size();
	descriptorSetInfoGraphics.pBindings = descriptorSetLayoutBindingsRenderPass0.data();
	VkDescriptorSetLayout descriptorSetLayoutGraphicsRenderPass0;
	CHECK_VK_RESULT(vkCreateDescriptorSetLayout(vkApp.vkDevice, &descriptorSetInfoGraphics, NULL, &descriptorSetLayoutGraphicsRenderPass0))

	VkPipelineLayoutCreateInfo pipelineLayoutInfoGraphics = {};
	pipelineLayoutInfoGraphics.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfoGraphics.pNext = NULL;
	pipelineLayoutInfoGraphics.flags = 0;
	pipelineLayoutInfoGraphics.setLayoutCount = 1;
	pipelineLayoutInfoGraphics.pSetLayouts = &descriptorSetLayoutGraphicsRenderPass0;
	pipelineLayoutInfoGraphics.pushConstantRangeCount = 0;
	pipelineLayoutInfoGraphics.pPushConstantRanges = NULL;
	VkPipelineLayout pipelineLayoutGraphicsRenderPass0;
	CHECK_VK_RESULT(vkCreatePipelineLayout(vkApp.vkDevice, &pipelineLayoutInfoGraphics, NULL, &pipelineLayoutGraphicsRenderPass0))
	
	std::vector<VkDescriptorPoolSize> poolSizesGraphics = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
	};
	VkDescriptorPoolCreateInfo descriptorPoolInfoGraphics = {};
	descriptorPoolInfoGraphics.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfoGraphics.pNext = NULL;
	descriptorPoolInfoGraphics.flags = 0;
	descriptorPoolInfoGraphics.maxSets = 1;
	descriptorPoolInfoGraphics.poolSizeCount = poolSizesGraphics.size();
	descriptorPoolInfoGraphics.pPoolSizes = poolSizesGraphics.data();
	VkDescriptorPool descriptorPoolGraphics;
	CHECK_VK_RESULT(vkCreateDescriptorPool(vkApp.vkDevice, &descriptorPoolInfoGraphics, NULL, &descriptorPoolGraphics))
	
	VkDescriptorSetAllocateInfo descriptorSetGraphicsAllocateInfo = {};
	descriptorSetGraphicsAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetGraphicsAllocateInfo.pNext = NULL;
	descriptorSetGraphicsAllocateInfo.descriptorPool = descriptorPoolGraphics;
	descriptorSetGraphicsAllocateInfo.descriptorSetCount = 1;
	descriptorSetGraphicsAllocateInfo.pSetLayouts = &descriptorSetLayoutGraphicsRenderPass0;
	VkDescriptorSet descriptorSetGraphicsRenderPass0;
	CHECK_VK_RESULT(vkAllocateDescriptorSets(vkApp.vkDevice, &descriptorSetGraphicsAllocateInfo, &descriptorSetGraphicsRenderPass0))
	
    std::vector<VkWriteDescriptorSet> descriptorSetGraphicsWritesRenderPass0(3);
    // Color image
	VkDescriptorImageInfo descriptorRayTracingImageInfoGraphics = {};
    descriptorRayTracingImageInfoGraphics.sampler = defaultSampler;
    descriptorRayTracingImageInfoGraphics.imageView = rayTracingImageView;
    descriptorRayTracingImageInfoGraphics.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkWriteDescriptorSet& rayTracingImageWriteGraphics = descriptorSetGraphicsWritesRenderPass0[0];
    rayTracingImageWriteGraphics.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    rayTracingImageWriteGraphics.pNext = NULL;
    rayTracingImageWriteGraphics.dstSet = descriptorSetGraphicsRenderPass0;
    rayTracingImageWriteGraphics.dstBinding = 0;
    rayTracingImageWriteGraphics.dstArrayElement = 0;
    rayTracingImageWriteGraphics.descriptorCount = 1;
    rayTracingImageWriteGraphics.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    rayTracingImageWriteGraphics.pImageInfo = &descriptorRayTracingImageInfoGraphics;
    rayTracingImageWriteGraphics.pBufferInfo = NULL;
    rayTracingImageWriteGraphics.pTexelBufferView = NULL;
    // Occlusion image
    VkDescriptorImageInfo descriptorRayTracingOcclusionImageInfoGraphics = {};
    descriptorRayTracingOcclusionImageInfoGraphics.sampler = defaultSampler;
    descriptorRayTracingOcclusionImageInfoGraphics.imageView = rayTracingOcclusionImageView;
    descriptorRayTracingOcclusionImageInfoGraphics.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkWriteDescriptorSet& rayTracingOcclusionImageWriteGraphics = descriptorSetGraphicsWritesRenderPass0[1];
    rayTracingOcclusionImageWriteGraphics.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    rayTracingOcclusionImageWriteGraphics.pNext = NULL;
    rayTracingOcclusionImageWriteGraphics.dstSet = descriptorSetGraphicsRenderPass0;
    rayTracingOcclusionImageWriteGraphics.dstBinding = 1;
    rayTracingOcclusionImageWriteGraphics.dstArrayElement = 0;
    rayTracingOcclusionImageWriteGraphics.descriptorCount = 1;
    rayTracingOcclusionImageWriteGraphics.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    rayTracingOcclusionImageWriteGraphics.pImageInfo = &descriptorRayTracingOcclusionImageInfoGraphics;
    rayTracingOcclusionImageWriteGraphics.pBufferInfo = NULL;
    rayTracingOcclusionImageWriteGraphics.pTexelBufferView = NULL;
    // Blur variable uniform
    VkDescriptorBufferInfo blurBufferInfoGraphics = {};
    blurBufferInfoGraphics.buffer = blurBuffer;
    blurBufferInfoGraphics.offset = 0;
    blurBufferInfoGraphics.range = VK_WHOLE_SIZE;
    VkWriteDescriptorSet& blurBufferWriteGraphics = descriptorSetGraphicsWritesRenderPass0[2];
    blurBufferWriteGraphics.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    blurBufferWriteGraphics.pNext = NULL;
    blurBufferWriteGraphics.dstSet = descriptorSetGraphicsRenderPass0;
    blurBufferWriteGraphics.dstBinding = 2;
    blurBufferWriteGraphics.dstArrayElement = 0;
    blurBufferWriteGraphics.descriptorCount = 1;
    blurBufferWriteGraphics.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    blurBufferWriteGraphics.pImageInfo = NULL;
    blurBufferWriteGraphics.pBufferInfo = &blurBufferInfoGraphics;
    blurBufferWriteGraphics.pTexelBufferView = NULL;
    // Update render pass 0
    vkUpdateDescriptorSets(vkApp.vkDevice, descriptorSetGraphicsWritesRenderPass0.size(), descriptorSetGraphicsWritesRenderPass0.data(), 0, NULL);

	std::vector<VkAttachmentDescription> attachments(1);
	attachments[0].flags = 0;
	attachments[0].format = vkApp.GetDefaultFramebufferFormat();
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	std::vector<VkAttachmentReference> colorAttachmentRefs(1);
	colorAttachmentRefs[0].attachment = 0;
	colorAttachmentRefs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	std::vector<VkSubpassDescription> subpasses(1);
	subpasses[0].flags = 0;
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].inputAttachmentCount = 0;
	subpasses[0].pInputAttachments = NULL;
	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = &colorAttachmentRefs[0];
	subpasses[0].pResolveAttachments = NULL;
	subpasses[0].pDepthStencilAttachment = NULL;
	subpasses[0].preserveAttachmentCount = 0;
	subpasses[0].pPreserveAttachments = NULL;

	std::vector<VkSubpassDependency> subpassDependencies(1);
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].srcAccessMask = 0;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = 0;

	VkRenderPassCreateInfo renderpassInfo = {};
	renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassInfo.pNext = NULL;
	renderpassInfo.flags = 0;
	renderpassInfo.attachmentCount = 1;
	renderpassInfo.pAttachments = &attachments[0];
	renderpassInfo.subpassCount = subpasses.size();
	renderpassInfo.pSubpasses = subpasses.data();
	renderpassInfo.dependencyCount = subpassDependencies.size();
	renderpassInfo.pDependencies = subpassDependencies.data();
	VkRenderPass renderPass0;
	CHECK_VK_RESULT(vkCreateRenderPass(vkApp.vkDevice, &renderpassInfo, NULL, &renderPass0))

	std::vector<VkGraphicsPipelineCreateInfo> graphicsPipelineInfos(1);
	graphicsPipelineInfos[0].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineInfos[0].pNext = NULL;
	graphicsPipelineInfos[0].flags = 0;
	graphicsPipelineInfos[0].stageCount = shaderStageInfosRenderPass0.size();
	graphicsPipelineInfos[0].pStages = shaderStageInfosRenderPass0.data();
	graphicsPipelineInfos[0].pVertexInputState = &vertexInputInfo;
	graphicsPipelineInfos[0].pInputAssemblyState = &inputAssemblyInfo;
	graphicsPipelineInfos[0].pTessellationState = NULL;
	graphicsPipelineInfos[0].pViewportState = &viewportInfo;
	graphicsPipelineInfos[0].pRasterizationState = &rasterizationInfo;
	graphicsPipelineInfos[0].pMultisampleState = &multisampleInfo;
	graphicsPipelineInfos[0].pDepthStencilState = NULL;
	graphicsPipelineInfos[0].pColorBlendState = &colorBlendInfo;
	graphicsPipelineInfos[0].pDynamicState = NULL;
	graphicsPipelineInfos[0].layout = pipelineLayoutGraphicsRenderPass0;
	graphicsPipelineInfos[0].renderPass = renderPass0;
	graphicsPipelineInfos[0].subpass = 0;
	graphicsPipelineInfos[0].basePipelineHandle = VK_NULL_HANDLE;
	std::vector<VkPipeline> graphicsPipelines(2);
	CHECK_VK_RESULT(vkCreateGraphicsPipelines(vkApp.vkDevice, VK_NULL_HANDLE, graphicsPipelineInfos.size(), graphicsPipelineInfos.data(), NULL, graphicsPipelines.data()))
    
    // FRAMEBUFFERS
	std::vector<VkFramebuffer> framebuffersRenderPass0;
	vkApp.CreateDefaultFramebuffers(framebuffersRenderPass0, renderPass0);
    
	////////////////////////////
	///////////RECORD///////////
	////////////////////////////
    std::vector<VkCommandBuffer> graphicsQueueCommandBuffers;
	vkApp.AllocateDefaultGraphicsQueueCommandBuffers(graphicsQueueCommandBuffers);
	VkDeviceSize offset = 0;
	for (size_t i = 0; i < graphicsQueueCommandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo rayTraceBeginInfo = {};
		rayTraceBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		rayTraceBeginInfo.pNext = NULL;
		rayTraceBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		rayTraceBeginInfo.pInheritanceInfo = NULL;
		CHECK_VK_RESULT(vkBeginCommandBuffer(graphicsQueueCommandBuffers[i], &rayTraceBeginInfo))
		
		//Ray trace
		vkCmdBindPipeline(graphicsQueueCommandBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, rayTracingPipeline);
		vkCmdBindDescriptorSets(graphicsQueueCommandBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, rayTracingPipelineLayout, 0, descriptorSets.size(), descriptorSets.data(), 0, NULL);
		vkCmdTraceRaysNV(graphicsQueueCommandBuffers[i], 
			shaderBindingTableBuffer, 0 * physicalDeviceRayTracingProperties.shaderGroupHandleSize,
			shaderBindingTableBuffer, 3 * physicalDeviceRayTracingProperties.shaderGroupHandleSize, physicalDeviceRayTracingProperties.shaderGroupHandleSize,
			shaderBindingTableBuffer, 1 * physicalDeviceRayTracingProperties.shaderGroupHandleSize, physicalDeviceRayTracingProperties.shaderGroupHandleSize,
			 VK_NULL_HANDLE, 0, 0, vkApp.vkSurfaceExtent.width, vkApp.vkSurfaceExtent.height, 1);
		
		//Barrier - wait for ray tracing to finish and transition images
		vkApp.TransitionImageLayoutInProgress(rayTracingImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, graphicsQueueCommandBuffers[i]);
		vkApp.TransitionImageLayoutInProgress(rayTracingOcclusionImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, graphicsQueueCommandBuffers[i]);
		
		// Blur ambient occlusion result and combine with light result
		VkClearColorValue clearColorValue = { 0.0f, 0.0f, 0.0f, 1.0f };
		VkClearValue clearValues[] = { clearColorValue };
		// Render pass 0
		const uint32_t renderPassID = 0;
		VkRenderPassBeginInfo renderpassInfo = {};
		renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpassInfo.pNext = NULL;
		renderpassInfo.renderPass = renderPass0;
		renderpassInfo.framebuffer = framebuffersRenderPass0[i];
		renderpassInfo.renderArea.offset = { 0, 0 };
		renderpassInfo.renderArea.extent = vkApp.vkSurfaceExtent;
		renderpassInfo.clearValueCount = 1;
		renderpassInfo.pClearValues = clearValues;
		vkCmdBeginRenderPass(graphicsQueueCommandBuffers[i], &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(graphicsQueueCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelines[renderPassID]);
		vkCmdBindVertexBuffers(graphicsQueueCommandBuffers[i], 0, 1, &vertexBuffer, &offset);
		vkCmdBindIndexBuffer(graphicsQueueCommandBuffers[i], indexBuffer, offset, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(graphicsQueueCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutGraphicsRenderPass0, 0, 1, &descriptorSetGraphicsRenderPass0, 0, NULL);
		vkCmdDrawIndexed(graphicsQueueCommandBuffers[i], uint32_t(indexData.size()), 1, 0, 0, 0);
		vkCmdEndRenderPass(graphicsQueueCommandBuffers[i]);
		
		//Barrier - wait for ambient occlusion to finish and transition images from SHADER_READ_ONLY to GENERAL
		vkApp.TransitionImageLayoutInProgress(rayTracingImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, graphicsQueueCommandBuffers[i]);
		vkApp.TransitionImageLayoutInProgress(rayTracingOcclusionImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, graphicsQueueCommandBuffers[i]);
		
		CHECK_VK_RESULT(vkEndCommandBuffer(graphicsQueueCommandBuffers[i]))
	}
	
	// Render
	glfwSetCursorPosCallback(vkApp.window, MouseCallback);
	glfwSetInputMode(vkApp.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(vkApp.window, KeyCallback);
	while (!glfwWindowShouldClose(vkApp.window))
	{
		glfwPollEvents();
		// Camera
		vkApp.camera.Update();
		cameraData = {
			vkApp.camera.origin.x, vkApp.camera.origin.y, vkApp.camera.origin.z, 0.0f,
			vkApp.camera.topLeftCorner.x, vkApp.camera.topLeftCorner.y, vkApp.camera.topLeftCorner.z, 0.0f,
			vkApp.camera.horizontalEnd.x, vkApp.camera.horizontalEnd.y, vkApp.camera.horizontalEnd.z, 0.0f,
			vkApp.camera.verticalEnd.x, vkApp.camera.verticalEnd.y, vkApp.camera.verticalEnd.z, 0.0f
		};
		// Lights
		int numLights = lights.size() / numFloatsPerLight;
		// Update
		vkApp.UpdateHostVisibleBuffer(cameraBufferSize, cameraData.data(), cameraBufferMemory);
		vkApp.UpdateHostVisibleBuffer(cameraBufferSize, cameraData.data(), cameraBufferMemory);
		vkApp.UpdateHostVisibleBuffer(blurBufferSize, &blurVariable, blurBufferMemory);
		
		vkApp.Render(graphicsQueueCommandBuffers.data());
		//vkApp.RenderOffscreen(graphicsQueueCommandBuffers.data());
	}
	vkDeviceWaitIdle(vkApp.vkDevice);
	
	// Cleanup
	// Graphics pipeline stuff
	
	// Raytracing stuff
	vkDestroyDescriptorPool(vkApp.vkDevice, descriptorPool, NULL);
	vkDestroyImageView(vkApp.vkDevice, rayTracingOcclusionImageView, NULL);
	vkDestroyImageView(vkApp.vkDevice, rayTracingImageView, NULL);
	vkFreeMemory(vkApp.vkDevice, rayTracingImageMemory, NULL);
	vkDestroyImage(vkApp.vkDevice, rayTracingImage, NULL);
	vkFreeMemory(vkApp.vkDevice, shaderBindindTableBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, shaderBindingTableBuffer, NULL);
	vkDestroyPipeline(vkApp.vkDevice, rayTracingPipeline, NULL);
	vkDestroyPipelineLayout(vkApp.vkDevice, rayTracingPipelineLayout, NULL);
	for (auto& descriptorSetLayout : rayTracingPipelineDescriptorSetLayouts)
	{
		vkDestroyDescriptorSetLayout(vkApp.vkDevice, descriptorSetLayout, NULL);
	}
	vkDestroySampler(vkApp.vkDevice, defaultSampler, NULL);
	for (VkShaderModule& shaderModule : shaderModules)
	{
		vkDestroyShaderModule(vkApp.vkDevice, shaderModule, NULL);
	}
	vkFreeMemory(vkApp.vkDevice, customIDToAttributeArrayIndexBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, customIDToAttributeArrayIndexBuffer, NULL);
	vkFreeMemory(vkApp.vkDevice, perVertexAttributeBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, perVertexAttributeBuffer, NULL);
	vkFreeMemory(vkApp.vkDevice, perMeshAttributeBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, perMeshAttributeBuffer, NULL);
	vkFreeMemory(vkApp.vkDevice, otherDataBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, otherDataBuffer, NULL);
	vkFreeMemory(vkApp.vkDevice, lightsBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, lightsBuffer, NULL);
	vkFreeMemory(vkApp.vkDevice, cameraBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, cameraBuffer, NULL);
}

int main(int argc, char** argv)
{
	Raytrace(argv[1]);
	return EXIT_SUCCESS;
}
