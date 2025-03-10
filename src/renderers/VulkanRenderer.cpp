// #ifdef USE_VULKAN
#include <zg/Logger.hpp>
#include <zg/Window.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/renderers/VulkanRenderer.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/textures/TextureFactory.hpp>
#include <zg/vaos/VAOFactory.hpp>
#ifdef _WIN32
#include <zg/windows/WIN32Window.hpp>
#endif
#ifdef __linux__
#include <zg/windows/WaylandWindow.hpp>
#include <zg/windows/XCBWindow.hpp>
#endif
#ifdef MACOS
#include <zg/windows/MacOSWindow.hpp>
#endif
#include <zg/crypto/vector.hpp>
using namespace zg;
bool VulkanRenderer::fallbackToSwiftshader = false;
bool VulkanRenderer::attempedCoreVulkan = false;
#ifdef MACOS
static std::string libPrefix("lib");
static std::string libSuffix(".dylib");
#elif defined(__linux__)
static std::string libPrefix("lib");
static std::string libSuffix(".so");
#elif defined(_WIN32)
static std::string libPrefix("");
static std::string libSuffix(".dll");
#endif
static std::filesystem::path vulkanLibrarySSName(libPrefix + "vk_swiftshader" + libSuffix);
SharedLibrary zg::VulkanRenderer::vulkanLibrarySS((ZG_LIB_INSTALL_PREFIX / vulkanLibrarySSName).string());
#ifdef MACOS
SharedLibrary zg::VulkanRenderer::vulkanLibraryCore(libPrefix + "vulkan.1" + libSuffix);
#elif defined(__linux__)
static std::string vulkanCorePath(libPrefix + "vulkan" + libSuffix + ".1");
SharedLibrary zg::VulkanRenderer::vulkanLibraryCore("/usr/lib/x86_64-linux-gnu/" + vulkanCorePath, vulkanCorePath);
#endif
static std::unordered_map<shaders::ShaderType, shaderc_shader_kind> stageEShaderc = {
	{shaders::ShaderType::Vertex, shaderc_vertex_shader},
	{shaders::ShaderType::Fragment, shaderc_fragment_shader},
	{shaders::ShaderType::Compute, shaderc_compute_shader}};
static std::unordered_map<shaders::ShaderType, VkShaderStageFlagBits> stageStageFlag = {
	{shaders::ShaderType::Vertex, VK_SHADER_STAGE_VERTEX_BIT},
	{shaders::ShaderType::Fragment, VK_SHADER_STAGE_FRAGMENT_BIT},
	{shaders::ShaderType::Compute, VK_SHADER_STAGE_COMPUTE_BIT},
};
static std::unordered_map<textures::Texture::Format, VkFormat> textureFormat_Format = {
	{textures::Texture::Format::RGBA8, VK_FORMAT_R8G8B8A8_SRGB},
	{textures::Texture::Format::Depth, VK_FORMAT_D32_SFLOAT},
	{textures::Texture::Format::DepthStencil, VK_FORMAT_D32_SFLOAT_S8_UINT}};
static std::unordered_map<textures::Texture::Format, VkImageLayout> textureFormat_imageLayout = {
	{textures::Texture::Format::RGBA8, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
	{textures::Texture::Format::Depth, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL},
	{textures::Texture::Format::DepthStencil, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}};
static std::unordered_map<textures::Texture::Format, VkImageLayout> textureFormat_descriptor_imageLayout = {
	{textures::Texture::Format::RGBA8, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
	{textures::Texture::Format::Depth, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL},
	{textures::Texture::Format::DepthStencil, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL}};
static std::unordered_map<textures::Texture::Format, VkImageAspectFlags> textureFormat_imageAspect = {
	{textures::Texture::Format::RGBA8, VK_IMAGE_ASPECT_COLOR_BIT},
	{textures::Texture::Format::Depth, VK_IMAGE_ASPECT_DEPTH_BIT},
	{textures::Texture::Format::DepthStencil, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT}};
// static std::unordered_map<EFramebufferAttachmentType, VkFormat> attachmentType_Format = {
// 	{EFramebufferAttachmentType::Color, VK_FORMAT_R8G8B8A8_SRGB},
// 	{EFramebufferAttachmentType::Depth, VK_FORMAT_D32_SFLOAT},
// 	{EFramebufferAttachmentType::DepthStencil, VK_FORMAT_D32_SFLOAT_S8_UINT}
// };
// static std::unordered_map<EFramebufferAttachmentType, VkImageLayout> attachmentType_finalLayout = {
// 	{EFramebufferAttachmentType::Color, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
// 	{EFramebufferAttachmentType::Depth, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
// 	{EFramebufferAttachmentType::DepthStencil, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}
// };
// static std::unordered_map<EFramebufferAttachmentType, VkImageLayout> attachmentType_attachment_finalLayout = {
// 	{EFramebufferAttachmentType::Color, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
// 	{EFramebufferAttachmentType::Depth, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL},
// 	{EFramebufferAttachmentType::DepthStencil, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL}
// };
VulkanRenderer::VulkanRenderer() {}
VulkanRenderer::~VulkanRenderer() {}
GetProcAddrFunc VulkanRenderer::doGetProcAddr()
{
	if (fallbackToSwiftshader)
	{
		try
		{
			return vulkanLibrarySS.getProc<GetProcAddrFunc>("vk_icdGetInstanceProcAddr");
		}
		catch (...)
		{
		}
	}
	if (!attempedCoreVulkan)
	{
		attempedCoreVulkan = true;
		try
		{
#if defined(__linux__) || defined(MACOS)
			return vulkanLibraryCore.getProc<GetProcAddrFunc>("vkGetInstanceProcAddr");
#elif defined(_WIN32)
			return vkGetInstanceProcAddr;
#endif
		}
		catch (...)
		{
		}
	}
	return (GetProcAddrFunc)0;
};
void VulkanRenderer::createContext(IPlatformWindow* platformWindowPointer)
{
	this->renderer = RENDERER_VULKAN;
	this->platformWindowPointer = platformWindowPointer;
	createInstance();
	setupPFNs();
#ifndef NDEBUG
	setupDebugMessenger();
#endif
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createFramebuffers();
	createCommandPool();
	createCommandBuffers();
	createSyncObjects();
	if (fallbackToSwiftshader)
		createImageStagingBuffer();
}
void VulkanRenderer::createInstance()
{
	if (!attempedCoreVulkan)
	{
		getProcAddr = doGetProcAddr();
		setupGlobalPFNs();
	}
	else if (!fallbackToSwiftshader)
	{
		getProcAddr = doGetProcAddr();
		setupGlobalPFNs();
	}
	else
	{
		throw std::runtime_error("No Vulkan or SwiftShader support found!");
	}
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = platformWindowPointer->renderWindowPointer->title;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Zeungine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	if (fallbackToSwiftshader)
		appInfo.apiVersion = VK_API_VERSION_1_0;
	else
		appInfo.apiVersion = VK_API_VERSION_1_2;
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	//
	std::vector<const char*> extensions;
	extensions.push_back("VK_KHR_surface");
	auto& windowType = platformWindowPointer->windowType;
	switch (windowType)
	{
	case WINDOW_TYPE_XCB:
		{
			extensions.push_back("VK_KHR_xcb_surface");
			break;
		}
	case WINDOW_TYPE_X11:
		{
			extensions.push_back("VK_KHR_xlib_surface");
			break;
		}
	case WINDOW_TYPE_WAYLAND:
		{
			extensions.push_back("VK_KHR_wayland_surface");
			break;
		}
	case WINDOW_TYPE_WIN32:
		{
			extensions.push_back("VK_KHR_win32_surface");
			break;
		}
	case WINDOW_TYPE_ANDROID:
		{
			extensions.push_back("VK_KHR_android_surface");
			break;
		}
	case WINDOW_TYPE_MACOS:
		{
			extensions.push_back("VK_MVK_macos_surface");
			break;
		}
	case WINDOW_TYPE_IOS:
		{
			extensions.push_back("VK_MVK_ios_surface");
			break;
		}
	}
#if defined(MACOS)
	if (!fallbackToSwiftshader)
	{
		extensions.push_back("VK_KHR_portability_enumeration");
		createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
	}
#endif
	if (fallbackToSwiftshader)
		extensions.push_back("VK_EXT_headless_surface");
#ifndef NDEBUG
	extensions.push_back("VK_EXT_debug_utils");
#endif
	createInfo.enabledExtensionCount = extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();
	//
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	std::vector<const char*> layers;
#if !defined(NDEBUG)
	if (checkValidationLayersSupport())
	{
		layers.push_back("VK_LAYER_KHRONOS_validation");
		createInfo.enabledLayerCount = layers.size();
		createInfo.ppEnabledLayerNames = layers.data();
		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		std::cout << "Validation layers requested, but not available" << std::endl;
		createInfo.enabledLayerCount = 0;
	}
#endif
	auto createdInstance = VKcheck("vkCreateInstance", _vkCreateInstance(&createInfo, 0, &instance));
	if (!createdInstance)
	{
		createInstance();
	}
}
void VulkanRenderer::setupGlobalPFNs()
{
	VK_GLOBAL(_vkCreateInstance, PFN_vkCreateInstance, "vkCreateInstance");
	VK_GLOBAL(_vkGetInstanceProcAddr, PFN_vkGetInstanceProcAddr, "vkGetInstanceProcAddr");
	VK_GLOBAL(_vkEnumerateInstanceLayerProperties, PFN_vkEnumerateInstanceLayerProperties,
						"vkEnumerateInstanceLayerProperties");
}
void VulkanRenderer::setupPFNs()
{
	VK_INSTANCE(_vkCreateDebugUtilsMessengerEXT, PFN_vkCreateDebugUtilsMessengerEXT, "vkCreateDebugUtilsMessengerEXT");
#ifdef __linux__
	VK_INSTANCE(_vkCreateXcbSurfaceKHR, PFN_vkCreateXcbSurfaceKHR, "vkCreateXcbSurfaceKHR");
	VK_INSTANCE(_vkCreateWaylandSurfaceKHR, PFN_vkCreateWaylandSurfaceKHR, "vkCreateWaylandSurfaceKHR");
#elif defined(MACOS)
	VK_INSTANCE(_vkCreateMacOSSurfaceMVK, PFN_vkCreateMacOSSurfaceMVK, "vkCreateMacOSSurfaceMVK");
	VK_INSTANCE(_vkCreateHeadlessSurfaceEXT, PFN_vkCreateHeadlessSurfaceEXT, "vkCreateHeadlessSurfaceEXT");
#elif defined(_WIN32)
	VK_INSTANCE(_vkCreateWin32SurfaceKHR, PFN_vkCreateWin32SurfaceKHR, "vkCreateWin32SurfaceKHR");
#endif
	VK_INSTANCE(_vkEnumeratePhysicalDevices, PFN_vkEnumeratePhysicalDevices, "vkEnumeratePhysicalDevices");
	VK_INSTANCE(_vkGetPhysicalDeviceProperties, PFN_vkGetPhysicalDeviceProperties, "vkGetPhysicalDeviceProperties");
	VK_INSTANCE(_vkGetPhysicalDeviceFeatures, PFN_vkGetPhysicalDeviceFeatures, "vkGetPhysicalDeviceFeatures");
	VK_INSTANCE(_vkGetPhysicalDeviceQueueFamilyProperties, PFN_vkGetPhysicalDeviceQueueFamilyProperties,
							"vkGetPhysicalDeviceQueueFamilyProperties");
	VK_INSTANCE(_vkGetPhysicalDeviceSurfaceSupportKHR, PFN_vkGetPhysicalDeviceSurfaceSupportKHR,
							"vkGetPhysicalDeviceSurfaceSupportKHR");
	VK_INSTANCE(_vkEnumerateDeviceExtensionProperties, PFN_vkEnumerateDeviceExtensionProperties,
							"vkEnumerateDeviceExtensionProperties");
	VK_INSTANCE(_vkGetPhysicalDeviceFeatures2, PFN_vkGetPhysicalDeviceFeatures2, "vkGetPhysicalDeviceFeatures2");
	VK_INSTANCE(_vkCreateDevice, PFN_vkCreateDevice, "vkCreateDevice");
	VK_INSTANCE(_vkGetDeviceQueue, PFN_vkGetDeviceQueue, "vkGetDeviceQueue");
	VK_INSTANCE(_vkGetPhysicalDeviceSurfaceCapabilitiesKHR, PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
							"vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
	VK_INSTANCE(_vkCreateSwapchainKHR, PFN_vkCreateSwapchainKHR, "vkCreateSwapchainKHR");
	VK_INSTANCE(_vkGetSwapchainImagesKHR, PFN_vkGetSwapchainImagesKHR, "vkGetSwapchainImagesKHR");
	VK_INSTANCE(_vkGetPhysicalDeviceSurfaceFormatsKHR, PFN_vkGetPhysicalDeviceSurfaceFormatsKHR,
							"vkGetPhysicalDeviceSurfaceFormatsKHR");
	VK_INSTANCE(_vkGetPhysicalDeviceSurfacePresentModesKHR, PFN_vkGetPhysicalDeviceSurfacePresentModesKHR,
							"vkGetPhysicalDeviceSurfacePresentModesKHR");
	VK_INSTANCE(_vkCreateImageView, PFN_vkCreateImageView, "vkCreateImageView");
	VK_INSTANCE(_vkCreateRenderPass, PFN_vkCreateRenderPass, "vkCreateRenderPass");
	VK_INSTANCE(_vkCreateFramebuffer, PFN_vkCreateFramebuffer, "vkCreateFramebuffer");
	VK_INSTANCE(_vkCreateCommandPool, PFN_vkCreateCommandPool, "vkCreateCommandPool");
	VK_INSTANCE(_vkAllocateCommandBuffers, PFN_vkAllocateCommandBuffers, "vkAllocateCommandBuffers");
	VK_INSTANCE(_vkCreateSemaphore, PFN_vkCreateSemaphore, "vkCreateSemaphore");
	VK_INSTANCE(_vkGetBufferMemoryRequirements, PFN_vkGetBufferMemoryRequirements, "vkGetBufferMemoryRequirements");
	VK_INSTANCE(_vkAllocateMemory, PFN_vkAllocateMemory, "vkAllocateMemory");
	VK_INSTANCE(_vkBindBufferMemory, PFN_vkBindBufferMemory, "vkBindBufferMemory");
	VK_INSTANCE(_vkMapMemory, PFN_vkMapMemory, "vkMapMemory");
	VK_INSTANCE(_vkFreeMemory, PFN_vkFreeMemory, "vkFreeMemory");
	VK_INSTANCE(_vkDestroyBuffer, PFN_vkDestroyBuffer, "vkDestroyBuffer");
	VK_INSTANCE(_vkWaitForFences, PFN_vkWaitForFences, "vkWaitForFences");
	VK_INSTANCE(_vkCreateFence, PFN_vkCreateFence, "vkCreateFence");
	VK_INSTANCE(_vkAcquireNextImageKHR, PFN_vkAcquireNextImageKHR, "vkAcquireNextImageKHR");
	VK_INSTANCE(_vkResetFences, PFN_vkResetFences, "vkResetFences");
	VK_INSTANCE(_vkResetCommandBuffer, PFN_vkResetCommandBuffer, "vkResetCommandBuffer");
	VK_INSTANCE(_vkBeginCommandBuffer, PFN_vkBeginCommandBuffer, "vkBeginCommandBuffer");
	VK_INSTANCE(_vkCmdEndRenderPass, PFN_vkCmdEndRenderPass, "vkCmdEndRenderPass");
	VK_INSTANCE(_vkEndCommandBuffer, PFN_vkEndCommandBuffer, "vkEndCommandBuffer");
	VK_INSTANCE(_vkQueueSubmit, PFN_vkQueueSubmit, "vkQueueSubmit");
	VK_INSTANCE(_vkQueueWaitIdle, PFN_vkQueueWaitIdle, "vkQueueWaitIdle");
	VK_INSTANCE(_vkQueuePresentKHR, PFN_vkQueuePresentKHR, "vkQueuePresentKHR");
	VK_INSTANCE(_vkCmdBindPipeline, PFN_vkCmdBindPipeline, "vkCmdBindPipeline");
	VK_INSTANCE(_vkCmdSetViewport, PFN_vkCmdSetViewport, "vkCmdSetViewport");
	VK_INSTANCE(_vkCmdSetScissor, PFN_vkCmdSetScissor, "vkCmdSetScissor");
	VK_INSTANCE(_vkUnmapMemory, PFN_vkUnmapMemory, "vkUnmapMemory");
	VK_INSTANCE(_vkUpdateDescriptorSets, PFN_vkUpdateDescriptorSets, "vkUpdateDescriptorSets");
	VK_INSTANCE(_vkCreateShaderModule, PFN_vkCreateShaderModule, "vkCreateShaderModule");
	VK_INSTANCE(_vkCreateDescriptorSetLayout, PFN_vkCreateDescriptorSetLayout, "vkCreateDescriptorSetLayout");
	VK_INSTANCE(_vkCreatePipelineLayout, PFN_vkCreatePipelineLayout, "vkCreatePipelineLayout");
	VK_INSTANCE(_vkCreateGraphicsPipelines, PFN_vkCreateGraphicsPipelines, "vkCreateGraphicsPipelines");
	VK_INSTANCE(_vkCmdBeginRenderPass, PFN_vkCmdBeginRenderPass, "vkCmdBeginRenderPass");
	VK_INSTANCE(_vkCreateImage, PFN_vkCreateImage, "vkCreateImage");
	VK_INSTANCE(_vkGetImageMemoryRequirements, PFN_vkGetImageMemoryRequirements, "vkGetImageMemoryRequirements");
	VK_INSTANCE(_vkBindImageMemory, PFN_vkBindImageMemory, "vkBindImageMemory");
	VK_INSTANCE(_vkGetPhysicalDeviceFormatProperties, PFN_vkGetPhysicalDeviceFormatProperties,
							"vkGetPhysicalDeviceFormatProperties");
	VK_INSTANCE(_vkCmdCopyBufferToImage, PFN_vkCmdCopyBufferToImage, "vkCmdCopyBufferToImage");
	VK_INSTANCE(_vkCmdPipelineBarrier, PFN_vkCmdPipelineBarrier, "vkCmdPipelineBarrier");
	VK_INSTANCE(_vkCreateSampler, PFN_vkCreateSampler, "vkCreateSampler");
	VK_INSTANCE(_vkCmdBindVertexBuffers, PFN_vkCmdBindVertexBuffers, "vkCmdBindVertexBuffers");
	VK_INSTANCE(_vkCmdBindIndexBuffer, PFN_vkCmdBindIndexBuffer, "vkCmdBindIndexBuffer");
	VK_INSTANCE(_vkCmdBindDescriptorSets, PFN_vkCmdBindDescriptorSets, "vkCmdBindDescriptorSets");
	VK_INSTANCE(_vkCmdDrawIndexed, PFN_vkCmdDrawIndexed, "vkCmdDrawIndexed");
	VK_INSTANCE(_vkCreateDescriptorPool, PFN_vkCreateDescriptorPool, "vkCreateDescriptorPool");
	VK_INSTANCE(_vkAllocateDescriptorSets, PFN_vkAllocateDescriptorSets, "vkAllocateDescriptorSets");
	VK_INSTANCE(_vkFreeCommandBuffers, PFN_vkFreeCommandBuffers, "vkFreeCommandBuffers");
	VK_INSTANCE(_vkGetPhysicalDeviceMemoryProperties, PFN_vkGetPhysicalDeviceMemoryProperties,
							"vkGetPhysicalDeviceMemoryProperties");
	VK_INSTANCE(_vkCreateBuffer, PFN_vkCreateBuffer, "vkCreateBuffer");
	VK_INSTANCE(_vkInvalidateMappedMemoryRanges, PFN_vkInvalidateMappedMemoryRanges, "vkInvalidateMappedMemoryRanges");
	VK_INSTANCE(_vkCmdCopyImageToBuffer, PFN_vkCmdCopyImageToBuffer, "vkCmdCopyImageToBuffer");
	VK_INSTANCE(_vkDestroyFence, PFN_vkDestroyFence, "vkDestroyFence");
	VK_INSTANCE(_vkDestroyDescriptorPool, PFN_vkDestroyDescriptorPool, "vkDestroyDescriptorPool");
	VK_INSTANCE(_vkDestroySampler, PFN_vkDestroySampler, "vkDestroySampler");
	VK_INSTANCE(_vkDestroyFramebuffer, PFN_vkDestroyFramebuffer, "vkDestroyFramebuffer");
	VK_INSTANCE(_vkDestroyCommandPool, PFN_vkDestroyCommandPool, "vkDestroyCommandPool");
	VK_INSTANCE(_vkDestroyPipeline, PFN_vkDestroyPipeline, "vkDestroyPipeline");
	VK_INSTANCE(_vkDestroyPipelineLayout, PFN_vkDestroyPipelineLayout, "vkDestroyPipelineLayout");
	VK_INSTANCE(_vkDestroyImageView, PFN_vkDestroyImageView, "vkDestroyImageView");
	VK_INSTANCE(_vkDestroySwapchainKHR, PFN_vkDestroySwapchainKHR, "vkDestroySwapchainKHR");
	VK_INSTANCE(_vkDestroyDescriptorSetLayout, PFN_vkDestroyDescriptorSetLayout, "vkDestroyDescriptorSetLayout");
	VK_INSTANCE(_vkDestroySemaphore, PFN_vkDestroySemaphore, "vkDestroySemaphore");
	VK_INSTANCE(_vkDestroyImage, PFN_vkDestroyImage, "vkDestroyImage");
	VK_INSTANCE(_vkDestroyDevice, PFN_vkDestroyDevice, "vkDestroyDevice");
	VK_INSTANCE(_vkDestroyRenderPass, PFN_vkDestroyRenderPass, "vkDestroyRenderPass");
	VK_INSTANCE(_vkDeviceWaitIdle, PFN_vkDeviceWaitIdle, "vkDeviceWaitIdle");
	VK_INSTANCE(_vkDestroyShaderModule, PFN_vkDestroyShaderModule, "vkDestroyShaderModule");
}
#ifndef NDEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
																						 VkDebugUtilsMessageTypeFlagsEXT messageType,
																						 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		std::cout << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}
void VulkanRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
};
bool VulkanRenderer::checkValidationLayersSupport()
{
	uint32_t layerCount;
	_vkEnumerateInstanceLayerProperties(&layerCount, 0);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	_vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}
		if (!layerFound)
		{
			return false;
		}
	}
	return true;
};
VkResult VulkanRenderer::CreateDebugUtilsMessengerEXT(VkInstance instance,
																											const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
																											const VkAllocationCallbacks* pAllocator,
																											VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	if (_vkCreateDebugUtilsMessengerEXT != 0)
	{
		return _vkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}
void VulkanRenderer::setupDebugMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);
	if (!VKcheck("CreateDebugUtilsMessengerEXT", CreateDebugUtilsMessengerEXT(instance, &createInfo, 0, &debugMessenger)))
	{
		throw std::runtime_error("VulkanRenderer-setupDebugMessenger: failed to set up debug messenger!");
	}
};
#endif
void VulkanRenderer::createSurface()
{
	auto& windowType = platformWindowPointer->windowType;
#ifdef __linux__
	// if (windowType == WINDOW_TYPE_X11)
	// {
	// 	auto& x11Window = *dynamic_cast<X11Window*>(platformWindowPointer);
	// 	VkXcbSurfaceCreateInfoKHR surfaceCreateInfo{};
	// 	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	// 	surfaceCreateInfo.connection = x11Window.connection;
	// 	surfaceCreateInfo.window = x11Window.window;
	// 	if (!VKcheck("vkCreateXcbSurfaceKHR", _vkCreateXcbSurfaceKHR(instance, &surfaceCreateInfo, 0, &surface)))
	// 	{
	// 		throw std::runtime_error("VulkanRenderer-createSurface: failed to create XCB surface");
	// 	}
	// }
	/*else*/ if (windowType == WINDOW_TYPE_XCB)
	{
		auto& xcbWindow = *dynamic_cast<XCBWindow*>(platformWindowPointer);
		VkXcbSurfaceCreateInfoKHR surfaceCreateInfo{};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.connection = xcbWindow.connection;
		surfaceCreateInfo.window = xcbWindow.window;
		if (!VKcheck("vkCreateXcbSurfaceKHR", _vkCreateXcbSurfaceKHR(instance, &surfaceCreateInfo, 0, &surface)))
		{
			throw std::runtime_error("VulkanRenderer-createSurface: failed to create XCB surface");
		}
	}
	else if (windowType == WINDOW_TYPE_WAYLAND)
	{
		auto& waylandWindow = *dynamic_cast<WaylandWindow*>(platformWindowPointer);
		VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo{};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.display = waylandWindow.display;
		surfaceCreateInfo.surface = waylandWindow.surface;
		if (!VKcheck("vkCreateWaylandSurfaceKHR", _vkCreateWaylandSurfaceKHR(instance, &surfaceCreateInfo, 0, &surface)))
		{
			throw std::runtime_error("VulkanRenderer-createSurface: failed to create XCB surface");
		}
	}
#elif defined(ANDROID)
#elif defined(_WIN32)
	auto& win32Window = *dynamic_cast<WIN32Window*>(platformWindowPointer);
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = win32Window.hInstance;
	surfaceCreateInfo.hwnd = win32Window.hwnd;
	if (!VKcheck("vkCreateWin32SurfaceKHR", _vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, 0, &surface)))
	{
		throw std::runtime_error("VulkanRenderer-createSurface: failed to create Xlib surface");
	}
#elif defined(MACOS)
	if (fallbackToSwiftshader)
	{
		VkHeadlessSurfaceCreateInfoEXT surfaceCreateInfo{};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT;
		if (!VKcheck("vkCreateHeadlessSurfaceEXT", _vkCreateHeadlessSurfaceEXT(instance, &surfaceCreateInfo, 0, &surface)))
		{
			throw std::runtime_error("Failed to create Vulkan headless surface!");
		}
	}
	else
	{
		auto& macWindow = *dynamic_cast<MacOSWindow*>(platformWindowPointer);
		VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo{};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT;
		surfaceCreateInfo.pView = macWindow.nsView;
		if (!VKcheck("vkCreateMacOSSurfaceMVK", _vkCreateMacOSSurfaceMVK(instance, &surfaceCreateInfo, 0, &surface)))
		{
			throw std::runtime_error("Failed to create MacOS surface!");
		}
	}
#endif
}
void VulkanRenderer::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	if (!VKcheck("vkEnumeratePhysicalDevices", _vkEnumeratePhysicalDevices(instance, &deviceCount, 0)))
	{
		throw std::runtime_error("VulkanRenderer-vkEnumeratePhysicalDevices failed");
	}
	if (deviceCount == 0)
	{
		throw std::runtime_error("VulkanRenderer-getPhysicalDevice: failed to find GPUs with Vulkan support!");
	}
	std::vector<VkPhysicalDevice> devices;
	devices.resize(deviceCount);
	if (!VKcheck("vkEnumeratePhysicalDevices", _vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data())))
	{
		throw std::runtime_error("VulkanRenderer-vkEnumeratePhysicalDevices failed");
	}
	std::map<uint32_t, VkPhysicalDevice> physicalDeviceScores;
	for (auto& device : devices)
	{
		physicalDeviceScores[rateDeviceSuitability(device)] = device;
	}
	auto end = physicalDeviceScores.rend();
	auto begin = physicalDeviceScores.rbegin();
	uint32_t selectedDeviceScore;
	for (auto iter = begin; iter != end; ++iter)
	{
		auto device = iter->second;
		if (isDeviceSuitable(device))
		{
			physicalDevice = device;
			selectedDeviceScore = iter->first;
			break;
		}
		continue;
	}
	if (physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("VulkanRenderer-getPhysicalDevice: failed to find a suitable GPU!");
	}
	VkPhysicalDeviceProperties physicalDeviceProperties;
	_vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
	std::cout << "Selected Physical Device: '" << physicalDeviceProperties.deviceName
						<< "' with score of: " << selectedDeviceScore << std::endl;
	return;
}
uint32_t VulkanRenderer::rateDeviceSuitability(VkPhysicalDevice device)
{
	uint32_t score = 0;
	VkPhysicalDeviceProperties properties;
	_vkGetPhysicalDeviceProperties(device, &properties);
	VkPhysicalDeviceFeatures features;
	_vkGetPhysicalDeviceFeatures(device, &features);
	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		score += 1000;
	}
	else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
	{
		score += 500;
	}
	score += properties.limits.maxImageDimension2D;
	if (!features.geometryShader)
	{
		return 0;
	}
	auto indices = findQueueFamilies(device);
	if (indices.graphicsFamily != indices.presentFamily)
	{
		score += 1000;
	}
	std::cout << "Rated physical device [" << properties.deviceName << "] a score of: " << score << std::endl;
	return score;
}
bool VulkanRenderer::isDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = findQueueFamilies(device);
	return indices.isComplete();
}
QueueFamilyIndices VulkanRenderer::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0;
	_vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 0);
	std::vector<VkQueueFamilyProperties> queueFamilies;
	queueFamilies.resize(queueFamilyCount);
	_vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
	int32_t index = 0;
	for (auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = index;
		}
		VkBool32 presentSupport = false;
		if (!VKcheck("vkGetPhysicalDeviceSurfaceSupportKHR",
								 _vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &presentSupport)))
		{
			throw std::runtime_error("VulkanRenderer-vkGetPhysicalDeviceSurfaceSupportKHR failed");
		}
		if (presentSupport)
		{
			indices.presentFamily = index;
		}
		if (indices.isComplete())
		{
			break;
		}
		index++;
		continue;
	}
	return indices;
}
void VulkanRenderer::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::vector<int32_t> uniqueQueueFamilies({indices.graphicsFamily, indices.presentFamily});
	float queuePriority = 1;
	int32_t index = 0;
	for (auto& queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
		if (index == 0 && queueFamily == uniqueQueueFamilies[1])
		{
			break;
		}
		continue;
	}
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = queueCreateInfos.size();
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	std::vector<const char*> extensions;
	// check extensions
	uint32_t extensionCount = 0;
	// Query number of available extensions
	if (!VKcheck("vkEnumerateDeviceExtensionProperties",
							 _vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &extensionCount, 0)))
	{
		throw std::runtime_error("VulkanRenderer-vkEnumerateDeviceExtensionProperties failed");
	}
	std::vector<VkExtensionProperties> deviceExtensions(extensionCount);
	if (!VKcheck("vkEnumerateDeviceExtensionProperties",
							 _vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &extensionCount, deviceExtensions.data())))
	{
		throw std::runtime_error("VulkanRenderer-vkEnumerateDeviceExtensionProperties failed");
	}
	for (const auto& ext : deviceExtensions)
	{
		if (std::strcmp(ext.extensionName, "VK_KHR_portability_subset") == 0)
		{
			extensions.push_back("VK_KHR_portability_subset");
		}
	}
	extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	extensions.push_back("VK_KHR_maintenance1");
	extensions.push_back("VK_KHR_swapchain");
	// extensions[2] = VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME;
	// extensions[3] = VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME;
	// extensions[4] = VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME;
	createInfo.enabledExtensionCount = extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledLayerCount = 0;
	VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{};
	descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
	descriptorIndexingFeatures.pNext = 0;
	VkPhysicalDeviceFeatures2 deviceFeatures{};
	deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures.pNext = &descriptorIndexingFeatures;
	_vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures);
	deviceFeatures.features.depthClamp = VK_TRUE;
	deviceFeatures.features.depthBiasClamp = VK_TRUE;
	deviceFeatures.features.samplerAnisotropy = VK_TRUE;
	deviceFeatures.features.robustBufferAccess = VK_TRUE;
	// descriptorIndexingFeatures.robustBufferAccessUpdateAfterBind = VK_FALSE;
	descriptorIndexingFeatures.descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE;
	descriptorIndexingFeatures.descriptorBindingStorageBufferUpdateAfterBind = VK_FALSE;
	descriptorIndexingFeatures.descriptorBindingUniformTexelBufferUpdateAfterBind = VK_FALSE;
	descriptorIndexingFeatures.descriptorBindingStorageTexelBufferUpdateAfterBind = VK_FALSE;
	// assert(descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing);
	// assert(descriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind);
	// assert(descriptorIndexingFeatures.shaderUniformBufferArrayNonUniformIndexing);
	// #ifndef MACOS
	// 	assert(descriptorIndexingFeatures.descriptorBindingUniformBufferUpdateAfterBind);
	// #endif
	// 	assert(descriptorIndexingFeatures.shaderStorageBufferArrayNonUniformIndexing);
	// 	assert(descriptorIndexingFeatures.descriptorBindingStorageBufferUpdateAfterBind);
	createInfo.pNext = &deviceFeatures;
	auto createdDevice = VKcheck("vkCreateDevice", _vkCreateDevice(physicalDevice, &createInfo, 0, &device));
	if (!createdDevice)
	{
		throw std::runtime_error("VulkanRenderer-createLogicalDevice: failed to create device");
	}
	_vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
	_vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);
	return;
}
void VulkanRenderer::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = {(uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily};
	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	if (!VKcheck("vkGetPhysicalDeviceSurfaceCapabilitiesKHR",
							 _vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities)))
	{
		throw std::runtime_error("VulkanRenderer-vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed");
	}
	if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
	{
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	}
	else if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
	{
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	}
	else if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
	{
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
	}
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	if (!VKcheck("vkCreateSwapchainKHR", _vkCreateSwapchainKHR(device, &createInfo, 0, &swapChain)))
	{
		throw std::runtime_error("VulkanRenderer-createSwapChain: failed to create Swapchain");
	}
	if (!VKcheck("vkGetSwapchainImagesKHR", _vkGetSwapchainImagesKHR(device, swapChain, &imageCount, 0)))
	{
		throw std::runtime_error("VulkanRenderer-vkGetSwapchainImagesKHR failed");
	}
	swapChainImages.resize(imageCount);
	if (!VKcheck("vkGetSwapchainImagesKHR",
							 _vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data())))
	{
		throw std::runtime_error("VulkanRenderer-vkGetSwapchainImagesKHR failed");
	}
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
	return;
}
SwapChainSupportDetails VulkanRenderer::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;
	if (!VKcheck("vkGetPhysicalDeviceSurfaceCapabilitiesKHR",
							 _vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities)))
	{
		throw std::runtime_error("VulkanRenderer-vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed");
	}
	uint32_t formatCount;
	if (!VKcheck("vkGetPhysicalDeviceSurfaceFormatsKHR",
							 _vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, 0)))
	{
		throw std::runtime_error("VulkanRenderer-vkGetPhysicalDeviceSurfaceFormatsKHR failed");
	}
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		if (!VKcheck("vkGetPhysicalDeviceSurfaceFormatsKHR",
								 _vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data())))
		{
			throw std::runtime_error("VulkanRenderer-vkGetPhysicalDeviceSurfaceFormatsKHR failed");
		}
	}
	uint32_t presentModeCount;
	if (!VKcheck("vkGetPhysicalDeviceSurfacePresentModesKHR",
							 _vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, 0)))
	{
		throw std::runtime_error("VulkanRenderer-vkGetPhysicalDeviceSurfacePresentModesKHR failed");
	}
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		if (!VKcheck(
					"vkGetPhysicalDeviceSurfacePresentModesKHR",
					_vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data())))
		{
			throw std::runtime_error("VulkanRenderer-vkGetPhysicalDeviceSurfacePresentModesKHR failed");
		}
	}
	return details;
}
VkSurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
		{
			return availableFormat;
		}
	}
	return availableFormats[0];
}
VkPresentModeKHR VulkanRenderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	auto& renderWindow = *platformWindowPointer->renderWindowPointer;
	for (auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR && !renderWindow.vsync)
		{
			return availablePresentMode;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR && renderWindow.vsync)
		{
			return availablePresentMode;
		}
	}
	return VK_PRESENT_MODE_IMMEDIATE_KHR;
}
VkExtent2D VulkanRenderer::chooseSwapExtent(VkSurfaceCapabilitiesKHR capabilities)
{
	auto& renderWindow = *platformWindowPointer->renderWindowPointer;
	VkExtent2D actualExtent = {static_cast<uint32_t>(renderWindow.windowWidth),
														 static_cast<uint32_t>(renderWindow.windowHeight)};
	actualExtent.width =
		std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actualExtent.height =
		std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
	return actualExtent;
}
void VulkanRenderer::createImageViews()
{
	auto swapChainImagesSize = swapChainImages.size();
	swapChainImageViews.resize(swapChainImagesSize);
	for (uint32_t index = 0; index < swapChainImagesSize; index++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[index];
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
		if (!VKcheck("vkCreateImageView", _vkCreateImageView(device, &createInfo, 0, &(swapChainImageViews[index]))))
		{
			throw std::runtime_error("VulkanRenderer-createImageViews: failed to create imageView");
		}
	}
	return;
}
void VulkanRenderer::createRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;
	if (!VKcheck("vkCreateRenderPass", _vkCreateRenderPass(device, &renderPassInfo, 0, &renderPass)))
	{
		throw std::runtime_error("VulkanRenderer-createRenderPass: failed to create render pass!");
	}
	return;
}
void VulkanRenderer::createFramebuffers()
{
	auto swapChainImageViewsSize = swapChainImageViews.size();
	swapChainFramebuffers.resize(swapChainImageViewsSize);
	for (uint32_t index = 0; index < swapChainImageViewsSize; index++)
	{
		VkImageView attachments[] = {swapChainImageViews[index]};
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;
		if (!VKcheck("vkCreateFramebuffer",
								 _vkCreateFramebuffer(device, &framebufferInfo, 0, &(swapChainFramebuffers[index]))))
		{
			throw std::runtime_error("VulkanRenderer-createFramebuffers: failed to create framebuffer!");
		}
	}
	return;
}
void VulkanRenderer::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	if (!VKcheck("vkCreateCommandPool", _vkCreateCommandPool(device, &poolInfo, 0, &commandPool)))
	{
		throw std::runtime_error("VulkanRenderer-createCommandPool: failed to create command pool!");
	}
	return;
}
void VulkanRenderer::createCommandBuffers()
{
	commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = commandBuffers.size();
	if (!VKcheck("vkAllocateCommandBuffers", _vkAllocateCommandBuffers(device, &allocInfo, &commandBuffers[0])))
	{
		throw std::runtime_error("VulkanRenderer-createCommandBuffers: failed to allocate command buffers!");
	}
	return;
}
void VulkanRenderer::createSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
	{
		if (!VKcheck("vkCreateSemaphore", _vkCreateSemaphore(device, &semaphoreInfo, 0, &imageAvailableSemaphores[j])) ||
				!VKcheck("vkCreateSemaphore", _vkCreateSemaphore(device, &semaphoreInfo, 0, &renderFinishedSemaphores[j])) ||
				!VKcheck("vkCreateFence", _vkCreateFence(device, &fenceInfo, 0, &inFlightFences[j])))
		{
			throw std::runtime_error(
				"VulkanRenderer-createSyncObjects: failed to create synchronization objects for a frame!");
		}
	}
	return;
}
void VulkanRenderer::createImageStagingBuffer()
{
	auto& renderWindow = *platformWindowPointer->renderWindowPointer;
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = renderWindow.windowWidth * renderWindow.windowHeight * 4; // RGBA8
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (!VKcheck("vkCreateBuffer", _vkCreateBuffer(device, &bufferCreateInfo, 0, &stagingBuffer)))
	{
		throw std::runtime_error("VulkanRenderer-vkCreateBuffer failed");
	}
	VkMemoryRequirements memRequirements;
	_vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(
		memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	if (!VKcheck("vkAllocateMemory", _vkAllocateMemory(device, &allocInfo, 0, &stagingBufferMemory)))
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}
	if (!VKcheck("vkBindBufferMemory", _vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0)))
	{
		throw std::runtime_error("VulkanRenderer-vkBindBufferMemory failed");
	}
}
void VulkanRenderer::init()
{
	waitStages[0] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = 0;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.commandBufferCount = 1;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = 0;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.swapchainCount = 1;
	presentInfo.pResults = 0;
	if (fallbackToSwiftshader)
		_vkMapMemory(device, stagingBufferMemory, 0, VK_WHOLE_SIZE, 0, &bitmap);
}
void VulkanRenderer::destroyAtRenderPassEndOrDestroy(const std::function<void()>& function)
{
	destroyAtRenderPassEndOrDestroyVector.push_back(function);
}
void VulkanRenderer::callDestroyAtRenderPassEndOrDestroy()
{
	for (auto& function : destroyAtRenderPassEndOrDestroyVector)
	{
		function();
	}
	destroyAtRenderPassEndOrDestroyVector.clear();
}
void VulkanRenderer::destroy()
{
	_vkDeviceWaitIdle(device);
	if (fallbackToSwiftshader)
	{
		_vkUnmapMemory(device, stagingBufferMemory);
		_vkFreeMemory(device, stagingBufferMemory, 0);
		_vkDestroyBuffer(device, stagingBuffer, 0);
	}
	destroySwapChain();
	_vkDestroyRenderPass(device, renderPass, 0);
	for (auto& imageAvailableSemaphore : imageAvailableSemaphores)
	{
		_vkDestroySemaphore(device, imageAvailableSemaphore, 0);
	}
	for (auto& renderFinishedSemaphore : renderFinishedSemaphores)
	{
		_vkDestroySemaphore(device, renderFinishedSemaphore, 0);
	}
	for (auto& inFlightFence : inFlightFences)
	{
		_vkDestroyFence(device, inFlightFence, 0);
	}
	for (auto& renderPassPair : renderPassMap)
	{
		_vkDestroyRenderPass(device, renderPassPair.second, 0);
	}
	callDestroyAtRenderPassEndOrDestroy();
	_vkDestroyCommandPool(device, commandPool, 0);
	_vkDestroyDevice(device, 0);
}
void VulkanRenderer::destroySwapChain()
{
	for (auto framebuffer : swapChainFramebuffers)
	{
		_vkDestroyFramebuffer(device, framebuffer, 0);
	}
	for (auto imageView : swapChainImageViews)
	{
		_vkDestroyImageView(device, imageView, 0);
	}
	_vkDestroySwapchainKHR(device, swapChain, 0);
}
void VulkanRenderer::preBeginRenderPass()
{
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	if (!VKcheck("vkWaitForFences", _vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX)))
	{
		throw std::runtime_error("VulkanRenderer-vkWaitForFences failed");
	}
	if (!VKcheck("vkAcquireNextImageKHR",
							 _vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame],
																			VK_NULL_HANDLE, &imageIndex)))
	{
		throw std::runtime_error("VulkanRenderer-vkAcquireNextImageKHR failed");
	}
	if (!VKcheck("vkResetFences", _vkResetFences(device, 1, &inFlightFences[currentFrame])))
	{
		throw std::runtime_error("VulkanRenderer-vkResetFences failed");
	}
	commandBuffer = &commandBuffers[currentFrame];
	if (!VKcheck("vkResetCommandBuffer", _vkResetCommandBuffer(*commandBuffer, 0)))
	{
		throw std::runtime_error("VulkanRenderer-vkResetCommandBuffer failed");
	}
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	if (!VKcheck("vkBeginCommandBuffer", _vkBeginCommandBuffer(*commandBuffer, &beginInfo)))
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}
	return;
}
void VulkanRenderer::beginRenderPass()
{
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	if (currentFramebufferImpl)
	{
		renderPassInfo.renderPass = currentFramebufferImpl->renderPass;
	}
	else
	{
		renderPassInfo.renderPass = renderPass;
	}
	renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = swapChainExtent;
	auto& renderWindow = *platformWindowPointer->renderWindowPointer;
	glm::vec4 clearColor(0, 0, 0, 1);
	if (renderWindow.scene)
	{
		clearColor = ((Scene&)*renderWindow.scene).clearColor;
	}
	VkClearValue vkClearColor = {{{clearColor.r, clearColor.g, clearColor.b, clearColor.a}}};
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &vkClearColor;
	_vkCmdBeginRenderPass(*commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	return;
}
void VulkanRenderer::postRenderPass()
{
	_vkCmdEndRenderPass(*commandBuffer);
	if (!VKcheck("vkEndCommandBuffer", _vkEndCommandBuffer(*commandBuffer)))
	{
		throw std::runtime_error("failed to record command buffer!");
	}
	VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.pCommandBuffers = commandBuffer;
	signalSemaphores[0] = renderFinishedSemaphores[currentFrame];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	if (!VKcheck("vkQueueSubmit", _vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame])))
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}
	if (!VKcheck("vkQueueWaitIdle", _vkQueueWaitIdle(graphicsQueue)))
	{
		throw std::runtime_error("VulkanRenderer-vkQueueWaitIdle failed");
	}
	presentInfo.pWaitSemaphores = signalSemaphores;
	swapChains[0] = {swapChain};
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	if (!VKcheck("vkQueueWaitIdle", _vkQueueWaitIdle(presentQueue)))
	{
		throw std::runtime_error("VulkanRenderer-vkQueueWaitIdle failed");
	}
	callDestroyAtRenderPassEndOrDestroy();
	return;
}
#ifndef MACOS
void VulkanRenderer::swapBuffers()
{
	if (!VKcheck("vkQueuePresentKHR", _vkQueuePresentKHR(presentQueue, &presentInfo)))
	{
		throw std::runtime_error("VulkanRenderer-vkQueuePresentKHR failed");
	}
}
#endif
IRenderer* zg::createRenderer()
{
#if true
	return new VulkanRenderer();
#endif
}
void VulkanRenderer::clearColor(glm::vec4 color) {}
void VulkanRenderer::clear() {}
void VulkanRenderer::viewport(glm::ivec4 vp) const {}
void VulkanRenderer::initShader(shaders::Shader& shader, const shaders::RuntimeConstants& constants,
																const std::vector<shaders::ShaderType>& shaderTypes)
{
	shader.rendererData = new VulkanShaderImpl();
	auto& shaderImpl = *(VulkanShaderImpl*)shader.rendererData;
	shaderImpl.shaders = shaders::ShaderFactory::generateShaderMap(constants, shader, shaderTypes);
}
void VulkanRenderer::setUniform(shaders::Shader& shader, vaos::VAO& vao, const std::string_view name,
																const void* pointer, uint32_t size, enums::EUniformType uniformType)
{
	setBlock(shader, vao, name, pointer, size);
}
void VulkanRenderer::setBlock(shaders::Shader& shader, vaos::VAO& vao, const std::string_view name, const void* pointer,
															size_t size)
{
	int32_t location = getUniformLocation(shader, vao, name);
	if (location == -1)
	{
		return;
	}
	auto& vaoImpl = *(VulkanVAOImpl*)vao.rendererData;
	memcpy(vaoImpl.uniformBuffersMapped[location], pointer, size);
}
int32_t VulkanRenderer::getUniformLocation(shaders::Shader& shader, vaos::VAO& vao, const std::string_view& name)
{
	auto& vaoImpl = *(VulkanVAOImpl*)vao.rendererData;
	auto& table = vaoImpl.uniformLocationTable;
	std::string stringName(name);
	auto iter = table.find(stringName);
	if (iter == table.end())
	{
		return -1;
	}
	return iter->second;
}
void VulkanRenderer::deleteBuffer(uint32_t id) {}
void VulkanRenderer::bindShader(shaders::Shader& shader, Entity& entity)
{
	if (!shader.compiled)
	{
		compileProgram(shader);
	}
	if (!entity.ensured)
	{
		ensureEntity(shader, entity);
	}
	auto& shaderImpl = *(VulkanShaderImpl*)shader.rendererData;
	_vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderImpl.graphicsPipeline);
	if (currentFramebufferImpl)
	{
		auto& framebufferImpl = *currentFramebufferImpl;
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = framebufferImpl.height;
		viewport.width = (float)framebufferImpl.width;
		viewport.height = -(float)framebufferImpl.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		_vkCmdSetViewport(*commandBuffer, 0, 1, &viewport);
		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent.width = framebufferImpl.width;
		scissor.extent.height = framebufferImpl.height;
		_vkCmdSetScissor(*commandBuffer, 0, 1, &scissor);
	}
	else
	{
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = swapChainExtent.height;
		viewport.width = (float)swapChainExtent.width;
		viewport.height = -(float)swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		_vkCmdSetViewport(*commandBuffer, 0, 1, &viewport);
		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = swapChainExtent;
		_vkCmdSetScissor(*commandBuffer, 0, 1, &scissor);
	}
}
void VulkanRenderer::unbindShader(shaders::Shader& shader) {}
void VulkanRenderer::addSSBO(shaders::Shader& shader, shaders::ShaderType shaderType, const std::string_view name,
														 uint32_t bindingIndex)
{
	auto& shaderImpl = *(VulkanShaderImpl*)shader.rendererData;
	std::string stringName(name);
	auto ssboIter = shaderImpl.ssboBindings.find(stringName);
	std::tuple<uint32_t, uint32_t, uint32_t>* ssboBindingPointer = 0;
	if (ssboIter == shaderImpl.ssboBindings.end())
	{
		ssboBindingPointer = &shaderImpl.ssboBindings[stringName];
		std::get<1>(*ssboBindingPointer) = bindingIndex;
		VkDescriptorSetLayoutBinding layoutBinding = {(uint32_t)bindingIndex, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
																									(VkShaderStageFlags)stageStageFlag[shaderType], 0};
		shaderImpl.uboLayoutBindings.push_back({{ELayoutBindingType::SSBO, 0, "", bindingIndex, false}, layoutBinding});
		int32_t uboLayoutBindingIndex = shaderImpl.uboLayoutBindings.size() - 1;
		std::get<2>(*ssboBindingPointer) = uboLayoutBindingIndex;
	}
	else
	{
		ssboBindingPointer = &ssboIter->second;
		auto uboLayoutBindingIndex = std::get<2>(*ssboBindingPointer);
		shaderImpl.uboLayoutBindings[uboLayoutBindingIndex].second.stageFlags |= stageStageFlag[shaderType];
	}
	std::get<0>(*ssboBindingPointer) |= (uint32_t)shaderType;
}
void VulkanRenderer::addUBO(shaders::Shader& shader, shaders::ShaderType shaderType, const std::string_view name,
														uint32_t bindingIndex, uint32_t bufferSize, uint32_t descriptorCount, bool isArray)
{
	auto& shaderImpl = *(VulkanShaderImpl*)shader.rendererData;
	std::string stringName(name);
	VkDescriptorSetLayoutBinding layoutBinding = {(uint32_t)bindingIndex, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (uint32_t)1,
																								(VkShaderStageFlags)stageStageFlag[shaderType], 0};
	shaderImpl.uboLayoutBindings.push_back(
		{{ELayoutBindingType::UniformBuffer, bufferSize, stringName, bindingIndex, isArray}, layoutBinding});
	shaderImpl.uboStringBindings[stringName] = bindingIndex;
	for (uint32_t index = 0; index < 1; index++)
	{
		shaderImpl.uboStringBindings[stringName + "[" + std::to_string(index) + "]"] = bindingIndex + index;
	}
}
void VulkanRenderer::addTexture(shaders::Shader& shader, uint32_t bindingIndex, shaders::ShaderType shaderType,
																std::string_view textureName, uint32_t descriptorCount)
{
	auto& shaderImpl = *(VulkanShaderImpl*)shader.rendererData;
	std::string stringName(textureName);
	VkDescriptorSetLayoutBinding layoutBinding = {(uint32_t)bindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
																								(uint32_t)descriptorCount,
																								(VkShaderStageFlags)stageStageFlag[shaderType], 0};
	shaderImpl.uboLayoutBindings.push_back(
		{{ELayoutBindingType::ImageSampler, 0, "", bindingIndex, false}, layoutBinding});
	shaderImpl.textureBindings[stringName] = bindingIndex;
	for (uint32_t index = 0; index < descriptorCount; index++)
	{
		shaderImpl.textureArrayBindings[stringName + "[" + std::to_string(index) + "]"] = bindingIndex + index;
	}
}
void VulkanRenderer::setSSBO(shaders::Shader& shader, vaos::VAO& vao, const std::string_view name, const void* pointer,
														 size_t size)
{
	auto& shaderImpl = *(VulkanShaderImpl*)shader.rendererData;
	auto& vaoImpl = *(VulkanVAOImpl*)vao.rendererData;
	std::string stringName(name);
	auto ssboIter = shaderImpl.ssboBindings.find(stringName);
	if (ssboIter == shaderImpl.ssboBindings.end())
	{
		return;
	}
	auto ssboBufferIter = vaoImpl.ssboBuffers.find(stringName);
	if (ssboBufferIter == vaoImpl.ssboBuffers.end())
	{
		return;
	}
	auto& buffer = std::get<0>(ssboBufferIter->second);
	auto& deviceMemory = std::get<1>(ssboBufferIter->second);
	auto& bindingIndex = std::get<1>(ssboIter->second);
	if (buffer == VK_NULL_HANDLE)
	{
		createBuffer(
			size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, buffer, deviceMemory);
	}
	void* bufferData;
	if (!VKcheck("vkMapMemory", _vkMapMemory(device, deviceMemory, 0, size, 0, &bufferData)))
	{
		throw std::runtime_error("VulkanRenderer-vkMapMemory failed");
	}
	uint32_t bufferOffset = 0;
	memcpy((char*)bufferData, pointer, size);
	_vkUnmapMemory(device, deviceMemory);
	VkDescriptorBufferInfo storageBufferInfo{};
	storageBufferInfo.buffer = buffer;
	storageBufferInfo.offset = 0;
	storageBufferInfo.range = size;
	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = vaoImpl.descriptorSet;
	descriptorWrite.dstBinding = bindingIndex;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &storageBufferInfo;
	_vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, 0);
}
void VulkanRenderer::setTexture(shaders::Shader& shader, vaos::VAO& vao, const std::string_view name,
																const textures::Texture& texture, const int32_t unit)
{
	texture.bind();
	std::string stringName(name);
	auto& textureImpl = *(VulkanTextureImpl*)texture.rendererData;
	auto& shaderImpl = *(VulkanShaderImpl*)shader.rendererData;
	auto& vaoImpl = *(VulkanVAOImpl*)vao.rendererData;
	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = vaoImpl.descriptorSet;
	auto& bindingIndex = shaderImpl.textureBindings[stringName];
	descriptorWrite.dstBinding = bindingIndex;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = textureFormat_descriptor_imageLayout[texture.format];
	imageInfo.imageView = textureImpl.textureImageView;
	imageInfo.sampler = textureImpl.textureSampler;
	descriptorWrite.pImageInfo = &imageInfo;
	_vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, 0);
	texture.unbind();
}
bool VulkanRenderer::compileShader(shaders::Shader& shader, shaders::ShaderType shaderType,
																	 shaders::ShaderPair& shaderPair)
{
	shaderc::Compiler compiler;
	shaderc::CompileOptions compileOptions;
	auto& shaderString = shaderPair.first;
	auto& shaderModule = shaderPair.second;
	shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(shaderString.c_str(), shaderString.size(),
																																	 stageEShaderc[shaderType], "main", compileOptions);
	if (!checkCompileErrors(module, true, shaders::stageShaderNames[shaderType].c_str()))
	{
		return false;
	}
	std::vector<uint32_t> vertexSpv(module.cbegin(), module.cend());
	auto& shaderImpl = *(VulkanShaderImpl*)shader.rendererData;
	shaderModule = createShaderModule(vertexSpv);
	shaderImpl.shaders[shaderType] = shaderPair;
	return true;
}
VkShaderModule VulkanRenderer::createShaderModule(const std::vector<uint32_t>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = 4 * code.size();
	createInfo.pCode = code.data();
	VkShaderModule shaderModule;
	if (!VKcheck("vkCreateShaderModule", _vkCreateShaderModule(device, &createInfo, 0, &shaderModule)))
	{
		throw std::runtime_error("failed to create shader module!");
	}
	return shaderModule;
}
bool VulkanRenderer::compileProgram(shaders::Shader& shader)
{
	if (shader.compiled)
		return true;
	auto& shaderImpl = *(VulkanShaderImpl*)shader.rendererData;
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	for (auto& shaderPair : shaderImpl.shaders)
	{
		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = stageStageFlag[shaderPair.first];
		shaderStageInfo.module = (VkShaderModule)shaderPair.second.second;
		shaderStageInfo.pName = "main";
		shaderStages.push_back(shaderStageInfo);
	}
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = vaos::VAOFactory::getStride(shader.constants);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	uint32_t attribIndex = 0;
	uint32_t offset = 0;
	for (auto& constant : shader.constants)
	{
		if (!vaos::VAOFactory::isVAOConstant(constant))
			continue;
		auto& constantSize = vaos::VAOFactory::constantSizes[constant];
		VkVertexInputAttributeDescription vertexInputAttributeDescription;
		vertexInputAttributeDescription.location = attribIndex;
		vertexInputAttributeDescription.binding = 0;
		vertexInputAttributeDescription.format = (VkFormat)std::get<2>(constantSize);
		vertexInputAttributeDescription.offset = offset;
		attributeDescriptions.push_back(vertexInputAttributeDescription);
		offset += std::get<0>(constantSize) * std::get<1>(constantSize);
		attribIndex++;
	}
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	VkViewport viewport{};
	VkRect2D scissor{};
	viewport.x = 0.0f;
	viewport.y = swapChainExtent.height;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = -(float)swapChainExtent.height;
	scissor.extent = swapChainExtent;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	scissor.offset = {0, 0};
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_TRUE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
	for (auto& uboLayoutBinding : shaderImpl.uboLayoutBindings)
	{
		layoutBindings.push_back(uboLayoutBinding.second);
	}
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
	descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo.bindingCount = layoutBindings.size();
	descriptorSetLayoutInfo.pBindings = layoutBindings.data();
	if (!VKcheck("vkCreateDescriptorSetLayout",
							 _vkCreateDescriptorSetLayout(device, &descriptorSetLayoutInfo, 0, &shaderImpl.descriptorSetLayout)))
	{
		throw std::runtime_error("Failed to create descriptor set layout");
	}
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &shaderImpl.descriptorSetLayout;
	if (!VKcheck("vkCreatePipelineLayout",
							 _vkCreatePipelineLayout(device, &pipelineLayoutInfo, 0, &shaderImpl.pipelineLayout)))
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}
	std::vector<VkDynamicState> dynamicStates({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR});
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = dynamicStates.size();
	dynamicState.pDynamicStates = dynamicStates.data();
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = shaderImpl.pipelineLayout;
	if (currentFramebufferImpl)
		pipelineInfo.renderPass = currentFramebufferImpl->renderPass;
	else
		pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	if (!VKcheck("vkCreateGraphicsPipelines",
							 _vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, 0, &shaderImpl.graphicsPipeline)))
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}
	shader.compiled = true;
	return true;
}
bool VulkanRenderer::checkCompileErrors(const shaderc::SpvCompilationResult& module, bool isShader,
																				const char* shaderType)
{
	if (module.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		std::cout << "SHADER_COMPILATION_ERROR[" << shaderType << "]: " << module.GetErrorMessage().c_str() << std::endl;
		return false;
	}
	return true;
}
void VulkanRenderer::deleteShader(shaders::Shader& shader) {}
void VulkanRenderer::destroyShader(shaders::Shader& shader)
{
	auto& shaderImpl = *(VulkanShaderImpl*)shader.rendererData;
	destroyAtRenderPassEndOrDestroy(
		[&, descriptorSetLayout = shaderImpl.descriptorSetLayout, graphicsPipeline = shaderImpl.graphicsPipeline,
		 pipelineLayout = shaderImpl.pipelineLayout, shaders = shaderImpl.shaders]
		{
			_vkDestroyDescriptorSetLayout(device, descriptorSetLayout, 0);
			_vkDestroyPipeline(device, graphicsPipeline, 0);
			_vkDestroyPipelineLayout(device, pipelineLayout, 0);
			for (auto& shaderPair : shaders)
				_vkDestroyShaderModule(device, (VkShaderModule)shaderPair.second.second, 0);
		});
	delete &shaderImpl;
}
void VulkanRenderer::bindFramebuffer(const textures::Framebuffer& framebuffer)
{
	auto& framebufferImpl = *(const VulkanFramebufferImpl*)framebuffer.rendererData;
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = framebufferImpl.renderPass;
	renderPassInfo.framebuffer = framebufferImpl.framebuffer;
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent.width = framebufferImpl.width;
	renderPassInfo.renderArea.extent.height = framebufferImpl.height;
	std::vector<VkClearValue> clearValues;
	VkClearValue clearValue;
	switch (framebuffer.texture.format)
	{
	case textures::Texture::Depth:
		{
			clearValue.depthStencil = {0.0f, 0};
			break;
		}
	default:
		{
			glm::vec4 clearColor = framebuffer.clearColor;
			if (framebuffer.scenePointer)
			{
				clearColor = framebuffer.scenePointer->clearColor;
			}
			clearValue.color = {{clearColor.r, clearColor.g, clearColor.b, clearColor.a}};
			break;
		}
	}
	clearValues.push_back(clearValue);
	renderPassInfo.clearValueCount = clearValues.size();
	renderPassInfo.pClearValues = clearValues.data();
	_vkCmdBeginRenderPass(*commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	currentFramebufferImpl = &framebufferImpl;
}
void VulkanRenderer::unbindFramebuffer(const textures::Framebuffer& framebuffer)
{
	_vkCmdEndRenderPass(*commandBuffer);
	currentFramebufferImpl = 0;
}
void VulkanRenderer::initFramebuffer(textures::Framebuffer& framebuffer)
{
	framebuffer.rendererData = new VulkanFramebufferImpl();
	auto& framebufferImpl = *(VulkanFramebufferImpl*)framebuffer.rendererData;
	size_t renderPassHash = 0;
	uint8_t shift = 0;
	std::vector<VkAttachmentDescription> attachments;
	VkAttachmentDescription attachment{};
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	renderPassHash ^= attachment.samples ^ (attachment.loadOp << ++shift) ^ (attachment.storeOp << ++shift) ^
		(attachment.stencilLoadOp << ++shift) ^ (attachment.stencilStoreOp << ++shift) ^
		(attachment.initialLayout << ++shift);
	switch (framebuffer.texture.format)
	{
	case textures::Texture::Depth:
		{
			attachment.format = VK_FORMAT_D32_SFLOAT;
			attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			break;
		}
	default:
		attachment.format = VK_FORMAT_R8G8B8A8_SRGB;
		attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		break;
	}
	renderPassHash ^= (attachment.format << ++shift) ^ (attachment.finalLayout << ++shift);
	attachments.push_back(attachment);
	std::vector<VkAttachmentReference> colorAttachmentRefs;
	std::vector<VkAttachmentReference> depthAttachmentRefs;
	uint32_t attachmentIndex = 0;
	VkAttachmentReference ref{};
	ref.attachment = attachmentIndex++;
	switch (framebuffer.texture.format)
	{
	case textures::Texture::Depth:
		{
			ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthAttachmentRefs.push_back(ref);
			break;
		}
	default:
		ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachmentRefs.push_back(ref);
		break;
	}
	renderPassHash ^= (ref.layout << ++shift);
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = colorAttachmentRefs.size();
	subpass.pColorAttachments = colorAttachmentRefs.data();
	subpass.pDepthStencilAttachment = depthAttachmentRefs.data();
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

	switch (framebuffer.texture.format)
	{
	case textures::Texture::Depth:
		{
			dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
		}
	default:
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	}
	renderPassHash ^= (dependency.srcSubpass << ++shift) ^ (dependency.dstSubpass << ++shift) ^
		(dependency.srcStageMask << ++shift) ^ (dependency.srcAccessMask << ++shift) ^
		(dependency.dstStageMask << ++shift) ^ (dependency.dstAccessMask << ++shift);
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = attachments.size();
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;
	auto renderPassIter = renderPassMap.find(renderPassHash);
	if (renderPassIter != renderPassMap.end())
	{
		framebufferImpl.renderPass = renderPassIter->second;
	}
	else
	{
		if (!VKcheck("vkCreateRenderPass", _vkCreateRenderPass(device, &renderPassInfo, 0, &framebufferImpl.renderPass)))
		{
			throw std::runtime_error("VulkanRenderer-framebufferCreate: failed to create render pass!");
		}
		renderPassMap[renderPassHash] = framebufferImpl.renderPass;
	}
	std::vector<VkImageView> imageViews;
	auto& textureImpl = *(VulkanTextureImpl*)framebuffer.texture.rendererData;
	framebufferImpl.width = framebuffer.texture.size.x;
	framebufferImpl.height = framebuffer.texture.size.y;
	imageViews.push_back(textureImpl.textureImageView);
	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = framebufferImpl.renderPass;
	framebufferInfo.attachmentCount = imageViews.size();
	framebufferInfo.pAttachments = imageViews.data();
	framebufferInfo.width = framebufferImpl.width;
	framebufferInfo.height = framebufferImpl.height;
	framebufferInfo.layers = 1;
	if (!VKcheck("vkCreateFramebuffer", _vkCreateFramebuffer(device, &framebufferInfo, 0, &framebufferImpl.framebuffer)))
	{
		throw std::runtime_error("VulkanRenderer-framebufferCreate: failed to create framebuffer!");
	}
}
void VulkanRenderer::destroyFramebuffer(textures::Framebuffer& framebuffer)
{
	auto& framebufferImpl = *(VulkanFramebufferImpl*)framebuffer.rendererData;
	destroyAtRenderPassEndOrDestroy(
		[&, renderPass = framebufferImpl.renderPass, framebuffer = framebufferImpl.framebuffer]
		{ _vkDestroyFramebuffer(device, framebuffer, 0); });
	delete &framebufferImpl;
}
void VulkanRenderer::bindTexture(const textures::Texture& texture)
{
	auto& textureImpl = *(VulkanTextureImpl*)texture.rendererData;
	if (textureImpl.layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		VkFormat format;
		if (texture.format == textures::Texture::Format::RGBA8)
		{
			format = textureFormat_Format[texture.format];
		}
		else if (texture.format == textures::Texture::Format::Depth ||
						 texture.format == textures::Texture::Format::DepthStencil)
		{
			format = findDepthFormat(texture.format);
		}
		auto aspectMask = textureFormat_imageAspect[texture.format];
		transitionImageLayout(textureImpl, textureImpl.textureImage, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
													VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, aspectMask);
	}
}
void VulkanRenderer::unbindTexture(const textures::Texture& texture) {}
void VulkanRenderer::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
																 VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image,
																 VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (!VKcheck("vkCreateImage", _vkCreateImage(device, &imageInfo, 0, &image)))
	{
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	_vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (!VKcheck("vkAllocateMemory", _vkAllocateMemory(device, &allocInfo, 0, &imageMemory)))
	{
		throw std::runtime_error("failed to allocate image memory!");
	}

	_vkBindImageMemory(device, image, imageMemory, 0);
}
VkImageView VulkanRenderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	VkImageView imageView;
	if (_vkCreateImageView(device, &viewInfo, 0, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture image view!");
	}
	return imageView;
}
VkFormat VulkanRenderer::findDepthFormat(uint32_t _format)
{
	std::vector<VkFormat> candidates;
	textures::Texture::Format format = (textures::Texture::Format)_format;
	switch (format)
	{
	case textures::Texture::Format::Depth:
		{
			candidates.push_back(VK_FORMAT_D32_SFLOAT);
			break;
		};
	case textures::Texture::Format::DepthStencil:
		{
			candidates.push_back(VK_FORMAT_D32_SFLOAT_S8_UINT);
			candidates.push_back(VK_FORMAT_D24_UNORM_S8_UINT);
			break;
		};
	}
	return findSupportedFormat(candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
VkFormat VulkanRenderer::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
																						 VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		_vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}
	throw std::runtime_error("failed to find supported format!");
}
void VulkanRenderer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {0, 0, 0};
	region.imageExtent = {width, height, 1};
	_vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	endSingleTimeCommands(commandBuffer);
}
void VulkanRenderer::transitionImageLayout(VulkanTextureImpl& textureImpl, VkImage image, VkFormat format,
																					 VkImageLayout oldLayout, VkImageLayout newLayout,
																					 VkImageAspectFlags aspectMask)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = aspectMask;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else
	{
		throw std::invalid_argument("unsupported layout transition!");
	}

	_vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, 0, 0, 0, 1, &barrier);

	endSingleTimeCommands(commandBuffer);

	textureImpl.layout = newLayout;
}
void VulkanRenderer::preInitTexture(textures::Texture& texture)
{
	texture.rendererData = new VulkanTextureImpl();
	auto& textureImpl = *(VulkanTextureImpl*)texture.rendererData;
	VkFormat format;
	if (texture.format == textures::Texture::Format::RGBA8)
	{
		format = textureFormat_Format[texture.format];
	}
	else if (texture.format == textures::Texture::Format::Depth ||
					 texture.format == textures::Texture::Format::DepthStencil)
	{
		format = findDepthFormat(texture.format);
	}
	VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	switch (texture.format)
	{
	case textures::Texture::Format::RGBA8:
		{
			usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			break;
		};
	case textures::Texture::Format::Depth:
	case textures::Texture::Format::DepthStencil:
		{
			usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			break;
		}
	}
	// TODO: Vulkan: Implement createImage1D && createImage3D
	createImage(texture.size.x, texture.size.y, format, VK_IMAGE_TILING_LINEAR, usage,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImpl.textureImage, textureImpl.textureImageMemory);
	auto aspectMask = textureFormat_imageAspect[texture.format];
	textureImpl.textureImageView = createImageView(textureImpl.textureImage, format, aspectMask);
	transitionImageLayout(textureImpl, textureImpl.textureImage, format, VK_IMAGE_LAYOUT_UNDEFINED,
												VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, aspectMask);
	// create sampler
	VkPhysicalDeviceProperties properties{};
	_vkGetPhysicalDeviceProperties(physicalDevice, &properties);
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	VkFilter filter;
	VkSamplerMipmapMode mipmapMode;
	switch (texture.filterType)
	{
	case textures::Texture::FilterType::Linear:
		{
			filter = VK_FILTER_LINEAR;
			mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		}
	case textures::Texture::FilterType::Nearest:
		{
			filter = VK_FILTER_NEAREST;
			mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		}
	}
	samplerInfo.magFilter = filter;
	samplerInfo.minFilter = filter;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = mipmapMode;
	if (!VKcheck("vkCreateSampler", _vkCreateSampler(device, &samplerInfo, 0, &textureImpl.textureSampler)))
	{
		throw std::runtime_error("failed to create texture sampler!");
	}
}
void VulkanRenderer::midInitTexture(const textures::Texture& texture,
																		const std::vector<images::ImageLoader::ImagePair>& images)
{
	void* bytes = images.size() ? std::get<1>(images[0]).get() : 0;
	if (!bytes)
	{
		return;
	}
	VkFormat format;
	if (texture.format == textures::Texture::Format::RGBA8)
	{
		format = textureFormat_Format[texture.format];
	}
	else if (texture.format == textures::Texture::Format::Depth ||
					 texture.format == textures::Texture::Format::DepthStencil)
	{
		format = findDepthFormat(texture.format);
	}
	auto& textureImpl = *(VulkanTextureImpl*)texture.rendererData;
	VkDeviceSize imageSize = texture.size.x * texture.size.y * 4;
	VkBuffer _stagingBuffer;
	VkDeviceMemory _stagingBufferMemory;
	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _stagingBuffer,
							 _stagingBufferMemory);
	void* data = 0;
	_vkMapMemory(device, _stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, bytes, static_cast<size_t>(imageSize));
	_vkUnmapMemory(device, _stagingBufferMemory);
	copyBufferToImage(_stagingBuffer, textureImpl.textureImage, static_cast<uint32_t>(texture.size.x),
										static_cast<uint32_t>(texture.size.y));
	transitionImageLayout(textureImpl, textureImpl.textureImage, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
												textureFormat_descriptor_imageLayout[texture.format],
												textureFormat_imageAspect[texture.format]);
	_vkDestroyBuffer(device, _stagingBuffer, 0);
	_vkFreeMemory(device, _stagingBufferMemory, 0);
}
void VulkanRenderer::postInitTexture(const textures::Texture& texture)
{
	texture.bind();
}
void VulkanRenderer::destroyTexture(textures::Texture& texture)
{
	auto& textureImpl = *(VulkanTextureImpl*)texture.rendererData;
	destroyAtRenderPassEndOrDestroy(
		[&, textureSampler = textureImpl.textureSampler, textureImageView = textureImpl.textureImageView,
		 textureImage = textureImpl.textureImage, textureImageMemory = textureImpl.textureImageMemory]
		{
			_vkDestroySampler(device, textureSampler, 0);
			_vkDestroyImageView(device, textureImageView, 0);
			_vkDestroyImage(device, textureImage, 0);
			_vkFreeMemory(device, textureImageMemory, 0);
		});
	delete &textureImpl;
}
void VulkanRenderer::updateIndicesVAO(const vaos::VAO& vao, const std::vector<uint32_t>& indices)
{
	auto& vaoImpl = *(VulkanVAOImpl*)vao.rendererData;
	memcpy(vaoImpl.indiceData, indices.data(), vaoImpl.indiceBufferSize);
}
void VulkanRenderer::updateElementsVAO(const vaos::VAO& vao, const std::string_view constant, uint8_t* elementsAsChar)
{
	auto& vaoImpl = *(VulkanVAOImpl*)vao.rendererData;
	auto& constantSize = vaos::VAOFactory::constantSizes[constant];
	auto offset = vaos::VAOFactory::getOffset(vao.constants, constant);
	auto elementStride = std::get<0>(constantSize) * std::get<1>(constantSize);
	for (size_t index = offset, c = 1, elementIndex = 0; c <= vao.elementCount;
			 index += vao.stride, c++, elementIndex += elementStride)
	{
		memcpy((char*)vaoImpl.vertexData + index, &elementsAsChar[elementIndex], elementStride);
	}
}
void VulkanRenderer::drawVAO(const vaos::VAO& vao)
{
	auto& vaoImpl = *(VulkanVAOImpl*)vao.rendererData;
	auto& shader = *(dynamic_cast<const Entity&>(vao)).shader;
	auto& shaderImpl = *(VulkanShaderImpl*)shader.rendererData;
	if (vaoImpl.vertexBuffer == VK_NULL_HANDLE)
	{
		return;
	}
	VkBuffer vertexBuffers[] = {vaoImpl.vertexBuffer};
	VkDeviceSize offsets[] = {0};
	_vkCmdBindVertexBuffers(*commandBuffer, 0, 1, vertexBuffers, offsets);
	_vkCmdBindIndexBuffer(*commandBuffer, vaoImpl.indiceBuffer, 0, VK_INDEX_TYPE_UINT32);
	_vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderImpl.pipelineLayout, 0, 1,
													 &vaoImpl.descriptorSet, 0, 0);
	auto& indices = vao.indiceCount;
	if (!indices)
	{
		return;
	}
	_vkCmdDrawIndexed(*commandBuffer, indices, 1, 0, 0, 0);
}
void VulkanRenderer::generateVAO(vaos::VAO& vao)
{
	vao.rendererData = new VulkanVAOImpl();
	auto& vaoImpl = *(VulkanVAOImpl*)vao.rendererData;
	auto stride = vaos::VAOFactory::getStride(vao.constants);
	VkDeviceSize vertexBufferSize = stride * vao.elementCount;
	if (!vertexBufferSize)
	{
		return;
	}
	auto& constantSize = vaos::VAOFactory::constantSizes["Indice"];
	VkDeviceSize indiceBufferSize = vao.indiceCount * std::get<1>(constantSize);
	if (!indiceBufferSize)
	{
		return;
	}
	ensureBuffer(vaoImpl.vertexBuffer, vaoImpl.vertexBufferMemory, vaoImpl.vertexData, vaoImpl.vertexBufferSize,
							 vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	ensureBuffer(vaoImpl.indiceBuffer, vaoImpl.indiceBufferMemory, vaoImpl.indiceData, vaoImpl.indiceBufferSize,
							 indiceBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	return;
}
void VulkanRenderer::destroyVAO(vaos::VAO& vao)
{
	auto& vaoImpl = *(VulkanVAOImpl*)vao.rendererData;
	destroyAtRenderPassEndOrDestroy(
		[&, uniformBuffers = vaoImpl.uniformBuffers, uniformBuffersMemory = vaoImpl.uniformBuffersMemory,
		 descriptorPool = vaoImpl.descriptorPool, indiceBuffer = vaoImpl.indiceBuffer,
		 indiceBufferMemory = vaoImpl.indiceBufferMemory, vertexBuffer = vaoImpl.vertexBuffer,
		 vertexBufferMemory = vaoImpl.vertexBufferMemory]
		{
			for (auto& uniformBuffer : uniformBuffers)
			{
				_vkDestroyBuffer(device, uniformBuffer, 0);
			}
			for (auto& uniformBufferMemory : uniformBuffersMemory)
			{
				_vkFreeMemory(device, uniformBufferMemory, 0);
			}
			_vkDestroyDescriptorPool(device, descriptorPool, 0);
			_vkDestroyBuffer(device, indiceBuffer, 0);
			_vkFreeMemory(device, indiceBufferMemory, 0);
			_vkDestroyBuffer(device, vertexBuffer, 0);
			_vkFreeMemory(device, vertexBufferMemory, 0);
		});
	delete &vaoImpl;
}
void VulkanRenderer::ensureEntity(shaders::Shader& shader, vaos::VAO& vao)
{
	if (!shader.compiled)
		return;
	auto& entity = dynamic_cast<Entity&>(vao);
	if (entity.ensured)
		return;
	auto& shaderImpl = *(VulkanShaderImpl*)shader.rendererData;
	auto& vaoImpl = *(VulkanVAOImpl*)vao.rendererData;
	std::vector<VkDescriptorPoolSize> poolSizes;
	auto getPoolSize = [&](auto type) -> VkDescriptorPoolSize&
	{
		for (auto& poolSize : poolSizes)
		{
			if (poolSize.type == type)
			{
				return poolSize;
			}
		}
		poolSizes.push_back({type, 0});
		return poolSizes[poolSizes.size() - 1];
	};
	for (auto& uboLayoutBinding : shaderImpl.uboLayoutBindings)
	{
		auto& layoutBinding = uboLayoutBinding.second;
		auto& poolSize = getPoolSize(layoutBinding.descriptorType);
		poolSize.descriptorCount++;
	}
	if (shaderImpl.ssboBindings.size())
	{
		auto layoutBinding = getPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutBinding.descriptorCount = shaderImpl.ssboBindings.size();
	}

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = shaderImpl.uboLayoutBindings.size() + shaderImpl.ssboBindings.size();

	auto& descriptorPool = vaoImpl.descriptorPool;

	if (!VKcheck("vkCreateDescriptorPool", _vkCreateDescriptorPool(device, &poolInfo, 0, &descriptorPool)))
	{
		throw std::runtime_error("Failed to create descriptor pool!");
	}

	auto& descriptorSet = vaoImpl.descriptorSet;

	std::vector<VkDescriptorSetLayout> layouts(1, shaderImpl.descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layouts[0];

	if (!VKcheck("vkAllocateDescriptorSets", _vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet)))
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	auto& bufferInfos = vaoImpl.bufferInfos;
	for (auto uboLayoutBindingPair : shaderImpl.uboLayoutBindings)
	{
		if (std::get<0>(uboLayoutBindingPair.first) != ELayoutBindingType::UniformBuffer)
		{
			continue;
		}
		auto& descriptorSetLayoutBinding = uboLayoutBindingPair.second;
		auto& uniformBuffers = vaoImpl.uniformBuffers;
		auto& uniformBuffersMemory = vaoImpl.uniformBuffersMemory;
		auto& uniformBuffersMapped = vaoImpl.uniformBuffersMapped;
		uniformBuffers.resize(uniformBuffers.size() + 1);
		uniformBuffersMemory.resize(uniformBuffersMemory.size() + 1);
		const auto& bufferSize = std::get<1>(uboLayoutBindingPair.first);
		int32_t uniformBuffersIndex = -1, uniformBuffersMemoryIndex = -1, uniformBuffersMappedIndex = -1;
		createBuffer(bufferSize * descriptorSetLayoutBinding.descriptorCount, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
								 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
								 uniformBuffers[uniformBuffersIndex = (uniformBuffers.size() - 1)],
								 uniformBuffersMemory[uniformBuffersMemoryIndex = (uniformBuffersMemory.size() - 1)]);
		auto& isArray = std::get<4>(uboLayoutBindingPair.first);
		if (!isArray)
		{
			uniformBuffersMapped.resize(uniformBuffersMapped.size() + 1);
			_vkMapMemory(device, uniformBuffersMemory[uniformBuffersMemoryIndex], 0, bufferSize, 0,
									 &uniformBuffersMapped[uniformBuffersMappedIndex = (uniformBuffersMapped.size() - 1)]);
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[uniformBuffersIndex];
			bufferInfo.offset = 0;
			bufferInfo.range = bufferSize;
			bufferInfos.push_back(
				{{std::get<0>(uboLayoutBindingPair.first), std::get<3>(uboLayoutBindingPair.first)}, bufferInfo});
			vaoImpl.uniformLocationTable[std::get<2>(uboLayoutBindingPair.first)] = uniformBuffersMappedIndex;
		}
		else
		{
			for (uint32_t index = 0; index < descriptorSetLayoutBinding.descriptorCount; index++)
			{
				uniformBuffersMapped.resize(uniformBuffersMapped.size() + 1);
				_vkMapMemory(device, uniformBuffersMemory[uniformBuffersMemoryIndex], index * bufferSize, bufferSize, 0,
										 &uniformBuffersMapped[uniformBuffersMappedIndex = (uniformBuffersMapped.size() - 1)]);
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = uniformBuffers[uniformBuffersIndex];
				bufferInfo.offset = index * bufferSize;
				bufferInfo.range = bufferSize;
				bufferInfos.push_back(
					{{std::get<0>(uboLayoutBindingPair.first), std::get<3>(uboLayoutBindingPair.first) + index}, bufferInfo});
				vaoImpl.uniformLocationTable[std::get<2>(uboLayoutBindingPair.first) + "[" + std::to_string(index) + "]"] =
					uniformBuffersMappedIndex;
			}
		}
	}

	for (auto& bufferInfoPair : bufferInfos)
	{
		if (std::get<0>(bufferInfoPair.first) != ELayoutBindingType::UniformBuffer)
		{
			continue;
		}
		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = std::get<1>(bufferInfoPair.first);
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfoPair.second;
		_vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, 0);
	}

	entity.ensured = true;
};
VkCommandBuffer VulkanRenderer::beginSingleTimeCommands()
{
	VkCommandBuffer commandBuffer;
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	if (_vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffer!");
	}
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = 0;
	if (_vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin command buffer!");
	}
	return commandBuffer;
}
void VulkanRenderer::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	if (_vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to end command buffer!");
	}
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	if (_vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit command buffer!");
	}
	_vkQueueWaitIdle(graphicsQueue);
	_vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}
uint32_t VulkanRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	_vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}
	throw std::runtime_error("VulkanRenderer-findMemoryType: failed to find suitable memory type!");
}
void VulkanRenderer::ensureBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, void*& bufferData,
																	uint32_t& bufferSize, uint32_t newBufferSize, VkBufferUsageFlagBits extraUsageFlags)
{
	if (buffer == VK_NULL_HANDLE || newBufferSize != bufferSize)
	{
		if (buffer)
		{
			// size has changed create new buffer
			VkBuffer newBuffer;
			VkDeviceMemory newBufferMemory;
			void* newBufferData;
			createBuffer(newBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | extraUsageFlags,
									 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, newBuffer,
									 newBufferMemory);
			_vkMapMemory(device, newBufferMemory, 0, newBufferSize, 0, (void**)&newBufferData);
			memcpy(newBufferData, bufferData, newBufferSize < bufferSize ? newBufferSize : bufferSize);
			_vkDestroyBuffer(device, buffer, 0);
			_vkFreeMemory(device, bufferMemory, 0);
			buffer = newBuffer;
			bufferMemory = newBufferMemory;
			bufferData = newBufferData;
		}
		else
		{
			createBuffer(newBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | extraUsageFlags,
									 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory);
			_vkMapMemory(device, bufferMemory, 0, newBufferSize, 0, (void**)&bufferData);
		}
		bufferSize = newBufferSize;
	}
}
void VulkanRenderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
																	VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (!VKcheck("vkCreateBuffer", _vkCreateBuffer(device, &bufferInfo, 0, &buffer)))
	{
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	_vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (!VKcheck("vkAllocateMemory", _vkAllocateMemory(device, &allocInfo, 0, &bufferMemory)))
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	_vkBindBufferMemory(device, buffer, bufferMemory, 0);
}
void VulkanRenderer::getCurrentImageToBitmap()
{
	auto& renderWindow = *platformWindowPointer->renderWindowPointer;
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();
	VkImage image = swapChainImages[currentFrame];
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	_vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, 0, 0, 0,
												1, &barrier);
	endSingleTimeCommands(commandBuffer);
	commandBuffer = beginSingleTimeCommands();
	VkMappedMemoryRange range{};
	range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	range.memory = stagingBufferMemory;
	range.offset = 0;
	range.size = VK_WHOLE_SIZE;
	_vkInvalidateMappedMemoryRanges(device, 1, &range);
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageExtent.width = renderWindow.windowWidth;
	region.imageExtent.height = renderWindow.windowHeight;
	region.imageExtent.depth = 1;
	_vkCmdCopyImageToBuffer(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &region);
	endSingleTimeCommands(commandBuffer);
	commandBuffer = beginSingleTimeCommands();
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	_vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0, 0,
												0, 1, &barrier);
	endSingleTimeCommands(commandBuffer);
	_vkQueueWaitIdle(graphicsQueue);
}
bool zg::VKcheck(const char* fn, VkResult result)
{
	switch (result)
	{
	case VK_SUCCESS:
		{
			return true;
		};
	default:
		{
			return false;
		}
	}
}
// #endif
