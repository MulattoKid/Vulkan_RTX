#include "glm/gtc/constants.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include "glm/vec3.hpp"
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "VulkanApp.h"

struct VkGeometryInstanceNV
{
    float transform[12];
    uint32_t instanceCustomIndex : 24;
    uint32_t mask : 8;
    uint32_t instanceOffset : 24;
    uint32_t flags : 8;
    uint64_t accelerationStructureHandle;
};

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

	//Vertex buffer
	std::vector<float> vertexData = {
        -0.5f,  0.5f, -2.0f,
		-0.5f, -0.5f, -2.0f,
         0.5f, -0.5f, -2.0f
	};
	VkDeviceSize vertexDataSize = vertexData.size() * sizeof(float);
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	vkApp.CreateDeviceBuffer(vertexDataSize, (void*)(vertexData.data()), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &vertexBuffer, &vertexBufferMemory);
	//Index buffer
	std::vector<uint32_t> indexData = {
		0, 1, 2,
	};
	VkDeviceSize indexDataSize = indexData.size() * sizeof(uint32_t);
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	vkApp.CreateDeviceBuffer(indexDataSize, (void*)(indexData.data()), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, &indexBuffer, &indexBufferMemory);
	//Camera
	glm::vec3 cameraOrigin(0.0f, 0.0f, 6.0f);
	glm::vec3 cameraDir(0.0f, 0.0f, -1.0f);
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
		//Origin 
		cameraOrigin.x, cameraOrigin.y, cameraOrigin.z, 0.0f,
		cameraTopLeftCorner.x, cameraTopLeftCorner.y, cameraTopLeftCorner.z, 0.0f,
		cameraHorizontalEnd.x, cameraHorizontalEnd.y, cameraHorizontalEnd.z, 0.0f,
		cameraVerticalEnd.x, cameraVerticalEnd.y, cameraVerticalEnd.z, 0.0f
	};
	VkDeviceSize cameraBufferSize = cameraData.size() * sizeof(float);
	VkBuffer cameraBuffer;
	VkDeviceMemory cameraBufferMemory;
	vkApp.CreateDeviceBuffer(cameraBufferSize, (void*)(cameraData.data()), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &cameraBuffer, &cameraBufferMemory);
	
	/*
	Acceleration structure steps:
	2) Create TopLevel acceleration structure
		a) Specify acceleration structure (bottom level) and connect to d)
		b) Create
		c) Allocate memory for the acceleration structure
		d) Bind memory to acceleration structure
		e) Get uint64_t handle to acceleration structure
	3) Build acceleration structures
		a) Create a buffer that will be used as scratch when building
			- Should have the largest size that will be needed = max(largest_bottom_level, top_level)
			- Size is of type VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV
		b) Allocate command buffer
		c) Begin command buffer
		d) Build BottomLevel
		e) Pipeline barrier
		f) Build TopLevel
		g) Pipeline barrier
		h) End command buffer
		i) Submit command buffer to graphics queue
		j) Wait on graphics queue to be idle
		k) Free command buffer
		l) Destroy scratch buffer and free scratch memory
	*/
	
	VulkanAccelerationStructureBottom triangleAccStruct = vkApp.CreateVulkanAccelerationStructureBottom(vertexData, indexData);
	
	//2.a
	VkAccelerationStructureInfoNV topAccelerationStructureInfo = {};
	topAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	topAccelerationStructureInfo.pNext = NULL;
	topAccelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
	topAccelerationStructureInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV;
	topAccelerationStructureInfo.instanceCount = 1;
	topAccelerationStructureInfo.geometryCount = 0;
	topAccelerationStructureInfo.pGeometries = NULL;
	
	//2.b
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VkAccelerationStructureCreateInfoNV
	VkAccelerationStructureCreateInfoNV topAccelerationStructureCreateInfo = {};
	topAccelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
	topAccelerationStructureCreateInfo.pNext = NULL;
	topAccelerationStructureCreateInfo.compactedSize = 0;
	topAccelerationStructureCreateInfo.info = topAccelerationStructureInfo;	
	VkAccelerationStructureNV topAccelerationStructure;
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateAccelerationStructureNV
	CHECK_VK_RESULT(vkCreateAccelerationStructureNV(vkApp.vkDevice, &topAccelerationStructureCreateInfo, NULL, &topAccelerationStructure))
	
	//2.c
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#resources-acceleration-structures
	VkAccelerationStructureMemoryRequirementsInfoNV topAccelerationStructureMemoryRequirementInfo;
	topAccelerationStructureMemoryRequirementInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
	topAccelerationStructureMemoryRequirementInfo.pNext = NULL;
	topAccelerationStructureMemoryRequirementInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
	topAccelerationStructureMemoryRequirementInfo.accelerationStructure = topAccelerationStructure;
	VkMemoryRequirements2 topAccelerationStructMemoryRequirements;
	vkGetAccelerationStructureMemoryRequirementsNV(vkApp.vkDevice, &topAccelerationStructureMemoryRequirementInfo, &topAccelerationStructMemoryRequirements);
	
	VkMemoryAllocateInfo topAccelerationStructureMemoryInfo = {};
	topAccelerationStructureMemoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	topAccelerationStructureMemoryInfo.pNext = NULL;
	topAccelerationStructureMemoryInfo.allocationSize = topAccelerationStructMemoryRequirements.memoryRequirements.size;
	topAccelerationStructureMemoryInfo.memoryTypeIndex = vkApp.FindMemoryType(topAccelerationStructMemoryRequirements.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VkDeviceMemory topAccelerationStructureMemory;
	CHECK_VK_RESULT(vkAllocateMemory(vkApp.vkDevice, &topAccelerationStructureMemoryInfo, NULL, &topAccelerationStructureMemory))
	
	//2.d
	VkBindAccelerationStructureMemoryInfoNV topBindInfo = {};
	topBindInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
	topBindInfo.pNext = NULL;
	topBindInfo.accelerationStructure = topAccelerationStructure;
	topBindInfo.memory = topAccelerationStructureMemory;
	topBindInfo.memoryOffset = 0;
	topBindInfo.deviceIndexCount = 0;
	topBindInfo.pDeviceIndices = NULL;
	CHECK_VK_RESULT(vkBindAccelerationStructureMemoryNV(vkApp.vkDevice, 1, &topBindInfo))
	
	//2.e
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetAccelerationStructureHandleNV
	uint64_t topAccelerationStructureHandle;
	CHECK_VK_RESULT(vkGetAccelerationStructureHandleNV(vkApp.vkDevice, topAccelerationStructure, sizeof(uint64_t), &topAccelerationStructureHandle))
	
	//3.a
	//BottomLevel
	VkAccelerationStructureMemoryRequirementsInfoNV triangleAccelerationStructureMemoryRequirementInfoScratch;
	triangleAccelerationStructureMemoryRequirementInfoScratch.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
	triangleAccelerationStructureMemoryRequirementInfoScratch.pNext = NULL;
	triangleAccelerationStructureMemoryRequirementInfoScratch.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
	triangleAccelerationStructureMemoryRequirementInfoScratch.accelerationStructure = triangleAccStruct.accelerationStructure;
	VkMemoryRequirements2 triangleAccelerationStructMemoryRequirementsScratch;
	vkGetAccelerationStructureMemoryRequirementsNV(vkApp.vkDevice, &triangleAccelerationStructureMemoryRequirementInfoScratch, &triangleAccelerationStructMemoryRequirementsScratch);
	//TopLevel
	VkAccelerationStructureMemoryRequirementsInfoNV topAccelerationStructureMemoryRequirementInfoScratch;
	topAccelerationStructureMemoryRequirementInfoScratch.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
	topAccelerationStructureMemoryRequirementInfoScratch.pNext = NULL;
	topAccelerationStructureMemoryRequirementInfoScratch.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
	topAccelerationStructureMemoryRequirementInfoScratch.accelerationStructure = topAccelerationStructure;
	VkMemoryRequirements2 topAccelerationStructMemoryRequirementsScratch;
	vkGetAccelerationStructureMemoryRequirementsNV(vkApp.vkDevice, &topAccelerationStructureMemoryRequirementInfoScratch, &topAccelerationStructMemoryRequirementsScratch);
	//Largest size
	VkDeviceSize scratchBufferSize = std::max(triangleAccelerationStructMemoryRequirementsScratch.memoryRequirements.size, topAccelerationStructMemoryRequirementsScratch.memoryRequirements.size);
	VkBuffer scratchBuffer;
	VkDeviceMemory scratchBufferMemory;
	vkApp.CreateDeviceBuffer(scratchBufferSize, (void*)(NULL), VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, &scratchBuffer, &scratchBufferMemory);
	
	//3.b
	VkCommandBuffer tmpCommandBuffer;
	vkApp.AllocateGraphicsQueueCommandBuffer(&tmpCommandBuffer);
	
	//3.c
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = NULL;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = NULL;
	vkBeginCommandBuffer(tmpCommandBuffer, &beginInfo);
	
	//3.d
	vkCmdBuildAccelerationStructureNV(tmpCommandBuffer, &triangleAccStruct.accelerationStructureInfo, VK_NULL_HANDLE, 0, VK_FALSE, triangleAccStruct.accelerationStructure, VK_NULL_HANDLE, scratchBuffer, 0);
	
	//3.e
	VkMemoryBarrier memoryBarrier = {};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.pNext = NULL;
	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
	memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
	vkCmdPipelineBarrier(tmpCommandBuffer, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, 0, 1, &memoryBarrier, 0, NULL, 0, NULL);
	
	//3.f
	vkCmdBuildAccelerationStructureNV(tmpCommandBuffer, &topAccelerationStructureInfo, triangleAccStruct.geometryInstanceBuffer, 0, VK_FALSE, topAccelerationStructure, VK_NULL_HANDLE, scratchBuffer, 0);
	
	//3.g
	vkCmdPipelineBarrier(tmpCommandBuffer, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, 0, 1, &memoryBarrier, 0, NULL, 0, NULL);
	
	//3.h
	vkEndCommandBuffer(tmpCommandBuffer);
	
	//3.i
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = NULL;
	submitInfo.pWaitDstStageMask = NULL;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &tmpCommandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = NULL;
	vkQueueSubmit(vkApp.vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	
	//3.j
	vkQueueWaitIdle(vkApp.vkGraphicsQueue);
	
	//3.k
	vkApp.FreeGraphicsQueueCommandBuffer(&tmpCommandBuffer);
	
	//3.l
	vkFreeMemory(vkApp.vkDevice, scratchBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, scratchBuffer, NULL);
	
	//Ray tracing pipeline
	VkShaderModule rayGenShaderModule, closestHitShaderModule, missShaderModule;
	vkApp.CreateShaderModule("src/shaders/raygen.spv", &rayGenShaderModule);
	vkApp.CreateShaderModule("src/shaders/closesthit.spv", &closestHitShaderModule);
	vkApp.CreateShaderModule("src/shaders/miss.spv", &missShaderModule);
	
	std::vector<VkPipelineShaderStageCreateInfo> rayTracingShaderStageInfos(3);
	rayTracingShaderStageInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	rayTracingShaderStageInfos[0].pNext = NULL;
	rayTracingShaderStageInfos[0].flags = 0;
	rayTracingShaderStageInfos[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
	rayTracingShaderStageInfos[0].module = rayGenShaderModule;
	rayTracingShaderStageInfos[0].pName = "main";
	rayTracingShaderStageInfos[0].pSpecializationInfo = NULL;
	rayTracingShaderStageInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	rayTracingShaderStageInfos[1].pNext = NULL;
	rayTracingShaderStageInfos[1].flags = 0;
	rayTracingShaderStageInfos[1].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	rayTracingShaderStageInfos[1].module = closestHitShaderModule;
	rayTracingShaderStageInfos[1].pName = "main";
	rayTracingShaderStageInfos[1].pSpecializationInfo = NULL;
	rayTracingShaderStageInfos[2].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	rayTracingShaderStageInfos[2].pNext = NULL;
	rayTracingShaderStageInfos[2].flags = 0;
	rayTracingShaderStageInfos[2].stage = VK_SHADER_STAGE_MISS_BIT_NV;
	rayTracingShaderStageInfos[2].module = missShaderModule;
	rayTracingShaderStageInfos[2].pName = "main";
	rayTracingShaderStageInfos[2].pSpecializationInfo = NULL;
	
	std::vector<VkRayTracingShaderGroupCreateInfoNV> rayTracingGroupInfos(3);
	rayTracingGroupInfos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
	rayTracingGroupInfos[0].pNext = NULL;
	rayTracingGroupInfos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
	rayTracingGroupInfos[0].generalShader = 0;
	rayTracingGroupInfos[0].closestHitShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[0].anyHitShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[0].intersectionShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
	rayTracingGroupInfos[1].pNext = NULL;
	rayTracingGroupInfos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
	rayTracingGroupInfos[1].generalShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[1].closestHitShader = 1;
	rayTracingGroupInfos[1].anyHitShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[1].intersectionShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[2].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
	rayTracingGroupInfos[2].pNext = NULL;
	rayTracingGroupInfos[2].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
	rayTracingGroupInfos[2].generalShader = 2;
	rayTracingGroupInfos[2].closestHitShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[2].anyHitShader = VK_SHADER_UNUSED_NV;
	rayTracingGroupInfos[2].intersectionShader = VK_SHADER_UNUSED_NV;
	
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings(3);
	VkDescriptorSetLayoutBinding& accelerationStructureDescriptorSetLayoutBinding = descriptorSetLayoutBindings[0];
	accelerationStructureDescriptorSetLayoutBinding.binding = 0;
	accelerationStructureDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
	accelerationStructureDescriptorSetLayoutBinding.descriptorCount = 1;
	accelerationStructureDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;
	accelerationStructureDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
	VkDescriptorSetLayoutBinding& rayTracingImageDescriptorSetLayoutBinding = descriptorSetLayoutBindings[1];
	rayTracingImageDescriptorSetLayoutBinding.binding = 1;
	rayTracingImageDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	rayTracingImageDescriptorSetLayoutBinding.descriptorCount = 1;
	rayTracingImageDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;
	rayTracingImageDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
	VkDescriptorSetLayoutBinding& cameraDescriptorSetLayoutBinding = descriptorSetLayoutBindings[2];
	cameraDescriptorSetLayoutBinding.binding = 2;
	cameraDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	cameraDescriptorSetLayoutBinding.descriptorCount = 1;
	cameraDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;
	cameraDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {};
	descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo.pNext = NULL;
	descriptorSetLayoutInfo.flags = 0;
	descriptorSetLayoutInfo.bindingCount = descriptorSetLayoutBindings.size();
	descriptorSetLayoutInfo.pBindings = descriptorSetLayoutBindings.data();
	VkDescriptorSetLayout rayTracingPipelineDescriptorSetLayout;
	CHECK_VK_RESULT(vkCreateDescriptorSetLayout(vkApp.vkDevice, &descriptorSetLayoutInfo, NULL, &rayTracingPipelineDescriptorSetLayout))
	
	VkPipelineLayoutCreateInfo rayTracingPipelineLayoutInfo = {};
	rayTracingPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	rayTracingPipelineLayoutInfo.pNext = NULL;
	rayTracingPipelineLayoutInfo.flags = 0;
	rayTracingPipelineLayoutInfo.setLayoutCount = 1;
	rayTracingPipelineLayoutInfo.pSetLayouts = &rayTracingPipelineDescriptorSetLayout;
	rayTracingPipelineLayoutInfo.pushConstantRangeCount = 0;
	rayTracingPipelineLayoutInfo.pPushConstantRanges = NULL;
	VkPipelineLayout rayTracingPipelineLayout;
	CHECK_VK_RESULT(vkCreatePipelineLayout(vkApp.vkDevice, &rayTracingPipelineLayoutInfo, NULL, &rayTracingPipelineLayout))
	
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#pipelines-raytracing
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
	
	//Shader binding table
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
	
	//Descriptor set
	std::vector<VkDescriptorPoolSize> poolSizes = {
		{ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 }
	};
	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.pNext = NULL;
	descriptorPoolInfo.flags = 0;
	descriptorPoolInfo.maxSets = 1;
	descriptorPoolInfo.poolSizeCount = poolSizes.size();
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	VkDescriptorPool descriptorPool;
	CHECK_VK_RESULT(vkCreateDescriptorPool(vkApp.vkDevice, &descriptorPoolInfo, NULL, &descriptorPool))
	
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.pNext = NULL;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &rayTracingPipelineDescriptorSetLayout;
	VkDescriptorSet descriptorSet;
	CHECK_VK_RESULT(vkAllocateDescriptorSets(vkApp.vkDevice, &descriptorSetAllocateInfo, &descriptorSet))
	
	VkWriteDescriptorSetAccelerationStructureNV descriptorAccelerationStructureInfo = {};
    descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
    descriptorAccelerationStructureInfo.pNext = NULL;
    descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
    descriptorAccelerationStructureInfo.pAccelerationStructures = &topAccelerationStructure;

    VkWriteDescriptorSet accelerationStructureWrite = {};
    accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    accelerationStructureWrite.pNext = &descriptorAccelerationStructureInfo; // Notice that pNext is assigned here!
    accelerationStructureWrite.dstSet = descriptorSet;
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
    
    VkWriteDescriptorSet rayTracingImageWrite= {};
    rayTracingImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    rayTracingImageWrite.pNext = NULL;
    rayTracingImageWrite.dstSet = descriptorSet;
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
    
    VkWriteDescriptorSet cameraWrite= {};
    cameraWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    cameraWrite.pNext = NULL;
    cameraWrite.dstSet = descriptorSet;
    cameraWrite.dstBinding = 2;
    cameraWrite.dstArrayElement = 0;
    cameraWrite.descriptorCount = 1;
    cameraWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    cameraWrite.pImageInfo = NULL;
    cameraWrite.pBufferInfo = &descriptorCameraInfo;
    cameraWrite.pTexelBufferView = NULL;
    
    std::vector<VkWriteDescriptorSet> descriptorWrites = {
        accelerationStructureWrite,
        rayTracingImageWrite,
        cameraWrite
    };
    vkUpdateDescriptorSets(vkApp.vkDevice, descriptorWrites.size(), descriptorWrites.data(), 0, NULL);
    
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
		vkCmdBindDescriptorSets(graphicsQueueCommandBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, rayTracingPipelineLayout, 0, 1, &descriptorSet, 0, NULL);
		vkCmdTraceRaysNV(graphicsQueueCommandBuffers[i], 
			shaderBindingTableBuffer, 0 * physicalDeviceRayTracingProperties.shaderGroupHandleSize,
			shaderBindingTableBuffer, 2 * physicalDeviceRayTracingProperties.shaderGroupHandleSize, physicalDeviceRayTracingProperties.shaderGroupHandleSize,
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
	while (!glfwWindowShouldClose(vkApp.window)) {
		glfwPollEvents();
		vkApp.Render(graphicsQueueCommandBuffers.data());
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
	vkDestroyDescriptorSetLayout(vkApp.vkDevice, rayTracingPipelineDescriptorSetLayout, NULL);
	vkDestroyShaderModule(vkApp.vkDevice, missShaderModule, NULL);
	vkDestroyShaderModule(vkApp.vkDevice, closestHitShaderModule, NULL);
	vkDestroyShaderModule(vkApp.vkDevice, rayGenShaderModule, NULL);
	//TopLevel
	vkFreeMemory(vkApp.vkDevice, topAccelerationStructureMemory, NULL);
	vkDestroyAccelerationStructureNV(vkApp.vkDevice, topAccelerationStructure, NULL);
	//Basic data
	vkFreeMemory(vkApp.vkDevice, cameraBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, cameraBuffer, NULL);
	vkFreeMemory(vkApp.vkDevice, indexBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, indexBuffer, NULL);
	vkFreeMemory(vkApp.vkDevice, vertexBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, vertexBuffer, NULL);
}

int main(int argc, char** argv)
{
	//RasterizeTriangle();
	RaytraceTriangle();

	return EXIT_SUCCESS;
}
