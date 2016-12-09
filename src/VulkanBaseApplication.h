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

#include "VDeleter.h"


// debug validation layers
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// debug report funcs definitions
VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);


// const variables
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


struct QueueFamilyIndices {
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


/************************************************************/
//			Base Class for Vulkan Application
/************************************************************/
class VulkanBaseApplication {
public:
	void run();

	// clean up resources
	~VulkanBaseApplication();


private:
	GLFWwindow* window;

	VDeleter<VkInstance> instance{ vkDestroyInstance };
	VDeleter<VkDebugReportCallbackEXT> callback{ instance, DestroyDebugReportCallbackEXT };
	VDeleter<VkSurfaceKHR> surface{ instance, vkDestroySurfaceKHR};

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VDeleter<VkDevice> device{ vkDestroyDevice };

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	// Swap chain related
	VDeleter<VkSwapchainKHR> swapChain{ device, vkDestroySwapchainKHR };
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	// swap chain image views
	std::vector<VkImageView> swapChainImageViews;
	// swap chian frame buffers
	std::vector<VDeleter<VkFramebuffer>> swapChainFramebuffers;

	// Pipeline layout
	VDeleter<VkPipelineLayout> pipelineLayout{ device, vkDestroyPipelineLayout };
	VDeleter<VkPipelineLayout> computePipelineLayout = {device, vkDestroyPipelineLayout};

	// Descriptor pool
	VDeleter<VkDescriptorPool> descriptorPool{ device, vkDestroyDescriptorPool };

	// Descriptor set layout and descriptor set
	VDeleter<VkDescriptorSetLayout> descriptorSetLayout{ device, vkDestroyDescriptorSetLayout };
	VkDescriptorSet descriptorSet;

	// Render pass
	VDeleter<VkRenderPass> renderPass{ device, vkDestroyRenderPass };

	// Command pool
	VDeleter<VkCommandPool> commandPool{ device, vkDestroyCommandPool };

	// Command buffers
	std::vector<VkCommandBuffer> commandBuffers;
	VkCommandBuffer computeCommandBuffer;

	// Semaphores
	VDeleter<VkSemaphore> imageAvailableSemaphore{ device, vkDestroySemaphore };
	VDeleter<VkSemaphore> renderFinishedSemaphore{ device, vkDestroySemaphore };

	// Pipeline(s)
	struct Pipelines{
		VkPipeline graphics; // base pipeline
		VkPipeline axis; // axis pipeline
		VkPipeline quad; // quad pipeline
		VkPipeline compute; // compute pipeline
		VkPipeline computeFrustumGrid; // compute Frustum Grid pipeline

		void cleanup(VkDevice device) {
			vkDestroyPipeline(device, graphics, nullptr);
			vkDestroyPipeline(device, axis, nullptr);
			vkDestroyPipeline(device, quad, nullptr);
			vkDestroyPipeline(device, compute, nullptr);
			vkDestroyPipeline(device, computeFrustumGrid, nullptr);
		}

	} pipelines;


	// Vertex/Index buffer struct
	struct VertexBuffer {
		std::vector<Vertex> verticesData;
		VkBuffer buffer;
		VkDeviceMemory mem;
	};


	struct IndexBuffer {
		std::vector<uint32_t> indicesData;
		VkBuffer buffer;
		VkDeviceMemory mem;
	};

	struct Mesh {
		VertexBuffer vertices;
		IndexBuffer indices;

		void cleanup(VkDevice device) {
			vkDestroyBuffer(device, vertices.buffer, nullptr);
			vkFreeMemory(device, vertices.mem, nullptr);
			vkDestroyBuffer(device, indices.buffer, nullptr);
			vkFreeMemory(device, indices.mem, nullptr);
		}
	};

	struct {
		Mesh scene; // main scene mesh
		Mesh axis; // axis mesh
		Mesh quad; // quad mesh

		void cleanup(VkDevice device) {
			scene.cleanup(device);
			axis.cleanup(device);
			quad.cleanup(device);
		}

	} meshs;


	// Uniform buffers
	struct UniformData {
		VkBuffer buffer;
		VkDeviceMemory memory;
		//VkDescriptorBufferInfo descriptor;
		VkDeviceSize allocSize;

		void cleanup(VkDevice device) {
			vkDestroyBuffer(device, buffer, nullptr);
			vkFreeMemory(device, memory, nullptr);
		}
	};

	struct {
		UniformData vsScene, vsSceneStaging;
		UniformData csParams, csParamsStaging;

		void cleanup(VkDevice device) {
			vsScene.cleanup(device);
			vsSceneStaging.cleanup(device);
			csParams.cleanup(device);
			csParamsStaging.cleanup(device);
		}
	} uniformData;

	// storage buffers
	struct StorageData {
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDeviceSize allocSize;

		void cleanup(VkDevice device) {
			vkDestroyBuffer(device, buffer, nullptr);
			vkFreeMemory(device, memory, nullptr);
		}
	};

	struct {
		StorageData lights, lightsStaging;
		StorageData frustumGrid, frustumGridStaging;

		void cleanup(VkDevice device) {
			lights.cleanup(device);
			lightsStaging.cleanup(device);
			frustumGrid.cleanup(device);
			frustumGridStaging.cleanup(device);
		}
	} storageData;

	// uniform buffer object for vertex shader
	struct UBO_vsScene {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
		glm::vec4 cameraPos;
	};

	struct UBO_csParams {
		glm::mat4 inverseProj;
		glm::vec2 screenDimensions;
	};

	// uniform buffer host data
	struct {
		UBO_vsScene vsScene;
		UBO_csParams csParams;
	} ubos;

	// light information
	struct LightStruct {
		glm::vec4 beginPos; // beginPos.w = intensity
		glm::vec4 endPos; // endPos.w = radius
		glm::vec4 color; // color.w = t
	};

	// storage buffer object to store lights
	#define MAX_NUM_LIGHT 100
	struct SBO_lights {
		LightStruct lights[MAX_NUM_LIGHT];
		int numLights;
	};

	// frustum definition
	struct Frustum {
		// for each plane, use a vec4 to represent
		// xyz is normal, w is distance
		glm::vec4 planes[4];
	};

	#define NUM_FRUSTRUMS 2000
	struct SBO_frustums {
		Frustum frustums[NUM_FRUSTRUMS]; // 800*600 -> 50*40 
		int numFrustums;
	};
	
	// storage buffer host data
	struct {
		SBO_lights lights; 
		SBO_frustums frustums;
	} sbos;

	// Textures
	struct Texture {
		VkImage image;
		VkImageView imageView;
		VkDeviceMemory imageMemory;
		VkSampler sampler;
	};
	std::array<Texture, 2> textures;

	// Frame buffer attachment struct
	struct FrameBufferAttachment {
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
	};
	FrameBufferAttachment depth;

	// shader modules
	std::vector<VDeleter<VkShaderModule>> shaderModules;


	/************************************************************/
	//					Function Declaration
	/************************************************************/
	void initWindow();

	void initVulkan();

	void mainLoop();

	void updateUniformBuffer();

	void drawFrame();

	// Create Vulkan instance
	void createInstance();

	void setupDebugCallback();

	void createSurface();

	void pickPhysicalDevice();

	void createLogicalDevice();

	void createSwapChain();

	void createImageViews();

	void createGraphicsPipeline();

	void createComputePipeline();

	void createFramebuffers();

	void createCommandPool();

	void createCommandBuffers();

	void createComputeCommandBuffer();

	void createRenderPass();

	void createVertexBuffer(std::vector<Vertex> & verticesData, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void createIndexBuffer(std::vector<uint32_t> &indicesData, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void createSemaphores();

	// check device suitabiliy
	bool isDeviceSuitable(VkPhysicalDevice device);

	// find queue families
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	// get extension that are required by instance
	std::vector<const char*> getRequiredExtensions();

	// check validation layer support
	bool checkValidationLayerSupport();

	// helper func: print extension names
	void printInstanceExtensions();

	// check device extension support
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	// query swap chain support, assign SwapChainSupportDetails struct
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	// swap chain choose format
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	// swap chain, choose present mode
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);

	// swap chain, choose swap extent
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	// read shader file from compiled binary file
	static std::vector<char> readFile(const std::string& filename);

	void createShaderModule(const std::vector<char> & code, VDeleter<VkShaderModule> & shaderModule);

	VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage, int shaderModuleIndex);

	// find memory type
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	// abstracting buffer creation
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	// copy buffer from srcBuffer to dstBuffer
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	// descriptor set layout
	void createDescriptorSetLayout();

	void createLightInfos();

	void createUniformBuffer();

	void createStorageBuffer();

	void initStorageBuffer();

	void createDescriptorPool();

	void createDescriptorSet();

	void createTextureImage(const std::string& texFilename, VkImage & texImage, VkDeviceMemory & texImageMemory);

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage & image, VkDeviceMemory & imageMemory);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	void copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);

	void createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView & imageView);

	void createTextureImageView(VkImage & textureImage, VkImageView & textureImageView);

	void createTextureSampler(VkSampler & textureSampler);

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	VkFormat findDepthFormat();

	bool hasStencilComponent(VkFormat format);

	void createDepthResources();

	void loadModel(std::vector<Vertex> & vertices, std::vector<uint32_t> & indices, const std::string & modelFilename, float scale = 1.0f);

	// load axis info
	void loadAxisInfo();

	// assign vertex and index buffer for mesh
	void createMeshBuffer(Mesh& mesh);

	// load texture quad info
	void loadTextureQuad();

	void prepareTextures();

	VkCommandBuffer beginSingleTimeCommands();

	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
};

// callbacks
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);
void cursorPosCallback(GLFWwindow* window, double xPos, double yPos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
