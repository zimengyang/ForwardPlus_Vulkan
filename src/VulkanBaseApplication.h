#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <fstream>
#include <array>
#include <chrono>
#include <unordered_map>
#include <random>

#define _USE_MATH_DEFINES
#include <cmath> 

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "VDeleter.h"

extern const int WIDTH;
extern const int HEIGHT;

//const std::string MODEL_PATH = "../src/models/chalet/chalet.obj";
const std::string MODEL_PATH = "../src/models/sibenik/sibenik.obj";

const std::string TEXTURE_PATH1 = "../src/models/chalet/chalet.jpg";
const std::string TEXTURE_PATH2 = "../src/textures/bald_eagle_1280.jpg";
const std::vector<std::string> TEXTURES_PATH = { TEXTURE_PATH1 ,TEXTURE_PATH2 };


const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif


// mouse control related
glm::vec2 cursorPos;
bool lbuttonDown, rbuttonDown;
glm::vec2 modelRotAngles; // for model rotation

glm::vec3 cameraPos;
glm::vec3 lookAtDir;
glm::vec2 cameraRotAngles; // for camera rotation

// deubg mode int
float debugMode;

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr) {
		func(instance, callback, pAllocator);
	}
}

struct QueueFamilyIndices{
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete() {
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};


struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};


struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 normal;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, normal);
		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord && normal == other.normal;
	}

};


namespace std {

	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1 ^ 
				(hash<glm::vec3>()(vertex.normal) << 1));
		}
	};
}


struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

struct LightInfo {
	glm::vec4 pos; // pos.w = intensity
	glm::vec4 color; // color.w = radius
};

#define MAX_NUM_LIGHT 8
struct FragLightInfos {
	LightInfo lights[MAX_NUM_LIGHT];
	int numLights;
};
bool lightMoveDirs[MAX_NUM_LIGHT];

/************************************************************/
//			Base Class for Vulkan Application
/************************************************************/
class VulkanBaseApplication {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
	}

private:
	GLFWwindow* window;

	VDeleter<VkInstance> instance{ vkDestroyInstance };
	VDeleter<VkDebugReportCallbackEXT> callback{ instance, DestroyDebugReportCallbackEXT };
	VDeleter<VkSurfaceKHR> surface{ instance, vkDestroySurfaceKHR};

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VDeleter<VkDevice> device{ vkDestroyDevice };

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	// swap chain
	VDeleter<VkSwapchainKHR> swapChain{ device, vkDestroySwapchainKHR };
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	 
	// image views
	std::vector<VDeleter<VkImageView>> swapChainImageViews;

	// pipeline layout
	VDeleter<VkDescriptorSetLayout> descriptorSetLayout{ device, vkDestroyDescriptorSetLayout };
	VDeleter<VkPipelineLayout> pipelineLayout{ device, vkDestroyPipelineLayout };

	// render pass
	VDeleter<VkRenderPass> renderPass{ device, vkDestroyRenderPass };

	// graphics pipeline(s) !
	VDeleter<VkPipeline> graphicsPipeline{ device, vkDestroyPipeline };
	VDeleter<VkPipeline> graphicsPipeline_axis{ device, vkDestroyPipeline };
	VDeleter<VkPipeline> graphicsPipeline_quad{ device, vkDestroyPipeline };

	// frame buffers
	std::vector<VDeleter<VkFramebuffer>> swapChainFramebuffers;

	// command pool
	VDeleter<VkCommandPool> commandPool{ device, vkDestroyCommandPool };

	// command buffers
	std::vector<VkCommandBuffer> commandBuffers;

	// semaphores
	VDeleter<VkSemaphore> imageAvailableSemaphore{ device, vkDestroySemaphore };
	VDeleter<VkSemaphore> renderFinishedSemaphore{ device, vkDestroySemaphore };

	// vertices and indices
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	// vertex buffer
	VDeleter<VkBuffer> vertexBuffer{ device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> vertexBufferMemory{ device, vkFreeMemory };

	// index buffer
	VDeleter<VkBuffer> indexBuffer{ device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> indexBufferMemory{ device, vkFreeMemory };


	// vertices and indices for axis
	std::vector<Vertex> vertices_axis;
	std::vector<uint32_t> indices_axis;

	// vertex buffer for axis
	VDeleter<VkBuffer> vertexBuffer_axis{ device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> vertexBufferMemory_axis{ device, vkFreeMemory };

	// index buffer for axis 
	VDeleter<VkBuffer> indexBuffer_axis{ device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> indexBufferMemory_axis{ device, vkFreeMemory };


	// vertices and indices for texutre quad
	std::vector<Vertex> vertices_quad;
	std::vector<uint32_t> indices_quad;

	// vertex buffer
	VDeleter<VkBuffer> vertexBuffer_quad{ device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> vertexBufferMemory_quad{ device, vkFreeMemory };

	// index buffer
	VDeleter<VkBuffer> indexBuffer_quad{ device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> indexBufferMemory_quad{ device, vkFreeMemory };


	// uniform buffer 
	VDeleter<VkBuffer> uniformStagingBuffer{ device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> uniformStagingBufferMemory{ device, vkFreeMemory };
	VDeleter<VkBuffer> uniformBuffer{ device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> uniformBufferMemory{ device, vkFreeMemory };

	// fragment shader light infos
	VDeleter<VkBuffer> fragLightsStagingBuffer{ device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> fragLightsStagingBufferMemory{ device, vkFreeMemory };
	VDeleter<VkBuffer> fragLightsBuffer{ device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> fragLightsBufferMemory{ device, vkFreeMemory };

	FragLightInfos fragLightInfos;

	// descriptor pool
	VDeleter<VkDescriptorPool> descriptorPool{ device, vkDestroyDescriptorPool };
	VkDescriptorSet descriptorSet;

	// texture 
	std::vector<VDeleter<VkImage>> textureImages{ 
		VDeleter<VkImage> { device, vkDestroyImage},
		VDeleter<VkImage> { device, vkDestroyImage} };

	std::vector<VDeleter<VkDeviceMemory>> textureImageMemorys{
		VDeleter<VkDeviceMemory> { device, vkFreeMemory },
		VDeleter<VkDeviceMemory> { device, vkFreeMemory } };

	std::vector<VDeleter<VkImageView>> textureImageViews{
		VDeleter<VkImageView> { device, vkDestroyImageView },
		VDeleter<VkImageView> { device, vkDestroyImageView } };

	std::vector<VDeleter<VkSampler>> textureSamplers{
		VDeleter<VkSampler> { device, vkDestroySampler },
		VDeleter<VkSampler> { device, vkDestroySampler } };

	// depth image
	VDeleter<VkImage> depthImage{ device, vkDestroyImage };
	VDeleter<VkDeviceMemory> depthImageMemory{ device, vkFreeMemory };
	VDeleter<VkImageView> depthImageView{ device, vkDestroyImageView };

	// shader modules 
	std::vector<VDeleter<VkShaderModule>> shaderModules;


	/************************************************************/
	//					Function Declaration 
	/************************************************************/
	void initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

		// setup callback functions
		cameraPos = glm::vec3(2.5f, 2.5f, 2.5f);


		glfwSetCursorPosCallback(window, cursorPosCallback);
		glfwSetMouseButtonCallback(window, mouseButtonCallback);
		glfwSetKeyCallback(window, keyCallback);
		glfwSetScrollCallback(window, scrollCallback);
	}


	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();

			updateUniformBuffer();
			drawFrame();
		}

		vkDeviceWaitIdle(device);
	}


	void drawFrame() {
		uint32_t imageIndex;
		vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		vkQueuePresentKHR(presentQueue, &presentInfo);
	}


	void initVulkan() {

		createInstance();
		setupDebugCallback();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createCommandPool();
		createDepthResources();
		createFramebuffers();
		prepareTextures();
		/*createTextureImage();
		createTextureImageView();
		createTextureSampler();*/
		loadModel(vertices, indices, MODEL_PATH, 0.4f);
		loadAxisInfo();
		loadTextureQuad();

		// vertices and indices buffer
		createVertexBuffer(vertices, vertexBuffer, vertexBufferMemory);
		createIndexBuffer(indices, indexBuffer, indexBufferMemory);
		createVertexBuffer(vertices_axis, vertexBuffer_axis, vertexBufferMemory_axis);
		createIndexBuffer(indices_axis, indexBuffer_axis, indexBufferMemory_axis);
		createVertexBuffer(vertices_quad, vertexBuffer_quad, vertexBufferMemory_quad);
		createIndexBuffer(indices_quad, indexBuffer_quad, indexBufferMemory_quad);

		// light information
		createLightInfos();

		createUniformBuffer();
		createDescriptorPool();
		createDescriptorSet();
		createCommandBuffers();
		createSemaphores();
	}


	// Create Vulkan instance
	void createInstance() {
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Vulkan";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = uint32_t(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = uint32_t(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateInstance(&createInfo, nullptr, instance.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}

		printInstanceExtensions();
	}


	void setupDebugCallback() {
		if (!enableValidationLayers) return;

		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = debugCallback;

		if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, callback.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug callback!");
		}
	}


	void createSurface() {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}


	void pickPhysicalDevice() {

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}


	void createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

		float queuePriority = 1.0f;
		for (int queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back(queueCreateInfo);
		}


		VkPhysicalDeviceFeatures deviceFeatures = {};
		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = uint32_t(queueCreateInfos.size());

		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = uint32_t(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = uint32_t(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, device.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		// retrieving queue handles
		vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);

	}


	void createSwapChain() {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			//createInfo.queueFamilyIndexCount = 0; // Optional
			//createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, swapChain.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}


	void createImageViews() {
		swapChainImageViews.resize(swapChainImages.size(), VDeleter<VkImageView>{device, vkDestroyImageView});

		/*
		for (uint32_t i = 0; i < swapChainImages.size(); i++) {
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device, &createInfo, nullptr, swapChainImageViews[i].replace()) != VK_SUCCESS) {
				throw std::runtime_error("failed to create image views!");
			}
		}
		*/

		for (uint32_t i = 0; i < swapChainImages.size(); i++) {
			createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, swapChainImageViews[i]);
		}
	}


	void createGraphicsPipeline()
	{
#pragma region Vertex and Fragment Shader Stages
		// vertex and fragment shader stages

		shaderModules.resize(5);
		VkPipelineShaderStageCreateInfo vertShaderStageInfo = loadShader("../src/shaders/triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, 0);
		VkPipelineShaderStageCreateInfo fragShaderStageInfo = loadShader("../src/shaders/triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, 1);
		VkPipelineShaderStageCreateInfo vertShaderStageInfo_axis = loadShader("../src/shaders/axis.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, 2);
		VkPipelineShaderStageCreateInfo fragShaderStageInfo_axis = loadShader("../src/shaders/axis.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, 3);
		VkPipelineShaderStageCreateInfo fragShaderStageInfo_quad = loadShader("../src/shaders/quad.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, 4);


		//// load shader
		//auto triangleVertShaderCode = readFile("../src/shaders/triangle.vert.spv");
		//auto triangleFragShaderCode = readFile("../src/shaders/triangle.frag.spv");
		////std::cout << "vert shader size = " << triangleVertShaderCode.size() << ", "
		////	      << "frag shader size = " << triangleFragShaderCode.size() << std::endl;

		//// create shader module
		//VDeleter<VkShaderModule> vertShaderModule{ device, vkDestroyShaderModule };
		//VDeleter<VkShaderModule> fragShaderModule{ device, vkDestroyShaderModule };

		//createShaderModule(triangleVertShaderCode, vertShaderModule);
		//createShaderModule(triangleFragShaderCode, fragShaderModule);

		//// create pipeline shader stages vert and frag
		//VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		//vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		//vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		//vertShaderStageInfo.module = vertShaderModule;
		//vertShaderStageInfo.pName = "main";

		//VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		//fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		//fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		//fragShaderStageInfo.module = fragShaderModule;
		//fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
#pragma endregion


#pragma region Vertex Input State
		// vertex input state
		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; 
		vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
#pragma endregion


#pragma region Input Assembly State
		// input assembly state
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		//inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
#pragma endregion

	
#pragma region Viewport State
		// viewport state

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapChainExtent.width;
		viewport.height = (float)swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;
#pragma endregion

		
#pragma region Rasterizer State
		// rasterizer state

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
#pragma endregion

			
#pragma region Multisampling State
		// multisampling state
		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; /// Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional
#pragma endregion


#pragma region Depth and Stencil Testing
		// depth and stencil testing
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {}; // Optional
		depthStencil.back = {}; // Optional
#pragma endregion


#pragma region Dynamic state
		// dynamic state
		VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
		};

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;
#pragma endregion

		
#pragma region Pipeline Layout 
		// pipeline layout object
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		VkDescriptorSetLayout setLayouts[] = { descriptorSetLayout};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = setLayouts; 
		//pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		//pipelineLayoutInfo.pPushConstantRanges = 0; // Optional

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
			pipelineLayout.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
#pragma endregion

		
#pragma region Graphics Pipeline Creation
		// combine all information, generate graphics pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr; // Optional
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1; // Optional
#pragma endregion

		
		// create graphics pipeline finally!
		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, graphicsPipeline.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		// create graphics pipeline for quad render
		// input assembly state for texture quad, without culling
		shaderStages[1] = fragShaderStageInfo_quad;
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, graphicsPipeline_quad.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		// create graphics pipeline for line list
		// input assembly state for axis (lines)
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		shaderStages[0] = vertShaderStageInfo_axis;
		shaderStages[1] = fragShaderStageInfo_axis;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, graphicsPipeline_axis.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}
		

	}

	void createFramebuffers() {
		swapChainFramebuffers.resize(swapChainImageViews.size(), VDeleter<VkFramebuffer>{device, vkDestroyFramebuffer});
		
		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			std::array<VkImageView, 2> attachments = {
				swapChainImageViews[i],
				depthImageView
			};


			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = (uint32_t)attachments.size();
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, swapChainFramebuffers[i].replace()) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	
	void createCommandPool() {
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
		poolInfo.flags = 0; // Optional

		if (vkCreateCommandPool(device, &poolInfo, nullptr, commandPool.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}


	void createCommandBuffers() {
		commandBuffers.resize(swapChainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}

		for (size_t i = 0; i < commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

			vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = swapChainFramebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swapChainExtent;

			std::array<VkClearValue, 2> clearValues = {};
			clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };

			renderPassInfo.clearValueCount = (uint32_t)clearValues.size();
			renderPassInfo.pClearValues = clearValues.data();


			// render pass begin
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

				// draw model here (triangle list)
				vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

				// binding the vertex buffer
				VkBuffer vertexBuffers[] = { vertexBuffer };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

				vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

				vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
				
				//vkCmdDraw(commandBuffers[i], vertices.size(), 1, 0, 0);
				vkCmdDrawIndexed(commandBuffers[i], (uint32_t)indices.size(), 1, 0, 0, 0);


				// draw quad here
				//vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_quad);

				//// binding the vertex buffer
				//VkBuffer vertexBuffers_quad[] = { vertexBuffer_quad };
				//VkDeviceSize offsets_quad[] = { 0 };
				//vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers_quad, offsets_quad);

				//vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer_quad, 0, VK_INDEX_TYPE_UINT32);

				//vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

				////vkCmdDraw(commandBuffers[i], vertices.size(), 1, 0, 0);
				//vkCmdDrawIndexed(commandBuffers[i], (uint32_t)indices_quad.size(), 1, 0, 0, 0);

				
				// draw axis here (line list)
				// bind pipeline
				vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_axis);
				// binding the vertex buffer for axis
				VkBuffer vertexBuffers_axis[] = { vertexBuffer_axis };
				VkDeviceSize offsets_axis[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers_axis, offsets_axis);

				vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer_axis, 0, VK_INDEX_TYPE_UINT32);

				vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

				//vkCmdDraw(commandBuffers[i], vertices.size(), 1, 0, 0);
				vkCmdDrawIndexed(commandBuffers[i], (uint32_t)indices_axis.size(), 1, 0, 0, 0);


			vkCmdEndRenderPass(commandBuffers[i]);

			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}
	}



	void createRenderPass() {

		// color attachment
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// depth attachment
		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// subpass 
		VkSubpassDescription subPass = {};
		subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subPass.colorAttachmentCount = 1;
		subPass.pColorAttachments = &colorAttachmentRef;
		subPass.pDepthStencilAttachment = &depthAttachmentRef;

		// dependency
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		// render pass info
		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = (uint32_t)attachments.size();
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subPass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, renderPass.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}


	void createVertexBuffer(std::vector<Vertex> & verticesData, VDeleter<VkBuffer>& buffer, VDeleter<VkDeviceMemory>& bufferMemory) {
		VkDeviceSize bufferSize = sizeof(verticesData[0]) * verticesData.size();

		VDeleter<VkBuffer> stagingBuffer{ device, vkDestroyBuffer };
		VDeleter<VkDeviceMemory> stagingBufferMemory{ device, vkFreeMemory };
		createBuffer(bufferSize, 
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, verticesData.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		createBuffer(bufferSize, 
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			buffer, bufferMemory);

		copyBuffer(stagingBuffer, buffer, bufferSize);
	}


	// index buffer
	void createIndexBuffer(std::vector<uint32_t> &indicesData, VDeleter<VkBuffer>& buffer, VDeleter<VkDeviceMemory>& bufferMemory) {

		VkDeviceSize bufferSize = sizeof(indicesData[0]) * indicesData.size();

		VDeleter<VkBuffer> stagingBuffer{ device, vkDestroyBuffer };
		VDeleter<VkDeviceMemory> stagingBufferMemory{ device, vkFreeMemory };
		createBuffer(bufferSize, 
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, indicesData.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		createBuffer(bufferSize, 
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			buffer, bufferMemory);

		copyBuffer(stagingBuffer, buffer, bufferSize);
	}


	void createSemaphores() {
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, imageAvailableSemaphore.replace()) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, renderFinishedSemaphore.replace()) != VK_SUCCESS) {

			throw std::runtime_error("failed to create semaphores!");
		}
	}

	// check device suitabiliy
	bool isDeviceSuitable(VkPhysicalDevice device) {

		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && extensionSupported && swapChainAdequate;
	}


	// find queue families
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (queueFamily.queueCount > 0 && presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}
			i++;
		}

		return indices;
	}


	// get extension that are required by instance
	std::vector<const char*> getRequiredExtensions() {
		std::vector<const char*> extensions;

		unsigned int glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		for (unsigned int i = 0; i < glfwExtensionCount; i++) {
			extensions.push_back(glfwExtensions[i]);
		}

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}

		return extensions;
	}


	// check validation layer support
	bool checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}


	// debug call back func
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData) {
		std::cerr << "validation layer: " << msg << std::endl;

		return VK_FALSE;
	}


	// helper func: print extension names
	void printInstanceExtensions()
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> extensions(extensionCount);

		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		std::cout << "available extensions:" << std::endl;

		for (const auto& extension : extensions) {
			std::cout << "\t" << extension.extensionName << std::endl;
		}
	}


	// check device extension support
	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {

		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto & extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}


	// query swap chain support, assign SwapChainSupportDetails struct
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;

		// basic surface capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		// query supported surface format
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		// query supported presentation modes
		uint32_t presentModesCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, nullptr);

		if (presentModesCount != 0) {
			details.presentModes.resize(presentModesCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, details.presentModes.data());
		}

		return details;
	}


	// swap chain choose format
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
			return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}

		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}


	// swap chain, choose present mode
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}


	// swap chain, choose swap extent
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			VkExtent2D actualExtent = { (uint32_t)WIDTH, (uint32_t)HEIGHT };

			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}


	// read shader file from compiled binary file
	static std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	void createShaderModule(const std::vector<char> & code, VDeleter<VkShaderModule> & shaderModule) {

		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = (uint32_t*)code.data();

		if (vkCreateShaderModule(device, &createInfo, nullptr, shaderModule.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

	}


	VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage, int shaderModuleIndex) {
		VkPipelineShaderStageCreateInfo shaderStage = {};
		std::vector<char> codes = readFile(fileName);
		createShaderModule(codes, shaderModules[shaderModuleIndex]);

		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.stage = stage;
		shaderStage.module = shaderModules[shaderModuleIndex];
		shaderStage.pName = "main"; // todo : make param

		return shaderStage;
	}


	// find memory type
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	// abstracting buffer creation
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VDeleter<VkBuffer>& buffer, VDeleter<VkDeviceMemory>& bufferMemory) {
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, buffer.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, bufferMemory.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	// copy buffer from srcBuffer to dstBuffer
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		/*
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion = {};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
		*/

		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion = {};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		endSingleTimeCommands(commandBuffer);
	}


	// descriptor set layout
	void createDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding samplerLayoutBinding2 = {};
		samplerLayoutBinding2.binding = 2;
		samplerLayoutBinding2.descriptorCount = 1;
		samplerLayoutBinding2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding2.pImmutableSamplers = nullptr;
		samplerLayoutBinding2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding fragLightsLayoutBinding = {};
		fragLightsLayoutBinding.binding = 3;
		fragLightsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		fragLightsLayoutBinding.descriptorCount = 1;
		fragLightsLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragLightsLayoutBinding.pImmutableSamplers = nullptr; // Optional

		std::array<VkDescriptorSetLayoutBinding, 4> bindings = { uboLayoutBinding, samplerLayoutBinding, samplerLayoutBinding2, fragLightsLayoutBinding };
		
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = (uint32_t)bindings.size();
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, descriptorSetLayout.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

	}

	void createLightInfos() {

		fragLightInfos.numLights = 8;

		//fragLightInfos.lights[0].pos = glm::vec4(0.3f, 1, 1.2f, 1); 
		//fragLightInfos.lights[0].color = glm::vec4(1, 0, 0, 1.5f); 

		//fragLightInfos.lights[1].pos = glm::vec4(0.0f, 0.2f, 1.0f, 1.5f);
		//fragLightInfos.lights[1].color = glm::vec4(0, 0, 1, 3.0f);

		//fragLightInfos.lights[2].pos = glm::vec4(0, -1, -0.5f, 1);
		//fragLightInfos.lights[2].color = glm::vec4(0, 1, 0, 1.5f);

		//fragLightInfos.lights[3].pos = glm::vec4(-0.1f, 0.6f, -0.7f, 1);
		//fragLightInfos.lights[3].color = glm::vec4(0.3f, 0.1f, 0.5f, 2.5f);

		std::default_random_engine gen;
		std::uniform_real_distribution<float> uniformDistribution(0.0f, 1.0f);

		float scale = 3.0f;
		for (int i = 0; i < fragLightInfos.numLights; ++i) {
			float posX = uniformDistribution(gen) * 4.0f * scale - 2.0f * scale; 
			float posY = uniformDistribution(gen) * 2.0f - 1.0f;
			float posZ = uniformDistribution(gen) * 2.0f * scale - scale;
			float intensity = uniformDistribution(gen) * 0.8f; 
			fragLightInfos.lights[i].pos = glm::vec4(posX, posY, posZ, intensity);

			fragLightInfos.lights[i].color = glm::vec4(
				uniformDistribution(gen),
				uniformDistribution(gen), 
				uniformDistribution(gen),
				uniformDistribution(gen) * 2.0f * scale);

			lightMoveDirs[i] = false;
		}
	}

	void createUniformBuffer() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformStagingBuffer, uniformStagingBufferMemory);
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uniformBuffer, uniformBufferMemory);
		
		bufferSize = sizeof(FragLightInfos);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, fragLightsStagingBuffer, fragLightsStagingBufferMemory);
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, fragLightsBuffer, fragLightsBufferMemory);

	}

	void updateUniformBuffer() {

		//--------------------- update vertex uniform buffer----------------------------------
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;
	
		UniformBufferObject ubo = {};

		// update model rotations
		//ubo.model = glm::rotate(glm::mat4(), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//ubo.model = glm::rotate(glm::mat4(), modelRotAngles.x , glm::vec3(0.0f, 0.0f, 1.0f)) * glm::rotate(glm::mat4(), modelRotAngles.y, glm::vec3(0.0f, 1.0f, 0.0f));
		ubo.model = glm::translate(glm::mat4(), glm::vec3(0, 4, 0));

		// update camera rotations
		glm::vec4 rotCameraPos = glm::vec4(cameraPos, 1.0f);
		rotCameraPos = glm::rotate(glm::mat4(), -cameraRotAngles.x, glm::vec3(0.0f, 0.0f, 1.0f)) * rotCameraPos;
		//cameraPos = glm::vec3(rotCameraPos);
		//ubo.view = glm::lookAt(glm::vec3(rotCameraPos), glm::vec3(0,0,0), glm::vec3(0.0f, 1.0f, 0.0f));
		ubo.view = glm::lookAt(glm::vec3(-5, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0f));

		// projection matrix
		ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.01f, 100.0f);
		ubo.proj[1][1] *= -1;

		// light position
		//ubo.lightPosition = rotCameraPos;

		// debug mode
		//ubo.debugMode = debugMode;
		//ubo.lightPosition.w = debugMode;

		void* data;
		vkMapMemory(device, uniformStagingBufferMemory, 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, uniformStagingBufferMemory);

		copyBuffer(uniformStagingBuffer, uniformBuffer, sizeof(ubo));


		//--------------------- update frag uniform buffer (lights)----------------------------------

		for (int i = 0; i < fragLightInfos.numLights; ++i) {
			fragLightInfos.lights[i].pos.y += 0.0015f * ((float)lightMoveDirs[i] * 2.0f - 1.0f);

			if (fragLightInfos.lights[i].pos.y > 1.0f || fragLightInfos.lights[i].pos.y < -2.0f)
			{
				//fragLightInfos.lights[i].pos.y = 2.0f;
				lightMoveDirs[i] = !lightMoveDirs[i];
			}
		}

		vkMapMemory(device, fragLightsStagingBufferMemory, 0, sizeof(fragLightInfos), 0, &data);
		memcpy(data, &fragLightInfos, sizeof(fragLightInfos));
		vkUnmapMemory(device, fragLightsStagingBufferMemory);

		copyBuffer(fragLightsStagingBuffer, fragLightsBuffer, sizeof(fragLightInfos));
	}

	void createDescriptorPool() {

		std::array<VkDescriptorPoolSize, 4> poolSizes = {};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 1; 
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[2].descriptorCount = 1; 
		poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[3].descriptorCount = 1;

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = (uint32_t) poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 1;

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, descriptorPool.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void createDescriptorSet() {
		VkDescriptorSetLayout layouts[] = { descriptorSetLayout };
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = layouts;

		if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor set!");
		}


		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = uniformBuffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorBufferInfo lightsBufferInfo = {};
		lightsBufferInfo.buffer = fragLightsBuffer;
		lightsBufferInfo.offset = 0;
		lightsBufferInfo.range = sizeof(FragLightInfos);

		std::array<VkDescriptorImageInfo, 2> imageInfo = {};
		imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[0].imageView = textureImageViews[0];
		imageInfo[0].sampler = textureSamplers[0]; 
		
		imageInfo[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[1].imageView = textureImageViews[1];
		imageInfo[1].sampler = textureSamplers[1];

		std::array<VkWriteDescriptorSet, 4> descriptorWrites = {};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSet;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSet;
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo[0];

		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = descriptorSet;
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pImageInfo = &imageInfo[1];

		descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[3].dstSet = descriptorSet;
		descriptorWrites[3].dstBinding = 3;
		descriptorWrites[3].dstArrayElement = 0;
		descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[3].descriptorCount = 1;
		descriptorWrites[3].pBufferInfo = &lightsBufferInfo;

		vkUpdateDescriptorSets(device, (uint32_t)descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
	}


	void createTextureImage(const std::string& texFilename, VDeleter<VkImage> & texImage, VDeleter<VkDeviceMemory> & texImageMemory) {

		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(texFilename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}

		VDeleter<VkImage> stagingImage{ device, vkDestroyImage };
		VDeleter<VkDeviceMemory> stagingImageMemory{ device, vkFreeMemory };
		createImage(
			texWidth, texHeight, 
			VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, 
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			stagingImage, stagingImageMemory);

		void* data;
		vkMapMemory(device, stagingImageMemory, 0, imageSize, 0, &data);
			memcpy(data, pixels, (size_t)imageSize);
		vkUnmapMemory(device, stagingImageMemory);

		stbi_image_free(pixels);

		createImage(
			texWidth, texHeight, 
			VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			texImage, texImageMemory);

		transitionImageLayout(stagingImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		transitionImageLayout(texImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyImage(stagingImage, texImage, texWidth, texHeight);

		transitionImageLayout(texImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}


	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VDeleter<VkImage>& image, VDeleter<VkDeviceMemory>& imageMemory) {
		
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(device, &imageInfo, nullptr, image.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, imageMemory.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device, image, imageMemory, 0);
	}


	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		// use the right subresource aspect
		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (hasStencilComponent(format)) {
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}


		if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
			);

		endSingleTimeCommands(commandBuffer);
	}


	void copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageSubresourceLayers subResource = {};
		subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subResource.baseArrayLayer = 0;
		subResource.mipLevel = 0;
		subResource.layerCount = 1;

		VkImageCopy region = {};
		region.srcSubresource = subResource;
		region.dstSubresource = subResource;
		region.srcOffset = { 0, 0, 0 };
		region.dstOffset = { 0, 0, 0 };
		region.extent.width = width;
		region.extent.height = height;
		region.extent.depth = 1;

		vkCmdCopyImage(
			commandBuffer,
			srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &region
			);

		endSingleTimeCommands(commandBuffer);
	}


	VkCommandBuffer beginSingleTimeCommands() {
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}


	void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}


	void createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VDeleter<VkImageView>& imageView) {
		
		VkImageViewCreateInfo viewInfo = {};
		
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &viewInfo, nullptr, imageView.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}
	}


	void createTextureImageView(VDeleter<VkImage> & textureImage, VDeleter<VkImageView>& textureImageView) {
		createImageView(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, textureImageView);
	}


	void createTextureSampler(VDeleter<VkSampler> & textureSampler) {
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;

		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(device, &samplerInfo, nullptr, textureSampler.replace()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}


	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
		for (VkFormat format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		throw std::runtime_error("failed to find supported format!");
	}


	VkFormat findDepthFormat() {
		return findSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
			);
	}


	bool hasStencilComponent(VkFormat format) {
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	void createDepthResources() {
		VkFormat depthFormat = findDepthFormat();

		createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
		createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, depthImageView);
	
		transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}


	void loadModel(std::vector<Vertex> & vertices, std::vector<uint32_t> & indices, const std::string & modelFilename, float scale = 1.0f) {

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, modelFilename.c_str())) {
			throw std::runtime_error(err);
		}

		std::unordered_map<Vertex, int> uniqueVertices = {};

		for (const auto& shape : shapes) {
			//for (const auto& index : shape.mesh.indices) {
			for (int i = 0; i < shape.mesh.indices.size();i++) {
				auto& index = shape.mesh.indices[i];
				Vertex vertex = {};

				vertex.pos = {
					scale * attrib.vertices[3 * index.vertex_index + 0],
					scale * attrib.vertices[3 * index.vertex_index + 1],
					scale * attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.color = { 1.0f, 1.0f, 1.0f };

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = (uint32_t)vertices.size();
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}


		// compute triangle normal here
		for (int i = 0; i < indices.size(); i += 3) {
			glm::vec3 P1 = vertices[indices[i + 0]].pos;
			glm::vec3 P2 = vertices[indices[i + 1]].pos;
			glm::vec3 P3 = vertices[indices[i + 2]].pos;

			glm::vec3 V = P2 - P1;
			glm::vec3 W = P3 - P1;

			glm::vec3 N = glm::cross(V, W);

			N = glm::normalize(N);
			vertices[indices[i + 0]].normal = N;
			vertices[indices[i + 1]].normal = N;
			vertices[indices[i + 2]].normal = N;
		}
	}


	// load axis info
	void loadAxisInfo() {

		const float axisLen = 1.0f;
		const float axisDelta = 0.1f;
		vertices_axis = {

			{ { 0.0f, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
			{ { axisLen, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
			{ { axisLen - axisDelta, -axisDelta / 2.0f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
			{ { axisLen - axisDelta, axisDelta / 2.0f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },

			{ { 0.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f } },
			{ { 0.0f, axisLen, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f } },
			{ { -axisDelta / 2.0f, axisLen - axisDelta, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f } },
			{ { axisDelta / 2.0f, axisLen - axisDelta, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f } },

			{ { 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { 0.0f, 0.0f, axisLen },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { 0.0f, -axisDelta / 2.0f, axisLen - axisDelta },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { 0.0f, axisDelta / 2.0f, axisLen - axisDelta },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } }
		};

		indices_axis = {
			0, 1,
			1, 2,
			1, 3,

			4, 5,
			5, 6,
			5, 7,

			8, 9,
			9, 10,
			9, 11
		}; 

	}
	
	
	// load texture quad info 
	void loadTextureQuad() {

		const float x = 1.5f;
		const float axisDelta = 0.1f;
		vertices_quad = {
		
			{ { 0, 0.25f, 1.5f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
			{ { 0, -0.25f, 1.50f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },
			{ { 0, -0.25f, 1.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
			{ { 0, 0.25f, 1.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },
			
		};
		
		indices_quad = {
			0, 1, 2, 2, 3, 0,
		};

	}

	
	void prepareTextures()
	{
		/*createTextureImage();
		createTextureImageView();
		createTextureSampler();*/

		for (int i = 0; i < textureImages.size(); ++i) {
			createTextureImage(TEXTURES_PATH[i], textureImages[i], textureImageMemorys[i]);
			createTextureImageView(textureImages[i], textureImageViews[i]);
			createTextureSampler(textureSamplers[i]);
		}

	}

	// mouse control related
	static void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {
		//std::cout << xPos << "," << yPos << std::endl;
		if (!lbuttonDown && !rbuttonDown) {
			return;
		}
		else if (lbuttonDown) {
			glm::vec2 tmp(xPos, yPos);
			modelRotAngles += (tmp - cursorPos) * 0.01f;

			cursorPos = tmp;
		}
		else if (rbuttonDown) {
			glm::vec2 tmp(xPos, yPos);
			cameraRotAngles += (tmp - cursorPos) * 0.01f;

			cursorPos = tmp;
		}
			
	}

	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {

		if (action == GLFW_PRESS) {
			double x, y;
			glfwGetCursorPos(window, &x, &y);
			cursorPos = glm::vec2(x, y);

			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				lbuttonDown = true;
			}
			else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
				rbuttonDown = true;
			}
		}
		else if (action == GLFW_RELEASE) {
			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				lbuttonDown = false;
			}
			else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
				rbuttonDown = false;
			}
			
		}
		
	}

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

		if (action == GLFW_PRESS)
		{
			switch (key)
			{
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, GLFW_TRUE);
				break;
			case GLFW_KEY_0:
				debugMode = 0;
				break;
			case GLFW_KEY_1:
				debugMode = 1;
				break;
			case GLFW_KEY_2:
				debugMode = 2;
				break;
			case GLFW_KEY_3:
				debugMode = 3;
				break;
			case GLFW_KEY_4:
				debugMode = 4;
				break;
			case GLFW_KEY_5:
				debugMode = 5;
				break;
			default:
				break;
			}
		}
	
	}

	static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
		//std::cout << xoffset << "," << yoffset << std::endl;
		if (cameraPos.length() > FLT_EPSILON) {
			cameraPos -= (float)yoffset * glm::normalize(cameraPos) * 0.05f;
		}
	}
};
