#include <stdlib.h>
#include <vector>
#include "VulkanApp.h"

int main(int argc, char** argv)
{
	std::vector<const char*> validationLayerNames = {
		"VK_LAYER_LUNARG_standard_validation"
	};
	std::vector<const char*> extensionNames = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_NVX_RAYTRACING_EXTENSION_NAME
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
		0.0f, -1.0f,
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
	
	
	vkFreeMemory(vkApp.vkDevice, indexBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, indexBuffer, NULL);
	vkFreeMemory(vkApp.vkDevice, vertexBufferMemory, NULL);
	vkDestroyBuffer(vkApp.vkDevice, vertexBuffer, NULL);

	return EXIT_SUCCESS;
}
