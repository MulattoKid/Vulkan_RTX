#include <algorithm>
#include <cstring>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

//Window and GLFW
GLFWwindow* window = NULL;
const uint32_t window_width = 1024, window_height = 1024;

//Vulkan initialization
VkInstance vk_instance;
VkDebugUtilsMessengerEXT vk_debug_callback;
VkPhysicalDevice vk_physical_device = VK_NULL_HANDLE;
uint32_t vk_graphics_queue_index;
uint32_t vk_present_queue_index;
VkDevice vk_device;
VkQueue vk_graphics_queue;

//Vulkan presentation
VkSurfaceKHR vk_surface;
VkQueue vk_present_queue;
VkSurfaceCapabilitiesKHR vk_surface_capabilities;
std::vector<VkSurfaceFormatKHR> vk_surface_formats;
VkSurfaceFormatKHR vk_surface_format;
std::vector<VkPresentModeKHR> vk_present_modes;
VkPresentModeKHR vk_present_mode;
VkExtent2D vk_surface_extent;
VkSwapchainKHR vk_swap_chain;
std::vector<VkImage> vk_swap_chain_images;

//Vulkan extensions
const std::vector<const char*> vk_device_extensions = 
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	/*"VK_NV_ray_tracing"*/
};

#define CHECK_VK_RESULT(res) \
	if (res != VK_SUCCESS) printf("Vulkan call on line %i failed\n", __LINE__); \

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
	if (func != nullptr)
	{
		func(instance, callback, pAllocator);
	}
}

void CreateWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(window_width, window_height, "Vulkan RTX ON", nullptr, nullptr);
}

void DestroyWindow()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void InitVulkan()
{
	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = NULL;
	app_info.pApplicationName = "Vulkan RTX ON";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "RTX ON";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_1;

	//List available validation layers
	uint32_t layer_count = 0;
	CHECK_VK_RESULT(vkEnumerateInstanceLayerProperties(&layer_count, NULL))
	std::vector<VkLayerProperties> available_layers(layer_count);
	CHECK_VK_RESULT(vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data()))
	printf("Available validation layers:\n");
	for (uint32_t i = 0; i < layer_count; i++)
	{
		printf("\t%s\n", available_layers[i].layerName);
	}

	//Requested validation layers
	const uint32_t validation_layer_count = 1;
	const char* validation_layers[validation_layer_count] =
	{
		"VK_LAYER_LUNARG_standard_validation"
	};
	//Check for layer support
	for (uint32_t i = 0; i < validation_layer_count; i++)
	{
		bool found_vali = false;
		for (uint32_t j = 0; j < layer_count; j++)
		{
			if (strcmp(validation_layers[i], available_layers[j].layerName) == 0)
			{
				found_vali = true;
				break;
			}
		}

		if (!found_vali)
		{
			printf("Requested validation layer '%s' is not available\n", validation_layers[i]);
		}
	}

	//List available instance extensions
	uint32_t available_extension_count = 0;
	CHECK_VK_RESULT(vkEnumerateInstanceExtensionProperties(NULL, &available_extension_count, NULL))
	std::vector<VkExtensionProperties> available_extensions(available_extension_count);
	CHECK_VK_RESULT(vkEnumerateInstanceExtensionProperties(NULL, &available_extension_count, available_extensions.data()))
	printf("Available extensions:\n");
	for (uint32_t i = 0; i < available_extension_count; i++)
	{
		printf("\t%s\n", available_extensions[i].extensionName);
	}

	//Instance extensions
	uint32_t glfw_extension_count = 0;
	const char** glfw_extensions;
	glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
	std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	//Check for extension support
	for (uint32_t i = 0; i < extensions.size(); i++)
	{
		bool found_ext = false;
		for (uint32_t j = 0; j < available_extension_count; j++)
		{
			if (strcmp(extensions[i], available_extensions[j].extensionName) == 0)
			{
				found_ext = true;
				break;
			}
		}

		if (!found_ext)
		{
			printf("Required instance extension '%s' is not supported\n", extensions[i]);
		}
	}

	VkInstanceCreateInfo instance_info = {};
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pNext = NULL;
	instance_info.flags = 0;
	instance_info.pApplicationInfo = &app_info;
	instance_info.enabledLayerCount = validation_layer_count;
	instance_info.ppEnabledLayerNames = validation_layers;
	instance_info.enabledExtensionCount = extensions.size();
	instance_info.ppEnabledExtensionNames = extensions.data();

	CHECK_VK_RESULT(vkCreateInstance(&instance_info, NULL, &vk_instance))

	VkDebugUtilsMessengerCreateInfoEXT debug_info = {};
	debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debug_info.pfnUserCallback = DebugCallback;
	debug_info.pUserData = NULL;

	CHECK_VK_RESULT(CreateDebugUtilsMessengerEXT(vk_instance, &debug_info, NULL, &vk_debug_callback))
}

void CreateSurface()
{
	CHECK_VK_RESULT(glfwCreateWindowSurface(vk_instance, window, NULL, &vk_surface))
}

void QuerySwapChainSupport(VkPhysicalDevice physical_device)
{
	CHECK_VK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, vk_surface, &vk_surface_capabilities))

		uint32_t format_count;
	CHECK_VK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk_surface, &format_count, NULL))
		if (format_count > 0)
		{
			vk_surface_formats.resize(format_count);
			CHECK_VK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk_surface, &format_count, vk_surface_formats.data()))
		}

	uint32_t present_mode_count;
	CHECK_VK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, vk_surface, &present_mode_count, NULL))
		if (present_mode_count > 0)
		{
			vk_present_modes.resize(present_mode_count);
			CHECK_VK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, vk_surface, &present_mode_count, vk_present_modes.data()))
		}
}

bool IsSuitablePhysicalDevice(const VkPhysicalDevice& phyiscal_device)
{
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(phyiscal_device, &properties);
	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(phyiscal_device, &features);

	uint32_t ext_count = 0;
	CHECK_VK_RESULT(vkEnumerateDeviceExtensionProperties(phyiscal_device, NULL, &ext_count, NULL))
	std::vector<VkExtensionProperties> ext_props(ext_count);
	CHECK_VK_RESULT(vkEnumerateDeviceExtensionProperties(phyiscal_device, NULL, &ext_count, ext_props.data()))

	std::set<std::string> required_extensions(vk_device_extensions.begin(), vk_device_extensions.end());
	for (auto& ext_prop : ext_props)
	{
		required_extensions.erase(ext_prop.extensionName);
	}

	QuerySwapChainSupport(phyiscal_device);

	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && required_extensions.empty() && !vk_surface_formats.empty() && !vk_present_modes.empty())
	{
		printf("Chose %s as device\n", properties.deviceName);
		return true;
	}
	
	return false;
}

void PickPhysicalDevice()
{
	//Find devices
	uint32_t device_count = 0;
	CHECK_VK_RESULT(vkEnumeratePhysicalDevices(vk_instance, &device_count, NULL));
	if (device_count == 0)
	{
		printf("Failed to find any GPU with Vulkan support\n");
		exit(1);
	}
	std::vector<VkPhysicalDevice> devices(device_count);
	CHECK_VK_RESULT(vkEnumeratePhysicalDevices(vk_instance, &device_count, devices.data()));

	//Pick device
	for (uint32_t i = 0; i < device_count; i++)
	{
		if (IsSuitablePhysicalDevice(devices[i]))
		{
			vk_physical_device = devices[i];
			break;
		}
	}
	if (vk_physical_device == VK_NULL_HANDLE)
	{
		printf("Did not find a suitable GPU\n");
	}
}

void FindQueues()
{
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &queue_family_count, NULL);
	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &queue_family_count, queue_families.data());

	bool found_graphics_queue = false;
	bool found_present_queue = false;
	for (uint32_t i = 0; i < queue_family_count; i++)
	{
		if (queue_families[i].queueCount > 0 && queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			vk_graphics_queue_index = i;
			found_graphics_queue = true;
		}

		VkBool32 present_queue_support;
		vkGetPhysicalDeviceSurfaceSupportKHR(vk_physical_device, i, vk_surface, &present_queue_support);
		if (queue_families[i].queueCount > 0 && present_queue_support)
		{
			vk_present_queue_index = i;
			found_present_queue = true;
		}
	}

	if (!found_graphics_queue)
	{
		printf("Failed to find graphics queue index\n");
		exit(1);
	}
	if (!found_present_queue)
	{
		printf("Present queue not supported on device\n");
		exit(1);
	}
}

void CreateLogicalDevice()
{
	uint32_t num_queues = 0;
	if (vk_graphics_queue_index == vk_present_queue_index) { num_queues = 1; }
	else { num_queues = 2; }
	VkDeviceQueueCreateInfo queues[num_queues];
	
	VkDeviceQueueCreateInfo& graphics_queue_info = queues[0];
	graphics_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	graphics_queue_info.pNext = NULL;
	graphics_queue_info.flags = 0;
	graphics_queue_info.queueFamilyIndex = vk_graphics_queue_index;
	graphics_queue_info.queueCount = 1;
	float priority = 1.0f;
	graphics_queue_info.pQueuePriorities = &priority;

	if (vk_graphics_queue_index != vk_present_queue_index)
	{
		VkDeviceQueueCreateInfo& present_queue_info = queues[1];
		present_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		present_queue_info.pNext = NULL;
		present_queue_info.flags = 0;
		present_queue_info.queueFamilyIndex = vk_present_queue_index;
		present_queue_info.queueCount = 1;
		present_queue_info.pQueuePriorities = &priority;
	}

	VkDeviceCreateInfo device_info = {};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pNext = NULL;
	device_info.flags = 0;
	device_info.queueCreateInfoCount = num_queues;
	device_info.pQueueCreateInfos = queues;
	//device_info.enabledLayerCount = 0; //Deprecated
	//device_info.ppEnabledLayerNames = NULL; //Deprecated
	device_info.enabledExtensionCount = vk_device_extensions.size();
	device_info.ppEnabledExtensionNames = vk_device_extensions.data();
	VkPhysicalDeviceFeatures enabled_features = {};
	device_info.pEnabledFeatures = &enabled_features;
	CHECK_VK_RESULT(vkCreateDevice(vk_physical_device, &device_info, NULL, &vk_device))

	vkGetDeviceQueue(vk_device, vk_graphics_queue_index, 0, &vk_graphics_queue);
	vkGetDeviceQueue(vk_device, vk_present_queue_index, 0, &vk_present_queue);
}

void ChooseSwapChainFormat()
{
	//Free to choose any format
	if (vk_surface_formats.size() == 1 && vk_surface_formats[0].format == VK_FORMAT_UNDEFINED)
	{
		vk_surface_format = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		return;
	}

	//Check if B8G8R8A8_UNORM is available
	for (auto& surface_format : vk_surface_formats)
	{
		if (surface_format.format == VK_FORMAT_B8G8R8A8_UNORM && surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			vk_surface_format = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
			return;
		}
	}

	vk_surface_format = { vk_surface_formats[0].format, vk_surface_formats[0].colorSpace };
}

void ChoosePresentMode()
{
	VkPresentModeKHR best_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;

	for (auto& present_mode : vk_present_modes)
	{
		if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			vk_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
			return;
		}
		if (present_mode == VK_PRESENT_MODE_FIFO_KHR)
		{
			best_mode = VK_PRESENT_MODE_FIFO_KHR;
		}
	}

	vk_present_mode = best_mode;
}

void ChooseSwapExtent()
{
	if (vk_surface_capabilities.currentExtent.width < std::numeric_limits<uint32_t>::max())
	{
		vk_surface_extent = vk_surface_capabilities.currentExtent;
	}
	else
	{
		vk_surface_extent = { window_width, window_height };
		vk_surface_extent.width = std::max(vk_surface_capabilities.minImageExtent.width, std::min(vk_surface_capabilities.maxImageExtent.width, vk_surface_extent.width));
		vk_surface_extent.height = std::max(vk_surface_capabilities.minImageExtent.height, std::min(vk_surface_capabilities.maxImageExtent.height, vk_surface_extent.height));
	}
}

void CreateSwapChain()
{
	ChooseSwapChainFormat();
	ChoosePresentMode();
	ChooseSwapExtent();

	uint32_t image_count = vk_surface_capabilities.minImageCount + 1;
	if (vk_surface_capabilities.maxImageCount > 0 && image_count > vk_surface_capabilities.maxImageCount)
	{
		image_count = vk_surface_capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swap_chain_info = {};
	swap_chain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swap_chain_info.pNext = NULL;
	swap_chain_info.flags = 0;
	swap_chain_info.surface = vk_surface;
	swap_chain_info.minImageCount = image_count;
	swap_chain_info.imageFormat = vk_surface_format.format;
	swap_chain_info.imageColorSpace = vk_surface_format.colorSpace;
	swap_chain_info.imageExtent = vk_surface_extent;
	swap_chain_info.imageArrayLayers = 1;
	swap_chain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (vk_graphics_queue_index != vk_present_queue_index)
	{
		swap_chain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swap_chain_info.queueFamilyIndexCount = 2;
		uint32_t queue_family_indices[] = { vk_graphics_queue_index, vk_present_queue_index };
		swap_chain_info.pQueueFamilyIndices = queue_family_indices;
	}
	else
	{
		swap_chain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swap_chain_info.queueFamilyIndexCount = 0;
		swap_chain_info.pQueueFamilyIndices = NULL;
	}
	swap_chain_info.preTransform = vk_surface_capabilities.currentTransform;
	swap_chain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swap_chain_info.presentMode = vk_present_mode;
	swap_chain_info.clipped = VK_TRUE;
	swap_chain_info.oldSwapchain = NULL;
	CHECK_VK_RESULT(vkCreateSwapchainKHR(vk_device, &swap_chain_info, NULL, &vk_swap_chain))

	//Retrieve swap chain images
	uint32_t swap_chain_image_count;
	CHECK_VK_RESULT(vkGetSwapchainImagesKHR(vk_device, vk_swap_chain, &swap_chain_image_count, NULL))
	vk_swap_chain_images.resize(swap_chain_image_count);
	CHECK_VK_RESULT(vkGetSwapchainImagesKHR(vk_device, vk_swap_chain, &swap_chain_image_count, vk_swap_chain_images.data()))
}

void CleanUpVulkan()
{
	vkDestroySwapchainKHR(vk_device, vk_swap_chain, NULL);
	vkDestroySurfaceKHR(vk_instance, vk_surface, NULL);
	vkDestroyDevice(vk_device, NULL);
	DestroyDebugUtilsMessengerEXT(vk_instance, vk_debug_callback, NULL);
	vkDestroyInstance(vk_instance, NULL);
}

//https://iorange.github.io/
//https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Image_views
int main()
{
	CreateWindow();
	InitVulkan();
	CreateSurface();
	PickPhysicalDevice();
	FindQueues();
	CreateLogicalDevice();
	CreateSwapChain();

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	CleanUpVulkan();
	DestroyWindow();

	return 0;
}
