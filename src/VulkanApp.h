#ifndef VULKAN_APP_H
#define VULKAN_APP_H

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <vulkan/vulkan.h>

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

	//Functions
public:
	VulkanApp(const VulkanAppCreateInfo* createInfo);
	~VulkanApp();
	void CreateShaderModule(const char* spirvFile, VkShaderModule* shaderModule);
	void CreateHostVisibleBuffer(uint32_t bufferSize, void* bufferData, VkBufferUsageFlags bufferUsageFlags, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
	void CreateDeviceBuffer(uint32_t bufferSize, void* bufferData, VkBufferUsageFlags bufferUsageFlags, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
	
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
