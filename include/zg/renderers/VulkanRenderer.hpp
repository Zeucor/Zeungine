#pragma once
// #ifdef USE_VULKAN
#include <tuple>
#include "../common.hpp"
#include "../enums/ELayoutBindingType.hpp"
#include "../interfaces/IPlatformWindow.hpp"
#include "../interfaces/IRenderer.hpp"
#ifdef ANDROID
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(IOS)
#define VK_USE_PLATFORM_IOS_MVK
#elif defined(__APPLE__)
#define VK_USE_PLATFORM_MACOS_MVK
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__linux__)
// #if defined(USE_X11)
// #define VK_USE_PLATFORM_XLIB_KHR
// #endif
// #if defined(USE_XCB) || defined(USE_X11)
#define VK_USE_PLATFORM_XCB_KHR
// #endif
// #if defined(USE_WAYLAND)
#define VK_USE_PLATFORM_WAYLAND_KHR
// #endif
#endif
#include <shaderc/shaderc.hpp>
#include <vulkan/vulkan.h>
#include <zg/SharedLibrary.hpp>
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
		VkBuffer vertexBuffer = 0;
		VkDeviceMemory vertexBufferMemory = 0;
		void* vertexData = 0;
		uint32_t vertexBufferSize;
		VkBuffer indiceBuffer = 0;
		VkDeviceMemory indiceBufferMemory = 0;
		void* indiceData = 0;
		uint32_t indiceBufferSize;
		VkDescriptorPool descriptorPool;
		VkDescriptorSet descriptorSet;
		std::unordered_map<std::string, std::pair<VkBuffer, VkDeviceMemory>> ssboBuffers;
		std::vector<VkBuffer> uniformBuffers;
		std::vector<VkDeviceMemory> uniformBuffersMemory;
		std::vector<void*> uniformBuffersMapped;
		std::vector<std::pair<std::tuple<ELayoutBindingType, uint32_t>, VkDescriptorBufferInfo>> bufferInfos;
		std::unordered_map<std::string, int32_t> uniformLocationTable;
	};
	struct VulkanShaderImpl
	{
		/**
		 * @brief
		 * tuple: 0: layoutBindingType
		 *        1: bufferSize
		 *        2: bindingName
		 *        3: bindingIndex
		 *        4: isArray
		 */
		std::vector<
			std::pair<std::tuple<ELayoutBindingType, uint32_t, std::string, uint32_t, bool>, VkDescriptorSetLayoutBinding>>
			uboLayoutBindings;
		VkDescriptorSetLayout descriptorSetLayout;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
		shaders::ShaderMap shaders;
		std::unordered_map<std::string, std::tuple<uint32_t, uint32_t, uint32_t>> ssboBindings;
		std::unordered_map<std::string, uint32_t> textureArrayBindings;
		std::unordered_map<std::string, uint32_t> textureBindings;
		std::unordered_map<std::string, uint32_t> uboStringBindings;
	};
	static inline std::unordered_map<size_t, VkRenderPass> renderPassMap = {};
	constexpr int MAX_FRAMES_IN_FLIGHT = 1;
	#define GET_PROC_ADDR_MEMBER(NAME) PFN_vkVoidFunction(*NAME)(VkInstance instance, const char *pName)
	#define VK_GLOBAL(N, PFN, NAME) N = (PFN)VulkanRenderer::getProcAddr(0, NAME)
	#define VK_INSTANCE(N, PFN, NAME) N = (PFN)VulkanRenderer::getProcAddr(instance, NAME)
	using GetProcAddrFunc = PFN_vkVoidFunction(*)(VkInstance, const char *);
	struct VulkanRenderer : IRenderer
	{
		static bool fallbackToSwiftshader;
		static bool attempedCoreVulkan;
		static SharedLibrary vulkanLibrarySS;
		static SharedLibrary vulkanLibraryCore;
		GET_PROC_ADDR_MEMBER(getProcAddr);
		PFN_vkCreateInstance _vkCreateInstance;
		PFN_vkGetInstanceProcAddr _vkGetInstanceProcAddr;
		PFN_vkEnumerateInstanceLayerProperties _vkEnumerateInstanceLayerProperties;
		PFN_vkCreateDebugUtilsMessengerEXT _vkCreateDebugUtilsMessengerEXT;
#ifdef __linux__
		PFN_vkCreateXcbSurfaceKHR _vkCreateXcbSurfaceKHR;
		PFN_vkCreateWaylandSurfaceKHR _vkCreateWaylandSurfaceKHR;
#elif defined(__APPLE__)
		PFN_vkCreateMacOSSurfaceMVK _vkCreateMacOSSurfaceMVK;
		PFN_vkCreateHeadlessSurfaceEXT _vkCreateHeadlessSurfaceEXT;
#elif defined(_WIN32)
		PFN_vkCreateWin32SurfaceKHR _vkCreateWin32SurfaceKHR;
#endif
		PFN_vkEnumeratePhysicalDevices _vkEnumeratePhysicalDevices;
		PFN_vkGetPhysicalDeviceProperties _vkGetPhysicalDeviceProperties;
		PFN_vkGetPhysicalDeviceFeatures _vkGetPhysicalDeviceFeatures;
		PFN_vkGetPhysicalDeviceQueueFamilyProperties _vkGetPhysicalDeviceQueueFamilyProperties;
		PFN_vkGetPhysicalDeviceSurfaceSupportKHR _vkGetPhysicalDeviceSurfaceSupportKHR;
		PFN_vkEnumerateDeviceExtensionProperties _vkEnumerateDeviceExtensionProperties;
		PFN_vkGetPhysicalDeviceFeatures2 _vkGetPhysicalDeviceFeatures2;
		PFN_vkCreateDevice _vkCreateDevice;
		PFN_vkGetDeviceQueue _vkGetDeviceQueue;
		PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR _vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
		PFN_vkCreateSwapchainKHR _vkCreateSwapchainKHR;
		PFN_vkGetSwapchainImagesKHR _vkGetSwapchainImagesKHR;
		PFN_vkGetPhysicalDeviceSurfaceFormatsKHR _vkGetPhysicalDeviceSurfaceFormatsKHR;
		PFN_vkGetPhysicalDeviceSurfacePresentModesKHR _vkGetPhysicalDeviceSurfacePresentModesKHR;
		PFN_vkCreateImageView _vkCreateImageView;
		PFN_vkCreateRenderPass _vkCreateRenderPass;
		PFN_vkCreateFramebuffer _vkCreateFramebuffer;
		PFN_vkCreateCommandPool _vkCreateCommandPool;
		PFN_vkAllocateCommandBuffers _vkAllocateCommandBuffers;
		PFN_vkCreateSemaphore _vkCreateSemaphore;
		PFN_vkGetBufferMemoryRequirements _vkGetBufferMemoryRequirements;
		PFN_vkAllocateMemory _vkAllocateMemory;
		PFN_vkBindBufferMemory _vkBindBufferMemory;
		PFN_vkMapMemory _vkMapMemory;
		PFN_vkFreeMemory _vkFreeMemory;
		PFN_vkDestroyBuffer _vkDestroyBuffer;
		PFN_vkWaitForFences _vkWaitForFences;
		PFN_vkCreateFence _vkCreateFence;
		PFN_vkAcquireNextImageKHR _vkAcquireNextImageKHR;
		PFN_vkResetFences _vkResetFences;
		PFN_vkResetCommandBuffer _vkResetCommandBuffer;
		PFN_vkBeginCommandBuffer _vkBeginCommandBuffer;
		PFN_vkCmdEndRenderPass _vkCmdEndRenderPass;
		PFN_vkEndCommandBuffer _vkEndCommandBuffer;
		PFN_vkQueueSubmit _vkQueueSubmit;
		PFN_vkQueueWaitIdle _vkQueueWaitIdle;
		PFN_vkQueuePresentKHR _vkQueuePresentKHR;
		PFN_vkCmdBindPipeline _vkCmdBindPipeline;
		PFN_vkCmdSetViewport _vkCmdSetViewport;
		PFN_vkCmdSetScissor _vkCmdSetScissor;
		PFN_vkUnmapMemory _vkUnmapMemory;
		PFN_vkUpdateDescriptorSets _vkUpdateDescriptorSets;
		PFN_vkCreateShaderModule _vkCreateShaderModule;
		PFN_vkCreateDescriptorSetLayout _vkCreateDescriptorSetLayout;
		PFN_vkCreatePipelineLayout _vkCreatePipelineLayout;
		PFN_vkCreateGraphicsPipelines _vkCreateGraphicsPipelines;
		PFN_vkCmdBeginRenderPass _vkCmdBeginRenderPass;
		PFN_vkCreateImage _vkCreateImage;
		PFN_vkGetImageMemoryRequirements _vkGetImageMemoryRequirements;
		PFN_vkBindImageMemory _vkBindImageMemory;
		PFN_vkGetPhysicalDeviceFormatProperties _vkGetPhysicalDeviceFormatProperties;
		PFN_vkCmdCopyBufferToImage _vkCmdCopyBufferToImage;
		PFN_vkCmdPipelineBarrier _vkCmdPipelineBarrier;
		PFN_vkCreateSampler _vkCreateSampler;
		PFN_vkCmdBindVertexBuffers _vkCmdBindVertexBuffers;
		PFN_vkCmdBindIndexBuffer _vkCmdBindIndexBuffer;
		PFN_vkCmdBindDescriptorSets _vkCmdBindDescriptorSets;
		PFN_vkCmdDrawIndexed _vkCmdDrawIndexed;
		PFN_vkCreateDescriptorPool _vkCreateDescriptorPool;
		PFN_vkAllocateDescriptorSets _vkAllocateDescriptorSets;
		PFN_vkFreeCommandBuffers _vkFreeCommandBuffers;
		PFN_vkGetPhysicalDeviceMemoryProperties _vkGetPhysicalDeviceMemoryProperties;
		PFN_vkCreateBuffer _vkCreateBuffer;
		PFN_vkInvalidateMappedMemoryRanges _vkInvalidateMappedMemoryRanges;
		PFN_vkCmdCopyImageToBuffer _vkCmdCopyImageToBuffer;
		PFN_vkDestroyFence _vkDestroyFence;
		PFN_vkDestroyDescriptorPool _vkDestroyDescriptorPool;
		PFN_vkDestroySampler _vkDestroySampler;
		PFN_vkDestroyFramebuffer _vkDestroyFramebuffer;
		PFN_vkDestroyCommandPool _vkDestroyCommandPool;
		PFN_vkDestroyPipeline _vkDestroyPipeline;
		PFN_vkDestroyPipelineLayout _vkDestroyPipelineLayout;
		PFN_vkDestroyImageView _vkDestroyImageView;
		PFN_vkDestroySwapchainKHR _vkDestroySwapchainKHR;
		PFN_vkDestroyDescriptorSetLayout _vkDestroyDescriptorSetLayout;
		PFN_vkDestroySemaphore _vkDestroySemaphore;
		PFN_vkDestroyImage _vkDestroyImage;
		PFN_vkDestroyDevice _vkDestroyDevice;
		PFN_vkDestroyRenderPass _vkDestroyRenderPass;
		PFN_vkDeviceWaitIdle _vkDeviceWaitIdle;
		PFN_vkDestroyShaderModule _vkDestroyShaderModule;
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
		const VulkanFramebufferImpl* currentFramebufferImpl = 0;
		VkSubmitInfo submitInfo;
		VkPresentInfoKHR presentInfo;
		VkPipelineStageFlags waitStages[1];
		VkSemaphore signalSemaphores[1];
		VkSwapchainKHR swapChains[1];
		void* bitmap = 0;
		uint32_t swapBufferCount = 0;
		std::vector<std::function<void()>> destroyAtRenderPassEndOrDestroyVector;
		VulkanRenderer();
		~VulkanRenderer() override;
		static GetProcAddrFunc doGetProcAddr();
		void createContext(IPlatformWindow* platformWindowPointer) override;
		void createInstance();
		void setupGlobalPFNs();
		void setupPFNs();
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
		void destroyAtRenderPassEndOrDestroy(const std::function<void()> &function);
		void callDestroyAtRenderPassEndOrDestroy();
		void destroy() override;
		void destroySwapChain();
		void preBeginRenderPass() override;
		void beginRenderPass() override;
		void postRenderPass() override;
		void clearColor(glm::vec4 color) override;
		void clear() override;
		void viewport(glm::ivec4 vp) const override;
		void initShader(shaders::Shader& shader, const shaders::RuntimeConstants& constants,
										const std::vector<shaders::ShaderType>& shaderTypes) override;
		void setUniform(shaders::Shader& shader, vaos::VAO& vao, const std::string_view name, const void* pointer,
										uint32_t size, enums::EUniformType uniformType) override;
		void setBlock(shaders::Shader& shader, vaos::VAO& vao, const std::string_view name, const void* pointer,
									size_t size) override;
		int32_t getUniformLocation(shaders::Shader& shader, vaos::VAO& vao, const std::string_view& name);
		void deleteBuffer(uint32_t id) override;
		void bindShader(shaders::Shader& shader, Entity& entity) override;
		void unbindShader(shaders::Shader& shader) override;
		void addSSBO(shaders::Shader& shader, shaders::ShaderType shaderType, const std::string_view name,
								 uint32_t bindingIndex) override;
		void addUBO(shaders::Shader& shader, shaders::ShaderType shaderType, const std::string_view name,
								uint32_t bindingIndex, uint32_t bufferSize, uint32_t descriptorCount, bool isArray) override;
		void addTexture(shaders::Shader& shader, uint32_t bindingIndex, shaders::ShaderType shaderType,
										std::string_view textureName, uint32_t descriptorCount) override;
		void setSSBO(shaders::Shader& shader, vaos::VAO& vao, const std::string_view name, const void* pointer,
								 size_t size) override;
		void setTexture(shaders::Shader& shader, vaos::VAO& vao, const std::string_view name,
										const textures::Texture& texture, const int32_t unit) override;
		bool compileShader(shaders::Shader& shader, shaders::ShaderType shaderType,
											 shaders::ShaderPair& shaderPair) override;
		VkShaderModule createShaderModule(const std::vector<uint32_t>& code);
		bool compileProgram(shaders::Shader& shader) override;
		bool checkCompileErrors(const shaderc::SpvCompilationResult& module, bool isShader, const char* shaderType);
		void deleteShader(shaders::Shader& shader) override;
		void destroyShader(shaders::Shader& shader) override;
		void bindFramebuffer(const textures::Framebuffer& framebuffer) override;
		void unbindFramebuffer(const textures::Framebuffer& framebuffer) override;
		void initFramebuffer(textures::Framebuffer& framebuffer) override;
		void destroyFramebuffer(textures::Framebuffer& framebuffer) override;
		void bindTexture(const textures::Texture& texture) override;
		void unbindTexture(const textures::Texture& texture) override;
		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
										 VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
		VkFormat findDepthFormat(uint32_t format);
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
																 VkFormatFeatureFlags features);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void transitionImageLayout(VulkanTextureImpl& textureImpl, VkImage image, VkFormat format, VkImageLayout oldLayout,
															 VkImageLayout newLayout, VkImageAspectFlags aspectMask);
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
		void ensureEntity(shaders::Shader& shader, vaos::VAO& vao) override;
		void swapBuffers() override;
		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void ensureBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, void*& bufferData, uint32_t& bufferSize,
											uint32_t newBufferSize, VkBufferUsageFlagBits extraUsageFlags);
		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
											VkDeviceMemory& bufferMemory);
		void getCurrentImageToBitmap();
	};
	bool VKcheck(const char* fn, VkResult result);
} // namespace zg
// #endif