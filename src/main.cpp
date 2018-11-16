#include <algorithm>
#include <cstring>
#include <fstream>
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

//Vulkan extensions
const std::vector<const char*> vk_device_extensions = 
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	/*"VK_NV_ray_tracing"*/
};

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
std::vector<VkImageView> vk_swap_chain_image_views;

//Vulkan graphics pipeline
VkShaderModule vk_vertex_shader_module;
VkShaderModule vk_fragment_shader_module;
VkPipelineLayout vk_graphics_pipeline_layout;
VkRenderPass vk_renderpass;
VkPipeline vk_graphics_pipeline;

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

std::vector<char> ReadFile(const std::string& file)
{
	std::ifstream f(file, std::ios::ate | std::ios::binary);
	if (!f.is_open())
	{
		printf("Failed to open file %s\n", file.c_str());
		exit(1);
	}
	
	size_t file_size = size_t(f.tellg());
	std::vector<char> buffer(file_size);
	f.seekg(0);
	f.read(buffer.data(), file_size);
	f.close();
	
	return buffer;
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
	vk_swap_chain_image_views.resize(vk_swap_chain_images.size());
}

void CreateImageViews()
{
	//Create image views
	VkImageSubresourceRange srr = {};
	srr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	srr.baseMipLevel = 0;
	srr.levelCount = 1;
	srr.baseArrayLayer = 0;
	srr.layerCount = 1;
	
	VkImageViewCreateInfo image_info = {};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_info.pNext = NULL;
	image_info.flags = 0;
	image_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_info.format = vk_surface_format.format;
	image_info.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	image_info.subresourceRange = srr;
	
	for (size_t i = 0; i < vk_swap_chain_images.size(); i++)
	{
		image_info.image = vk_swap_chain_images[i];
		CHECK_VK_RESULT(vkCreateImageView(vk_device, &image_info, NULL, &vk_swap_chain_image_views[i]))
	}
}

VkShaderModule CreateShaderModule(const std::vector<char>& shader_code)
{
	VkShaderModule shader_module;
	VkShaderModuleCreateInfo shader_module_info = {};
	shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_info.pNext = NULL;
	shader_module_info.flags = 0;
	shader_module_info.codeSize = shader_code.size();
	shader_module_info.pCode = (uint32_t*)(shader_code.data());
	CHECK_VK_RESULT(vkCreateShaderModule(vk_device, &shader_module_info, NULL, &shader_module))
	
	return shader_module;
}

void CreateGraphicsPipeline()
{
	vk_vertex_shader_module = CreateShaderModule(ReadFile("src/shaders/vert.spv"));
	vk_fragment_shader_module = CreateShaderModule(ReadFile("src/shaders/frag.spv"));
	
	VkPipelineShaderStageCreateInfo shader_stage_infos[2];
	shader_stage_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_infos[0].pNext = NULL;
	shader_stage_infos[0].flags = 0;
	shader_stage_infos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shader_stage_infos[0].module = vk_vertex_shader_module;
	shader_stage_infos[0].pName = "main";
	shader_stage_infos[0].pSpecializationInfo = NULL;
	shader_stage_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_infos[1].pNext = NULL;
	shader_stage_infos[1].flags = 0;
	shader_stage_infos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shader_stage_infos[1].module = vk_fragment_shader_module;
	shader_stage_infos[1].pName = "main";
	shader_stage_infos[1].pSpecializationInfo = NULL;
	
	VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.pNext = NULL;
	vertex_input_info.flags = 0;
	vertex_input_info.vertexBindingDescriptionCount = 0;
	vertex_input_info.pVertexBindingDescriptions = NULL;
	vertex_input_info.vertexAttributeDescriptionCount = 0;
	vertex_input_info.pVertexAttributeDescriptions = NULL;
	
	VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {};
	input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_info.pNext = NULL;
	input_assembly_info.flags = 0;
	input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_info.primitiveRestartEnable = VK_FALSE;
	
	VkPipelineTessellationStateCreateInfo tessellation_info = {};
	tessellation_info.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
	tessellation_info.pNext = NULL;
	tessellation_info.flags = 0;
	tessellation_info.patchControlPoints = 0;
	
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = vk_surface_extent.width;
	viewport.height = vk_surface_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	
	VkRect2D scissor = {};
	scissor.offset.x = 0.0f;
	scissor.offset.y = 0.0f;
	scissor.extent.width = vk_surface_extent.width;
	scissor.extent.height = vk_surface_extent.height;
	
	VkPipelineViewportStateCreateInfo viewport_info = {};
	viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_info.pNext = NULL;
	viewport_info.flags = 0;
	viewport_info.viewportCount = 1;
	viewport_info.pViewports = &viewport;
	viewport_info.scissorCount = 1;
	viewport_info.pScissors = &scissor;
	
	VkPipelineRasterizationStateCreateInfo rasterization_info = {};
	rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_info.pNext = NULL;
	rasterization_info.flags = 0;
	rasterization_info.depthClampEnable = VK_FALSE;
	rasterization_info.rasterizerDiscardEnable = VK_FALSE;
	rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization_info.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterization_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization_info.depthBiasEnable = VK_FALSE;
	/* IGNORED
	rasterization_info.depthBiasConstantFactor
	rasterization_info.depthBiasClamp
	rasterization_info.depthBiasSlopeFactor*/
	rasterization_info.lineWidth = 1.0f;
	
	VkPipelineMultisampleStateCreateInfo multisample_info = {};
	multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_info.pNext = NULL;
	multisample_info.flags = 0;
	multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisample_info.sampleShadingEnable = VK_FALSE;
	/* IGNORED
	multisample_info.minSampleShading
	multisample_info.pSampleMask*/
	multisample_info.alphaToCoverageEnable = VK_FALSE;
	multisample_info.alphaToOneEnable = VK_FALSE;
	
	VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {};
	depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_info.pNext = NULL;
	depth_stencil_info.flags = 0;
	depth_stencil_info.depthTestEnable = VK_FALSE;
	/* IGNORED
	depth_stencil_info.depthWriteEnable
	depth_stencil_info.deothCompareOp*/
	depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
	depth_stencil_info.stencilTestEnable = VK_FALSE;
	/* IGNORED
	depth_stencil_info.front
	depth_stencil_info.back
	depth_stencil_info.minDepthBounds
	depth_stencil_info.maxDepthBounds*/
	
	VkPipelineColorBlendAttachmentState color_blend_attachment = {};
	color_blend_attachment.blendEnable = VK_FALSE;
	/* IGNORED
	color_blend_attachment.srcColorBlendFactor 
	color_blend_attachment.dstColorBlendFactor
	color_blend_attachment.colorBlendOp
	color_blend_attachment.srcAlphaBlendFactor
	color_blend_attachment.dstAlphaBlendFactor
	color_blend_attachment.alphaBlendOp
	color_blend_attachment.colorWriteMask*/
	
	VkPipelineColorBlendStateCreateInfo color_blend_info = {};
	color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_info.pNext = NULL;
	color_blend_info.flags = 0;
	color_blend_info.logicOpEnable = VK_FALSE;
	/* INGORED
	color_blend_info.logicOp*/
	color_blend_info.attachmentCount = 1;
	color_blend_info.pAttachments = &color_blend_attachment;
	color_blend_info.blendConstants[0] = 0.0f;
	color_blend_info.blendConstants[1] = 0.0f;
	color_blend_info.blendConstants[2] = 0.0f;
	color_blend_info.blendConstants[3] = 0.0f;
	
	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.pNext = NULL;
	pipeline_layout_info.flags = 0;
	pipeline_layout_info.setLayoutCount = 0;
	pipeline_layout_info.pSetLayouts = NULL;
	pipeline_layout_info.pushConstantRangeCount = 0;
	pipeline_layout_info.pPushConstantRanges = NULL;
	CHECK_VK_RESULT(vkCreatePipelineLayout(vk_device, &pipeline_layout_info, NULL, &vk_graphics_pipeline_layout))
	
	VkAttachmentDescription attachment = {};
	attachment.flags = 0;
	attachment.format = vk_surface_format.format;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	
	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	
	VkSubpassDescription subpass = {};
	subpass.flags = 0;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = NULL;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;
	
	VkRenderPassCreateInfo renderpass_info = {};
	renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpass_info.pNext = NULL;
	renderpass_info.flags = 0;
	renderpass_info.attachmentCount = 1;
	renderpass_info.pAttachments = &attachment;
	renderpass_info.subpassCount = 1;
	renderpass_info.pSubpasses = &subpass;
	renderpass_info.dependencyCount = 0;
	renderpass_info.pDependencies = NULL; //Implicit dependency
	CHECK_VK_RESULT(vkCreateRenderPass(vk_device, &renderpass_info, NULL, &vk_renderpass))
	
	VkGraphicsPipelineCreateInfo graphics_pipeline_info = {};
	graphics_pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphics_pipeline_info.pNext = NULL;
	graphics_pipeline_info.flags = 0;
	graphics_pipeline_info.stageCount = 2;
	graphics_pipeline_info.pStages = shader_stage_infos;
	graphics_pipeline_info.pVertexInputState = &vertex_input_info;
	graphics_pipeline_info.pInputAssemblyState = &input_assembly_info;
	graphics_pipeline_info.pTessellationState = &tessellation_info;
	graphics_pipeline_info.pViewportState = &viewport_info;
	graphics_pipeline_info.pRasterizationState = &rasterization_info;
	graphics_pipeline_info.pMultisampleState = &multisample_info;
	graphics_pipeline_info.pDepthStencilState = &depth_stencil_info;
	graphics_pipeline_info.pColorBlendState = &color_blend_info;
	graphics_pipeline_info.pDynamicState = NULL;
	graphics_pipeline_info.layout = vk_graphics_pipeline_layout;
	graphics_pipeline_info.renderPass = vk_renderpass;
	graphics_pipeline_info.subpass = 0;
	graphics_pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	/* IGNORED
	graphics_pipeline_info.basePipelineIndex*/
	CHECK_VK_RESULT(vkCreateGraphicsPipelines(vk_device, VK_NULL_HANDLE, 1, &graphics_pipeline_info, NULL, &vk_graphics_pipeline))
}

void CleanUpVulkan()
{
	vkDestroyPipeline(vk_device, vk_graphics_pipeline, NULL);
	vkDestroyRenderPass(vk_device, vk_renderpass, NULL);
	vkDestroyPipelineLayout(vk_device, vk_graphics_pipeline_layout, NULL);
	vkDestroyShaderModule(vk_device, vk_fragment_shader_module, NULL);
	vkDestroyShaderModule(vk_device, vk_vertex_shader_module, NULL);
	for (auto& image_view : vk_swap_chain_image_views)
	{
		vkDestroyImageView(vk_device, image_view, NULL);
	}
	vkDestroySwapchainKHR(vk_device, vk_swap_chain, NULL);
	vkDestroySurfaceKHR(vk_instance, vk_surface, NULL);
	vkDestroyDevice(vk_device, NULL);
	DestroyDebugUtilsMessengerEXT(vk_instance, vk_debug_callback, NULL);
	vkDestroyInstance(vk_instance, NULL);
}

//https://iorange.github.io/
//https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Conclusion
int main()
{
	CreateWindow();
	InitVulkan();
	CreateSurface();
	PickPhysicalDevice();
	FindQueues();
	CreateLogicalDevice();
	CreateSwapChain();
	CreateImageViews();
	CreateGraphicsPipeline();

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	CleanUpVulkan();
	DestroyWindow();

	return 0;
}
