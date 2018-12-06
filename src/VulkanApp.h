#ifndef VULKAN_APP_H
#define VULKAN_APP_H

#define VK_NO_PROTOTYPES
#define VK_DEBUG 1

#include "volk/volk.h"
#include "GLFW/glfw3.h"
#include <stdio.h>

#ifdef VK_DEBUG
#define CHECK_VK_RESULT(res) if (res != VK_SUCCESS) { printf("Vulkan call on line %i failed with error %i\n", __LINE__, res); exit(EXIT_FAILURE); }
#else
#define CHECK_VK_RESULT(res)
#endif

struct VulkanAppCreateInfo
{
	VkBool32 graphicsApp;
	uint32_t windowWidth;
	uint32_t windowHeight;
	const char* windowName;
	const char* appName;
	const char* engineName;
	uint32_t validationLayerCount;
	const char** validationLayerNames;
	uint32_t extensionCount;
	const char** extensionNames;
	uint32_t maxFramesInFlight;
};

struct VkGeometryInstanceNV
{
    float transform[12];
    uint32_t instanceCustomIndex : 24;
    uint32_t mask : 8;
    uint32_t instanceOffset : 24;
    uint32_t flags : 8;
    uint64_t accelerationStructureHandle;
};

struct BottomAccStruct
{
	VkDevice device;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	VkGeometryTrianglesNV triangleInfo;
	VkGeometryAABBNV aabbInfo;
	VkGeometryDataNV geometryDataInfo;
	VkGeometryNV geometryInfo;
	VkAccelerationStructureInfoNV accelerationStructureInfo;
	VkAccelerationStructureNV accelerationStructure;
	VkDeviceMemory accelerationStructureMemory;
	uint64_t accelerationStructureHandle;
	uint32_t geometryInstanceCustomIndex;
	
	~BottomAccStruct();
};

struct TopAccStruct
{
	VkDevice device;
	VkAccelerationStructureInfoNV accelerationStructureInfo;
	VkAccelerationStructureNV accelerationStructure;
	VkDeviceMemory accelerationStructureMemory;
	uint64_t accelerationStructureHandle;
	
	~TopAccStruct();
};

struct VulkanAccelerationStructure
{
	VkDevice device;
	std::vector<BottomAccStruct> bottomAccStructs;
	std::vector<VkGeometryInstanceNV> geometryInstances;
	VkBuffer geometryInstancesBuffer;
	VkDeviceMemory geometryInstancesBufferMemory;
	TopAccStruct topAccStruct;
	
	~VulkanAccelerationStructure();
};

struct VulkanApp
{
	//Window
	uint32_t windowWidth, windowHeight;
	GLFWwindow* window;
	
	//Instance
	VkInstance vkInstance;
	VkDebugUtilsMessengerEXT vkDebugCallback;
	
	//Surface
	VkSurfaceKHR vkSurface;
	VkSurfaceCapabilitiesKHR vkSurfaceCapabilities;
	std::vector<VkSurfaceFormatKHR> vkSupportedSurfaceFormats;
	VkSurfaceFormatKHR vkSurfaceFormat;
	std::vector<VkPresentModeKHR> vkSupportedPresentModes;
	VkPresentModeKHR vkPresentMode;
	VkExtent2D vkSurfaceExtent;
	
	//Queue info
	uint32_t vkGraphicsQueueIndex;
	VkQueue vkGraphicsQueue;
	uint32_t vkPresentQueueIndex;
	VkQueue vkPresentQueue;
	
	//Physical and logical device
	VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
	VkDevice vkDevice;
	
	//Swap chain
	VkSwapchainKHR vkSwapchain;
	std::vector<VkImage> vkSwapchainImages;
	std::vector<VkImageView> vkSwapchainImageViews;
	
	//Command pool and buffers
	VkCommandPool vkGraphicsQueueCommandPool;
	
	//Acceleration structure info
	uint32_t numAccelerationStructures = 0;
	
	//Synchronization objects
	uint32_t maxFramesInFlight;
	std::vector<VkSemaphore> vkImageAvailableSemaphores;
	std::vector<VkSemaphore> vkRenderFinishedSemaphores;
	std::vector<VkFence> vkInFlightFences;
	uint32_t currentFrame = 0;

	//Functions
public:
	VulkanApp(const VulkanAppCreateInfo* createInfo);
	~VulkanApp();
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void CreateShaderModule(const char* spirvFile, VkShaderModule* shaderModule);
	void CreateBuffer(uint32_t bufferSize, VkBufferUsageFlags bufferUsageFlags, VkBuffer* buffer, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceMemory* bufferMemory);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize copySize);
	void CreateHostVisibleBuffer(uint32_t bufferSize, void* bufferData, VkBufferUsageFlags bufferUsageFlags, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
	void CreateDeviceBuffer(uint32_t bufferSize, void* bufferData, VkBufferUsageFlags bufferUsageFlags, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
	void TransitionImageLayoutSingle(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStage, VkAccessFlags srcAccessMask, VkPipelineStageFlags dstStage, VkAccessFlags dstAccessMask);
	void TransitionImageLayoutInProgress(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStage, VkAccessFlags srcAccessMask, VkPipelineStageFlags dstStage, VkAccessFlags dstAccessMask, VkCommandBuffer commandBuffer);
	void AllocateGraphicsQueueCommandBuffer(VkCommandBuffer* commandBuffer);
	void FreeGraphicsQueueCommandBuffer(VkCommandBuffer* commandBuffer);
	VkViewport GetDefaultViewport();
	VkRect2D GetDefaultScissor();
	void CreateDefaultFramebuffers(std::vector<VkFramebuffer>& framebuffers, VkRenderPass renderPass);
	VkFormat GetDefaultFramebufferFormat();
	void AllocateDefaultGraphicsQueueCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers);
	void Render(VkCommandBuffer* commandBuffers);
	void RenderOffscreen(VkCommandBuffer* commandBuffers);
	void CreateVulkanAccelerationStructure(const std::vector<std::pair<std::vector<float>, std::vector<uint32_t>>>& geometryData, VulkanAccelerationStructure* accStruct);
	void BuildAccelerationStructure(const VulkanAccelerationStructure& accStruct);
	
private:
	void QuerySwapChainSupport(VkPhysicalDevice physicalDevice);
	VkBool32 PhysicalDeviceIsSuitable(const VkPhysicalDevice& phyiscalDevice, uint32_t extensionCount, const char** extensionNames);
	void PickPhysicalDevice(uint32_t extensionCount, const char** extensionNames);
	void FindQueueIndices();
	void CreateLogicalDevice(uint32_t extensionCount, const char** extensionNames);
	void ChooseSwapChainFormat();
	void ChoosePresentMode();
	void ChooseSwapExtent();
	void CreateSwapChain();
	void CreateImageViews();
	void CreateGraphicsQueueCommandPool();
	void CreateSyncObjects();
	std::vector<char> ReadShaderFile(const char* spirvFile);
	void CreateBottomAccStruct(const std::pair<std::vector<float>, std::vector<uint32_t>>& geometry, VkGeometryInstanceNV* geometryInstance, BottomAccStruct* bottomAccStruct, VkDevice device);
	void CreateTopAccStruct(uint32_t numInstances, TopAccStruct* topAccStruct, VkDevice device);
};

#endif
