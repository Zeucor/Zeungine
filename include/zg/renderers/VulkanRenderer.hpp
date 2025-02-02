#pragma once
#ifdef USE_VULKAN
#include "../common.hpp"
#include "../enums/ELayoutBindingType.hpp"
#include "../interfaces/IPlatformWindow.hpp"
#include "../interfaces/IRenderer.hpp"
#ifdef ANDROID
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(IOS)
#define VK_USE_PLATFORM_IOS_MVK
#elif defined(MACOS)
#define VK_USE_PLATFORM_MACOS_MVK
#elif defined(WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(LINUX)
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#include <vulkan/vulkan.h>
namespace zg
{
	namespace textures
	{
		struct Framebuffer;
	}
	struct QueueFamilyIndices
	{
		int32_t graphicsFamily = -1;
		int32_t presentFamily = -1;
		bool isComplete() { return graphicsFamily > -1 && presentFamily > -1; };
	};
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
	constexpr int MAX_FRAMES_IN_FLIGHT = 1;
	struct VulkanRenderer : IRenderer
	{
		std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice device;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkSwapchainKHR swapChain;
		std::vector<VkImage> swapChainImages;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		std::vector<VkImageView> swapChainImageViews;
		std::vector<VkFramebuffer> swapChainFramebuffers;
		VkRenderPass renderPass;
		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;
		VkCommandBuffer* commandBuffer;
		std::shared_ptr<textures::Framebuffer> currentFramebuffer;
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		uint32_t currentFrame = 0;
		uint32_t imageIndex = -1;
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		VulkanRenderer();
		~VulkanRenderer() override;
		void createContext(IPlatformWindow* platformWindowPointer) override;
		void createInstance();
#ifndef NDEBUG
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		bool checkValidationLayersSupport();
		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
																					const VkAllocationCallbacks* pAllocator,
																					VkDebugUtilsMessengerEXT* pDebugMessenger);
		void setupDebugMessenger();
#endif
		void createSurface();
		void pickPhysicalDevice();
		uint32_t rateDeviceSuitability(VkPhysicalDevice device);
		bool isDeviceSuitable(VkPhysicalDevice device);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		void createLogicalDevice();
		void createSwapChain();
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR capabilities);
		void createImageViews();
		void createRenderPass();
		void createFramebuffers();
		void createCommandPool();
		void createCommandBuffers();
		void createSyncObjects();
		void createImageStagingBuffer();
		void init() override;
		void destroy() override;
		void preBeginRenderPass() override;
		void beginRenderPass() override;
		void postRenderPass() override;
		void clearColor(glm::vec4 color) override;
		void clear() override;
		void viewport(glm::ivec4 vp) const override;
		void initShader(shaders::Shader& shader, const shaders::RuntimeConstants& constants,
										const std::vector<shaders::ShaderType>& shaderTypes) override;
		void setUniform(shaders::Shader& shader, const std::string_view name, const void* pointer, uint32_t size,
										enums::EUniformType uniformType) override;
		void setBlock(shaders::Shader& shader, const std::string_view name, const void* pointer, size_t size) override;
		void deleteBuffer(uint32_t id) override;
		void bindShader(const shaders::Shader& shader) override;
		void unbindShader(const shaders::Shader& shader) override;
		void addSSBO(shaders::Shader& shader, const std::string_view name, uint32_t bindingIndex) override;
		void addUBO(shaders::Shader& shader, const std::string_view name, uint32_t bindingIndex) override;
		void setSSBO(shaders::Shader& shader, const std::string_view name, const void* pointer, size_t size) override;
		void setTexture(shaders::Shader& shader, const std::string_view name, const textures::Texture& texture,
										const int32_t unit) override;
		bool compileShader(shaders::Shader& shader, shaders::ShaderType shaderType,
											 shaders::ShaderPair& shaderPair) override;
		bool compileProgram(shaders::Shader& shader, const shaders::ShaderMap& shaderMap) override;
		bool checkCompileErrors(shaders::Shader& shader, const uint32_t& id, bool isShader, const char* shaderType);
		void deleteShader(shaders::Shader& shader) override;
		void destroyShader(shaders::Shader& shader) override;
		void bindFramebuffer(const textures::Framebuffer& framebuffer) const override;
		void unbindFramebuffer(const textures::Framebuffer& framebuffer) const override;
		void initFramebuffer(textures::Framebuffer& framebuffer) override;
		void destroyFramebuffer(textures::Framebuffer& framebuffer) override;
		void bindTexture(const textures::Texture& texture) override;
		void unbindTexture(const textures::Texture& texture) override;
		void preInitTexture(textures::Texture& texture) override;
		void midInitTexture(const textures::Texture& texture,
												const std::vector<images::ImageLoader::ImagePair>& images) override;
		void postInitTexture(const textures::Texture& texture) override;
		void destroyTexture(textures::Texture& texture) override;
		void updateIndicesVAO(const vaos::VAO& vao, const std::vector<uint32_t>& indices) override;
		void updateElementsVAO(const vaos::VAO& vao, const std::string_view constant, uint8_t* elementsAsChar) override;
		void drawVAO(const vaos::VAO& vao) override;
		void generateVAO(vaos::VAO& vao) override;
		void destroyVAO(vaos::VAO& vao) override;
		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void ensureBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, void*& bufferData, uint32_t& bufferSize,
											uint32_t newBufferSize, VkBufferUsageFlagBits extraUsageFlags);
		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
											VkDeviceMemory& bufferMemory);
	};
	bool VKcheck(const char* fn, VkResult result);
	struct VulkanTextureImpl
	{
		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
		VkSampler textureSampler;
		VkImageLayout layout;
	};
	struct VulkanFramebufferImpl
	{
		VkRenderPass renderPass;
		VkFramebuffer framebuffer;
		uint32_t attachmentsSize;
		uint32_t width;
		uint32_t height;
	};
	struct VulkanVAOImpl
	{
		VkBuffer vertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
		void* vertexData = 0;
		uint32_t vertexBufferSize;
		VkBuffer indiceBuffer = VK_NULL_HANDLE;
		VkDeviceMemory indiceBufferMemory = VK_NULL_HANDLE;
		void* indiceData = 0;
		uint32_t indiceBufferSize;
		std::unordered_map<uint32_t, VkDescriptorPool> shaderDescriptorPool;
		std::unordered_map<uint32_t, VkDescriptorSet> shaderDescriptorSet;
		std::unordered_map<uint32_t, std::vector<VkBuffer>> shaderUniformBuffers;
		std::unordered_map<uint32_t, std::vector<VkDeviceMemory>> shaderUniformBuffersMemory;
		std::unordered_map<uint32_t, std::vector<void*>> shaderUniformBuffersMapped;
		std::unordered_map<uint32_t,
											 std::vector<std::pair<std::tuple<ELayoutBindingType, uint32_t>, VkDescriptorBufferInfo>>>
			shaderBufferInfos;
		std::unordered_map<uint32_t, std::unordered_map<std::string, int32_t>> shaderUniformLocationTable;
	};
} // namespace zg
#endif
