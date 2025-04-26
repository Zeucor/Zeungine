// #ifdef USE_VULKAN
#include <zg/Logger.hpp>
#include <zg/Window.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/renderers/VulkanRenderer.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/system/ErrorPopup.hpp>
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
#include <zg/PostProcessingPipeline.hpp>
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
std::shared_ptr<SharedLibrary> zg::VulkanRenderer::vulkanLibrarySS = ([]()->std::shared_ptr<SharedLibrary>
{
	try
	{
		return std::make_shared<SharedLibrary>(ZG_LIB_INSTALL_PREFIX / vulkanLibrarySSName, vulkanLibrarySSName);
	}
	catch (...)
	{
		return {};
	}
})();
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
	{textures::Texture::Format::DepthStencil, VK_FORMAT_D32_SFLOAT_S8_UINT},
	{textures::Texture::Format::Stencil, VK_FORMAT_R8_UINT}};
static std::unordered_map<textures::Texture::Format, VkImageLayout> textureFormat_imageLayout = {
	{textures::Texture::Format::RGBA8, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
	{textures::Texture::Format::Depth, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
	{textures::Texture::Format::DepthStencil, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
	{textures::Texture::Format::Stencil, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}};
static std::unordered_map<textures::Texture::Format, VkImageLayout> textureFormat_descriptor_imageLayout = {
	{textures::Texture::Format::RGBA8, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
	{textures::Texture::Format::Depth, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL},
	{textures::Texture::Format::DepthStencil, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL},
	{textures::Texture::Format::Stencil, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL}};
static std::unordered_map<textures::Texture::Format, VkImageAspectFlags> textureFormat_imageAspect = {
	{textures::Texture::Format::RGBA8, VK_IMAGE_ASPECT_COLOR_BIT},
	{textures::Texture::Format::Depth, VK_IMAGE_ASPECT_DEPTH_BIT},
	{textures::Texture::Format::DepthStencil, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT},
	{textures::Texture::Format::Stencil, VK_IMAGE_ASPECT_STENCIL_BIT}};
// static std::unordered_map<EFramebufferAttachmentType, VkFormat> attachmentType_Format = {
// 	{EFramebufferAttachmentType::Color, VK_FORMAT_R8G8B8A8_SRGB},
// 	{EFramebufferAttachmentType::Depth, VK_FORMAT_D32_SFLOA_S8_UINTT},
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
VkSampleCountFlagBits TextureMultisamplingToSampleCountBit(textures::Texture::Multisampling multisampling)
{
	switch (multisampling)
	{
	case textures::Texture::x1:
		return VK_SAMPLE_COUNT_1_BIT;
	case textures::Texture::x2:
		return VK_SAMPLE_COUNT_2_BIT;
	case textures::Texture::x4:
		return VK_SAMPLE_COUNT_4_BIT;
	case textures::Texture::x8:
		return VK_SAMPLE_COUNT_8_BIT;
	case textures::Texture::x16:
		return VK_SAMPLE_COUNT_16_BIT;
	case textures::Texture::x32:
		return VK_SAMPLE_COUNT_32_BIT;
	case textures::Texture::x64:
		return VK_SAMPLE_COUNT_64_BIT;
	}
	return VK_SAMPLE_COUNT_1_BIT;
}
textures::Texture::Multisampling SampleCountBitToTextureMultisampling(VkSampleCountFlagBits sampleCountFlagBits)
{
	switch (sampleCountFlagBits)
	{
	case VK_SAMPLE_COUNT_1_BIT:
		return textures::Texture::x1;
	case VK_SAMPLE_COUNT_2_BIT:
		return textures::Texture::x2;
	case VK_SAMPLE_COUNT_4_BIT:
		return textures::Texture::x4;
	case VK_SAMPLE_COUNT_8_BIT:
		return textures::Texture::x8;
	case VK_SAMPLE_COUNT_16_BIT:
		return textures::Texture::x16;
	case VK_SAMPLE_COUNT_32_BIT:
		return textures::Texture::x32;
	case VK_SAMPLE_COUNT_64_BIT:
		return textures::Texture::x64;
	}
	return textures::Texture::x1;
}
VulkanRenderer::VulkanRenderer() {}
VulkanRenderer::~VulkanRenderer() {}
GetProcAddrFunc VulkanRenderer::doGetProcAddr()
{
	if (fallbackToSwiftshader && vulkanLibrarySS)
	{
		try
		{
			return vulkanLibrarySS->getProc<GetProcAddrFunc>("vk_icdGetInstanceProcAddr");
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
	createCommandPool();
	createCommandBuffers();
	createImageViews();
	createRenderPass();
	createDepthResources();
	createFramebuffers();
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
	appInfo.pApplicationName = platformWindowPointer->renderWindowPointer->title.c_str();
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
	VK_INSTANCE(_vkCreateRenderPass2, PFN_vkCreateRenderPass2, "vkCreateRenderPass2");
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
	if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ||
			messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		system::ErrorPopup::show(pCallbackData->pMessage);
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
VkSampleCountFlagBits VulkanRenderer::getMaxUsableSampleCount()
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
		physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT)
		return VK_SAMPLE_COUNT_64_BIT;
	if (counts & VK_SAMPLE_COUNT_32_BIT)
		return VK_SAMPLE_COUNT_32_BIT;
	if (counts & VK_SAMPLE_COUNT_16_BIT)
		return VK_SAMPLE_COUNT_16_BIT;
	if (counts & VK_SAMPLE_COUNT_8_BIT)
		return VK_SAMPLE_COUNT_8_BIT;
	if (counts & VK_SAMPLE_COUNT_4_BIT)
		return VK_SAMPLE_COUNT_4_BIT;
	if (counts & VK_SAMPLE_COUNT_2_BIT)
		return VK_SAMPLE_COUNT_2_BIT;
	return VK_SAMPLE_COUNT_1_BIT;
}
void VulkanRenderer::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	if (!VKcheck("vkEnumeratePhysicalDevices", _vkEnumeratePhysicalDevices(instance, &deviceCount, 0)))
		throw std::runtime_error("VulkanRenderer-vkEnumeratePhysicalDevices failed");
	if (deviceCount == 0)
		throw std::runtime_error("VulkanRenderer-getPhysicalDevice: failed to find GPUs with Vulkan support!");
	std::vector<VkPhysicalDevice> devices;
	devices.resize(deviceCount);
	if (!VKcheck("vkEnumeratePhysicalDevices", _vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data())))
		throw std::runtime_error("VulkanRenderer-vkEnumeratePhysicalDevices failed");
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
			maxMSAASamples = getMaxUsableSampleCount();
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
	deviceFeatures.features.sampleRateShading = VK_TRUE;
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
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB)
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
	mainColorTexture = std::make_shared<textures::Texture>(
		this,
		glm::ivec4(platformWindowPointer->renderWindowPointer->windowWidth,
							 platformWindowPointer->renderWindowPointer->windowHeight, 0, 0),
		(const void*)0, textures::Texture::Format::RGBA8, textures::Texture::Type::UnsignedByte,
		textures::Texture::FilterType::Linear, true, textures::Texture::Multisampling::x8);
	mainColorResolveTexture = std::make_shared<textures::Texture>(
		this,
		glm::ivec4(platformWindowPointer->renderWindowPointer->windowWidth,
								platformWindowPointer->renderWindowPointer->windowHeight, 0, 0),
		(const void*)0, textures::Texture::Format::RGBA8, textures::Texture::Type::UnsignedByte,
		textures::Texture::FilterType::Linear, true, textures::Texture::Multisampling::x1);
	TextureOutputRegistry::registerOutput((std::numeric_limits<float>::lowest)(), "ColorTexture", mainColorResolveTexture);
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
	VkAttachmentDescription2 colorAttachment{};
	colorAttachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;//maxMSAASamples;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	VkAttachmentReference2 colorAttachmentRef{};
	colorAttachmentRef.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	// VkAttachmentDescription depthAttachment{};
	// depthAttachment.format = findDepthFormat((uint32_t)textures::Texture::Format::Depth);
	// depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;//maxMSAASamples;
	// depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	// depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	// depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	// depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	// VkAttachmentReference depthAttachmentRef{};
	// depthAttachmentRef.attachment = 1;
	// depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	// VkAttachmentDescription colorAttachmentResolve{};
	// colorAttachmentResolve.format = swapChainImageFormat;
	// colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	// colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	// colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	// colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	// colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	// colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// colorAttachmentResolve.finalLayout = ;
	// VkAttachmentReference colorAttachmentResolveRef{};
	// colorAttachmentResolveRef.attachment = 2;
	// colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription2 subpass{};
	subpass.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	// subpass.pDepthStencilAttachment = &depthAttachmentRef;
	// subpass.pResolveAttachments = &colorAttachmentResolveRef;
	VkSubpassDependency2 dependency{};
	dependency.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	std::array<VkAttachmentDescription2, 1> attachments = {colorAttachment/*, depthAttachment*//*, colorAttachmentResolve*/};
	VkRenderPassCreateInfo2 renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
	renderPassInfo.attachmentCount = attachments.size();
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;
	if (!VKcheck("vkCreateRenderPass2", _vkCreateRenderPass2(device, &renderPassInfo, 0, &renderPass)))
	{
		throw std::runtime_error("VulkanRenderer-createRenderPass: failed to create render pass!");
	}
	return;
}
void VulkanRenderer::createFramebuffers()
{
	std::vector<textures::Framebuffer::TextureAttachmentPair> attachments = {
		{mainColorTexture.get(), textures::Framebuffer::AttachmentType::Color},
		{mainDepthTexture.get(), textures::Framebuffer::AttachmentType::Depth},
		{mainColorResolveTexture.get(), textures::Framebuffer::AttachmentType::ColorResolve},
		{mainDepthResolveTexture.get(), textures::Framebuffer::AttachmentType::DepthResolve}
	};
	mainFramebuffer = std::make_shared<textures::Framebuffer>(this, attachments);
	auto swapChainImageViewsSize = swapChainImageViews.size();
	swapChainFramebuffers.resize(swapChainImageViewsSize);
	for (uint32_t index = 0; index < swapChainImageViewsSize; index++)
	{
		std::vector<VkImageView> attachments({{swapChainImageViews[index]/*, depthImageView*/}});
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = attachments.size();
		framebufferInfo.pAttachments = attachments.data();
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
void VulkanRenderer::createDepthResources()
{
	mainDepthTexture =
		std::make_shared<textures::Texture>(this,
		glm::ivec4(platformWindowPointer->renderWindowPointer->windowWidth,
								platformWindowPointer->renderWindowPointer->windowHeight, 0, 0),
		(const void*)0, textures::Texture::Format::Depth,
		textures::Texture::Type::Float, textures::Texture::FilterType::Linear, true, textures::Texture::Multisampling::x8);
	mainDepthResolveTexture = std::make_shared<textures::Texture>(
		this,
		glm::ivec4(platformWindowPointer->renderWindowPointer->windowWidth,
								platformWindowPointer->renderWindowPointer->windowHeight, 0, 0),
		(const void*)0, textures::Texture::Format::Depth, textures::Texture::Type::Float,
		textures::Texture::FilterType::Linear, true, textures::Texture::Multisampling::x1);
	TextureOutputRegistry::registerOutput((std::numeric_limits<float>::lowest)(), "DepthTexture", mainDepthResolveTexture);
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
	mainColorTexture.reset();
	mainColorResolveTexture.reset();
	mainDepthResolveTexture.reset();
	mainDepthTexture.reset();
	mainFramebuffer.reset();
	for (auto framebuffer : swapChainFramebuffers)
	{
		_vkDestroyFramebuffer(device, framebuffer, 0);
	}
	for (auto imageView : swapChainImageViews)
	{
		_vkDestroyImageView(device, imageView, 0);
	}
	_vkDestroySwapchainKHR(device, swapChain, 0);
	// _vkDestroyImage(device, depthImage, 0);
	// _vkDestroyImageView(device, depthImageView, 0);
	// _vkFreeMemory(device, depthImageMemory, 0);
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
		renderPassInfo.framebuffer = currentFramebufferImpl->framebuffer;
	}
	else
	{
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
	}
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = swapChainExtent;
	std::array<VkClearValue, 2> clearValues{};
	auto& renderWindow = *platformWindowPointer->renderWindowPointer;
	glm::vec4 clearColor(0, 0, 0, 1);
	if (renderWindow.scenes.size())
	{
		clearColor = renderWindow.scenes.data()[0].clearColor;
	}
	clearValues[0].color = {{clearColor.r, clearColor.g, clearColor.b, clearColor.a}};
	clearValues[1].depthStencil = {1.0f, 0};
	renderPassInfo.clearValueCount = clearValues.size();
	renderPassInfo.pClearValues = clearValues.data();
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
void VulkanRenderer::beginMainFramebuffer()
{
	auto& mainFramebufferRef = *mainFramebuffer;
	if (platformWindowPointer->renderWindowPointer->scenes.size())
	{
		auto& scene = *platformWindowPointer->renderWindowPointer->scenes.begin();
		if (mainFramebufferRef.scenePointer != &scene)
		{
			mainFramebufferRef.scenePointer = &scene;
		}
	}
	mainFramebuffer->bind();
}
void VulkanRenderer::postMainFramebuffer() { mainFramebuffer->unbind(); }
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
	auto data = vaos::VAO::getShaderData(shader.iRenderer);
	auto& uniformBuffersMapped = vaoImpl.getUniformBuffersMapped(data);
	memcpy(uniformBuffersMapped[location], pointer, size);
}
int32_t VulkanRenderer::getUniformLocation(shaders::Shader& shader, vaos::VAO& vao, const std::string_view& name)
{
	auto& vaoImpl = *(VulkanVAOImpl*)vao.rendererData;
	auto data = vaos::VAO::getShaderData(shader.iRenderer);
	auto& table = vaoImpl.getUniformLocationTable(data);
	std::string stringName(name);
	auto iter = table.find(stringName);
	if (iter == table.end())
	{
		return -1;
	}
	return iter->second;
}
void VulkanRenderer::deleteBuffer(uint32_t id) {}
void VulkanRenderer::bindShader(shaders::Shader& shader, vaos::VAO& vao)
{
	if (!shader.compiled)
	{
		compileProgram(shader);
	}
	if (!vao.isEnsured())
	{
		ensureVAO(shader, vao);
	}
	auto& shaderImpl = *(VulkanShaderImpl*)shader.rendererData;
	_vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderImpl.graphicsPipeline);
	if (currentFramebufferImpl)
	{
		auto& framebufferImpl = *currentFramebufferImpl;
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)framebufferImpl.width;
		viewport.height = (float)framebufferImpl.height;
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
		viewport.y = 0.0f;
		viewport.width = (float)swapChainExtent.width;
		viewport.height = (float)swapChainExtent.height;
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
	auto data = vaos::VAO::getShaderData(shader.iRenderer);
	auto& ssboBindings = shaderImpl.getSsboBindings(data);
	auto& uboLayoutBindings = shaderImpl.getUboLayoutBindings(data);
	auto ssboIter = ssboBindings.find(stringName);
	std::tuple<uint32_t, uint32_t, uint32_t>* ssboBindingPointer = 0;
	if (ssboIter == ssboBindings.end())
	{
		ssboBindingPointer = &ssboBindings[stringName];
		std::get<1>(*ssboBindingPointer) = bindingIndex;
		VkDescriptorSetLayoutBinding layoutBinding = {(uint32_t)bindingIndex, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
																									(VkShaderStageFlags)stageStageFlag[shaderType], 0};
		uboLayoutBindings.push_back({{ELayoutBindingType::SSBO, 0, "", bindingIndex, false}, layoutBinding});
		int32_t uboLayoutBindingIndex = uboLayoutBindings.size() - 1;
		std::get<2>(*ssboBindingPointer) = uboLayoutBindingIndex;
	}
	else
	{
		ssboBindingPointer = &ssboIter->second;
		auto uboLayoutBindingIndex = std::get<2>(*ssboBindingPointer);
		uboLayoutBindings[uboLayoutBindingIndex].second.stageFlags |= stageStageFlag[shaderType];
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
	auto data = vaos::VAO::getShaderData(shader.iRenderer);
	auto& uboLayoutBindings = shaderImpl.getUboLayoutBindings(data);
	auto& uboStringBindings = shaderImpl.getUboStringBindings(data);
	uboLayoutBindings.push_back(
		{{ELayoutBindingType::UniformBuffer, bufferSize, stringName, bindingIndex, isArray}, layoutBinding});
	uboStringBindings[stringName] = bindingIndex;
	for (uint32_t index = 0; index < 1; index++)
	{
		uboStringBindings[stringName + "[" + std::to_string(index) + "]"] = bindingIndex + index;
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
	auto data = vaos::VAO::getShaderData(shader.iRenderer);
	auto& uboLayoutBindings = shaderImpl.getUboLayoutBindings(data);
	auto& textureBindings = shaderImpl.getTextureBindings(data);
	uboLayoutBindings.push_back({{ELayoutBindingType::ImageSampler, 0, "", bindingIndex, false}, layoutBinding});
	textureBindings[stringName] = bindingIndex;
	for (uint32_t index = 0; index < descriptorCount; index++)
	{
		textureBindings[stringName + "[" + std::to_string(index) + "]"] = bindingIndex + index;
	}
}
void VulkanRenderer::setSSBO(shaders::Shader& shader, vaos::VAO& vao, const std::string_view name, const void* pointer,
														 size_t size)
{
	auto& shaderImpl = *(VulkanShaderImpl*)shader.rendererData;
	auto& vaoImpl = *(VulkanVAOImpl*)vao.rendererData;
	std::string stringName(name);
	auto data = vaos::VAO::getShaderData(shader.iRenderer);
	auto& ssboBindings = shaderImpl.getSsboBindings(data);
	auto ssboIter = ssboBindings.find(stringName);
	if (ssboIter == ssboBindings.end())
	{
		return;
	}
	auto& ssboBuffers = vaoImpl.getSsboBuffers(data);
	auto ssboBufferIter = ssboBuffers.find(stringName);
	if (ssboBufferIter == ssboBuffers.end())
	{
		ssboBuffers[stringName] = {0, 0};
		ssboBufferIter = ssboBuffers.find(stringName);
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
	descriptorWrite.dstSet = vaoImpl.getDescriptorSet(data);
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
	auto data = vaos::VAO::getShaderData(vao.vaoIRenderer);
	auto& textureBindings = shaderImpl.getTextureBindings(data);
	auto bindingIndexIter = textureBindings.find(stringName);
	if (bindingIndexIter == textureBindings.end())
		return;
	auto& bindingIndex = bindingIndexIter->second;
	if (!bindingIndex)
		return;
	auto& vaoImpl = *(VulkanVAOImpl*)vao.rendererData;
	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = vaoImpl.getDescriptorSet(data);
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
	auto& shaderImpl = *static_cast<VulkanShaderImpl*>(shader.rendererData);
	auto data = vaos::VAO::getShaderData(shader.iRenderer);
	auto& uboLayoutBindings = shaderImpl.getUboLayoutBindings(data);
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
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
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
	if (cullMode == zg::NOCULL)
		rasterizer.cullMode = VK_CULL_MODE_NONE;
	else if (cullMode == zg::BACK)
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	else if (cullMode == zg::FRONTANDBACK)
		rasterizer.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
	else if (cullMode == zg::FRONT)
		rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
	rasterizer.frontFace = frontFace == zg::CLOCKWISE ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	VkSampleCountFlagBits samples;
	if (currentFramebufferImpl)
	{
		textures::Texture* textureP = 0;
		textureP = (textures::Texture*)currentFramebufferImpl->zgFramebuffer->getColorTexture();
		if (!textureP)
			textureP = (textures::Texture*)currentFramebufferImpl->zgFramebuffer->getDepthTexture();
		samples = TextureMultisamplingToSampleCountBit(textureP->multisampling);
		if (samples > maxMSAASamples)
			samples = maxMSAASamples;
	}
	else
		samples = VK_SAMPLE_COUNT_1_BIT;//maxMSAASamples;
	multisampling.rasterizationSamples = samples;
	multisampling.sampleShadingEnable = VK_TRUE;
	multisampling.minSampleShading = 0.2f;
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
	for (auto& uboLayoutBinding : uboLayoutBindings)
	{
		layoutBindings.push_back(uboLayoutBinding.second);
	}
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
	descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo.bindingCount = layoutBindings.size();
	descriptorSetLayoutInfo.pBindings = layoutBindings.data();
	auto& descriptorSetLayout = shaderImpl.getDescriptorSetLayout(data);
	if (!VKcheck("vkCreateDescriptorSetLayout",
							 _vkCreateDescriptorSetLayout(device, &descriptorSetLayoutInfo, 0, &descriptorSetLayout)))
	{
		throw std::runtime_error("Failed to create descriptor set layout");
	}
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
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
	VkPipelineDepthStencilStateCreateInfo depthStencilState{};
	bool hasDepthAttachment = false;
	if ((currentFramebufferImpl && currentFramebufferImpl->zgFramebuffer->hasDepthAttachment()) ||
			!currentFramebufferImpl)
	{
		hasDepthAttachment = true;
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.stencilTestEnable = VK_FALSE;
		depthStencilState.front = {};
		depthStencilState.back = {};
		pipelineInfo.pDepthStencilState = &depthStencilState;
	}
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
		[&, descriptorSetLayouts = shaderImpl.descriptorSetLayouts, graphicsPipeline = shaderImpl.graphicsPipeline,
		 pipelineLayout = shaderImpl.pipelineLayout, shaders = shaderImpl.shaders]
		{
			for (auto& pair : descriptorSetLayouts)
			{
				_vkDestroyDescriptorSetLayout(device, pair.second, 0);
			}
			_vkDestroyPipeline(device, graphicsPipeline, 0);
			_vkDestroyPipelineLayout(device, pipelineLayout, 0);
			for (auto& shaderPair : shaders)
				_vkDestroyShaderModule(device, (VkShaderModule)shaderPair.second.second, 0);
		});
	delete &shaderImpl;
}
void VulkanRenderer::bindFramebuffer(const textures::Framebuffer& framebuffer)
{
	transitionColorLayoutForWriting(framebuffer);
	transitionDepthLayoutForWriting(framebuffer);
	transitionColorResolveLayoutForWriting(framebuffer);
	transitionDepthResolveLayoutForWriting(framebuffer);
	auto& framebufferImpl = *(const VulkanFramebufferImpl*)framebuffer.rendererData;
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = framebufferImpl.renderPass;
	renderPassInfo.framebuffer = framebufferImpl.framebuffer;
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent.width = framebufferImpl.width;
	renderPassInfo.renderArea.extent.height = framebufferImpl.height;
	std::vector<VkClearValue> clearValues;
	for (auto& pair : framebuffer.textureAttachmentPairs)
	{
		auto& texture = *pair.first;
		VkClearValue clearValue;
		switch (pair.second)
		{
		case textures::Framebuffer::AttachmentType::Depth:
		case textures::Framebuffer::AttachmentType::DepthStencil:
		case textures::Framebuffer::AttachmentType::Stencil:
			{
				clearValue.depthStencil = {1.0f, 0};
				break;
			}
		case textures::Framebuffer::AttachmentType::Color:
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
	}
	renderPassInfo.clearValueCount = clearValues.size();
	renderPassInfo.pClearValues = clearValues.data();
	_vkCmdBeginRenderPass(*commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	currentFramebufferImpl = &framebufferImpl;
	((VulkanFramebufferImpl*)currentFramebufferImpl)->zgFramebuffer = (zg::textures::Framebuffer*)&framebuffer;
}
void VulkanRenderer::unbindFramebuffer(const textures::Framebuffer& framebuffer)
{
	_vkCmdEndRenderPass(*commandBuffer);
	transitionColorLayoutForReading(framebuffer);
	transitionDepthLayoutForReading(framebuffer);
	transitionColorResolveLayoutForReading(framebuffer);
	transitionDepthResolveLayoutForReading(framebuffer);
	currentFramebufferImpl = 0;
}
void VulkanRenderer::initFramebuffer(zg::textures::Framebuffer& framebuffer)
{
	if (framebuffer.rendererData)
	{
		throw std::runtime_error("Framebuffer already initialized!");
	}
	framebuffer.rendererData = new VulkanFramebufferImpl();
	auto& framebufferImpl = *static_cast<VulkanFramebufferImpl*>(framebuffer.rendererData);
	framebufferImpl.zgFramebuffer = &framebuffer;
	size_t renderPassHash = 0;
	auto combine_hash = [&](size_t seed, auto val)
	{ return seed ^ (std::hash<decltype(val)>{}(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2)); };
	std::vector<VkAttachmentDescription2> vkAttachments;
	std::vector<VkAttachmentReference2> colorAttachmentRefs;
	std::vector<VkAttachmentReference2> resolveAttachmentRefs;
	VkAttachmentReference2 depthStencilRef{};
	VkSubpassDescriptionDepthStencilResolve subpassDepthStencilResolve{};
	bool requiresDepthResolve = false;
	subpassDepthStencilResolve.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE;
	VkAttachmentReference2 depthResolveRef{};
	depthStencilRef.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
	bool hasDepthStencil = false;
	uint32_t attachmentIndex = 0;
	VkPipelineStageFlags inputSrcStageMask = 0;
	VkPipelineStageFlags inputDstStageMask = 0;
	VkAccessFlags inputDstAccessMask = 0;
	VkPipelineStageFlags outputSrcStageMask = 0;
	VkPipelineStageFlags outputDstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	VkAccessFlags outputSrcAccessMask = 0;
	VkAccessFlags outputDstAccessMask = 0;
	std::vector<VkImageView> vkImageViews;
	vkImageViews.reserve(framebuffer.textureAttachmentPairs.size());
	for (const auto& pair : framebuffer.textureAttachmentPairs)
	{
		if (!pair.first || !pair.first->rendererData)
		{
			throw std::runtime_error("Framebuffer attachment texture is null or not initialized!");
		}
		auto& texture = *pair.first;
		auto& textureImpl = *static_cast<VulkanTextureImpl*>(texture.rendererData);
		if (textureImpl.textureImageView == VK_NULL_HANDLE)
		{
			throw std::runtime_error("Framebuffer attachment texture image view is null!");
		}
		vkImageViews.push_back(textureImpl.textureImageView);
		VkAttachmentDescription2 attachment{};
		attachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Store results
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		if (framebufferImpl.width == 0 && framebufferImpl.height == 0 && texture.size.x > 0 && texture.size.y > 0)
		{
			framebufferImpl.width = texture.size.x;
			framebufferImpl.height = texture.size.y;
		}
		else if ((framebufferImpl.width != texture.size.x || framebufferImpl.height != texture.size.y) &&
						 texture.size.x > 0 && texture.size.y > 0)
		{
			throw std::runtime_error("Framebuffer attachments have inconsistent dimensions!");
		}
		VkImageLayout subpassLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.format = textureImpl.format;
		if (attachment.format == VK_FORMAT_UNDEFINED)
			throw std::runtime_error("Attachment format is undefined!");
		switch (pair.second)
		{
		case zg::textures::Framebuffer::AttachmentType::ColorResolve:
		{
			attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			subpassLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			VkAttachmentReference2 _ref_{};
			_ref_.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
			_ref_.attachment = attachmentIndex;
			_ref_.layout = subpassLayout;
			resolveAttachmentRefs.push_back(_ref_);
			break;
		}
		case zg::textures::Framebuffer::AttachmentType::DepthResolve:
		{
			attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			subpassLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthResolveRef.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
			depthResolveRef.attachment = attachmentIndex;
			depthResolveRef.layout = subpassLayout;
			subpassDepthStencilResolve.depthResolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
			subpassDepthStencilResolve.pDepthStencilResolveAttachment = &depthResolveRef;
			requiresDepthResolve = true;
			break;
		}
		case zg::textures::Framebuffer::AttachmentType::Color:
		{
			attachment.samples = TextureMultisamplingToSampleCountBit(texture.multisampling);
			if (attachment.samples > maxMSAASamples)
				attachment.samples = maxMSAASamples;
			attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachment.finalLayout = (framebuffer.hasColorResolveAttachment() ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			subpassLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			VkAttachmentReference2 _ref_{};
			_ref_.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
			_ref_.attachment = attachmentIndex;
			_ref_.layout = subpassLayout;
			colorAttachmentRefs.push_back(_ref_);
			inputSrcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			inputDstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			inputDstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			outputSrcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			outputSrcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			outputDstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			outputDstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		}

		case zg::textures::Framebuffer::AttachmentType::Depth:
		case zg::textures::Framebuffer::AttachmentType::DepthStencil:
		case zg::textures::Framebuffer::AttachmentType::Stencil:
		{
			if (hasDepthStencil)
				throw std::runtime_error("Framebuffer cannot have multiple depth/stencil attachments!");
			attachment.samples = TextureMultisamplingToSampleCountBit(texture.multisampling);
			if (attachment.samples > maxMSAASamples)
				attachment.samples = maxMSAASamples;
			attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			attachment.finalLayout = (framebuffer.hasDepthResolveAttachment() ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			subpassLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			if (pair.second != zg::textures::Framebuffer::AttachmentType::Depth)
			{
				attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
			}
			VkAttachmentReference2 _ref_{};
			_ref_.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
			_ref_.attachment = attachmentIndex;
			_ref_.layout = subpassLayout;
			depthStencilRef = _ref_;
			hasDepthStencil = true;
			inputSrcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			inputDstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			inputDstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			outputSrcStageMask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			outputSrcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			// Adjust output dependency if final layout is ATTACHMENT_OPTIMAL
			// Next stage could still be early fragment tests if reused immediately
			outputDstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			outputDstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
		}

		default:
			throw std::runtime_error("Unsupported attachment type");
		}
		vkAttachments.push_back(attachment);

		// --- Update Hash ---
		renderPassHash = combine_hash(renderPassHash, attachment.flags);
		renderPassHash = combine_hash(renderPassHash, attachment.format);
		// ... hash other attachment fields ...
		renderPassHash = combine_hash(renderPassHash, attachment.finalLayout);
		attachmentIndex++;
	}

	// --- Define Subpass ---
	VkSubpassDescription2 subpass{};
	subpass.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
	subpass.pColorAttachments = colorAttachmentRefs.empty() ? nullptr : colorAttachmentRefs.data();
	subpass.pDepthStencilAttachment = hasDepthStencil ? &depthStencilRef : nullptr;
	subpass.pResolveAttachments = resolveAttachmentRefs.data();
	if (requiresDepthResolve)
		subpass.pNext = &subpassDepthStencilResolve;

	// --- Define Dependencies ---
	std::array<VkSubpassDependency2, 2> dependencies;
	memset(dependencies.data(), 0, sizeof(VkSubpassDependency2) * 2);
	// Input Dependency (External -> 0)
	dependencies[0].sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = inputSrcStageMask;
	dependencies[0].dstStageMask = inputDstStageMask;
	dependencies[0].srcAccessMask = 0;
	dependencies[0].dstAccessMask = inputDstAccessMask;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	// Output Dependency (0 -> External)
	dependencies[1].sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = outputSrcStageMask;
	dependencies[1].dstStageMask = outputDstStageMask; // Updated based on finalLayout
	dependencies[1].srcAccessMask = outputSrcAccessMask;
	dependencies[1].dstAccessMask = outputDstAccessMask; // Updated based on finalLayout
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// --- Update Hash with Dependencies ---
	for (const auto& dep : dependencies)
	{
		renderPassHash = combine_hash(renderPassHash, dep.srcSubpass);
		// ... hash other dependency fields ...
		renderPassHash = combine_hash(renderPassHash, dep.dependencyFlags);
	}
	renderPassHash = combine_hash(renderPassHash, subpass.pipelineBindPoint);
	renderPassHash = combine_hash(renderPassHash, subpass.colorAttachmentCount);

	// --- Create/Cache RenderPass ---
	auto renderPassIter = renderPassMap.find(renderPassHash);
	if (renderPassIter != renderPassMap.end())
	{
		framebufferImpl.renderPass = renderPassIter->second;
	}
	else
	{
		VkRenderPassCreateInfo2 renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(vkAttachments.size());
		renderPassInfo.pAttachments = vkAttachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		if (!VKcheck("vkCreateRenderPass2",
								 _vkCreateRenderPass2(device, &renderPassInfo, nullptr, &framebufferImpl.renderPass)))
		{
			delete static_cast<VulkanFramebufferImpl*>(framebuffer.rendererData);
			framebuffer.rendererData = nullptr;
			throw std::runtime_error("VulkanRenderer::initFramebuffer - failed to create render pass!");
		}
		renderPassMap[renderPassHash] = framebufferImpl.renderPass;
	}

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = framebufferImpl.renderPass;
	framebufferInfo.attachmentCount = static_cast<uint32_t>(vkImageViews.size());
	framebufferInfo.pAttachments = vkImageViews.data();
	framebufferInfo.width = framebufferImpl.width;
	framebufferInfo.height = framebufferImpl.height;
	framebufferInfo.layers = 1;

	if (!VKcheck("vkCreateFramebuffer",
							 _vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebufferImpl.framebuffer)))
	{
		delete static_cast<VulkanFramebufferImpl*>(framebuffer.rendererData);
		framebuffer.rendererData = nullptr;
		throw std::runtime_error("VulkanRenderer::initFramebuffer - failed to create framebuffer!");
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
void VulkanRenderer::createImage(uint32_t width, uint32_t height, VkSampleCountFlagBits numSamples, VkFormat format,
																 VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
																 VkImage& image, VkDeviceMemory& imageMemory)
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
	imageInfo.samples = numSamples;
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
	case textures::Texture::Format::Stencil:
		{
			candidates.push_back(VK_FORMAT_R8_UINT);
			break;
		};
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
	default:
		{
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
bool hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VulkanRenderer::prepareImageBarrier(VkCommandBuffer commandBuffer, VkImage image, VkFormat format,
																				 VkImageLayout oldLayout, VkImageLayout newLayout,
																				 VkImageAspectFlags aspectMask, VkPipelineStageFlags& sourceStage,
																				 VkPipelineStageFlags& destinationStage, VkImageMemoryBarrier& barrier)
{
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
	barrier.pNext = nullptr; // Initialize pNext

	sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	// Determine Source Access Mask and Stage based on oldLayout
	switch (oldLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		barrier.srcAccessMask = 0;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		break;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_HOST_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Or other relevant shader stage
		break;
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		break;
	default:
		barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		break;
	}

	// Determine Destination Access Mask and Stage based on newLayout
	switch (newLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT; // Or SHADER_READ
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // Or FRAGMENT_SHADER
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Or other relevant shader stage
		break;
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		break;
	default:
		barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
		destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		break;
	}

	// If transitioning from UNDEFINED, srcStage must be TOP_OF_PIPE_BIT
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED)
	{
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}
}
void VulkanRenderer::transitionImageLayout(VulkanTextureImpl& textureImpl, VkImage image, VkFormat format,
																					 VkImageLayout oldLayout, VkImageLayout newLayout,
																					 VkImageAspectFlags aspectMask)
{
	if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		newLayout = newLayout;
	}
	if (oldLayout == newLayout)
	{
		textureImpl.layout = newLayout; // Ensure tracked layout is correct
		return;
	}

	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier;
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	prepareImageBarrier(commandBuffer, image, format, oldLayout, newLayout, aspectMask, sourceStage, destinationStage,
											barrier);


	_vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage,
												0, // No dependency flags
												0, nullptr, // No memory barriers
												0, nullptr, // No buffer memory barriers
												1, &barrier); // One image memory barrier

	endSingleTimeCommands(commandBuffer);

	// Update the tracked layout *after* the command buffer is submitted and waited on
	textureImpl.layout = newLayout;
}
void VulkanRenderer::preInitTexture(textures::Texture& texture)
{
	if (texture.rendererData)
	{
		throw std::runtime_error("Texture already initialized!");
	}

	// *** ASSUMPTION: zg::textures::Texture now has a member like: ***
	// bool isFramebufferAttachment = false;
	// You must set this boolean correctly when creating the Texture object.

	texture.rendererData = new VulkanTextureImpl();
	auto& textureImpl = *static_cast<VulkanTextureImpl*>(texture.rendererData);
	textureImpl.layout = VK_IMAGE_LAYOUT_UNDEFINED; // Initialize tracked layout

	VkFormat format = VK_FORMAT_UNDEFINED;
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkImageUsageFlags usage = 0;
	VkImageLayout initialTargetLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Determined below
	VkImageAspectFlags aspectMask = 0;

	// --- Determine Format ---
	auto formatIt = textureFormat_Format.find(texture.format);
	if (formatIt != textureFormat_Format.end())
	{
		format = formatIt->second;
	}
	else
	{
		delete static_cast<VulkanTextureImpl*>(texture.rendererData);
		texture.rendererData = nullptr;
		throw std::runtime_error("Unsupported texture format enum!");
	}
	textureImpl.format = format;

	// --- Determine Aspect Mask ---
	auto aspectIt = textureFormat_imageAspect.find(texture.format);
	if (aspectIt != textureFormat_imageAspect.end())
	{
		aspectMask = aspectIt->second;
	}
	else
	{
		delete static_cast<VulkanTextureImpl*>(texture.rendererData);
		texture.rendererData = nullptr;
		throw std::runtime_error("Unsupported texture format enum for aspect mask!");
	}

	// --- Determine Usage Flags and Initial Target Layout based on Format and isFramebufferAttachment ---
	// Base usage: Always allow sampling and transfer destination/source
	usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	// Determine based on format and intended use (framebuffer attachment or general texture)
	switch (texture.format)
	{
	case zg::textures::Texture::Format::RGBA8:
		tiling = VK_IMAGE_TILING_OPTIMAL;
		if (texture.isFramebufferAttachment)
		{ // Check the boolean flag
			usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			initialTargetLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		else
		{
			// Default for non-attachment: Ready for sampling after potential upload
			initialTargetLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			// Ensure TRANSFER_DST is set for potential upload before sampling
			// (already included in base usage)
		}
		break;

	case zg::textures::Texture::Format::Depth:
	case zg::textures::Texture::Format::DepthStencil:
	case zg::textures::Texture::Format::Stencil:
		tiling = VK_IMAGE_TILING_OPTIMAL; // Required for depth/stencil
		if (texture.isFramebufferAttachment)
		{ // Check the boolean flag
			usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			initialTargetLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			// Non-attachment depth maps might be sampled (e.g., shadow maps)
			// Or transferred. Default to SHADER_READ_ONLY after potential transfer.
			initialTargetLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			// Ensure TRANSFER_DST is set (already included)
			// Note: Sampling depth often requires specific sampler/shader setup.
		}
		break;
	default:
		// Default for unknown formats: Assume general texture, ready for sampling
		initialTargetLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		break;
	}


	// --- Create Image ---
	auto pref_samples = TextureMultisamplingToSampleCountBit(texture.multisampling);
	if (pref_samples > maxMSAASamples)
		pref_samples = maxMSAASamples;
	createImage(texture.size.x, texture.size.y, pref_samples, format, tiling, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
							textureImpl.textureImage, textureImpl.textureImageMemory);

	// --- Create Image View ---
	textureImpl.textureImageView = createImageView(textureImpl.textureImage, format, aspectMask);
	if (textureImpl.textureImageView == VK_NULL_HANDLE)
	{
		vkDestroyImage(device, textureImpl.textureImage, nullptr);
		vkFreeMemory(device, textureImpl.textureImageMemory, nullptr);
		delete static_cast<VulkanTextureImpl*>(texture.rendererData);
		texture.rendererData = nullptr;
		throw std::runtime_error("Failed to create texture image view!");
	}

	// --- Perform Initial Layout Transition ---
	// Transition from UNDEFINED to the determined initial target layout.
	transitionImageLayout(textureImpl, textureImpl.textureImage, format, VK_IMAGE_LAYOUT_UNDEFINED,
												initialTargetLayout, // Use determined layout
												aspectMask);
	// textureImpl.layout is now updated by transitionImageLayout


	// --- Create Sampler (if applicable) ---
	if (usage & VK_IMAGE_USAGE_SAMPLED_BIT)
	{
		VkPhysicalDeviceProperties properties{};
		_vkGetPhysicalDeviceProperties(physicalDevice, &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		VkFilter filter;
		VkSamplerMipmapMode mipmapMode;
		switch (texture.filterType)
		{
		case zg::textures::Texture::FilterType::Linear:
			filter = VK_FILTER_LINEAR;
			mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		case zg::textures::Texture::FilterType::Nearest:
		default:
			filter = VK_FILTER_NEAREST;
			mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		}
		samplerInfo.magFilter = filter;
		samplerInfo.minFilter = filter;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = mipmapMode;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;


		if (!VKcheck("vkCreateSampler", _vkCreateSampler(device, &samplerInfo, nullptr, &textureImpl.textureSampler)))
		{
			vkDestroyImageView(device, textureImpl.textureImageView, nullptr);
			vkDestroyImage(device, textureImpl.textureImage, nullptr);
			vkFreeMemory(device, textureImpl.textureImageMemory, nullptr);
			delete static_cast<VulkanTextureImpl*>(texture.rendererData);
			texture.rendererData = nullptr;
			throw std::runtime_error("failed to create texture sampler!");
		}
	}
	else
	{
		textureImpl.textureSampler = VK_NULL_HANDLE;
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
void VulkanRenderer::postInitTexture(const textures::Texture& texture) { texture.bind(); }
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
	for (size_t index = offset, c = 1, elementIndex = 0; c <= vao.vertexCount;
			 index += vao.stride, c++, elementIndex += elementStride)
	{
		memcpy((char*)vaoImpl.vertexData + index, &elementsAsChar[elementIndex], elementStride);
	}
}
void VulkanRenderer::drawVAO(const vaos::VAO& vao)
{
	auto& vaoImpl = *(VulkanVAOImpl*)vao.rendererData;
	auto data = vaos::VAO::getShaderData(vao.vaoIRenderer);
	auto& shader = *((vaos::VAO&)vao).addShader();
	auto& shaderImpl = *(VulkanShaderImpl*)(shader.rendererData);
	if (vaoImpl.vertexBuffer == VK_NULL_HANDLE)
	{
		return;
	}
	VkBuffer vertexBuffers[] = {vaoImpl.vertexBuffer};
	VkDeviceSize offsets[] = {0};
	_vkCmdBindVertexBuffers(*commandBuffer, 0, 1, vertexBuffers, offsets);
	_vkCmdBindIndexBuffer(*commandBuffer, vaoImpl.indiceBuffer, 0, VK_INDEX_TYPE_UINT32);
	_vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderImpl.pipelineLayout, 0, 1,
													 &vaoImpl.getDescriptorSet(data), 0, 0);
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
	VkDeviceSize vertexBufferSize = stride * vao.vertexCount;
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
		[&, ssboBuffers = vaoImpl.ssboBuffers, uniformBuffers = vaoImpl.uniformBuffers,
		 uniformBuffersMemory = vaoImpl.uniformBuffersMemory, descriptorPools = vaoImpl.descriptorPools,
		 indiceBuffer = vaoImpl.indiceBuffer, indiceBufferMemory = vaoImpl.indiceBufferMemory,
		 vertexBuffer = vaoImpl.vertexBuffer, vertexBufferMemory = vaoImpl.vertexBufferMemory]
		{
			for (auto& pair : ssboBuffers)
			{
				for (auto& pair2 : pair.second)
				{
					_vkDestroyBuffer(device, pair2.second.first, 0);
					_vkFreeMemory(device, pair2.second.second, 0);
				}
			}
			for (auto& pair : uniformBuffers)
				for (auto& uniformBuffer : pair.second)
					_vkDestroyBuffer(device, uniformBuffer, 0);
			for (auto& pair : uniformBuffersMemory)
				for (auto& uniformBufferMemory : pair.second)
					_vkFreeMemory(device, uniformBufferMemory, 0);
			for (auto& pair : descriptorPools)
			{
				_vkDestroyDescriptorPool(device, pair.second, 0);
			}
			_vkDestroyBuffer(device, indiceBuffer, 0);
			_vkFreeMemory(device, indiceBufferMemory, 0);
			_vkDestroyBuffer(device, vertexBuffer, 0);
			_vkFreeMemory(device, vertexBufferMemory, 0);
		});
	delete &vaoImpl;
}
void VulkanRenderer::ensureVAO(shaders::Shader& shader, vaos::VAO& vao)
{
	if (!shader.compiled)
		return;
	if (vao.isEnsured())
		return;
	auto data = vaos::VAO::getShaderData(vao.vaoIRenderer);
	auto& shaderImpl = *(VulkanShaderImpl*)shader.rendererData;
	auto& vaoImpl = *(VulkanVAOImpl*)vao.rendererData;
	auto& uboLayoutBindings = shaderImpl.getUboLayoutBindings(data);
	auto& ssboBindings = shaderImpl.getSsboBindings(data);
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
	for (auto& uboLayoutBinding : uboLayoutBindings)
	{
		auto& layoutBinding = uboLayoutBinding.second;
		auto& poolSize = getPoolSize(layoutBinding.descriptorType);
		poolSize.descriptorCount++;
	}
	if (ssboBindings.size())
	{
		auto layoutBinding = getPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutBinding.descriptorCount = ssboBindings.size();
	}

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = uboLayoutBindings.size() + ssboBindings.size();

	auto& descriptorPool = vaoImpl.getDescriptorPool(data);

	if (!VKcheck("vkCreateDescriptorPool", _vkCreateDescriptorPool(device, &poolInfo, 0, &descriptorPool)))
	{
		throw std::runtime_error("Failed to create descriptor pool!");
	}

	auto& descriptorSet = vaoImpl.getDescriptorSet(data);

	std::vector<VkDescriptorSetLayout> layouts(1, shaderImpl.getDescriptorSetLayout(data));
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layouts[0];

	if (!VKcheck("vkAllocateDescriptorSets", _vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet)))
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	auto& bufferInfos = vaoImpl.getBufferInfos(data);
	auto& uniformBuffers = vaoImpl.getUniformBuffers(data);
	auto& uniformBuffersMemory = vaoImpl.getUniformBuffersMemory(data);
	auto& uniformBuffersMapped = vaoImpl.getUniformBuffersMapped(data);
	auto& uniformLocationTable = vaoImpl.getUniformLocationTable(data);
	for (auto uboLayoutBindingPair : uboLayoutBindings)
	{
		if (std::get<0>(uboLayoutBindingPair.first) != ELayoutBindingType::UniformBuffer)
		{
			continue;
		}
		auto& descriptorSetLayoutBinding = uboLayoutBindingPair.second;
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
			uniformLocationTable[std::get<2>(uboLayoutBindingPair.first)] = uniformBuffersMappedIndex;
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
				uniformLocationTable[std::get<2>(uboLayoutBindingPair.first) + "[" + std::to_string(index) + "]"] =
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
	vao.setEnsured();
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
	if (result == VK_SUCCESS)
	{
		return true;
	}

	std::string resultString;
	switch (result)
	{
	// Success Codes (should not normally be handled here unless logic changes)
	// case VK_SUCCESS: resultString = "VK_SUCCESS"; break; // Handled above
	case VK_NOT_READY:
		resultString = "VK_NOT_READY";
		break;
	case VK_TIMEOUT:
		resultString = "VK_TIMEOUT";
		break;
	case VK_EVENT_SET:
		resultString = "VK_EVENT_SET";
		break;
	case VK_EVENT_RESET:
		resultString = "VK_EVENT_RESET";
		break;
	case VK_INCOMPLETE:
		resultString = "VK_INCOMPLETE";
		break;
	case VK_SUBOPTIMAL_KHR:
		resultString = "VK_SUBOPTIMAL_KHR";
		break; // Often needs special handling (like recreating swapchain) but is not a fatal error
	case VK_PIPELINE_COMPILE_REQUIRED:
		resultString = "VK_PIPELINE_COMPILE_REQUIRED";
		break; // Or VK_PIPELINE_COMPILE_REQUIRED_EXT
	case VK_THREAD_IDLE_KHR:
		resultString = "VK_THREAD_IDLE_KHR";
		break;
	case VK_THREAD_DONE_KHR:
		resultString = "VK_THREAD_DONE_KHR";
		break;
	case VK_OPERATION_DEFERRED_KHR:
		resultString = "VK_OPERATION_DEFERRED_KHR";
		break;
	case VK_OPERATION_NOT_DEFERRED_KHR:
		resultString = "VK_OPERATION_NOT_DEFERRED_KHR";
		break;

	// Error Codes
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		resultString = "VK_ERROR_OUT_OF_HOST_MEMORY";
		break;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		resultString = "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		break;
	case VK_ERROR_INITIALIZATION_FAILED:
		resultString = "VK_ERROR_INITIALIZATION_FAILED";
		break;
	case VK_ERROR_DEVICE_LOST:
		resultString = "VK_ERROR_DEVICE_LOST";
		break;
	case VK_ERROR_MEMORY_MAP_FAILED:
		resultString = "VK_ERROR_MEMORY_MAP_FAILED";
		break;
	case VK_ERROR_LAYER_NOT_PRESENT:
		resultString = "VK_ERROR_LAYER_NOT_PRESENT";
		break;
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		resultString = "VK_ERROR_EXTENSION_NOT_PRESENT";
		break;
	case VK_ERROR_FEATURE_NOT_PRESENT:
		resultString = "VK_ERROR_FEATURE_NOT_PRESENT";
		break;
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		resultString = "VK_ERROR_INCOMPATIBLE_DRIVER";
		break;
	case VK_ERROR_TOO_MANY_OBJECTS:
		resultString = "VK_ERROR_TOO_MANY_OBJECTS";
		break;
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		resultString = "VK_ERROR_FORMAT_NOT_SUPPORTED";
		break;
	case VK_ERROR_FRAGMENTED_POOL:
		resultString = "VK_ERROR_FRAGMENTED_POOL";
		break;
	case VK_ERROR_UNKNOWN:
		resultString = "VK_ERROR_UNKNOWN";
		break;
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		resultString = "VK_ERROR_OUT_OF_POOL_MEMORY";
		break; // Alias for VK_ERROR_OUT_OF_POOL_MEMORY_KHR
	case VK_ERROR_INVALID_EXTERNAL_HANDLE:
		resultString = "VK_ERROR_INVALID_EXTERNAL_HANDLE";
		break; // Alias for VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR
	case VK_ERROR_FRAGMENTATION:
		resultString = "VK_ERROR_FRAGMENTATION";
		break; // Alias for VK_ERROR_FRAGMENTATION_EXT
	case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
		resultString = "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
		break; // Alias for VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR
	case VK_ERROR_SURFACE_LOST_KHR:
		resultString = "VK_ERROR_SURFACE_LOST_KHR";
		break;
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		resultString = "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		break;
	case VK_ERROR_OUT_OF_DATE_KHR:
		resultString = "VK_ERROR_OUT_OF_DATE_KHR";
		break; // Often needs special handling (recreating swapchain)
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		resultString = "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		break;
	case VK_ERROR_VALIDATION_FAILED_EXT:
		resultString = "VK_ERROR_VALIDATION_FAILED_EXT";
		break;
	case VK_ERROR_INVALID_SHADER_NV:
		resultString = "VK_ERROR_INVALID_SHADER_NV";
		break;
	case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
		resultString = "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
		break;
	case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
		resultString = "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
		break;
	case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:
		resultString = "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
		break;
	case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:
		resultString = "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
		break;
	case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:
		resultString = "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
		break;
	case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:
		resultString = "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
		break;
	case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
		resultString = "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
		break;
	case VK_ERROR_NOT_PERMITTED_KHR:
		resultString = "VK_ERROR_NOT_PERMITTED_KHR";
		break; // Or VK_ERROR_NOT_PERMITTED_EXT
	case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
		resultString = "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
		break;
	case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
		resultString = "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
		break;
	case VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT:
		resultString = "VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT";
		break;
		// Add other VkResult values as needed based on extensions you use

	default:
		resultString = "Unknown VkResult code: " + std::to_string(result);
		break;
	}

	// Print error message to standard error stream
	std::cerr << "Vulkan Error: Function '" << fn << "' failed with " << resultString << " (" << result << ")"
						<< std::endl;

	return false; // Indicate failure
}
// #endif
void VulkanRenderer::transitionDepthLayoutForWriting(const textures::Framebuffer& framebuffer)
{
	auto shadowMapTexture = framebuffer.getDepthTexture();
	if (shadowMapTexture && shadowMapTexture->rendererData)
	{
		auto& textureImpl = *static_cast<VulkanTextureImpl*>(shadowMapTexture->rendererData);
		// Check if transition is needed (if it was left in READ_ONLY from previous frame)
		if (textureImpl.layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
		{
			VkImageMemoryBarrier barrier;
			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;
			prepareImageBarrier(*commandBuffer, textureImpl.textureImage, textureImpl.format,
													VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, // Old layout
													VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, // New layout for attachment use
													VK_IMAGE_ASPECT_DEPTH_BIT, sourceStage, destinationStage, barrier);
			vkCmdPipelineBarrier(*commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
			textureImpl.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Update tracked layout
		}
		else if (textureImpl.layout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			// Warn if it's in some other unexpected state
			std::cerr << "Warning: Directional shadow map in unexpected layout (" << textureImpl.layout
								<< ") before shadow pass.\n";
		}
	}
}
void VulkanRenderer::transitionDepthLayoutForReading(const textures::Framebuffer& framebuffer)
{
	auto shadowMapTexture = framebuffer.getDepthTexture();
	if (shadowMapTexture && shadowMapTexture->rendererData)
	{
		auto& textureImpl = *static_cast<VulkanTextureImpl*>(shadowMapTexture->rendererData);

		if (!framebuffer.hasDepthResolveAttachment())
		{
			textureImpl.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		// Ensure the layout we are transitioning *from* is correct
		if (textureImpl.layout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL &&
			textureImpl.layout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			// Log warning or throw? Indicates layout tracking might be off.
			// For now, let's assume it *should* be ATTACHMENT_OPTIMAL here.
			std::cerr << "Warning: Directional shadow map layout was not ATTACHMENT_OPTIMAL before transition!" << std::endl;
		}

		VkImageLayout oldLayout = textureImpl.layout;
		VkImageLayout newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		VkImageMemoryBarrier barrier;
		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		// Prepare barrier details: ATTACHMENT_OPTIMAL -> READ_ONLY_OPTIMAL
		prepareImageBarrier(*commandBuffer, textureImpl.textureImage, textureImpl.format,
												oldLayout, // Old layout
												newLayout, // New layout for sampling
												VK_IMAGE_ASPECT_DEPTH_BIT, // Assuming depth only
												sourceStage, destinationStage, barrier);

		// Record the barrier in the current command buffer
		vkCmdPipelineBarrier(*commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		// Update tracked layout
		textureImpl.layout = newLayout;
	}
}
void VulkanRenderer::transitionColorLayoutForWriting(const textures::Framebuffer& framebuffer)
{
	auto colorTexture = framebuffer.getColorTexture();
	if (colorTexture && colorTexture->rendererData)
	{
		auto& textureImpl = *static_cast<VulkanTextureImpl*>(colorTexture->rendererData);
		if (textureImpl.layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			VkImageMemoryBarrier colorBarrier = {};
			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;
			prepareImageBarrier(*commandBuffer, textureImpl.textureImage, textureImpl.format,
													VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
													VK_IMAGE_ASPECT_COLOR_BIT, sourceStage, destinationStage, colorBarrier);
			vkCmdPipelineBarrier(*commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &colorBarrier);
			textureImpl.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		else if (textureImpl.layout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
						 textureImpl.layout != VK_IMAGE_LAYOUT_UNDEFINED)
		{
			std::cerr << "Warning: Offscreen color texture in unexpected layout (" << textureImpl.layout
								<< ") before Pass 1.\n";
		}
	}
}
void VulkanRenderer::transitionColorLayoutForReading(const textures::Framebuffer& framebuffer)
{
	auto colorTexture = framebuffer.getColorTexture();
	if (colorTexture && colorTexture->rendererData && !framebuffer.hasColorResolveAttachment())
	{
		auto& textureImpl = *static_cast<VulkanTextureImpl*>(colorTexture->rendererData);
		textureImpl.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
}
void VulkanRenderer::transitionColorResolveLayoutForWriting(const textures::Framebuffer& framebuffer)
{
	auto colorResolveTexture = framebuffer.getColorResolveTexture();
	if (colorResolveTexture && colorResolveTexture->rendererData)
	{
		auto& textureImpl = *static_cast<VulkanTextureImpl*>(colorResolveTexture->rendererData);
		if (textureImpl.layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			VkImageMemoryBarrier colorBarrier = {};
			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;
			prepareImageBarrier(*commandBuffer, textureImpl.textureImage, textureImpl.format,
													VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
													VK_IMAGE_ASPECT_COLOR_BIT, sourceStage, destinationStage, colorBarrier);
			vkCmdPipelineBarrier(*commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &colorBarrier);
			textureImpl.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		else if (textureImpl.layout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
						 textureImpl.layout != VK_IMAGE_LAYOUT_UNDEFINED)
		{
			std::cerr << "Warning: Offscreen color texture in unexpected layout (" << textureImpl.layout
								<< ") before Pass 1.\n";
		}
	}
}
void VulkanRenderer::transitionColorResolveLayoutForReading(const textures::Framebuffer& framebuffer)
{
	auto colorResolveTexture = framebuffer.getColorResolveTexture();
	if (colorResolveTexture && colorResolveTexture->rendererData)
	{
		auto& textureImpl = *static_cast<VulkanTextureImpl*>(colorResolveTexture->rendererData);
		textureImpl.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
}
void VulkanRenderer::transitionDepthResolveLayoutForWriting(const textures::Framebuffer& framebuffer)
{
	auto depthResolveTexture = framebuffer.getDepthResolveTexture();
	if (depthResolveTexture && depthResolveTexture->rendererData)
	{
		auto& textureImpl = *static_cast<VulkanTextureImpl*>(depthResolveTexture->rendererData);
		if (textureImpl.layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			VkImageMemoryBarrier depthBarrier = {};
			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;
			prepareImageBarrier(*commandBuffer, textureImpl.textureImage, textureImpl.format,
													VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
													VK_IMAGE_ASPECT_DEPTH_BIT, sourceStage, destinationStage, depthBarrier);
			vkCmdPipelineBarrier(*commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &depthBarrier);
			textureImpl.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else if (textureImpl.layout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL &&
						 textureImpl.layout != VK_IMAGE_LAYOUT_UNDEFINED)
		{
			std::cerr << "Warning: Offscreen depth texture in unexpected layout (" << textureImpl.layout
								<< ") before Pass 1.\n";
		}
	}
}
void VulkanRenderer::transitionDepthResolveLayoutForReading(const textures::Framebuffer& framebuffer)
{
	auto depthResolveTexture = framebuffer.getDepthResolveTexture();
	if (depthResolveTexture && depthResolveTexture->rendererData)
	{
		auto& textureImpl = *static_cast<VulkanTextureImpl*>(depthResolveTexture->rendererData);

		textureImpl.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// Ensure the layout we are transitioning *from* is correct
		if (textureImpl.layout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL &&
			textureImpl.layout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			// Log warning or throw? Indicates layout tracking might be off.
			// For now, let's assume it *should* be ATTACHMENT_OPTIMAL here.
			std::cerr << "Warning: Directional shadow map layout was not ATTACHMENT_OPTIMAL before transition!" << std::endl;
		}

		VkImageLayout oldLayout = textureImpl.layout;
		VkImageLayout newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		VkImageMemoryBarrier barrier;
		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		// Prepare barrier details: ATTACHMENT_OPTIMAL -> READ_ONLY_OPTIMAL
		prepareImageBarrier(*commandBuffer, textureImpl.textureImage, textureImpl.format,
												oldLayout, // Old layout
												newLayout, // New layout for sampling
												VK_IMAGE_ASPECT_DEPTH_BIT, // Assuming depth only
												sourceStage, destinationStage, barrier);

		// Record the barrier in the current command buffer
		vkCmdPipelineBarrier(*commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		// Update tracked layout
		textureImpl.layout = newLayout;
	}
}
