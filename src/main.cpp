#include <stdlib.h>
#include <string.h>
#include <vector>
#include "VulkanApp.h"

void RasterizeTriangle()
{
	std::vector<const char*> validationLayerNames = {
		"VK_LAYER_LUNARG_standard_validation"
	};
	std::vector<const char*> extensionNames = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	VulkanAppCreateInfo vkAppInfo = {};
	vkAppInfo.graphicsApp = VK_TRUE;
	vkAppInfo.windowWidth = 1024;
	vkAppInfo.windowHeight = 1024;
	vkAppInfo.windowName = "Vulkan";
	vkAppInfo.appName = "Vulkan";
	vkAppInfo.engineName = "V";
	vkAppInfo.validationLayerCount = validationLayerNames.size();
	vkAppInfo.validationLayerNames = validationLayerNames.data();
	vkAppInfo.extensionCount = extensionNames.size();
	vkAppInfo.extensionNames = extensionNames.data();
	vkAppInfo.maxFramesInFlight = 2;
	VulkanApp vkApp(&vkAppInfo);

	//Shader modules
	VkShaderModule vertexShaderModule, fragmentShaderModule;
	vkApp.CreateShaderModule("src/shaders/vert.spv", &vertexShaderModule);
	vkApp.CreateShaderModule("src/shaders/frag.spv", &fragmentShaderModule);

	//Vertex buffer
	std::vector<float> vertexData = {
		0.0f, -0.5f,
		-0.5f, 0.5f,
		0.5f, 0.5f
	};
	VkDeviceSize vertexDataSize = vertexData.size() * sizeof(float);
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	vkApp.CreateDeviceBuffer(vertexDataSize, (void*)(vertexData.data()), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &vertexBuffer, &vertexBufferMemory);
	//Index buffer
	std::vector<uint32_t> indexData = {
		0, 1, 2
	};
	VkDeviceSize indexDataSize = indexData.size() * sizeof(uint32_t);
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	vkApp.CreateDeviceBuffer(indexDataSize, (void*)(indexData.data()), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, &indexBuffer, &indexBufferMemory);
	
	//Graphics pipeline
	std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos(2);
	shaderStageInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfos[0].pNext = NULL;
	shaderStageInfos[0].flags = 0;
	shaderStageInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageInfos[0].module = vertexShaderModule;
	shaderStageInfos[0].pName = "main";
	shaderStageInfos[0].pSpecializationInfo = NULL;
	shaderStageInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfos[1].pNext = NULL;
	shaderStageInfos[1].flags = 0;
	shaderStageInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageInfos[1].module = fragmentShaderModule;
	shaderStageInfos[1].pName = "main";
	shaderStageInfos[1].pSpecializationInfo = NULL;
	
	VkVertexInputBindingDescription inputDesc = {};
	inputDesc.binding = 0;
	inputDesc.stride = 2 * sizeof(float);
	inputDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	
	VkVertexInputAttributeDescription attributeDesc = {};
	attributeDesc.location = 0;
	attributeDesc.binding = 0;
	attributeDesc.format = VK_FORMAT_R32G32_SFLOAT;
	attributeDesc.offset = 0;
	
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.pNext = NULL;
	vertexInputInfo.flags = 0;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &inputDesc;
	vertexInputInfo.vertexAttributeDescriptionCount = 1;
	vertexInputInfo.pVertexAttributeDescriptions = &attributeDesc;
	
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
	
	VkPipelineLayout graphicsPipelineLayout;
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.pNext = NULL;
	pipelineLayoutInfo.flags = 0;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = NULL;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = NULL;
	CHECK_VK_RESULT(vkCreatePipelineLayout(vkApp.vkDevice, &pipelineLayoutInfo, NULL, &graphicsPipelineLayout))
	
	VkFormat outputFormat = vkApp.GetDefaultFramebufferFormat();
	VkAttachmentDescription attachment = {};
	attachment.flags = 0;
	attachment.format = outputFormat;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	
	VkSubpassDescription subpass = {};
	subpass.flags = 0;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = NULL;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;
	
	VkSubpassDependency subpassDependency = {};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependency.dependencyFlags = 0;
	
	VkRenderPass renderPass;
	VkRenderPassCreateInfo renderpassInfo = {};
	renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassInfo.pNext = NULL;
	renderpassInfo.flags = 0;
	renderpassInfo.attachmentCount = 1;
	renderpassInfo.pAttachments = &attachment;
	renderpassInfo.subpassCount = 1;
	renderpassInfo.pSubpasses = &subpass;
	renderpassInfo.dependencyCount = 1;
	renderpassInfo.pDependencies = &subpassDependency;
	CHECK_VK_RESULT(vkCreateRenderPass(vkApp.vkDevice, &renderpassInfo, NULL, &renderPass))
	
	VkPipeline graphicsPipeline;
	VkGraphicsPipelineCreateInfo graphicsPipelineInfo = {};
	graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineInfo.pNext = NULL;
	graphicsPipelineInfo.flags = 0;
	graphicsPipelineInfo.stageCount = shaderStageInfos.size();
	graphicsPipelineInfo.pStages = shaderStageInfos.data();
	graphicsPipelineInfo.pVertexInputState = &vertexInputInfo;
	graphicsPipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	graphicsPipelineInfo.pTessellationState = NULL;
	graphicsPipelineInfo.pViewportState = &viewportInfo;
	graphicsPipelineInfo.pRasterizationState = &rasterizationInfo;
	graphicsPipelineInfo.pMultisampleState = &multisampleInfo;
	graphicsPipelineInfo.pDepthStencilState = NULL;
	graphicsPipelineInfo.pColorBlendState = &colorBlendInfo;
	graphicsPipelineInfo.pDynamicState = NULL;
	graphicsPipelineInfo.layout = graphicsPipelineLayout;
	graphicsPipelineInfo.renderPass = renderPass;
	graphicsPipelineInfo.subpass = 0;
	graphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	/* IGNORED
	graphics_pipeline_info.basePipelineIndex*/
	CHECK_VK_RESULT(vkCreateGraphicsPipelines(vkApp.vkDevice, VK_NULL_HANDLE, 1, &graphicsPipelineInfo, NULL, &graphicsPipeline))
	
	//Framebuffers
	std::vector<VkFramebuffer> defaultFramebuffers;
	vkApp.CreateDefaultFramebuffers(defaultFramebuffers, renderPass);
	
	//Command buffers
	std::vector<VkCommandBuffer> graphicsQueueCommandBuffers;
	vkApp.AllocateDefaultGraphicsQueueCommandBuffers(graphicsQueueCommandBuffers);
	//Record
	for (size_t i = 0; i < graphicsQueueCommandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = NULL;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = NULL;
		CHECK_VK_RESULT(vkBeginCommandBuffer(graphicsQueueCommandBuffers[i], &beginInfo))
		
		VkClearColorValue clearColorValue = { 0.0f, 0.0f, 0.0f, 1.0f };
		VkClearValue clearValue = { clearColorValue };
		VkRenderPassBeginInfo renderpassInfo = {};
		renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpassInfo.pNext = NULL;
		renderpassInfo.renderPass = renderPass;
		renderpassInfo.framebuffer = defaultFramebuffers[i];
		renderpassInfo.renderArea.offset = { 0, 0 };
		renderpassInfo.renderArea.extent = vkApp.vkSurfaceExtent;
		renderpassInfo.clearValueCount = 1;
		renderpassInfo.pClearValues = &clearValue;
		vkCmdBeginRenderPass(graphicsQueueCommandBuffers[i], &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);
		
		vkCmdBindPipeline(graphicsQueueCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(graphicsQueueCommandBuffers[i], 0, 1, &vertexBuffer, &offset);
		vkCmdBindIndexBuffer(graphicsQueueCommandBuffers[i], indexBuffer, offset, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(graphicsQueueCommandBuffers[i], uint32_t(indexData.size()), 1, 0, 0, 0);
		vkCmdEndRenderPass(graphicsQueueCommandBuffers[i]);
		
		CHECK_VK_RESULT(vkEndCommandBuffer(graphicsQueueCommandBuffers[i]))
	}
	
	//Render
	while (!glfwWindowShouldClose(vkApp.window)) {
		glfwPollEvents();
		vkApp.Render(graphicsQueueCommandBuffers.data());
	}
	vkDeviceWaitIdle(vkApp.vkDevice);
	
	//Cleanup
	for (size_t i = 0; i < defaultFramebuffers.size(); i++)
	{
		vkDestroyFramebuffer(vkApp.vkDevice, defaultFramebuffers[i], NULL);
	}
	vkDestroyPipeline(vkApp.vkDevice, graphicsPipeline, NULL);
	vkDestroyRenderPass(vkApp.vkDevice, renderPass, NULL);
	vkDestroyPipelineLayout(vkApp.vkDevice, graphicsPipelineLayout, NULL);
	vkFreeMemory(vkApp.vkDevice, indexBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, indexBuffer, NULL);
	vkFreeMemory(vkApp.vkDevice, vertexBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, vertexBuffer, NULL);
	vkDestroyShaderModule(vkApp.vkDevice, fragmentShaderModule, NULL);
	vkDestroyShaderModule(vkApp.vkDevice, vertexShaderModule, NULL);
}

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

	//Shader modules
	VkShaderModule vertexShaderModule, fragmentShaderModule;
	vkApp.CreateShaderModule("src/shaders/vert.spv", &vertexShaderModule);
	vkApp.CreateShaderModule("src/shaders/frag.spv", &fragmentShaderModule);

	//Vertex buffer
	std::vector<float> vertexData = {
		0.0f, -0.5f,
		-0.5f, 0.5f,
		0.5f, 0.5f
	};
	VkDeviceSize vertexDataSize = vertexData.size() * sizeof(float);
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	vkApp.CreateDeviceBuffer(vertexDataSize, (void*)(vertexData.data()), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &vertexBuffer, &vertexBufferMemory);
	//Index buffer
	std::vector<uint32_t> indexData = {
		0, 1, 2
	};
	VkDeviceSize indexDataSize = indexData.size() * sizeof(uint32_t);
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	vkApp.CreateDeviceBuffer(indexDataSize, (void*)(indexData.data()), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, &indexBuffer, &indexBufferMemory);
	//Basic transform
	float basicTransform[12] = { 1.0f, 0.0f, 0.0f, 0.0f,
							0.0f, 1.0f, 0.0f, 0.0f,
							0.0f, 0.0f, 1.0f, 0.0f };
	
	/*
	Acceleration structure steps:
	1) Create BottomLevel acceleration structure
		a) Specify geometry data buffers and format
		b) Specify AABBs
		c) Link a) to b)
		d) Specify geometry type and link to c)
		e) Specify acceleration structure (bottom level) and connect to d)
		f) Create
		g) Allocate memory for the acceleration structure
		h) Bind memory to acceleration structure
		i) Get uint64_t handle to acceleration structure
		j) Create an instance of the geometry using the handle from g)
		k) Create a buffer containing the instance created in j)
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
		e) Build TopLevel
		f) End command buffer
		g) Submit command buffer
		h) Wait on device to be idle
		i) Free command buffer
		j) Destroy scratch buffer and free scratch memory
	*/
	
	//1.a
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VkGeometryTrianglesNV
	VkGeometryTrianglesNV triangleInfo = {};
	triangleInfo.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
	triangleInfo.pNext = NULL;
	triangleInfo.vertexData = vertexBuffer;
	triangleInfo.vertexOffset = 0;
	triangleInfo.vertexCount = vertexData.size() / 2;
	triangleInfo.vertexStride = 2 * sizeof(float);
	triangleInfo.vertexFormat = VK_FORMAT_R32G32_SFLOAT;
	triangleInfo.indexData = indexBuffer;
	triangleInfo.indexOffset = 0;
	triangleInfo.indexCount = indexData.size();
	triangleInfo.indexType = VK_INDEX_TYPE_UINT32;
	triangleInfo.transformData = VK_NULL_HANDLE;
	//triangleInfo.transformOffset IGNORED
	
	//1.b
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VkGeometryAABBNV
	VkGeometryAABBNV aabbInfo = {};
	aabbInfo.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
	aabbInfo.pNext = NULL;
	aabbInfo.aabbData = VK_NULL_HANDLE;
	/*IGNORED
	aabbInfo.numAABBs
	aabbInfo.stride
	aabbInfo.offset*/
	
	//1.c
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VkGeometryDataNV
	VkGeometryDataNV triangleGeometryDataInfo = {};
	triangleGeometryDataInfo.triangles = triangleInfo;
	triangleGeometryDataInfo.aabbs = aabbInfo;

	//1.d
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VkGeometryNV
	VkGeometryNV triangleGeometryInfo = {};
	triangleGeometryInfo.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
	triangleGeometryInfo.pNext = NULL;
	triangleGeometryInfo.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
	triangleGeometryInfo.geometry = triangleGeometryDataInfo;
	triangleGeometryInfo.flags = 0;
	
	//1.e
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VkAccelerationStructureInfoNV
	VkAccelerationStructureInfoNV triangleAccelerationStructureInfo = {};
	triangleAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	triangleAccelerationStructureInfo.pNext = NULL;
	triangleAccelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
	triangleAccelerationStructureInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV;
	triangleAccelerationStructureInfo.instanceCount = 0;
	triangleAccelerationStructureInfo.geometryCount = 1;
	triangleAccelerationStructureInfo.pGeometries = &triangleGeometryInfo;
	
	//1.f
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VkAccelerationStructureCreateInfoNV
	VkAccelerationStructureCreateInfoNV triangleAccelerationStructureCreateInfo = {};
	triangleAccelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
	triangleAccelerationStructureCreateInfo.pNext = NULL;
	triangleAccelerationStructureCreateInfo.compactedSize = 0;
	triangleAccelerationStructureCreateInfo.info = triangleAccelerationStructureInfo;	
	VkAccelerationStructureNV triangleAccelerationStructure;
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateAccelerationStructureNV
	CHECK_VK_RESULT(vkCreateAccelerationStructureNV(vkApp.vkDevice, &triangleAccelerationStructureCreateInfo, NULL, &triangleAccelerationStructure))
	
	//1.g
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#resources-acceleration-structures
	VkAccelerationStructureMemoryRequirementsInfoNV triangleAccelerationStructureMemoryRequirementInfo;
	triangleAccelerationStructureMemoryRequirementInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
	triangleAccelerationStructureMemoryRequirementInfo.pNext = NULL;
	triangleAccelerationStructureMemoryRequirementInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
	triangleAccelerationStructureMemoryRequirementInfo.accelerationStructure = triangleAccelerationStructure;
	VkMemoryRequirements2 triangleAccelerationStructMemoryRequirements;
	vkGetAccelerationStructureMemoryRequirementsNV(vkApp.vkDevice, &triangleAccelerationStructureMemoryRequirementInfo, &triangleAccelerationStructMemoryRequirements);
	
	VkMemoryAllocateInfo triangleAccelerationStructureMemoryInfo = {};
	triangleAccelerationStructureMemoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	triangleAccelerationStructureMemoryInfo.pNext = NULL;
	triangleAccelerationStructureMemoryInfo.allocationSize = triangleAccelerationStructMemoryRequirements.memoryRequirements.size;
	triangleAccelerationStructureMemoryInfo.memoryTypeIndex = vkApp.FindMemoryType(triangleAccelerationStructMemoryRequirements.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VkDeviceMemory triangleAccelerationStructureMemory;
	CHECK_VK_RESULT(vkAllocateMemory(vkApp.vkDevice, &triangleAccelerationStructureMemoryInfo, NULL, &triangleAccelerationStructureMemory))
	
	//1.h
	VkBindAccelerationStructureMemoryInfoNV triangleBindInfo = {};
	triangleBindInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
	triangleBindInfo.pNext = NULL;
	triangleBindInfo.accelerationStructure = triangleAccelerationStructure;
	triangleBindInfo.memory = triangleAccelerationStructureMemory;
	triangleBindInfo.memoryOffset = 0;
	triangleBindInfo.deviceIndexCount = 0;
	triangleBindInfo.pDeviceIndices = NULL;
	CHECK_VK_RESULT(vkBindAccelerationStructureMemoryNV(vkApp.vkDevice, 1, &triangleBindInfo))
	
	//1.i
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetAccelerationStructureHandleNV
	uint64_t triangleAccelerationStructureHandle;
	CHECK_VK_RESULT(vkGetAccelerationStructureHandleNV(vkApp.vkDevice, triangleAccelerationStructure, sizeof(uint64_t), &triangleAccelerationStructureHandle))
	
	//1.j
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#acceleration-structure-instance
	VkGeometryInstanceNV triangleGeometryInstance = {};
	memcpy(triangleGeometryInstance.transform, basicTransform, sizeof(float) * 12);
	triangleGeometryInstance.instanceCustomIndex = 0;
	triangleGeometryInstance.mask = 0xff;
	triangleGeometryInstance.instanceOffset = 0;
	triangleGeometryInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
	triangleGeometryInstance.accelerationStructureHandle = triangleAccelerationStructureHandle;
	
	//1.k
	VkBuffer triangleGeometryInstanceBuffer;
	VkDeviceMemory triangleGeometryInstanceBufferMemory;
	VkDeviceSize triangleGeometryInstanceBufferSize = sizeof(VkGeometryInstanceNV);
	vkApp.CreateDeviceBuffer(triangleGeometryInstanceBufferSize, (void*)(&triangleGeometryInstance), VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, &triangleGeometryInstanceBuffer, &triangleGeometryInstanceBufferMemory);
	
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
	triangleAccelerationStructureMemoryRequirementInfoScratch.accelerationStructure = triangleAccelerationStructure;
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
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdBuildAccelerationStructureNV
	//vkCmdBuildAccelerationStructureNV(tmpCommandBuffer, &triangleAccelerationStructureInfo, 
	
	//3.d
	//3.e
	//3.f
	//3.g
	//3.h
	//3.i
	vkApp.FreeGraphicsQueueCommandBuffer(&tmpCommandBuffer);
	
	//3.j
	vkFreeMemory(vkApp.vkDevice, scratchBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, scratchBuffer, NULL);
	
	//Graphics pipeline
	std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos(2);
	shaderStageInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfos[0].pNext = NULL;
	shaderStageInfos[0].flags = 0;
	shaderStageInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageInfos[0].module = vertexShaderModule;
	shaderStageInfos[0].pName = "main";
	shaderStageInfos[0].pSpecializationInfo = NULL;
	shaderStageInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfos[1].pNext = NULL;
	shaderStageInfos[1].flags = 0;
	shaderStageInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageInfos[1].module = fragmentShaderModule;
	shaderStageInfos[1].pName = "main";
	shaderStageInfos[1].pSpecializationInfo = NULL;
	
	VkVertexInputBindingDescription inputDesc = {};
	inputDesc.binding = 0;
	inputDesc.stride = 2 * sizeof(float);
	inputDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	
	VkVertexInputAttributeDescription attributeDesc = {};
	attributeDesc.location = 0;
	attributeDesc.binding = 0;
	attributeDesc.format = VK_FORMAT_R32G32_SFLOAT;
	attributeDesc.offset = 0;
	
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.pNext = NULL;
	vertexInputInfo.flags = 0;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &inputDesc;
	vertexInputInfo.vertexAttributeDescriptionCount = 1;
	vertexInputInfo.pVertexAttributeDescriptions = &attributeDesc;
	
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
	
	VkPipelineLayout graphicsPipelineLayout;
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.pNext = NULL;
	pipelineLayoutInfo.flags = 0;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = NULL;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = NULL;
	CHECK_VK_RESULT(vkCreatePipelineLayout(vkApp.vkDevice, &pipelineLayoutInfo, NULL, &graphicsPipelineLayout))
	
	VkFormat outputFormat = vkApp.GetDefaultFramebufferFormat();
	VkAttachmentDescription attachment = {};
	attachment.flags = 0;
	attachment.format = outputFormat;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	
	VkSubpassDescription subpass = {};
	subpass.flags = 0;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = NULL;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;
	
	VkSubpassDependency subpassDependency = {};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependency.dependencyFlags = 0;
	
	VkRenderPass renderPass;
	VkRenderPassCreateInfo renderpassInfo = {};
	renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassInfo.pNext = NULL;
	renderpassInfo.flags = 0;
	renderpassInfo.attachmentCount = 1;
	renderpassInfo.pAttachments = &attachment;
	renderpassInfo.subpassCount = 1;
	renderpassInfo.pSubpasses = &subpass;
	renderpassInfo.dependencyCount = 1;
	renderpassInfo.pDependencies = &subpassDependency;
	CHECK_VK_RESULT(vkCreateRenderPass(vkApp.vkDevice, &renderpassInfo, NULL, &renderPass))
	
	VkPipeline graphicsPipeline;
	VkGraphicsPipelineCreateInfo graphicsPipelineInfo = {};
	graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineInfo.pNext = NULL;
	graphicsPipelineInfo.flags = 0;
	graphicsPipelineInfo.stageCount = shaderStageInfos.size();
	graphicsPipelineInfo.pStages = shaderStageInfos.data();
	graphicsPipelineInfo.pVertexInputState = &vertexInputInfo;
	graphicsPipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	graphicsPipelineInfo.pTessellationState = NULL;
	graphicsPipelineInfo.pViewportState = &viewportInfo;
	graphicsPipelineInfo.pRasterizationState = &rasterizationInfo;
	graphicsPipelineInfo.pMultisampleState = &multisampleInfo;
	graphicsPipelineInfo.pDepthStencilState = NULL;
	graphicsPipelineInfo.pColorBlendState = &colorBlendInfo;
	graphicsPipelineInfo.pDynamicState = NULL;
	graphicsPipelineInfo.layout = graphicsPipelineLayout;
	graphicsPipelineInfo.renderPass = renderPass;
	graphicsPipelineInfo.subpass = 0;
	graphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	/* IGNORED
	graphics_pipeline_info.basePipelineIndex*/
	CHECK_VK_RESULT(vkCreateGraphicsPipelines(vkApp.vkDevice, VK_NULL_HANDLE, 1, &graphicsPipelineInfo, NULL, &graphicsPipeline))
	
	//Framebuffers
	std::vector<VkFramebuffer> defaultFramebuffers;
	vkApp.CreateDefaultFramebuffers(defaultFramebuffers, renderPass);
	
	//Command buffers
	std::vector<VkCommandBuffer> graphicsQueueCommandBuffers;
	vkApp.AllocateDefaultGraphicsQueueCommandBuffers(graphicsQueueCommandBuffers);
	//Record
	for (size_t i = 0; i < graphicsQueueCommandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = NULL;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = NULL;
		CHECK_VK_RESULT(vkBeginCommandBuffer(graphicsQueueCommandBuffers[i], &beginInfo))
		
		VkClearColorValue clearColorValue = { 0.0f, 0.0f, 0.0f, 1.0f };
		VkClearValue clearValue = { clearColorValue };
		VkRenderPassBeginInfo renderpassInfo = {};
		renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpassInfo.pNext = NULL;
		renderpassInfo.renderPass = renderPass;
		renderpassInfo.framebuffer = defaultFramebuffers[i];
		renderpassInfo.renderArea.offset = { 0, 0 };
		renderpassInfo.renderArea.extent = vkApp.vkSurfaceExtent;
		renderpassInfo.clearValueCount = 1;
		renderpassInfo.pClearValues = &clearValue;
		vkCmdBeginRenderPass(graphicsQueueCommandBuffers[i], &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);
		
		vkCmdBindPipeline(graphicsQueueCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(graphicsQueueCommandBuffers[i], 0, 1, &vertexBuffer, &offset);
		vkCmdBindIndexBuffer(graphicsQueueCommandBuffers[i], indexBuffer, offset, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(graphicsQueueCommandBuffers[i], uint32_t(indexData.size()), 1, 0, 0, 0);
		vkCmdEndRenderPass(graphicsQueueCommandBuffers[i]);
		
		CHECK_VK_RESULT(vkEndCommandBuffer(graphicsQueueCommandBuffers[i]))
	}
	
	//Render
	while (!glfwWindowShouldClose(vkApp.window)) {
		glfwPollEvents();
		vkApp.Render(graphicsQueueCommandBuffers.data());
	}
	vkDeviceWaitIdle(vkApp.vkDevice);
	
	
	//Cleanup
	for (size_t i = 0; i < defaultFramebuffers.size(); i++)
	{
		vkDestroyFramebuffer(vkApp.vkDevice, defaultFramebuffers[i], NULL);
	}
	vkDestroyPipeline(vkApp.vkDevice, graphicsPipeline, NULL);
	vkDestroyRenderPass(vkApp.vkDevice, renderPass, NULL);
	vkDestroyPipelineLayout(vkApp.vkDevice, graphicsPipelineLayout, NULL);
	
	//TopLevel
	vkFreeMemory(vkApp.vkDevice, topAccelerationStructureMemory, NULL);
	vkDestroyAccelerationStructureNV(vkApp.vkDevice, topAccelerationStructure, NULL);
	//BottomLevel
	vkFreeMemory(vkApp.vkDevice, triangleGeometryInstanceBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, triangleGeometryInstanceBuffer, NULL);
	vkFreeMemory(vkApp.vkDevice, triangleAccelerationStructureMemory, NULL);
	vkDestroyAccelerationStructureNV(vkApp.vkDevice, triangleAccelerationStructure, NULL);
	//Basic data
	vkFreeMemory(vkApp.vkDevice, indexBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, indexBuffer, NULL);
	vkFreeMemory(vkApp.vkDevice, vertexBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, vertexBuffer, NULL);
	vkDestroyShaderModule(vkApp.vkDevice, fragmentShaderModule, NULL);
	vkDestroyShaderModule(vkApp.vkDevice, vertexShaderModule, NULL);
}

int main(int argc, char** argv)
{
	//RasterizeTriangle();
	RaytraceTriangle();

	return EXIT_SUCCESS;
}
