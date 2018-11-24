#include <stdlib.h>
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

	auto vkCreateAccelerationStructureNVFunc = (PFN_vkCreateAccelerationStructureNV)vkGetInstanceProcAddr(vkApp.vkInstance, "vkCreateAccelerationStructureNV");
	if (vkCreateAccelerationStructureNVFunc == NULL)
	{
		printf("Failed to find address of function vkCreateAccelerationStructureNV\n");
		exit(EXIT_FAILURE);
	}
	auto vkDestroyAccelerationStructureNVFunc = (PFN_vkCreateAccelerationStructureNV)vkGetInstanceProcAddr(vkApp.vkInstance, "vkDestroyAccelerationStructureNV");
	if (vkDestroyAccelerationStructureNVFunc == NULL)
	{
		printf("Failed to find address of function vkDestroyAccelerationStructureNV\n");
		exit(EXIT_FAILURE);
	}

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
	
	//Bottom-level acceleration structure
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
	
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VkGeometryAABBNV
	VkGeometryAABBNV aabbInfo = {};
	aabbInfo.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
	aabbInfo.pNext = NULL;
	aabbInfo.aabbData = VK_NULL_HANDLE;
	/*IGNORED
	aabbInfo.numAABBs
	aabbInfo.stride
	aabbInfo.offset*/
	
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VkGeometryDataNV
	VkGeometryDataNV geometryDataInfo = {};
	geometryDataInfo.triangles = triangleInfo;
	geometryDataInfo.aabbs = aabbInfo;
	
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VkGeometryNV
	VkGeometryNV geometryInfo = {};
	geometryInfo.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
	geometryInfo.pNext = NULL;
	geometryInfo.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
	geometryInfo.geometry = geometryDataInfo;
	geometryInfo.flags = 0;
	
	//v1.1.85
	//https://vulkan.lunarg.com/doc/view/1.1.85.0/windows/vkspec.html
	/*VkAccelerationStructureCreateInfoNV accelerationStructureCreateInfo = {};
	accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
	accelerationStructureCreateInfo.pNext = NULL;
	accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
	accelerationStructureCreateInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV;
	accelerationStructureCreateInfo.compactedSize = 0;
	accelerationStructureCreateInfo.instanceCount = 0;
	accelerationStructureCreateInfo.geometryCount = 1;
	accelerationStructureCreateInfo.pGeometries = &geometryInfo;*/
	
	//v1.1.93
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VkAccelerationStructureInfoNV
	VkAccelerationStructureInfoNV accelerationStructureInfo = {};
	accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	accelerationStructureInfo.pNext = NULL;
	accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
	accelerationStructureInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV;
	accelerationStructureInfo.instanceCount = 1;
	accelerationStructureInfo.geometryCount = 1;
	accelerationStructureInfo.pGeometries = &geometryInfo;
	
	//https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdBuildAccelerationStructureNV
	VkAccelerationStructureCreateInfoNV accelerationStructureCreateInfo = {};
	accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
	accelerationStructureCreateInfo.pNext = NULL;
	accelerationStructureCreateInfo.compactedSize = 0;
	accelerationStructureCreateInfo.info = accelerationStructureInfo;
	
	VkAccelerationStructureNV bottomAccelerationStructure;
	CHECK_VK_RESULT(vkCreateAccelerationStructureNVFunc(vkApp.vkDevice, &accelerationStructureCreateInfo, NULL, &bottomAccelerationStructure))
	
	/*VkCommandBuffer tmpCommandBuffer;
	vkApp.AllocateGraphicsQueueCommandBuffer(&tmpCommandBuffer);
	
	vkCmdBuildAccelerationStructureNVX(tmpCommandBuffer, 
	
	vkApp.FreeGraphicsQueueCommandBuffer(&tmpCommandBuffer);*/
	
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
	vkDestroyAccelerationStructureNVFunc(vkApp.vkDevice, bottomAccelerationStructure, NULL);
	vkFreeMemory(vkApp.vkDevice, indexBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, indexBuffer, NULL);
	vkFreeMemory(vkApp.vkDevice, vertexBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, vertexBuffer, NULL);
	vkDestroyShaderModule(vkApp.vkDevice, fragmentShaderModule, NULL);
	vkDestroyShaderModule(vkApp.vkDevice, vertexShaderModule, NULL);
}

int main(int argc, char** argv)
{
	RasterizeTriangle();
	RaytraceTriangle();

	return EXIT_SUCCESS;
}
