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

	//Vertex buffer
	std::vector<float> vertexData = {
		0.25f, 0.25f, 0.0f,
        0.75f, 0.25f, 0.0f,
        0.50f, 0.75f, 0.0f
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
		e) Pipeline barrier
		f) Build TopLevel
		g) Pipeline barrier
		h) End command buffer
		i) Submit command buffer to graphics queue
		j) Wait on graphics queue to be idle
		k) Free command buffer
		l) Destroy scratch buffer and free scratch memory
	*/
	
	//Basic transform
	float basicTransform[12] = { 
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f
	};
	
	//1.a
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VkGeometryTrianglesNV
	VkGeometryTrianglesNV triangleInfo = {};
	triangleInfo.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
	triangleInfo.pNext = NULL;
	triangleInfo.vertexData = vertexBuffer;
	triangleInfo.vertexOffset = 0;
	triangleInfo.vertexCount = vertexData.size() / 3;
	triangleInfo.vertexStride = 3 * sizeof(float);
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
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = NULL;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = NULL;
	vkBeginCommandBuffer(tmpCommandBuffer, &beginInfo);
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdBuildAccelerationStructureNV
	//vkCmdBuildAccelerationStructureNV(tmpCommandBuffer, &triangleAccelerationStructureInfo, 
	
	//3.d
	vkCmdBuildAccelerationStructureNV(tmpCommandBuffer, &triangleAccelerationStructureInfo, VK_NULL_HANDLE, 0, VK_FALSE, triangleAccelerationStructure, VK_NULL_HANDLE, scratchBuffer, 0);
	
	//3.e
	VkMemoryBarrier memoryBarrier = {};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.pNext = NULL;
	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
	memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
	vkCmdPipelineBarrier(tmpCommandBuffer, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, 0, 1, &memoryBarrier, 0, NULL, 0, NULL);
	
	//3.f
	vkCmdBuildAccelerationStructureNV(tmpCommandBuffer, &topAccelerationStructureInfo, triangleGeometryInstanceBuffer, 0, VK_FALSE, topAccelerationStructure, VK_NULL_HANDLE, scratchBuffer, 0);
	
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
	
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings(2);
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
	vkApp.AllocateGraphicsQueueCommandBuffer(&tmpCommandBuffer);
	beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = NULL;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = NULL;
	vkBeginCommandBuffer(tmpCommandBuffer, &beginInfo);
	VkImageMemoryBarrier imageLayoutTransitionBarrier = {};
	imageLayoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageLayoutTransitionBarrier.pNext = NULL;
	imageLayoutTransitionBarrier.srcAccessMask = 0;
	imageLayoutTransitionBarrier.dstAccessMask = 0;
	imageLayoutTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageLayoutTransitionBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageLayoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageLayoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageLayoutTransitionBarrier.image = rayTracingImage;
	imageLayoutTransitionBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageLayoutTransitionBarrier.subresourceRange.baseMipLevel = 0;
	imageLayoutTransitionBarrier.subresourceRange.levelCount = 1;
	imageLayoutTransitionBarrier.subresourceRange.baseArrayLayer = 0;
	imageLayoutTransitionBarrier.subresourceRange.layerCount = 1;
	vkCmdPipelineBarrier(tmpCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &imageLayoutTransitionBarrier);
	vkEndCommandBuffer(tmpCommandBuffer);
	submitInfo = {};
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
	vkQueueWaitIdle(vkApp.vkGraphicsQueue);
	vkApp.FreeGraphicsQueueCommandBuffer(&tmpCommandBuffer);
	
	//Descriptor set
	std::vector<VkDescriptorPoolSize> poolSizes = {
		{ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
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
	
	VkWriteDescriptorSetAccelerationStructureNV descriptorAccelerationStructureInfo;
    descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
    descriptorAccelerationStructureInfo.pNext = NULL;
    descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
    descriptorAccelerationStructureInfo.pAccelerationStructures = &topAccelerationStructure;

    VkWriteDescriptorSet accelerationStructureWrite;
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
    
    VkDescriptorImageInfo descriptorOutputImageInfo;
    descriptorOutputImageInfo.sampler = VK_NULL_HANDLE;
    descriptorOutputImageInfo.imageView = rayTracingImageView;
    descriptorOutputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    
    VkWriteDescriptorSet rayTracingImageWrite;
    rayTracingImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    rayTracingImageWrite.pNext = NULL;
    rayTracingImageWrite.dstSet = descriptorSet;
    rayTracingImageWrite.dstBinding = 1;
    rayTracingImageWrite.dstArrayElement = 0;
    rayTracingImageWrite.descriptorCount = 1;
    rayTracingImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    rayTracingImageWrite.pImageInfo = &descriptorOutputImageInfo;
    rayTracingImageWrite.pBufferInfo = NULL;
    rayTracingImageWrite.pTexelBufferView = NULL;
    
    VkWriteDescriptorSet descriptorWrites[2] = {
        accelerationStructureWrite,
        rayTracingImageWrite
    };
    vkUpdateDescriptorSets(vkApp.vkDevice, 2, descriptorWrites, 0, NULL);
    
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
		VkImageMemoryBarrier imageBarrier = {};
		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier.pNext = NULL;
		imageBarrier.srcAccessMask = 0;
		imageBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier.image = rayTracingImage;
		imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBarrier.subresourceRange.baseMipLevel = 0;
		imageBarrier.subresourceRange.levelCount = 1;
		imageBarrier.subresourceRange.baseArrayLayer = 0;
		imageBarrier.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(graphicsQueueCommandBuffers[i], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, 0, 0, NULL, 0, NULL, 1, &imageBarrier);
		
		//Ray trace
		vkCmdBindPipeline(graphicsQueueCommandBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, rayTracingPipeline);
		vkCmdBindDescriptorSets(graphicsQueueCommandBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, rayTracingPipelineLayout, 0, 1, &descriptorSet, 0, NULL);
		vkCmdTraceRaysNV(graphicsQueueCommandBuffers[i], 
			shaderBindingTableBuffer, 0 * physicalDeviceRayTracingProperties.shaderGroupHandleSize,
			shaderBindingTableBuffer, 2 * physicalDeviceRayTracingProperties.shaderGroupHandleSize, physicalDeviceRayTracingProperties.shaderGroupHandleSize,
			shaderBindingTableBuffer, 1 * physicalDeviceRayTracingProperties.shaderGroupHandleSize, physicalDeviceRayTracingProperties.shaderGroupHandleSize,
			 VK_NULL_HANDLE, 0, 0, vkApp.vkSurfaceExtent.width, vkApp.vkSurfaceExtent.height, 1);
		
		//Barrier - wait for ray tracing to finish and transition rayTracingImage from GENERAL to TRANSFER_SRC
		imageBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageBarrier.image = rayTracingImage;
		vkCmdPipelineBarrier(graphicsQueueCommandBuffers[i], VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &imageBarrier);
		
		//Transition layout for swapchainImage from PRESENT_SRC to TRANSFER_DST
		imageBarrier.srcAccessMask = 0;
		imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier.image = vkApp.vkSwapchainImages[i];
		vkCmdPipelineBarrier(graphicsQueueCommandBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &imageBarrier);
		
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
		
		//Barrier - wait for copy to finish
		imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier.dstAccessMask = 0;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		imageBarrier.image = vkApp.vkSwapchainImages[i];
		vkCmdPipelineBarrier(graphicsQueueCommandBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &imageBarrier);
		
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
}

int main(int argc, char** argv)
{
	//RasterizeTriangle();
	RaytraceTriangle();

	return EXIT_SUCCESS;
}
