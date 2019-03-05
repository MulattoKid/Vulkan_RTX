#ifndef VULKAN_APP_H
#define VULKAN_APP_H

#define VK_NO_PROTOTYPES
#define VK_DEBUG 1

#include "volk/volk.h"
#include "GLFW/glfw3.h"
#include "Camera.h"
#include <chrono>
#include "BrhanFile.h"
#include <stdio.h>

#ifdef VK_DEBUG
#define CHECK_VK_RESULT(res) if (res != VK_SUCCESS) { printf("Vulkan call %s:%i failed with error %i\n", __FILE__, __LINE__, res); exit(EXIT_FAILURE); }
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
	VkDeviceSize geometryInstancesBufferSize;
	TopAccStruct topAccStruct;
	VkBuffer scratchBuffer;
	VkDeviceMemory scratchBufferMemory;
	VkCommandBuffer buildCommandBuffer;
	
	~VulkanAccelerationStructure();
};

struct VulkanTexture
{
	VkDevice device;
	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;
	
	~VulkanTexture();
};

struct Material
{
	float diffuseColor[4];
};

struct Mesh
{
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> uvs;
	Material material;
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
	
	//Limits
	uint32_t maxBoundDescriptorSets;
	uint32_t maxInlineUniformBlockSize;
	
	//Swap chain
	VkSwapchainKHR vkSwapchain;
	std::vector<VkImage> vkSwapchainImages;
	std::vector<VkImageView> vkSwapchainImageViews;
	
	//Command pool and buffers
	VkCommandPool vkGraphicsQueueCommandPool;
	
	//Synchronization objects
	uint32_t maxFramesInFlight;
	std::vector<VkSemaphore> vkImageAvailableSemaphores;
	std::vector<VkSemaphore> vkRenderFinishedSemaphores;
	std::vector<VkFence> vkInFlightFences;
	uint32_t currentFrame = 0;
	
	// Other data
	Camera camera;
	double lastMouseX, lastMouseY;

	//Functions
public:
	VulkanApp() {}
	VulkanApp(const VulkanAppCreateInfo* createInfo);
	~VulkanApp();
	std::chrono::high_resolution_clock::time_point GetTime();
	void CreateShaderModule(const char* spirvFile, VkShaderModule* shaderModule);
	void AllocateGraphicsQueueCommandBuffer(VkCommandBuffer* commandBuffer);
	void FreeGraphicsQueueCommandBuffer(VkCommandBuffer* commandBuffer);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void CreateBuffer(uint32_t bufferSize, VkBufferUsageFlags bufferUsageFlags, VkBuffer* buffer, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceMemory* bufferMemory);
	void CopyBufferToBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize copySize);
	void CreateHostVisibleBuffer(uint32_t bufferSize, void* bufferData, VkBufferUsageFlags bufferUsageFlags, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
	void CreateDeviceBuffer(uint32_t bufferSize, void* bufferData, VkBufferUsageFlags bufferUsageFlags, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
	void UpdateHostVisibleBuffer(VkDeviceSize bufferSize, void* updateData, VkDeviceMemory bufferMemory);
	void TransitionImageLayoutSingle(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStage, VkAccessFlags srcAccessMask, VkPipelineStageFlags dstStage, VkAccessFlags dstAccessMask);
	void TransitionImageLayoutInProgress(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStage, VkAccessFlags srcAccessMask, VkPipelineStageFlags dstStage, VkAccessFlags dstAccessMask, VkCommandBuffer commandBuffer);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t imageWidth, uint32_t imageHeight);
	void CreateTexture(const char* filename, VkFormat format, VulkanTexture* texture);
	void CreateDefaultSampler(VkSampler* sampler, VkFilter filter, VkSamplerAddressMode addressMode);
	void CreateDummyImage(VkImage* image, VkDeviceMemory* imageMemory, VkImageView* imageView);
	VkViewport GetDefaultViewport();
	VkRect2D GetDefaultScissor();
	void CreateDefaultFramebuffers(std::vector<VkFramebuffer>& framebuffers, VkRenderPass renderPass);
	VkFormat GetDefaultFramebufferFormat();
	void AllocateDefaultGraphicsQueueCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers);
	void Render(VkCommandBuffer* commandBuffers, float rebuildTime);
	void RenderOffscreen(VkCommandBuffer* commandBuffers, float rebuildTime);
	void LoadMesh(const ModelFromFile& model, std::vector<Mesh>* meshes);
	void BuildColorAndAttributeData(const std::vector<Mesh>& meshes, std::vector<float>* perMeshAttributeData, std::vector<float>* perVertexAttributeData, std::vector<uint32_t>* customIDToAttributeArrayIndex);
	void CreateVulkanAccelerationStructure(const std::vector<std::vector<float>>& geometryData, VulkanAccelerationStructure* accStruct);
	void BuildAccelerationStructure(VulkanAccelerationStructure& accStruct);
	float RebuildAccelerationStructure(VulkanAccelerationStructure& accStruct, const std::vector<glm::mat4x4>& transformationData);
	
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
	void CreateBottomAccStruct(const std::vector<float>& geometry, VkGeometryInstanceNV* geometryInstance, uint32_t i, BottomAccStruct* bottomAccStruct, VkDevice device);
	void CreateTopAccStruct(uint32_t numInstances, TopAccStruct* topAccStruct, VkDevice device);
};

#endif
