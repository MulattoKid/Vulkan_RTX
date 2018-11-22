#include <fstream>
#include <limits>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include "VulkanApp.h"

#define VK_DEBUG 1

#ifdef VK_DEBUG
#define CHECK_VK_RESULT(res) if (res != VK_SUCCESS) { printf("Vulkan call on line %i failed with error %i\n", __LINE__, res); exit(EXIT_FAILURE); }
#else
#define CHECK_VK_RESULT(res)
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	printf("validation layer: %s\n", pCallbackData->pMessage);
	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != NULL)
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != NULL)
	{
		func(instance, callback, pAllocator);
	}
}

void CheckValidationLayerSupport(uint32_t reqLayerCount, const char** reqLayerNames)
{
	//List available validation layers
	uint32_t layerCount = 0;
	CHECK_VK_RESULT(vkEnumerateInstanceLayerProperties(&layerCount, NULL))
	std::vector<VkLayerProperties> availableLayers(layerCount);
	CHECK_VK_RESULT(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()))
	printf("Available validation layers:\n");
	for (uint32_t i = 0; i < layerCount; i++)
	{
		printf("\t%s\n", availableLayers[i].layerName);
	}

	//Check for layer support
	for (uint32_t i = 0; i < reqLayerCount; i++)
	{
		bool foundLayer = false;
		for (uint32_t j = 0; j < layerCount; j++)
		{
			if (strcmp(reqLayerNames[i], availableLayers[j].layerName) == 0)
			{
				foundLayer = true;
				break;
			}
		}

		if (!foundLayer)
		{
			printf("Requested validation layer '%s' is not available\n", reqLayerNames[i]);
			exit(EXIT_FAILURE);
		}
	}
}

std::vector<const char*> CheckVulkanInstanceExtensionSupport()
{
	//List available instance extensions
	uint32_t availableExtensionCount = 0;
	CHECK_VK_RESULT(vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionCount, NULL))
	std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
	CHECK_VK_RESULT(vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionCount, availableExtensions.data()))
	printf("Available instance extensions:\n");
	for (uint32_t i = 0; i < availableExtensionCount; i++)
	{
		printf("\t%s\n", availableExtensions[i].extensionName);
	}

	//Instance extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	//Check for extension support
	for (uint32_t i = 0; i < extensions.size(); i++)
	{
		bool foundExtension = false;
		for (uint32_t j = 0; j < availableExtensionCount; j++)
		{
			if (strcmp(extensions[i], availableExtensions[j].extensionName) == 0)
			{
				foundExtension = true;
				break;
			}
		}

		if (!foundExtension)
		{
			printf("Required instance extension '%s' is not supported\n", extensions[i]);
			exit(EXIT_FAILURE);
		}
	}
	
	return extensions;
}

void SetupDebugCallback(VkInstance* vkInstance, VkDebugUtilsMessengerEXT* vkDebugCallback)
{
	VkDebugUtilsMessengerCreateInfoEXT debugInfo = {};
	debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugInfo.pfnUserCallback = DebugCallback;
	debugInfo.pUserData = NULL;
	CHECK_VK_RESULT(CreateDebugUtilsMessengerEXT(*vkInstance, &debugInfo, NULL, vkDebugCallback))
}

void VulkanApp::QuerySwapChainSupport(VkPhysicalDevice physicalDevice)
{
	CHECK_VK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, vkSurface, &vkSurfaceCapabilities))

	uint32_t formatCount;
	CHECK_VK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vkSurface, &formatCount, NULL))
	if (formatCount > 0)
	{
		vkSupportedSurfaceFormats.resize(formatCount);
		CHECK_VK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vkSurface, &formatCount, vkSupportedSurfaceFormats.data()))
	}

	uint32_t presentModeCount;
	CHECK_VK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vkSurface, &presentModeCount, NULL))
	if (presentModeCount > 0)
	{
		vkSupportedPresentModes.resize(presentModeCount);
		CHECK_VK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vkSurface, &presentModeCount, vkSupportedPresentModes.data()))
	}
}

VkBool32 VulkanApp::PhysicalDeviceIsSuitable(const VkPhysicalDevice& phyiscalDevice, uint32_t extensionCount, const char** extensionNames)
{
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(phyiscalDevice, &properties);
	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(phyiscalDevice, &features);

	uint32_t availableExtensionCount = 0;
	CHECK_VK_RESULT(vkEnumerateDeviceExtensionProperties(phyiscalDevice, NULL, &availableExtensionCount, NULL))
	std::vector<VkExtensionProperties> availableExtensionProperties(availableExtensionCount);
	CHECK_VK_RESULT(vkEnumerateDeviceExtensionProperties(phyiscalDevice, NULL, &availableExtensionCount, availableExtensionProperties.data()))

	std::vector<std::string> requiredExtensionsVector(extensionCount);
	for (uint32_t i = 0; i < extensionCount; i++)
	{
		requiredExtensionsVector[i] = extensionNames[i];
	}
	std::set<std::string> requiredExtensions(requiredExtensionsVector.begin(), requiredExtensionsVector.end());
	for (auto& extensionProperty : availableExtensionProperties)
	{
		requiredExtensions.erase(extensionProperty.extensionName);
	}

	QuerySwapChainSupport(phyiscalDevice);

	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && requiredExtensions.empty() && !vkSupportedSurfaceFormats.empty() && !vkSupportedPresentModes.empty())
	{
		printf("Chose %s as physical device\n", properties.deviceName);
		return VK_TRUE;
	}
	
	return VK_FALSE;
}

void VulkanApp::PickPhysicalDevice(uint32_t extensionCount, const char** extensionNames)
{
	//Find devices
	uint32_t deviceCount = 0;
	CHECK_VK_RESULT(vkEnumeratePhysicalDevices(vkInstance, &deviceCount, NULL));
	if (deviceCount == 0)
	{
		printf("Failed to find any GPU with Vulkan support\n");
		exit(1);
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	CHECK_VK_RESULT(vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data()));

	//Pick device
	for (uint32_t i = 0; i < deviceCount; i++)
	{
		if (PhysicalDeviceIsSuitable(devices[i], extensionCount, extensionNames))
		{
			vkPhysicalDevice = devices[i];
			return;
		}
	}
	
	printf("Did not find a suitable GPU\n");
	exit(EXIT_FAILURE);
}

void VulkanApp::FindQueueIndices()
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, NULL);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, queueFamilies.data());

	bool foundGraphicsQueue = false;
	bool foundPresentQueue = false;
	for (uint32_t i = 0; i < queueFamilyCount; i++)
	{
		if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			vkGraphicsQueueIndex = i;
			foundGraphicsQueue = true;
		}

		VkBool32 presentQueueSupport;
		vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, vkSurface, &presentQueueSupport);
		if (queueFamilies[i].queueCount > 0 && presentQueueSupport == VK_TRUE)
		{
			vkPresentQueueIndex = i;
			foundPresentQueue = true;
		}
	}

	if (!foundGraphicsQueue)
	{
		printf("Failed to find graphics queue index\n");
		exit(EXIT_FAILURE);
	}
	if (!foundPresentQueue)
	{
		printf("Present queue not supported on device\n");
		exit(EXIT_FAILURE);
	}
	
	printf("Found graphics queue index: %u\n", vkGraphicsQueueIndex);
	printf("Found present queue index: %u\n", vkPresentQueueIndex);
}

void VulkanApp::CreateLogicalDevice(uint32_t extensionCount, const char** extensionNames)
{
	uint32_t numQueues = 0;
	if (vkGraphicsQueueIndex == vkPresentQueueIndex) { numQueues = 1; }
	else { numQueues = 2; }
	VkDeviceQueueCreateInfo queues[numQueues];
	
	VkDeviceQueueCreateInfo& graphicsQueueInfo = queues[0];
	graphicsQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	graphicsQueueInfo.pNext = NULL;
	graphicsQueueInfo.flags = 0;
	graphicsQueueInfo.queueFamilyIndex = vkGraphicsQueueIndex;
	graphicsQueueInfo.queueCount = 1;
	float priority = 1.0f;
	graphicsQueueInfo.pQueuePriorities = &priority;

	if (vkGraphicsQueueIndex != vkPresentQueueIndex)
	{
		VkDeviceQueueCreateInfo& presentQueueInfo = queues[1];
		presentQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		presentQueueInfo.pNext = NULL;
		presentQueueInfo.flags = 0;
		presentQueueInfo.queueFamilyIndex = vkPresentQueueIndex;
		presentQueueInfo.queueCount = 1;
		presentQueueInfo.pQueuePriorities = &priority;
	}

	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = NULL;
	deviceInfo.flags = 0;
	deviceInfo.queueCreateInfoCount = numQueues;
	deviceInfo.pQueueCreateInfos = queues;
	//deviceInfo.enabledLayerCount = 0; //Deprecated
	//deviceInfo.ppEnabledLayerNames = NULL; //Deprecated
	deviceInfo.enabledExtensionCount = extensionCount;
	deviceInfo.ppEnabledExtensionNames = extensionNames;
	VkPhysicalDeviceFeatures enabled_features = {};
	deviceInfo.pEnabledFeatures = &enabled_features;
	CHECK_VK_RESULT(vkCreateDevice(vkPhysicalDevice, &deviceInfo, NULL, &vkDevice))
	printf("Successfully created logical device\n");

	vkGetDeviceQueue(vkDevice, vkGraphicsQueueIndex, 0, &vkGraphicsQueue);
	vkGetDeviceQueue(vkDevice, vkPresentQueueIndex, 0, &vkPresentQueue);
}

void VulkanApp::ChooseSwapChainFormat()
{
	//Free to choose any format
	if (vkSupportedSurfaceFormats.size() == 1 && vkSupportedSurfaceFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		vkSurfaceFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}
	else
	{
		//Check if B8G8R8A8_UNORM is available
		bool foundBGRA = false;
		for (auto& surfaceFormat : vkSupportedSurfaceFormats)
		{
			if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				vkSurfaceFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
				foundBGRA = true;
				break;
			}
		}
		if (!foundBGRA)
		{
			vkSurfaceFormat = { vkSupportedSurfaceFormats[0].format, vkSupportedSurfaceFormats[0].colorSpace };
		}
	}
	
	printf("Chose format %u and color space %u\n", vkSurfaceFormat.format, vkSurfaceFormat.colorSpace);
}

void VulkanApp::ChoosePresentMode()
{
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

	for (auto& presentMode : vkSupportedPresentModes)
	{
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			vkPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if (presentMode == VK_PRESENT_MODE_FIFO_KHR)
		{
			bestMode = VK_PRESENT_MODE_FIFO_KHR;
		}
	}

	vkPresentMode = bestMode;
	printf("Chose present mode %u\n", vkPresentMode);
}

void VulkanApp::ChooseSwapExtent()
{
	if (vkSurfaceCapabilities.currentExtent.width < std::numeric_limits<uint32_t>::max())
	{
		vkSurfaceExtent = vkSurfaceCapabilities.currentExtent;
	}
	else
	{
		vkSurfaceExtent = { windowWidth, windowHeight };
		vkSurfaceExtent.width = std::max(vkSurfaceCapabilities.minImageExtent.width, std::min(vkSurfaceCapabilities.maxImageExtent.width, vkSurfaceExtent.width));
		vkSurfaceExtent.height = std::max(vkSurfaceCapabilities.minImageExtent.height, std::min(vkSurfaceCapabilities.maxImageExtent.height, vkSurfaceExtent.height));
	}
	
	printf("Chose surface extent: %ux%u\n", vkSurfaceExtent.width, vkSurfaceExtent.height);
}

void VulkanApp::CreateSwapChain()
{
	ChooseSwapChainFormat();
	ChoosePresentMode();
	ChooseSwapExtent();

	uint32_t imageCount = vkSurfaceCapabilities.minImageCount + 1;
	if (vkSurfaceCapabilities.maxImageCount > 0 && imageCount > vkSurfaceCapabilities.maxImageCount)
	{
		imageCount = vkSurfaceCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchainInfo = {};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.pNext = NULL;
	swapchainInfo.flags = 0;
	swapchainInfo.surface = vkSurface;
	swapchainInfo.minImageCount = imageCount;
	swapchainInfo.imageFormat = vkSurfaceFormat.format;
	swapchainInfo.imageColorSpace = vkSurfaceFormat.colorSpace;
	swapchainInfo.imageExtent = vkSurfaceExtent;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (vkGraphicsQueueIndex != vkPresentQueueIndex)
	{
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainInfo.queueFamilyIndexCount = 2;
		uint32_t queueFamilyIndices[] = { vkGraphicsQueueIndex, vkPresentQueueIndex };
		swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainInfo.queueFamilyIndexCount = 0;
		swapchainInfo.pQueueFamilyIndices = NULL;
	}
	swapchainInfo.preTransform = vkSurfaceCapabilities.currentTransform;
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo.presentMode = vkPresentMode;
	swapchainInfo.clipped = VK_TRUE;
	swapchainInfo.oldSwapchain = NULL;
	CHECK_VK_RESULT(vkCreateSwapchainKHR(vkDevice, &swapchainInfo, NULL, &vkSwapchain))
	
	//Retrieve swapchain images
	uint32_t swapchainImageCount;
	CHECK_VK_RESULT(vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &swapchainImageCount, NULL))
	vkSwapchainImages.resize(swapchainImageCount);
	CHECK_VK_RESULT(vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &swapchainImageCount, vkSwapchainImages.data()))
	vkSwapchainImageViews.resize(vkSwapchainImages.size());
	
	printf("Successfully create swapchain with %u images and image views\n", swapchainImageCount);
}

void VulkanApp::CreateImageViews()
{
	VkImageSubresourceRange sRR = {};
	sRR.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	sRR.baseMipLevel = 0;
	sRR.levelCount = 1;
	sRR.baseArrayLayer = 0;
	sRR.layerCount = 1;
	
	VkImageViewCreateInfo imageViewInfo = {};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.pNext = NULL;
	imageViewInfo.flags = 0;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = vkSurfaceFormat.format;
	imageViewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	imageViewInfo.subresourceRange = sRR;
	
	for (size_t i = 0; i < vkSwapchainImages.size(); i++)
	{
		imageViewInfo.image = vkSwapchainImages[i];
		CHECK_VK_RESULT(vkCreateImageView(vkDevice, &imageViewInfo, NULL, &vkSwapchainImageViews[i]))
	}
	
	printf("Successfully created image views\n");
}

void VulkanApp::CreateGraphicsQueueCommandPool()
{
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.pNext = NULL;
	cmdPoolInfo.flags = 0;
	cmdPoolInfo.queueFamilyIndex = vkGraphicsQueueIndex;
	CHECK_VK_RESULT(vkCreateCommandPool(vkDevice, &cmdPoolInfo, NULL, &vkGraphicsQueueCommandPool))
	
	printf("Successfully created graphics queue command pool\n");
}

void VulkanApp::CreateSyncObjects()
{
	vkImageAvailableSemaphores.resize(maxFramesInFlight);
	vkRenderFinishedSemaphores.resize(maxFramesInFlight);
	vkInFlightFences.resize(maxFramesInFlight);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreInfo.pNext = NULL;
	semaphoreInfo.flags = 0;
	
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = NULL;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	
	for (size_t i = 0; i < maxFramesInFlight; i++)
	{
		CHECK_VK_RESULT(vkCreateSemaphore(vkDevice, &semaphoreInfo, NULL, &vkImageAvailableSemaphores[i]))
		CHECK_VK_RESULT(vkCreateSemaphore(vkDevice, &semaphoreInfo, NULL, &vkRenderFinishedSemaphores[i]))
		CHECK_VK_RESULT(vkCreateFence(vkDevice, &fenceInfo, NULL, &vkInFlightFences[i]))
	}
	
	printf("Successfully created synchronization objects\n");
}

VulkanApp::VulkanApp(const VulkanAppCreateInfo* createInfo)
{	
	if (createInfo->graphicsApp != VK_TRUE)
	{
		printf("Only graphics application are supported\n");
		exit(EXIT_FAILURE);
	}

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	windowWidth = createInfo->windowWidth;
	windowHeight = createInfo->windowHeight;
	window = glfwCreateWindow(windowWidth, windowHeight, createInfo->windowName, nullptr, nullptr);
	printf("Successfully create GLFW window\n");

#ifdef VK_DEBUG
	CheckValidationLayerSupport(createInfo->validationLayerCount, createInfo->validationLayerNames);
#endif
	std::vector<const char*> instanceExtensionNames = CheckVulkanInstanceExtensionSupport();
	
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = NULL;
	appInfo.pApplicationName = createInfo->appName;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = createInfo->engineName;
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;
	
	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = NULL;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &appInfo;
#ifdef VK_DEBUG
	instanceInfo.enabledLayerCount = createInfo->validationLayerCount;
#else
	instanceInfo.enabledLayerCount = 0;
#endif
	instanceInfo.ppEnabledLayerNames = createInfo->validationLayerNames;
	instanceInfo.enabledExtensionCount = instanceExtensionNames.size();
	instanceInfo.ppEnabledExtensionNames = instanceExtensionNames.data();
	CHECK_VK_RESULT(vkCreateInstance(&instanceInfo, NULL, &vkInstance))
	printf("Successfully created Vulkan instance\n");

#ifdef VK_DEBUG
	SetupDebugCallback(&vkInstance, &vkDebugCallback);
#endif

	CHECK_VK_RESULT(glfwCreateWindowSurface(vkInstance, window, NULL, &vkSurface))
	printf("Successfully created Vulkan window surface\n");
	PickPhysicalDevice(createInfo->extensionCount, createInfo->extensionNames);
	FindQueueIndices();
	CreateLogicalDevice(createInfo->extensionCount, createInfo->extensionNames);
	CreateSwapChain();
	CreateImageViews();
	CreateGraphicsQueueCommandPool();
	maxFramesInFlight = createInfo->maxFramesInFlight;
	CreateSyncObjects();
}

VulkanApp::~VulkanApp()
{
	for (uint32_t i = 0; i < maxFramesInFlight; i++)
	{
		vkDestroySemaphore(vkDevice, vkImageAvailableSemaphores[i], NULL);
		vkDestroySemaphore(vkDevice, vkRenderFinishedSemaphores[i], NULL);
		vkDestroyFence(vkDevice, vkInFlightFences[i], NULL);
	}
	vkDestroyCommandPool(vkDevice, vkGraphicsQueueCommandPool, NULL);
	for (size_t i = 0; i < vkSwapchainImageViews.size(); i++)
	{
		vkDestroyImageView(vkDevice, vkSwapchainImageViews[i], NULL);
	}
	vkDestroySwapchainKHR(vkDevice, vkSwapchain, NULL);
	vkDestroyDevice(vkDevice, NULL);
	vkDestroySurfaceKHR(vkInstance, vkSurface, NULL);
#ifdef VK_DEBUG
	DestroyDebugUtilsMessengerEXT(vkInstance, vkDebugCallback, NULL);
#endif
	vkDestroyInstance(vkInstance, NULL);
	glfwDestroyWindow(window);
	glfwTerminate();
}

std::vector<char> VulkanApp::ReadShaderFile(const char* spirvFile)
{
	std::ifstream file(spirvFile, std::ios::ate | std::ios::binary);
	if (!file.is_open())
	{
		printf("Failed to open file %s\n", spirvFile);
		exit(EXIT_FAILURE);
	}
	
	size_t file_size = size_t(file.tellg());
	std::vector<char> buffer(file_size);
	file.seekg(0);
	file.read(buffer.data(), file_size);
	file.close();
	
	return buffer;
}

void VulkanApp::CreateShaderModule(const char* spirvFile, VkShaderModule* shaderModule)
{
	std::vector<char> shaderCode = ReadShaderFile(spirvFile);
	
	VkShaderModuleCreateInfo shaderModuleInfo = {};
	shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleInfo.pNext = NULL;
	shaderModuleInfo.flags = 0;
	shaderModuleInfo.codeSize = shaderCode.size();
	shaderModuleInfo.pCode = (uint32_t*)(shaderCode.data());
	CHECK_VK_RESULT(vkCreateShaderModule(vkDevice, &shaderModuleInfo, NULL, shaderModule))
}

uint32_t VulkanApp::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memoryProp;
	vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memoryProp);
	
	for (uint32_t i = 0; i < memoryProp.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (memoryProp.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}
	
	printf("Failed to find any supported memory type\n");
	exit(EXIT_FAILURE);
}

void VulkanApp::CreateBuffer(uint32_t bufferSize, VkBufferUsageFlags bufferUsageFlags, VkBuffer* buffer, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceMemory* bufferMemory)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = NULL;
	bufferInfo.flags = 0;
	bufferInfo.size = bufferSize;
	bufferInfo.usage = bufferUsageFlags;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	/* IGNORED due to VK_SHARING_MODE_EXCLUSIVE
	buffer_info.queueFamilyIndexCount
	buffer_info.pQueueFamilyIndices*/
	CHECK_VK_RESULT(vkCreateBuffer(vkDevice, &bufferInfo, NULL, buffer))
	
	//Allocate memory
	VkMemoryRequirements bufferMemoryReq;
	vkGetBufferMemoryRequirements(vkDevice, *buffer, &bufferMemoryReq);
	VkMemoryAllocateInfo bufferMemoryInfo = {};
	bufferMemoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	bufferMemoryInfo.pNext = NULL;
	bufferMemoryInfo.allocationSize = bufferMemoryReq.size;
	bufferMemoryInfo.memoryTypeIndex = FindMemoryType(bufferMemoryReq.memoryTypeBits, memoryPropertyFlags);
	CHECK_VK_RESULT(vkAllocateMemory(vkDevice, &bufferMemoryInfo, NULL, bufferMemory))
	
	//Bind memory to buffer
	CHECK_VK_RESULT(vkBindBufferMemory(vkDevice, *buffer, *bufferMemory, 0))
}

void VulkanApp::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize copySize)
{
    VkCommandBuffer cmdBuffer;
	VkCommandBufferAllocateInfo allocationInfo = {};
    allocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocationInfo.commandPool = vkGraphicsQueueCommandPool;
    allocationInfo.commandBufferCount = 1;
    CHECK_VK_RESULT(vkAllocateCommandBuffers(vkDevice, &allocationInfo, &cmdBuffer))
    
    VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
    cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufferBeginInfo.pNext = NULL;
    cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmdBufferBeginInfo.pInheritanceInfo = NULL;
    CHECK_VK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo))
    VkBufferCopy bufferCopyRegion = {};
    bufferCopyRegion.srcOffset = 0;
    bufferCopyRegion.dstOffset = 0;
    bufferCopyRegion.size = copySize;
    vkCmdCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, 1, &bufferCopyRegion);
    CHECK_VK_RESULT(vkEndCommandBuffer(cmdBuffer))
    
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = NULL;
    submitInfo.pWaitDstStageMask = NULL;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = NULL;
    CHECK_VK_RESULT(vkQueueSubmit(vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE))
    CHECK_VK_RESULT(vkQueueWaitIdle(vkGraphicsQueue))
    
    vkFreeCommandBuffers(vkDevice, vkGraphicsQueueCommandPool, 1, &cmdBuffer);
}

void VulkanApp::CreateHostVisibleBuffer(uint32_t bufferSize, void* bufferData, VkBufferUsageFlags bufferUsageFlags, VkBuffer* buffer, VkDeviceMemory* bufferMemory)
{
	CreateBuffer(bufferSize, bufferUsageFlags, buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferMemory);
	
	void* data;
	CHECK_VK_RESULT(vkMapMemory(vkDevice, *bufferMemory, 0, bufferSize, 0, &data))
	memcpy(data, bufferData, bufferSize);
	vkUnmapMemory(vkDevice, *bufferMemory);
}

void VulkanApp::CreateDeviceBuffer(uint32_t bufferSize, void* bufferData, VkBufferUsageFlags bufferUsageFlags, VkBuffer* buffer, VkDeviceMemory* bufferMemory)
{
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBufferMemory);
	
	void* data;
	CHECK_VK_RESULT(vkMapMemory(vkDevice, stagingBufferMemory, 0, bufferSize, 0, &data))
	memcpy(data, bufferData, bufferSize);
	vkUnmapMemory(vkDevice, stagingBufferMemory);
	
	CreateBuffer(bufferSize, bufferUsageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT, buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferMemory);
	
	CopyBuffer(stagingBuffer, *buffer, bufferSize);
	
	vkFreeMemory(vkDevice, stagingBufferMemory, NULL);
	vkDestroyBuffer(vkDevice, stagingBuffer, NULL);
}








