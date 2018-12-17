#include "glm/gtc/constants.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include "glm/vec3.hpp"
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "VulkanApp.h"

void RaytraceTriangle()
{
	std::vector<const char*> validationLayerNames = {
		"VK_LAYER_LUNARG_standard_validation"
	};
	std::vector<const char*> extensionNames = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_NV_RAY_TRACING_EXTENSION_NAME
	};
	VulkanAppCreateInfo vkAppInfo = {};
	vkAppInfo.graphicsApp = VK_TRUE;
	vkAppInfo.windowWidth = 1024;
	vkAppInfo.windowHeight = 1024;
	vkAppInfo.windowName = "Vulkan RTX";
	vkAppInfo.appName = "Vulkan RTX";
	vkAppInfo.engineName = "VRTX";
	vkAppInfo.validationLayerCount = validationLayerNames.size();
	vkAppInfo.validationLayerNames = validationLayerNames.data();
	vkAppInfo.extensionCount = extensionNames.size();
	vkAppInfo.extensionNames = extensionNames.data();
	vkAppInfo.maxFramesInFlight = 2;
	VulkanApp vkApp(&vkAppInfo);
	
	////////////////////////////
	///////////CAMERA///////////
	////////////////////////////
	glm::vec3 cameraOrigin(0.0f, -0.5f, 6.0f);
	glm::vec3 cameraDir = glm::normalize(glm::vec3(0.0f, 0.05f, -1.0f));
	float cameraVerticalFOV = 19.0f;
	float cameraAspectRatio = vkApp.vkSurfaceExtent.width / float(vkApp.vkSurfaceExtent.height);
	float theta = (cameraVerticalFOV * glm::pi<float>()) / 180.0f; //Convert to radians
	float lensHeight = glm::tan(theta);
	float lensWidth = lensHeight * cameraAspectRatio;
	float lensHalfWidth = lensWidth / 2.0f;
	float lensHalfHeight = lensHeight / 2.0f;
	//Calculate the three vectors that define the camera	
	glm::vec3 baseUp(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::normalize(glm::cross(cameraDir, baseUp));
	glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, cameraDir));
	//Calculate top_left_corner
	//1) Start at camera's position
	//2) Go lens_half_width along the camera's left (-right) axis
	//3) Go lens_half_height along the camera's up axis
	//4) Go 1 unit along the camera's view_direction axis
	glm::vec3 cameraTopLeftCorner = cameraOrigin + (lensHalfWidth * (-cameraRight)) + (lensHalfHeight * cameraUp) + cameraDir;
	//Go width of lense along the camera's right axis
	glm::vec3 cameraHorizontalEnd = lensWidth * cameraRight;
	//Go height of lense along the camera's down (-up) axis
	glm::vec3 cameraVerticalEnd = lensHeight * (-cameraUp);
	std::vector<float> cameraData = {
		cameraOrigin.x, cameraOrigin.y, cameraOrigin.z, 0.0f,
		cameraTopLeftCorner.x, cameraTopLeftCorner.y, cameraTopLeftCorner.z, 0.0f,
		cameraHorizontalEnd.x, cameraHorizontalEnd.y, cameraHorizontalEnd.z, 0.0f,
		cameraVerticalEnd.x, cameraVerticalEnd.y, cameraVerticalEnd.z, 0.0f
	};
	VkDeviceSize cameraBufferSize = cameraData.size() * sizeof(float);
	VkBuffer cameraBuffer;
	VkDeviceMemory cameraBufferMemory;
	vkApp.CreateDeviceBuffer(cameraBufferSize, (void*)(cameraData.data()), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &cameraBuffer, &cameraBufferMemory);
	
	////////////////////////////
	//////////TEXTURE///////////
	////////////////////////////
	VkSampler defaultSampler;
	vkApp.CreateDefaultSampler(&defaultSampler);
	VulkanTexture bfvTexture;
	vkApp.CreateTexture("data/test_scene/bfv.jpg", VK_FORMAT_R8G8B8A8_UNORM, &bfvTexture);
	VulkanTexture dummyTexture;
	vkApp.CreateDummyImage(&dummyTexture.image, &dummyTexture.imageMemory, &dummyTexture.imageView);
	
	////////////////////////////
	//////////GEOMETRY//////////
	////////////////////////////
	/*const uint32_t numMeshes = 2;
	std::vector<Mesh> meshes(numMeshes);
	vkApp.LoadMesh("data/test_scene/floor.obj", &meshes[0]);
	vkApp.LoadMesh("data/test_scene/pyramid.obj", &meshes[1]);*/
	std::vector<Mesh> meshes;
	vkApp.LoadMesh("data/test_scene/floor.obj", &meshes);
	vkApp.LoadMesh("data/test_scene/pyramid.obj", &meshes);
	std::vector<std::vector<float>> geometryData;
	for (Mesh& mesh : meshes)
	{
		geometryData.push_back(mesh.vertices);
	}
	
	std::vector<float> colorData;
	std::vector<float> attributeData;
	std::vector<uint32_t> customIDToAttributeArrayIndex;
	vkApp.BuildColorAndAttributeData(meshes, &attributeData, &colorData, &customIDToAttributeArrayIndex);
	
	VkDeviceSize attributeBufferSize = attributeData.size() * sizeof(float);
	VkBuffer attributeBuffer;
	VkDeviceMemory attributeBufferMemory;
	vkApp.CreateDeviceBuffer(attributeBufferSize, (void*)(attributeData.data()), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &attributeBuffer, &attributeBufferMemory);
	
	VkDeviceSize colorBufferSize = colorData.size() * sizeof(float);
	VkBuffer colorBuffer;
	VkDeviceMemory colorBufferMemory;
	vkApp.CreateDeviceBuffer(colorBufferSize, (void*)(colorData.data()), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &colorBuffer, &colorBufferMemory);
	
	VkDeviceSize customIDToAttributeArrayIndexBufferSize = customIDToAttributeArrayIndex.size() * sizeof(uint32_t);
	VkBuffer customIDToAttributeArrayIndexBuffer;
	VkDeviceMemory customIDToAttributeArrayIndexBufferMemory;
	vkApp.CreateDeviceBuffer(customIDToAttributeArrayIndexBufferSize, (void*)(customIDToAttributeArrayIndex.data()), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &customIDToAttributeArrayIndexBuffer, &customIDToAttributeArrayIndexBufferMemory);
	
	////////////////////////////
	///ACCELERATION STRUCTURE///
	////////////////////////////
	VulkanAccelerationStructure accStruct;
	vkApp.CreateVulkanAccelerationStructure(geometryData, &accStruct);
	vkApp.BuildAccelerationStructure(accStruct);
	
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
	const uint32_t numDescriptorBindingsSet0 = 3;
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings0(numDescriptorBindingsSet0);
	const uint32_t numDescriptorBindingsSet1 = 4;
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings1(numDescriptorBindingsSet1);
	std::vector<VkDescriptorSetLayout> rayTracingPipelineDescriptorSetLayouts(numDescriptorSets);
	
	//Desriptor set 0
	VkDescriptorSetLayoutBinding& accelerationStructureDescriptorSetLayoutBinding = descriptorSetLayoutBindings0[0];
	accelerationStructureDescriptorSetLayoutBinding.binding = 0;
	accelerationStructureDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
	accelerationStructureDescriptorSetLayoutBinding.descriptorCount = 1;
	accelerationStructureDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;
	accelerationStructureDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
	VkDescriptorSetLayoutBinding& rayTracingImageDescriptorSetLayoutBinding = descriptorSetLayoutBindings0[1];
	rayTracingImageDescriptorSetLayoutBinding.binding = 1;
	rayTracingImageDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	rayTracingImageDescriptorSetLayoutBinding.descriptorCount = 1;
	rayTracingImageDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;
	rayTracingImageDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
	VkDescriptorSetLayoutBinding& cameraDescriptorSetLayoutBinding = descriptorSetLayoutBindings0[2];
	cameraDescriptorSetLayoutBinding.binding = 2;
	cameraDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	cameraDescriptorSetLayoutBinding.descriptorCount = 1;
	cameraDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;
	cameraDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
	VkDescriptorSetLayoutCreateInfo& descriptorSetLayoutInfo0 = descriptorSetLayoutInfos[0];
	descriptorSetLayoutInfo0.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo0.pNext = NULL;
	descriptorSetLayoutInfo0.flags = 0;
	descriptorSetLayoutInfo0.bindingCount = descriptorSetLayoutBindings0.size();
	descriptorSetLayoutInfo0.pBindings = descriptorSetLayoutBindings0.data();
	CHECK_VK_RESULT(vkCreateDescriptorSetLayout(vkApp.vkDevice, &descriptorSetLayoutInfo0, NULL, &rayTracingPipelineDescriptorSetLayouts[0]))
	
	//Descriptor set 1
	VkDescriptorSetLayoutBinding& customIDToAttributeArrayIndexDescriptorSetLayoutBinding = descriptorSetLayoutBindings1[0];
	customIDToAttributeArrayIndexDescriptorSetLayoutBinding.binding = 0;
	customIDToAttributeArrayIndexDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	customIDToAttributeArrayIndexDescriptorSetLayoutBinding.descriptorCount = 1;
	customIDToAttributeArrayIndexDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	customIDToAttributeArrayIndexDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
	VkDescriptorSetLayoutBinding& attributeDescriptorSetLayoutBinding = descriptorSetLayoutBindings1[1];
	attributeDescriptorSetLayoutBinding.binding = 1;
	attributeDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	attributeDescriptorSetLayoutBinding.descriptorCount = 1;
	attributeDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	attributeDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
	VkDescriptorSetLayoutBinding& colorDescriptorSetLayoutBinding = descriptorSetLayoutBindings1[2];
	colorDescriptorSetLayoutBinding.binding = 2;
	colorDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	colorDescriptorSetLayoutBinding.descriptorCount = 1;
	colorDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	colorDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
	VkDescriptorSetLayoutBinding& textureDescriptorSetLayoutBinding = descriptorSetLayoutBindings1[3];
	textureDescriptorSetLayoutBinding.binding = 3;
	textureDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureDescriptorSetLayoutBinding.descriptorCount = meshes.size();
	textureDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	textureDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
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
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
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
	
	//Descriptor pool
	std::vector<VkDescriptorPoolSize> poolSizes = {
		{ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, uint32_t(meshes.size()) }
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
    std::vector<VkWriteDescriptorSet> descriptorSet0Writes(numDescriptorBindingsSet0);
    
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

    VkWriteDescriptorSet& accelerationStructureWrite = descriptorSet0Writes[0];
    accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    accelerationStructureWrite.pNext = &descriptorAccelerationStructureInfo; // Notice that pNext is assigned here!
    accelerationStructureWrite.dstSet = descriptorSet0;
    accelerationStructureWrite.dstBinding = 0;
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
    
    VkWriteDescriptorSet& rayTracingImageWrite = descriptorSet0Writes[1];
    rayTracingImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    rayTracingImageWrite.pNext = NULL;
    rayTracingImageWrite.dstSet = descriptorSet0;
    rayTracingImageWrite.dstBinding = 1;
    rayTracingImageWrite.dstArrayElement = 0;
    rayTracingImageWrite.descriptorCount = 1;
    rayTracingImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    rayTracingImageWrite.pImageInfo = &descriptorRayTracingImageInfo;
    rayTracingImageWrite.pBufferInfo = NULL;
    rayTracingImageWrite.pTexelBufferView = NULL;
    
    VkDescriptorBufferInfo descriptorCameraInfo = {};
    descriptorCameraInfo.buffer = cameraBuffer;
    descriptorCameraInfo.offset = 0;
    descriptorCameraInfo.range = cameraBufferSize;
    
    VkWriteDescriptorSet& cameraWrite= descriptorSet0Writes[2];
    cameraWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    cameraWrite.pNext = NULL;
    cameraWrite.dstSet = descriptorSet0;
    cameraWrite.dstBinding = 2;
    cameraWrite.dstArrayElement = 0;
    cameraWrite.descriptorCount = 1;
    cameraWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    cameraWrite.pImageInfo = NULL;
    cameraWrite.pBufferInfo = &descriptorCameraInfo;
    cameraWrite.pTexelBufferView = NULL;
    
    vkUpdateDescriptorSets(vkApp.vkDevice, descriptorSet0Writes.size(), descriptorSet0Writes.data(), 0, NULL);
    
    //Descriptor set 1
	VkDescriptorSet& descriptorSet1 = descriptorSets[1];
    std::vector<VkWriteDescriptorSet> descriptorSet1Writes(numDescriptorBindingsSet1);
    
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
    
    VkWriteDescriptorSet& customIDToAttributeArrayIndexWrite = descriptorSet1Writes[0];
    customIDToAttributeArrayIndexWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    customIDToAttributeArrayIndexWrite.pNext = NULL;
    customIDToAttributeArrayIndexWrite.dstSet = descriptorSet1;
    customIDToAttributeArrayIndexWrite.dstBinding = 0;
    customIDToAttributeArrayIndexWrite.dstArrayElement = 0;
    customIDToAttributeArrayIndexWrite.descriptorCount = 1;
    customIDToAttributeArrayIndexWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    customIDToAttributeArrayIndexWrite.pImageInfo = NULL;
    customIDToAttributeArrayIndexWrite.pBufferInfo = &descriptorCustomIDToAttributeArrayIndexInfo;
    customIDToAttributeArrayIndexWrite.pTexelBufferView = NULL;
    
    VkDescriptorBufferInfo descriptorAttributeInfo = {};
    descriptorAttributeInfo.buffer = attributeBuffer;
    descriptorAttributeInfo.offset = 0;
    descriptorAttributeInfo.range = attributeBufferSize;
    
    VkWriteDescriptorSet& attributeWrite = descriptorSet1Writes[1];
    attributeWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    attributeWrite.pNext = NULL;
    attributeWrite.dstSet = descriptorSet1;
    attributeWrite.dstBinding = 1;
    attributeWrite.dstArrayElement = 0;
    attributeWrite.descriptorCount = 1;
    attributeWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    attributeWrite.pImageInfo = NULL;
    attributeWrite.pBufferInfo = &descriptorAttributeInfo;
    attributeWrite.pTexelBufferView = NULL;
    
    VkDescriptorBufferInfo descriptorColorInfo = {};
    descriptorColorInfo.buffer = colorBuffer;
    descriptorColorInfo.offset = 0;
    descriptorColorInfo.range = colorBufferSize;
    
    VkWriteDescriptorSet& colorWrite = descriptorSet1Writes[2];
    colorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    colorWrite.pNext = NULL;
    colorWrite.dstSet = descriptorSet1;
    colorWrite.dstBinding = 2;
    colorWrite.dstArrayElement = 0;
    colorWrite.descriptorCount = 1;
    colorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    colorWrite.pImageInfo = NULL;
    colorWrite.pBufferInfo = &descriptorColorInfo;
    colorWrite.pTexelBufferView = NULL;
    
    std::vector<VkDescriptorImageInfo> imageInfos(meshes.size());
    for (VkDescriptorImageInfo& imageInfo : imageInfos)
    {
		imageInfo.sampler = defaultSampler;
		imageInfo.imageView = dummyTexture.imageView;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    
    VkWriteDescriptorSet& textureWrite = descriptorSet1Writes[3];
    textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    textureWrite.pNext = NULL;
    textureWrite.dstSet = descriptorSet1;
    textureWrite.dstBinding = 3;
    textureWrite.dstArrayElement = 0;
    textureWrite.descriptorCount = imageInfos.size();
    textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureWrite.pImageInfo = imageInfos.data();
    textureWrite.pBufferInfo = NULL;
    textureWrite.pTexelBufferView = NULL;
    
    vkUpdateDescriptorSets(vkApp.vkDevice, descriptorSet1Writes.size(), descriptorSet1Writes.data(), 0, NULL);
    
	//Record
    std::vector<VkCommandBuffer> graphicsQueueCommandBuffers;
	vkApp.AllocateDefaultGraphicsQueueCommandBuffers(graphicsQueueCommandBuffers);
	for (size_t i = 0; i < graphicsQueueCommandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo rayTraceBeginInfo = {};
		rayTraceBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		rayTraceBeginInfo.pNext = NULL;
		rayTraceBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		rayTraceBeginInfo.pInheritanceInfo = NULL;
		CHECK_VK_RESULT(vkBeginCommandBuffer(graphicsQueueCommandBuffers[i], &rayTraceBeginInfo))
		
		//Transition rayTracingImage from UNDEFINED to GENERAL
		vkApp.TransitionImageLayoutInProgress(rayTracingImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, VK_ACCESS_SHADER_WRITE_BIT, graphicsQueueCommandBuffers[i]);
		
		//Ray trace
		vkCmdBindPipeline(graphicsQueueCommandBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, rayTracingPipeline);
		vkCmdBindDescriptorSets(graphicsQueueCommandBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, rayTracingPipelineLayout, 0, descriptorSets.size(), descriptorSets.data(), 0, NULL);
		vkCmdTraceRaysNV(graphicsQueueCommandBuffers[i], 
			shaderBindingTableBuffer, 0 * physicalDeviceRayTracingProperties.shaderGroupHandleSize,
			shaderBindingTableBuffer, 3 * physicalDeviceRayTracingProperties.shaderGroupHandleSize, physicalDeviceRayTracingProperties.shaderGroupHandleSize,
			shaderBindingTableBuffer, 1 * physicalDeviceRayTracingProperties.shaderGroupHandleSize, physicalDeviceRayTracingProperties.shaderGroupHandleSize,
			 VK_NULL_HANDLE, 0, 0, vkApp.vkSurfaceExtent.width, vkApp.vkSurfaceExtent.height, 1);
		
		//Barrier - wait for ray tracing to finish and transition rayTracingImage from GENERAL to TRANSFER_SRC
		vkApp.TransitionImageLayoutInProgress(rayTracingImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, graphicsQueueCommandBuffers[i]);
		
		//Transition swapchainImage from UNDEFINED to TRANSFER_DST
		vkApp.TransitionImageLayoutInProgress(vkApp.vkSwapchainImages[i], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, graphicsQueueCommandBuffers[i]);
		
		//Copy data from rayTracingImage to swapchainImage
		VkImageCopy imageCopyRegion = {};
		imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCopyRegion.srcSubresource.mipLevel = 0;
		imageCopyRegion.srcSubresource.baseArrayLayer = 0;
		imageCopyRegion.srcSubresource.layerCount = 1;
		imageCopyRegion.srcOffset.x = 0;
		imageCopyRegion.srcOffset.y = 0;
		imageCopyRegion.srcOffset.z = 0;
		imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCopyRegion.dstSubresource.mipLevel = 0;
		imageCopyRegion.dstSubresource.baseArrayLayer = 0;
		imageCopyRegion.dstSubresource.layerCount = 1;
		imageCopyRegion.dstOffset.x = 0;
		imageCopyRegion.dstOffset.y = 0;
		imageCopyRegion.dstOffset.z = 0;
		imageCopyRegion.extent.width = vkApp.vkSurfaceExtent.width;
		imageCopyRegion.extent.height = vkApp.vkSurfaceExtent.height;
		imageCopyRegion.extent.depth = 1;
		vkCmdCopyImage(graphicsQueueCommandBuffers[i], rayTracingImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vkApp.vkSwapchainImages[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
		
		//Barrier - wait for copy to finish and transition swapchainImage from TRANSFER_DST to PRESENT_SRC
		vkApp.TransitionImageLayoutInProgress(vkApp.vkSwapchainImages[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, graphicsQueueCommandBuffers[i]);
		
		CHECK_VK_RESULT(vkEndCommandBuffer(graphicsQueueCommandBuffers[i]))
	}
	
	//Render
	while (!glfwWindowShouldClose(vkApp.window))
	{
		glfwPollEvents();
		vkApp.Render(graphicsQueueCommandBuffers.data());
		//vkApp.RenderOffscreen(graphicsQueueCommandBuffers.data());
	}
	vkDeviceWaitIdle(vkApp.vkDevice);
	
	//Cleanup
	vkDestroyDescriptorPool(vkApp.vkDevice, descriptorPool, NULL);
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
	//Basic data
	vkFreeMemory(vkApp.vkDevice, customIDToAttributeArrayIndexBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, customIDToAttributeArrayIndexBuffer, NULL);
	vkFreeMemory(vkApp.vkDevice, attributeBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, attributeBuffer, NULL);
	vkFreeMemory(vkApp.vkDevice, colorBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, colorBuffer, NULL);
	vkFreeMemory(vkApp.vkDevice, cameraBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, cameraBuffer, NULL);
}

int main(int argc, char** argv)
{
	//RasterizeTriangle();
	RaytraceTriangle();

	return EXIT_SUCCESS;
}
