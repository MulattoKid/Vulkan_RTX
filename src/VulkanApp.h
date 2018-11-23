#ifndef VULKAN_APP_H
#define VULKAN_APP_H

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <stdio.h>
#include <vulkan/vulkan.h>

#define VK_DEBUG 1

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
	void CreateShaderModule(const char* spirvFile, VkShaderModule* shaderModule);
	void CreateHostVisibleBuffer(uint32_t bufferSize, void* bufferData, VkBufferUsageFlags bufferUsageFlags, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
	void CreateDeviceBuffer(uint32_t bufferSize, void* bufferData, VkBufferUsageFlags bufferUsageFlags, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
	void AllocateGraphicsQueueCommandBuffer(VkCommandBuffer* commandBuffer);
	void FreeGraphicsQueueCommandBuffer(VkCommandBuffer* commandBuffer);
	VkViewport GetDefaultViewport();
	VkRect2D GetDefaultScissor();
	void CreateDefaultFramebuffers(std::vector<VkFramebuffer>& framebuffers, VkRenderPass renderPass);
	VkFormat GetDefaultFramebufferFormat();
	void AllocateDefaultGraphicsQueueCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers);
	void Render(VkCommandBuffer* commandBuffers);
	
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
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void CreateBuffer(uint32_t bufferSize, VkBufferUsageFlags bufferUsageFlags, VkBuffer* buffer, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceMemory* bufferMemory);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize copySize);
};

#endif
