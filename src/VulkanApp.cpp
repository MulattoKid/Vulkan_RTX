#include <algorithm>
#include <chrono>
#include <fstream>
#include <limits>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <set>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include "VulkanApp.h"

std::chrono::high_resolution_clock::time_point GetTime()
{
	return std::chrono::high_resolution_clock::now();
}

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
	std::vector<VkDeviceQueueCreateInfo> queues(numQueues);
	
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
	deviceInfo.queueCreateInfoCount = queues.size();
	deviceInfo.pQueueCreateInfos = queues.data();
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
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
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
	
	CHECK_VK_RESULT(volkInitialize())
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
	volkLoadInstance(vkInstance);

#ifdef VK_DEBUG
	SetupDebugCallback(&vkInstance, &vkDebugCallback);
#endif

	CHECK_VK_RESULT(glfwCreateWindowSurface(vkInstance, window, NULL, &vkSurface))
	printf("Successfully created Vulkan window surface\n");
	PickPhysicalDevice(createInfo->extensionCount, createInfo->extensionNames);
	FindQueueIndices();
	CreateLogicalDevice(createInfo->extensionCount, createInfo->extensionNames);
	volkLoadDevice(vkDevice);
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
	printf("\n");
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

void VulkanApp::AllocateGraphicsQueueCommandBuffer(VkCommandBuffer* commandBuffer)
{
	VkCommandBufferAllocateInfo allocationInfo = {};
    allocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocationInfo.commandPool = vkGraphicsQueueCommandPool;
    allocationInfo.commandBufferCount = 1;
    CHECK_VK_RESULT(vkAllocateCommandBuffers(vkDevice, &allocationInfo, commandBuffer))
}

void VulkanApp::FreeGraphicsQueueCommandBuffer(VkCommandBuffer* commandBuffer)
{
	vkFreeCommandBuffers(vkDevice, vkGraphicsQueueCommandPool, 1, commandBuffer);
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

void VulkanApp::CopyBufferToBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize copySize)
{
    VkCommandBuffer tmpCmdBuffer;
    AllocateGraphicsQueueCommandBuffer(&tmpCmdBuffer);
    
    VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
    cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufferBeginInfo.pNext = NULL;
    cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmdBufferBeginInfo.pInheritanceInfo = NULL;
    CHECK_VK_RESULT(vkBeginCommandBuffer(tmpCmdBuffer, &cmdBufferBeginInfo))
    
    VkBufferCopy bufferCopyRegion = {};
    bufferCopyRegion.srcOffset = 0;
    bufferCopyRegion.dstOffset = 0;
    bufferCopyRegion.size = copySize;
    vkCmdCopyBuffer(tmpCmdBuffer, srcBuffer, dstBuffer, 1, &bufferCopyRegion);
    
    CHECK_VK_RESULT(vkEndCommandBuffer(tmpCmdBuffer))
    
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = NULL;
    submitInfo.pWaitDstStageMask = NULL;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &tmpCmdBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = NULL;
    CHECK_VK_RESULT(vkQueueSubmit(vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE))
    CHECK_VK_RESULT(vkQueueWaitIdle(vkGraphicsQueue))
    
    FreeGraphicsQueueCommandBuffer(&tmpCmdBuffer);
}

void VulkanApp::CreateHostVisibleBuffer(uint32_t bufferSize, void* bufferData, VkBufferUsageFlags bufferUsageFlags, VkBuffer* buffer, VkDeviceMemory* bufferMemory)
{
	CreateBuffer(bufferSize, bufferUsageFlags, buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferMemory);
	
	void* data;
	CHECK_VK_RESULT(vkMapMemory(vkDevice, *bufferMemory, 0, bufferSize, 0, &data))
	if (bufferData != NULL)
	{
		memcpy(data, bufferData, bufferSize);
	}
	vkUnmapMemory(vkDevice, *bufferMemory);
}

void VulkanApp::CreateDeviceBuffer(uint32_t bufferSize, void* bufferData, VkBufferUsageFlags bufferUsageFlags, VkBuffer* buffer, VkDeviceMemory* bufferMemory)
{
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBufferMemory);
	
	void* data;
	CHECK_VK_RESULT(vkMapMemory(vkDevice, stagingBufferMemory, 0, bufferSize, 0, &data))
	if (bufferData != NULL)
	{
		memcpy(data, bufferData, bufferSize);
	}
	vkUnmapMemory(vkDevice, stagingBufferMemory);
	
	CreateBuffer(bufferSize, bufferUsageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT, buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferMemory);
	
	CopyBufferToBuffer(stagingBuffer, *buffer, bufferSize);
	
	vkFreeMemory(vkDevice, stagingBufferMemory, NULL);
	vkDestroyBuffer(vkDevice, stagingBuffer, NULL);
}

void VulkanApp::TransitionImageLayoutSingle(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStage, VkAccessFlags srcAccessMask, VkPipelineStageFlags dstStage, VkAccessFlags dstAccessMask)
{
	VkCommandBuffer commandBuffer;
	AllocateGraphicsQueueCommandBuffer(&commandBuffer);
	
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = NULL;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = NULL;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	
	VkImageMemoryBarrier transitionBarrier = {};
	transitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	transitionBarrier.pNext = NULL;
	transitionBarrier.srcAccessMask = srcAccessMask;
	transitionBarrier.dstAccessMask = dstAccessMask;
	transitionBarrier.oldLayout = oldLayout;
	transitionBarrier.newLayout = newLayout;
	transitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	transitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	transitionBarrier.image = image;
	transitionBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	transitionBarrier.subresourceRange.baseMipLevel = 0;
	transitionBarrier.subresourceRange.levelCount = 1;
	transitionBarrier.subresourceRange.baseArrayLayer = 0;
	transitionBarrier.subresourceRange.layerCount = 1;
	vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, NULL, 0, NULL, 1, &transitionBarrier);
	
	vkEndCommandBuffer(commandBuffer);
	
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = NULL;
	submitInfo.pWaitDstStageMask = NULL;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = NULL;
	vkQueueSubmit(vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(vkGraphicsQueue);
	
	FreeGraphicsQueueCommandBuffer(&commandBuffer);
}

void VulkanApp::TransitionImageLayoutInProgress(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStage, VkAccessFlags srcAccessMask, VkPipelineStageFlags dstStage, VkAccessFlags dstAccessMask, VkCommandBuffer commandBuffer)
{
	VkImageMemoryBarrier transitionBarrier = {};
	transitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	transitionBarrier.pNext = NULL;
	transitionBarrier.srcAccessMask = srcAccessMask;
	transitionBarrier.dstAccessMask = dstAccessMask;
	transitionBarrier.oldLayout = oldLayout;
	transitionBarrier.newLayout = newLayout;
	transitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	transitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	transitionBarrier.image = image;
	transitionBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	transitionBarrier.subresourceRange.baseMipLevel = 0;
	transitionBarrier.subresourceRange.levelCount = 1;
	transitionBarrier.subresourceRange.baseArrayLayer = 0;
	transitionBarrier.subresourceRange.layerCount = 1;
	vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, NULL, 0, NULL, 1, &transitionBarrier);
}

void VulkanApp::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t imageWidth, uint32_t imageHeight)
{
	VkCommandBuffer tmpCmdBuffer;
    AllocateGraphicsQueueCommandBuffer(&tmpCmdBuffer);
    
    VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
    cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufferBeginInfo.pNext = NULL;
    cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmdBufferBeginInfo.pInheritanceInfo = NULL;
    CHECK_VK_RESULT(vkBeginCommandBuffer(tmpCmdBuffer, &cmdBufferBeginInfo))
    
    VkBufferImageCopy bufferImageCopy = {};
    bufferImageCopy.bufferOffset = 0;
    bufferImageCopy.bufferRowLength = 0;
    bufferImageCopy.bufferImageHeight = 0;
    bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferImageCopy.imageSubresource.mipLevel = 0;
    bufferImageCopy.imageSubresource.baseArrayLayer = 0;
    bufferImageCopy.imageSubresource.layerCount = 1;
    bufferImageCopy.imageOffset = { 0, 0, 0 };
    bufferImageCopy.imageExtent = { imageWidth, imageHeight, 1 };
    vkCmdCopyBufferToImage(tmpCmdBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
    
    CHECK_VK_RESULT(vkEndCommandBuffer(tmpCmdBuffer))
    
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = NULL;
    submitInfo.pWaitDstStageMask = NULL;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &tmpCmdBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = NULL;
    CHECK_VK_RESULT(vkQueueSubmit(vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE))
    CHECK_VK_RESULT(vkQueueWaitIdle(vkGraphicsQueue))
    
    FreeGraphicsQueueCommandBuffer(&tmpCmdBuffer);
}

void VulkanApp::CreateTexture(const char* filename, VkFormat format, VulkanTexture* texture)
{
	int requestedComponents = 0;
	switch (format)
	{
		case VK_FORMAT_R8G8B8A8_UNORM:
			requestedComponents = STBI_rgb_alpha;
			break;
		default:
			printf("Unsupported image format for texture %s\n", filename);
			exit(1);
	}	
	
	int x, y, comp;
	stbi_set_flip_vertically_on_load(1);
	stbi_uc* imageData = stbi_load(filename, &x, &y, &comp, requestedComponents);
	if (imageData == NULL)
	{
		printf("Failed to load image %s\n", filename);
		exit(1);
	}
	uint32_t width = x, height = y;
	texture->device = vkDevice;
	
	//Create image
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext = NULL;
	imageInfo.flags = 0;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = format;
	imageInfo.extent = { width, height, 1 };
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	//IGNORED
	//imageInfo.queueFamilyIndexCount = 
	//imageInfo.pQueueFamilyIndices = 
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	CHECK_VK_RESULT(vkCreateImage(vkDevice, &imageInfo, NULL, &texture->image))
	
	//Allocate memory
	VkMemoryRequirements imageMemoryRequirements;
	vkGetImageMemoryRequirements(vkDevice, texture->image, &imageMemoryRequirements);
	VkMemoryAllocateInfo imageMemoryInfo = {};
	imageMemoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	imageMemoryInfo.pNext = NULL;
	imageMemoryInfo.allocationSize = imageMemoryRequirements.size;
	imageMemoryInfo.memoryTypeIndex = FindMemoryType(imageMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	CHECK_VK_RESULT(vkAllocateMemory(vkDevice, &imageMemoryInfo, NULL, &texture->imageMemory))
	
	CHECK_VK_RESULT(vkBindImageMemory(vkDevice, texture->image, texture->imageMemory, 0))
	
	TransitionImageLayoutSingle(texture->image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0);
	
	VkBuffer imageStagingBuffer;
	VkDeviceMemory imageStagingBufferMemory;
	CreateHostVisibleBuffer(width * height * requestedComponents, (void*)(imageData), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &imageStagingBuffer, &imageStagingBufferMemory);
	CopyBufferToImage(imageStagingBuffer, texture->image, width, height);
	vkFreeMemory(vkDevice, imageStagingBufferMemory, NULL);
	vkDestroyBuffer(vkDevice, imageStagingBuffer, NULL);
	stbi_image_free(imageData);
	
	TransitionImageLayoutSingle(texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0);
	
	VkImageViewCreateInfo imageViewInfo = {};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.pNext = NULL;
	imageViewInfo.flags = 0;
	imageViewInfo.image = texture->image;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = format;
	imageViewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;
	CHECK_VK_RESULT(vkCreateImageView(vkDevice, &imageViewInfo, NULL, &texture->imageView))
}

VulkanTexture::~VulkanTexture()
{
	vkDestroyImageView(device, imageView, NULL);
	vkFreeMemory(device, imageMemory, NULL);
	vkDestroyImage(device, image, NULL);
}

void VulkanApp::CreateDefaultSampler(VkSampler* sampler)
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.pNext = NULL;
	samplerInfo.flags = 0;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.mipLodBias = 1.0f;
	samplerInfo.anisotropyEnable = VK_FALSE;
	//samplerInfo.maxAnisotropy = IGNORED
	samplerInfo.compareEnable = VK_FALSE;
	//samplerInfo.compareOp = IGNORED
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 1.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	CHECK_VK_RESULT(vkCreateSampler(vkDevice, &samplerInfo, NULL, sampler))
}

VkViewport VulkanApp::GetDefaultViewport()
{
	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = vkSurfaceExtent.width;
	viewport.height = vkSurfaceExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	return viewport;
}

VkRect2D VulkanApp::GetDefaultScissor()
{
	VkRect2D scissor;
	scissor.offset.x = 0.0f;
	scissor.offset.y = 0.0f;
	scissor.extent.width = vkSurfaceExtent.width;
	scissor.extent.height = vkSurfaceExtent.height;
	return scissor;
}

//Renderpass should be the final renderpass, i.e. the one rendering what is to be displayed
void VulkanApp::CreateDefaultFramebuffers(std::vector<VkFramebuffer>& framebuffers, VkRenderPass renderPass)
{
	framebuffers.resize(vkSwapchainImageViews.size());
	
	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.pNext = NULL;
	framebufferInfo.flags = 0;
	framebufferInfo.renderPass = renderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.width = vkSurfaceExtent.width;
	framebufferInfo.height = vkSurfaceExtent.height;
	framebufferInfo.layers = 1;
	for (size_t i = 0; i < framebuffers.size(); i++)
	{
		framebufferInfo.pAttachments = &vkSwapchainImageViews[i];
		CHECK_VK_RESULT(vkCreateFramebuffer(vkDevice, &framebufferInfo, NULL, &framebuffers[i]))
	}
}

VkFormat VulkanApp::GetDefaultFramebufferFormat()
{
	return vkSurfaceFormat.format;
}

void VulkanApp::AllocateDefaultGraphicsQueueCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers)
{
	commandBuffers.resize(vkSwapchainImageViews.size());
	VkCommandBufferAllocateInfo alloccationInfo = {};
	alloccationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloccationInfo.pNext = NULL;
	alloccationInfo.commandPool = vkGraphicsQueueCommandPool;
	alloccationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloccationInfo.commandBufferCount = uint32_t(commandBuffers.size());
	CHECK_VK_RESULT(vkAllocateCommandBuffers(vkDevice, &alloccationInfo, commandBuffers.data()))
}

void VulkanApp::Render(VkCommandBuffer* commandBuffers)
{
	auto start_time = GetTime();

	vkWaitForFences(vkDevice, 1, &vkInFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint32_t>::max());
	vkResetFences(vkDevice, 1, &vkInFlightFences[currentFrame]);

	uint32_t imageIndex;
	CHECK_VK_RESULT(vkAcquireNextImageKHR(vkDevice, vkSwapchain, std::numeric_limits<uint32_t>::max(), vkImageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex))
	
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &vkImageAvailableSemaphores[currentFrame];
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &vkRenderFinishedSemaphores[currentFrame];
	CHECK_VK_RESULT(vkQueueSubmit(vkGraphicsQueue, 1, &submitInfo, vkInFlightFences[currentFrame]))
	
	VkResult result;
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = NULL;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &vkRenderFinishedSemaphores[currentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &vkSwapchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = &result;
	vkQueuePresentKHR(vkGraphicsQueue, &presentInfo);
	CHECK_VK_RESULT(result)
	
	currentFrame = (currentFrame + 1) % maxFramesInFlight;
	
	auto end_time = GetTime();
	unsigned int ns = (unsigned int)(std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count());
	float ms = ns / 1000000.0f;
	printf("\rFrame time (ms): %.2f", ms);
}

void VulkanApp::RenderOffscreen(VkCommandBuffer* commandBuffers)
{
	auto start_time = GetTime();

	vkResetFences(vkDevice, 1, &vkInFlightFences[currentFrame]);

	uint32_t imageIndex = 0;
	
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = NULL;
	submitInfo.pWaitDstStageMask = NULL;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = NULL;
	CHECK_VK_RESULT(vkQueueSubmit(vkGraphicsQueue, 1, &submitInfo, vkInFlightFences[currentFrame]))
	
	//Wait here to make sure all submitted command buffers have completed execution
	vkWaitForFences(vkDevice, 1, &vkInFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint32_t>::max());
	currentFrame = (currentFrame + 1) % maxFramesInFlight;
	
	auto end_time = GetTime();
	unsigned int ns = (unsigned int)(std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count());
	float ms = ns / 1000000.0f;
	printf("\rFrame time (ms): %.2f", ms);
}

//Basic transform
float basicTransform[12] = { 
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f
};


void VulkanApp::CreateBottomAccStruct(const std::pair<std::vector<float>, std::vector<uint32_t>>& geometry, VkGeometryInstanceNV* geometryInstance, uint32_t i, BottomAccStruct* bottomAccStruct, VkDevice device)
{	
	bottomAccStruct->device = device;

	//Create vertex buffer
	VkDeviceSize vertexBufferSize = geometry.first.size() * sizeof(float);
	CreateDeviceBuffer(vertexBufferSize, (void*)(geometry.first.data()), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &bottomAccStruct->vertexBuffer, &bottomAccStruct->vertexBufferMemory);
	//Create index buffer
	VkDeviceSize indexBufferSize = geometry.second.size() * sizeof(uint32_t);
	CreateDeviceBuffer(indexBufferSize, (void*)(geometry.second.data()), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, &bottomAccStruct->indexBuffer, &bottomAccStruct->indexBufferMemory);

	//Steps
	/*
	a) Specify geometry data buffers and format
	b) Specify AABBs
	c) Link a) and b)
	d) Specify geometry type and link to c)
	e) Specify acceleration structure (bottom level) and connect to d)
	f) Create
	g) Allocate memory for the acceleration structure
	h) Bind memory to acceleration structure
	i) Get uint64_t handle to acceleration structure
	j) Create an instance of the geometry using the handle from g)
	*/
	
	//a
	bottomAccStruct->triangleInfo = {};
	bottomAccStruct->triangleInfo.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
	bottomAccStruct->triangleInfo.pNext = NULL;
	bottomAccStruct->triangleInfo.vertexData = bottomAccStruct->vertexBuffer;
	bottomAccStruct->triangleInfo.vertexOffset = 0;
	bottomAccStruct->triangleInfo.vertexCount = geometry.first.size() / 3;
	bottomAccStruct->triangleInfo.vertexStride = 3 * sizeof(float);
	bottomAccStruct->triangleInfo.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	bottomAccStruct->triangleInfo.indexData = bottomAccStruct->indexBuffer;
	bottomAccStruct->triangleInfo.indexOffset = 0;
	bottomAccStruct->triangleInfo.indexCount = geometry.second.size();
	bottomAccStruct->triangleInfo.indexType = VK_INDEX_TYPE_UINT32;
	bottomAccStruct->triangleInfo.transformData = VK_NULL_HANDLE;
	//bottomAccStruct->triangleInfo.transformOffset = IGNORED
		
	//b
	bottomAccStruct->aabbInfo = {};
	bottomAccStruct->aabbInfo.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
	bottomAccStruct->aabbInfo.pNext = NULL;
	bottomAccStruct->aabbInfo.aabbData = VK_NULL_HANDLE;
	//IGNORED
	//bottomAccStruct->aabbInfo.numAABBs
	//bottomAccStruct->aabbInfo.stride
	//bottomAccStruct->aabbInfo.offset
		
	//c
	bottomAccStruct->geometryDataInfo = {};
	bottomAccStruct->geometryDataInfo.triangles = bottomAccStruct->triangleInfo;
	bottomAccStruct->geometryDataInfo.aabbs = bottomAccStruct->aabbInfo;

	//d
	bottomAccStruct->geometryInfo = {};
	bottomAccStruct->geometryInfo.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
	bottomAccStruct->geometryInfo.pNext = NULL;
	bottomAccStruct->geometryInfo.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
	bottomAccStruct->geometryInfo.geometry = bottomAccStruct->geometryDataInfo;
	bottomAccStruct->geometryInfo.flags = VK_GEOMETRY_OPAQUE_BIT_NV;
		
	//e
	bottomAccStruct->accelerationStructureInfo = {};
	bottomAccStruct->accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	bottomAccStruct->accelerationStructureInfo.pNext = NULL;
	bottomAccStruct->accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
	bottomAccStruct->accelerationStructureInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV;
	bottomAccStruct->accelerationStructureInfo.instanceCount = 0;
	bottomAccStruct->accelerationStructureInfo.geometryCount = 1;
	bottomAccStruct->accelerationStructureInfo.pGeometries = &bottomAccStruct->geometryInfo;
		
	//f
	VkAccelerationStructureCreateInfoNV accelerationStructureCreateInfo = {};
	accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
	accelerationStructureCreateInfo.pNext = NULL;
	accelerationStructureCreateInfo.compactedSize = 0;
	accelerationStructureCreateInfo.info = bottomAccStruct->accelerationStructureInfo;	
	CHECK_VK_RESULT(vkCreateAccelerationStructureNV(vkDevice, &accelerationStructureCreateInfo, NULL, &bottomAccStruct->accelerationStructure))
		
	//g
	VkAccelerationStructureMemoryRequirementsInfoNV accelerationStructureMemoryRequirementInfo;
	accelerationStructureMemoryRequirementInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
	accelerationStructureMemoryRequirementInfo.pNext = NULL;
	accelerationStructureMemoryRequirementInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
	accelerationStructureMemoryRequirementInfo.accelerationStructure = bottomAccStruct->accelerationStructure;
	VkMemoryRequirements2 accelerationStructMemoryRequirements;
	vkGetAccelerationStructureMemoryRequirementsNV(vkDevice, &accelerationStructureMemoryRequirementInfo, &accelerationStructMemoryRequirements);
		
	VkMemoryAllocateInfo accelerationStructureMemoryInfo = {};
	accelerationStructureMemoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	accelerationStructureMemoryInfo.pNext = NULL;
	accelerationStructureMemoryInfo.allocationSize = accelerationStructMemoryRequirements.memoryRequirements.size;
	accelerationStructureMemoryInfo.memoryTypeIndex = FindMemoryType(accelerationStructMemoryRequirements.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	CHECK_VK_RESULT(vkAllocateMemory(vkDevice, &accelerationStructureMemoryInfo, NULL, &bottomAccStruct->accelerationStructureMemory))
		
	//h
	VkBindAccelerationStructureMemoryInfoNV bindInfo = {};
	bindInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
	bindInfo.pNext = NULL;
	bindInfo.accelerationStructure = bottomAccStruct->accelerationStructure;
	bindInfo.memory = bottomAccStruct->accelerationStructureMemory;
	bindInfo.memoryOffset = 0;
	bindInfo.deviceIndexCount = 0;
	bindInfo.pDeviceIndices = NULL;
	CHECK_VK_RESULT(vkBindAccelerationStructureMemoryNV(vkDevice, 1, &bindInfo))
		
	//i
	CHECK_VK_RESULT(vkGetAccelerationStructureHandleNV(vkDevice, bottomAccStruct->accelerationStructure, sizeof(uint64_t), &bottomAccStruct->accelerationStructureHandle))
		
	//j
	memcpy(geometryInstance->transform, basicTransform, sizeof(float) * 12);
	geometryInstance->instanceCustomIndex = i;
	geometryInstance->mask = 0xff;
	geometryInstance->instanceOffset = 0;
	geometryInstance->flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
	geometryInstance->accelerationStructureHandle = bottomAccStruct->accelerationStructureHandle;
}

void VulkanApp::CreateTopAccStruct(uint32_t numInstances, TopAccStruct* topAccStruct, VkDevice device)
{
	topAccStruct->device = device;

	//Steps
	/*
	a) Specify acceleration structure (bottom level) and connect to d)
	b) Create
	c) Allocate memory for the acceleration structure
	d) Bind memory to acceleration structure
	e) Get uint64_t handle to acceleration structure
	*/
	
	//a
	topAccStruct->accelerationStructureInfo = {};
	topAccStruct->accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	topAccStruct->accelerationStructureInfo.pNext = NULL;
	topAccStruct->accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
	topAccStruct->accelerationStructureInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV;
	topAccStruct->accelerationStructureInfo.instanceCount = numInstances;
	topAccStruct->accelerationStructureInfo.geometryCount = 0;
	topAccStruct->accelerationStructureInfo.pGeometries = NULL;
	
	//b
	VkAccelerationStructureCreateInfoNV accelerationStructureCreateInfo = {};
	accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
	accelerationStructureCreateInfo.pNext = NULL;
	accelerationStructureCreateInfo.compactedSize = 0;
	accelerationStructureCreateInfo.info = topAccStruct->accelerationStructureInfo;	
	CHECK_VK_RESULT(vkCreateAccelerationStructureNV(vkDevice, &accelerationStructureCreateInfo, NULL, &topAccStruct->accelerationStructure))
	
	//c
	VkAccelerationStructureMemoryRequirementsInfoNV accelerationStructureMemoryRequirementInfo;
	accelerationStructureMemoryRequirementInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
	accelerationStructureMemoryRequirementInfo.pNext = NULL;
	accelerationStructureMemoryRequirementInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
	accelerationStructureMemoryRequirementInfo.accelerationStructure = topAccStruct->accelerationStructure;
	VkMemoryRequirements2 accelerationStructMemoryRequirements;
	vkGetAccelerationStructureMemoryRequirementsNV(vkDevice, &accelerationStructureMemoryRequirementInfo, &accelerationStructMemoryRequirements);
	
	VkMemoryAllocateInfo accelerationStructureMemoryInfo = {};
	accelerationStructureMemoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	accelerationStructureMemoryInfo.pNext = NULL;
	accelerationStructureMemoryInfo.allocationSize = accelerationStructMemoryRequirements.memoryRequirements.size;
	accelerationStructureMemoryInfo.memoryTypeIndex = FindMemoryType(accelerationStructMemoryRequirements.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	CHECK_VK_RESULT(vkAllocateMemory(vkDevice, &accelerationStructureMemoryInfo, NULL, &topAccStruct->accelerationStructureMemory))
	
	//d
	VkBindAccelerationStructureMemoryInfoNV bindInfo = {};
	bindInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
	bindInfo.pNext = NULL;
	bindInfo.accelerationStructure = topAccStruct->accelerationStructure;
	bindInfo.memory = topAccStruct->accelerationStructureMemory;
	bindInfo.memoryOffset = 0;
	bindInfo.deviceIndexCount = 0;
	bindInfo.pDeviceIndices = NULL;
	CHECK_VK_RESULT(vkBindAccelerationStructureMemoryNV(vkDevice, 1, &bindInfo))
	
	//e
	CHECK_VK_RESULT(vkGetAccelerationStructureHandleNV(vkDevice, topAccStruct->accelerationStructure, sizeof(uint64_t), &topAccStruct->accelerationStructureHandle))
}

void VulkanApp::CreateVulkanAccelerationStructure(const std::vector<std::pair<std::vector<float>, std::vector<uint32_t>>>& geometryData, VulkanAccelerationStructure* accStruct)
{
	//Steps
	/*
	a) Create all bottom level acceleration structures
	b) Create buffer containing all geometry instances
	c) Create top level acceleration structure
	*/
	
	accStruct->device = vkDevice;
	const uint32_t numMeshes = geometryData.size();
	accStruct->bottomAccStructs.resize(numMeshes);
	std::vector<VkGeometryInstanceNV> geometryInstances(numMeshes);
	
	//a)
	for (uint32_t i = 0; i < numMeshes; i++)
	{
		const std::pair<std::vector<float>, std::vector<uint32_t>>& geometry = geometryData[i];
		VkGeometryInstanceNV* geometryInstance = &geometryInstances[i];
		BottomAccStruct* bottomAccStruct = &accStruct->bottomAccStructs[i];
		
		CreateBottomAccStruct(geometry, geometryInstance, i, bottomAccStruct, vkDevice);
	}
	
	//b)
	VkDeviceSize geometryInstanceBufferSize = geometryInstances.size() * sizeof(VkGeometryInstanceNV);
	CreateDeviceBuffer(geometryInstanceBufferSize, (void*)(geometryInstances.data()), VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, &accStruct->geometryInstancesBuffer, &accStruct->geometryInstancesBufferMemory);

	//c)
	CreateTopAccStruct(geometryInstances.size(), &accStruct->topAccStruct, vkDevice);
}

BottomAccStruct::~BottomAccStruct()
{
	vkFreeMemory(device, accelerationStructureMemory, NULL);
	vkDestroyAccelerationStructureNV(device, accelerationStructure, NULL);
	vkFreeMemory(device, indexBufferMemory, NULL);
	vkDestroyBuffer(device, indexBuffer, NULL);
	vkFreeMemory(device, vertexBufferMemory, NULL);
	vkDestroyBuffer(device, vertexBuffer, NULL);
}

TopAccStruct::~TopAccStruct()
{
	vkFreeMemory(device, accelerationStructureMemory, NULL);
	vkDestroyAccelerationStructureNV(device, accelerationStructure, NULL);
}

VulkanAccelerationStructure::~VulkanAccelerationStructure()
{
	vkFreeMemory(device, geometryInstancesBufferMemory, NULL);
	vkDestroyBuffer(device, geometryInstancesBuffer, NULL);
}


void VulkanApp::BuildAccelerationStructure(const VulkanAccelerationStructure& accStruct)
{
	//Steps
	/*
	a) Find largest acceleration structure
		- Should have the largest size that will be needed = max(largest_bottom_level, top_level)
		- Size is of type VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV
	b) Create a buffer that will be used as scratch when building
	c) Allocate command buffer
	d) Begin command buffer
	e) Build Bottom level acceleration structures
		- Repeat for each
		- Build
		- Pipeline barrier
	f) Build TopLevel
		- Build
		- Pipeline barrier
	g) End command buffer
	h) Submit command buffer to graphics queue
	i) Wait on graphics queue to be idle
	j) Free command buffer
	k) Destroy scratch buffer and free scratch memory
	*/
	
	//a
	VkDeviceSize scratchBufferSize = std::numeric_limits<uint32_t>::min();
	//Bottom level
	VkAccelerationStructureMemoryRequirementsInfoNV accelerationStructureMemoryRequirementInfo = {};
	accelerationStructureMemoryRequirementInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
	accelerationStructureMemoryRequirementInfo.pNext = NULL;
	accelerationStructureMemoryRequirementInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
	VkMemoryRequirements2 accelerationStructMemoryRequirements;
	for (const BottomAccStruct& bottomAccStruct : accStruct.bottomAccStructs)
	{
		accelerationStructureMemoryRequirementInfo.accelerationStructure = bottomAccStruct.accelerationStructure;
		vkGetAccelerationStructureMemoryRequirementsNV(vkDevice, &accelerationStructureMemoryRequirementInfo, &accelerationStructMemoryRequirements);
		scratchBufferSize = std::max(scratchBufferSize, accelerationStructMemoryRequirements.memoryRequirements.size);
	}
	
	//Top level
	accelerationStructureMemoryRequirementInfo.accelerationStructure = accStruct.topAccStruct.accelerationStructure;
	vkGetAccelerationStructureMemoryRequirementsNV(vkDevice, &accelerationStructureMemoryRequirementInfo, &accelerationStructMemoryRequirements);
	scratchBufferSize = std::max(scratchBufferSize, accelerationStructMemoryRequirements.memoryRequirements.size);
	
	
	//b
	VkBuffer scratchBuffer;
	VkDeviceMemory scratchBufferMemory;
	CreateDeviceBuffer(scratchBufferSize, (void*)(NULL), VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, &scratchBuffer, &scratchBufferMemory);
	
	//c
	VkCommandBuffer tmpCommandBuffer;
	AllocateGraphicsQueueCommandBuffer(&tmpCommandBuffer);
	
	//d
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = NULL;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = NULL;
	vkBeginCommandBuffer(tmpCommandBuffer, &beginInfo);
	
	//e
	VkMemoryBarrier memoryBarrier = {};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.pNext = NULL;
	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
	memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
	for (const BottomAccStruct& bottomAccStruct : accStruct.bottomAccStructs)
	{
		//Build
		vkCmdBuildAccelerationStructureNV(tmpCommandBuffer, &bottomAccStruct.accelerationStructureInfo, VK_NULL_HANDLE, 0, VK_FALSE, bottomAccStruct.accelerationStructure, VK_NULL_HANDLE, scratchBuffer, 0);
		//Barrier
		vkCmdPipelineBarrier(tmpCommandBuffer, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, 0, 1, &memoryBarrier, 0, NULL, 0, NULL);
	}
	
	//f
	//Build
	vkCmdBuildAccelerationStructureNV(tmpCommandBuffer, &accStruct.topAccStruct.accelerationStructureInfo, accStruct.geometryInstancesBuffer, 0, VK_FALSE, accStruct.topAccStruct.accelerationStructure, VK_NULL_HANDLE, scratchBuffer, 0);
	//Barrier
	vkCmdPipelineBarrier(tmpCommandBuffer, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, 0, 1, &memoryBarrier, 0, NULL, 0, NULL);
	
	//g
	vkEndCommandBuffer(tmpCommandBuffer);
	
	//h
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
	vkQueueSubmit(vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	
	//i
	vkQueueWaitIdle(vkGraphicsQueue);
	
	//j
	FreeGraphicsQueueCommandBuffer(&tmpCommandBuffer);
	
	//k
	vkFreeMemory(vkDevice, scratchBufferMemory, NULL);
	vkDestroyBuffer(vkDevice, scratchBuffer, NULL);
}





