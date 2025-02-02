#ifdef USE_VULKAN
#include <zg/Logger.hpp>
#include <zg/Window.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/renderers/VulkanRenderer.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/textures/TextureFactory.hpp>
#include <zg/vaos/VAOFactory.hpp>
#ifdef WINDOWS
#include <zg/windows/WIN32Window.hpp>
#endif
#ifdef LINUX
#include <zg/windows/X11Window.hpp>
#endif
#ifdef MACOS
#include <zg/windows/MacOSWindow.hpp>
#endif
using namespace zg;
static std::unordered_map<shaders::ShaderType, shaderc_shader_kind> stageEShaderc = {
	{shaders::ShaderType::Vertex, shaderc_vertex_shader},
	{shaders::ShaderType::Fragment, shaderc_fragment_shader},
	{shaders::ShaderType::Compute, shaderc_compute_shader}};
static std::unordered_map<shaders::ShaderType, VkShaderStageFlagBits> stageStageFlag = {
	{shaders::ShaderType::Vertex, VK_SHADER_STAGE_VERTEX_BIT},
	{shaders::ShaderType::Fragment, VK_SHADER_STAGE_FRAGMENT_BIT},
	{shaders::ShaderType::Compute, VK_SHADER_STAGE_COMPUTE_BIT},
};
VulkanRenderer::VulkanRenderer() {}
VulkanRenderer::~VulkanRenderer() {}
void VulkanRenderer::createContext(IPlatformWindow* platformWindowPointer)
{
	this->platformWindowPointer = platformWindowPointer;
	createInstance();
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
	createImageStagingBuffer();
}
void VulkanRenderer::createInstance()
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = platformWindowPointer->renderWindowPointer->title;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Zeungine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
	//
	std::vector<const char*> extensions;
	extensions.push_back("VK_KHR_surface");
#if defined(LINUX)
	extensions.push_back("VK_KHR_xlib_surface");
#elif defined(ANDROID)
	extensions.push_back("VK_KHR_android_surface");
#elif defined(MACOS)
	extensions.push_back("VK_MVK_macos_surface");
	extensions.push_back("VK_EXT_headless_surface");
#endif
	extensions.push_back("VK_KHR_portability_enumeration");
#ifndef NDEBUG
	extensions.push_back("VK_EXT_debug_utils");
#endif
	createInfo.enabledExtensionCount = extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();
	//
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	std::vector<const char*> layers;
#ifndef NDEBUG
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
	// VkAllocationCallbacks allocator = createAllocator("Instance");
	auto createdInstance = VKcheck("vkCreateInstance", vkCreateInstance(&createInfo, 0, &instance));
	if (!createdInstance)
	{
		throw std::runtime_error("VulkanRenderer-createInstance: failed to create instance!");
	}
};
#ifndef NDEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
																						 VkDebugUtilsMessageTypeFlagsEXT messageType,
																						 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << pCallbackData->pMessage << std::endl;
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
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
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
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
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
	if (!VKcheck("CreateDebugUtilsMessengerEXT",
							 CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger)))
	{
		throw std::runtime_error("VulkanRenderer-setupDebugMessenger: failed to set up debug messenger!");
	}
};
#endif
#if defined(LINUX) || defined(ANDROID) || defined(WINDOWS)
void VulkanRenderer::createSurface()
{
#ifdef LINUX
	auto& x11Window = *dynamic_cast<X11Window*>(platformWindowPointer);
	VkXlibSurfaceCreateInfoKHR surfaceCreateInfo{};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.dpy = x11Window.display;
	surfaceCreateInfo.window = x11Window.window;
	if (!VKcheck("vkCreateXlibSurfaceKHR", vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface)))
	{
		throw std::runtime_error("VulkanRenderer-createSurface: failed to create Xlib surface");
	}
#elif defined(ANDROID)
#elif defined(WINDOWS)
	auto& win32Window = *dynamic_cast<WIN32Window*>(platformWindowPointer);
	VkWin32SurfaceCreateFlagsKHR surfaceCreateInfo{};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.dpy = win32Window.hInstance;
	surfaceCreateInfo.window = win32Window.hwnd;
	if (!VKcheck("vkCreateWin32SurfaceKHR", vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface)))
	{
		throw std::runtime_error("VulkanRenderer-createSurface: failed to create Xlib surface");
	}
#endif
}
#endif
void VulkanRenderer::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		throw std::runtime_error("VulkanRenderer-getPhysicalDevice: failed to find GPUs with Vulkan support!");
	}
	std::vector<VkPhysicalDevice> devices;
	devices.resize(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
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
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
	std::cout << "Selected Physical Device: '" << physicalDeviceProperties.deviceName
						<< "' with score of: " << selectedDeviceScore << std::endl;
	return;
}
uint32_t VulkanRenderer::rateDeviceSuitability(VkPhysicalDevice device)
{
	uint32_t score = 0;
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(device, &properties);
	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(device, &features);
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
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies;
	queueFamilies.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
	int32_t index = 0;
	for (auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = index;
		}
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &presentSupport);
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
	extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	extensions.push_back("VK_KHR_maintenance1");
	// extensions[2] = VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME;
	// extensions[3] = VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME;
	// extensions[4] = VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME;
	createInfo.enabledExtensionCount = extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledLayerCount = 0;
	VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{};
	descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
	descriptorIndexingFeatures.pNext = nullptr;
	VkPhysicalDeviceFeatures2 deviceFeatures{};
	deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures.pNext = &descriptorIndexingFeatures;
	vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures);
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
	auto createdDevice = VKcheck("vkCreateDevice", vkCreateDevice(physicalDevice, &createInfo, nullptr, &device));
	if (!createdDevice)
	{
		throw std::runtime_error("VulkanRenderer-createLogicalDevice: failed to create device");
	}
	vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);
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
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
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
	if (!VKcheck("vkCreateSwapchainKHR", vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain)))
	{
		throw std::runtime_error("VulkanRenderer-createSwapChain: failed to create Swapchain");
	}
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
	return;
}
SwapChainSupportDetails VulkanRenderer::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}
	return details;
}
VkSurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM)
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
		if (!VKcheck("vkCreateImageView", vkCreateImageView(device, &createInfo, nullptr, &(swapChainImageViews[index]))))
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
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;
	if (!VKcheck("vkCreateRenderPass", vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass)))
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
								 vkCreateFramebuffer(device, &framebufferInfo, nullptr, &(swapChainFramebuffers[index]))))
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
	if (!VKcheck("vkCreateCommandPool", vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool)))
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
	if (!VKcheck("vkAllocateCommandBuffers", vkAllocateCommandBuffers(device, &allocInfo, &commandBuffers[0])))
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
		if (!VKcheck("vkCreateSemaphore",
								 vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[j])) ||
				!VKcheck("vkCreateSemaphore",
								 vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[j])) ||
				!VKcheck("vkCreateFence", vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[j])))
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
	vkCreateBuffer(device, &bufferCreateInfo, nullptr, &stagingBuffer);
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(
		memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	if (vkAllocateMemory(device, &allocInfo, nullptr, &stagingBufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}
	vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);
}
void VulkanRenderer::init() {}
void VulkanRenderer::destroy()
{
	vkFreeMemory(device, stagingBufferMemory, nullptr);
	vkDestroyBuffer(device, stagingBuffer, nullptr);
}
void VulkanRenderer::preBeginRenderPass()
{
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
	vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE,
												&imageIndex);
	vkResetFences(device, 1, &inFlightFences[currentFrame]);
	commandBuffer = &commandBuffers[currentFrame];
	vkResetCommandBuffer(*commandBuffer, 0);
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	if (!VKcheck("vkBeginCommandBuffer", vkBeginCommandBuffer(*commandBuffer, &beginInfo)))
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}
	return;
}
void VulkanRenderer::beginRenderPass()
{
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
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
	vkCmdBeginRenderPass(*commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	return;
}
void VulkanRenderer::postRenderPass()
{
	vkCmdEndRenderPass(*commandBuffer);
	if (!VKcheck("vkEndCommandBuffer", vkEndCommandBuffer(*commandBuffer)))
	{
		throw std::runtime_error("failed to record command buffer!");
	}
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = commandBuffer;
	VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	if (!VKcheck("vkQueueSubmit", vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame])))
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}
	vkQueueWaitIdle(graphicsQueue);
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR swapChains[] = {swapChain};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	vkQueueWaitIdle(presentQueue);
	vkQueuePresentKHR(presentQueue, &presentInfo);
	return;
}
std::shared_ptr<IRenderer> zg::createRenderer() { return std::shared_ptr<IRenderer>(new VulkanRenderer()); }
void VulkanRenderer::clearColor(glm::vec4 color) {}
void VulkanRenderer::clear() {}
void VulkanRenderer::viewport(glm::ivec4 vp) const {}
void VulkanRenderer::initShader(shaders::Shader& shader, const shaders::RuntimeConstants& constants,
																const std::vector<shaders::ShaderType>& shaderTypes)
{
	shader.rendererData = new VulkanShaderImpl();
	auto& shaderImpl = *(VulkanShaderImpl*)shader.rendererData;
	shaderImpl.shaders = shaders::ShaderFactory::generateShaderMap(constants, shader, shaderTypes);
	shaders::ShaderFactory::compileProgram(shader);
}
void VulkanRenderer::setUniform(shaders::Shader& shader, const std::string_view name, const void* pointer,
																uint32_t size, enums::EUniformType uniformType)
{
}
void VulkanRenderer::setBlock(shaders::Shader& shader, const std::string_view name, const void* pointer, size_t size) {}
void VulkanRenderer::deleteBuffer(uint32_t id) {}
void VulkanRenderer::bindShader(const shaders::Shader& shader) {}
void VulkanRenderer::unbindShader(const shaders::Shader& shader) {}
void VulkanRenderer::addSSBO(shaders::Shader& shader, shaders::ShaderType shaderType, const std::string_view name, uint32_t bindingIndex)
{
	auto &shaderImpl = *(VulkanShaderImpl *)shader.rendererData;
	std::string stringName(name);
	auto ssboIter = shaderImpl.ssboBindings.find(stringName);
	std::tuple<uint32_t, VkBuffer, VkDeviceMemory, uint32_t, uint32_t> *ssboBindingPointer = 0;
	if (ssboIter == shaderImpl.ssboBindings.end())
	{
		ssboBindingPointer = &shaderImpl.ssboBindings[stringName];
		std::get<3>(*ssboBindingPointer) = bindingIndex;
		shaderImpl.uboLayoutBindings.resize(shaderImpl.uboLayoutBindings.size() + 1);
		int32_t uboLayoutBindingIndex = -1;
		VkDescriptorSetLayoutBinding layoutBinding = {
		    (uint32_t)bindingIndex,
		    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		    1,
		    (VkShaderStageFlags)stageStageFlag[shaderType],
		    0};
		shaderImpl.uboLayoutBindings[uboLayoutBindingIndex = (shaderImpl.uboLayoutBindings.size() - 1)] = {{ELayoutBindingType::SSBO, 0, "", bindingIndex, false}, layoutBinding};
		std::get<4>(*ssboBindingPointer) = uboLayoutBindingIndex;
	}
	else
	{
		ssboBindingPointer = &ssboIter->second;
		auto uboLayoutBindingIndex = std::get<4>(*ssboBindingPointer);
		shaderImpl.uboLayoutBindings[uboLayoutBindingIndex].second.stageFlags |= stageStageFlag[shaderType];
	}
	std::get<0>(*ssboBindingPointer) |= (uint32_t)shaderType;
}
void VulkanRenderer::addUBO(shaders::Shader& shader, shaders::ShaderType shaderType, const std::string_view name, uint32_t bindingIndex, uint32_t bufferSize, uint32_t descriptorCount, bool isArray)
{
	auto &shaderImpl = *(VulkanShaderImpl *)shader.rendererData;
	std::string stringName(name);
	shaderImpl.uboLayoutBindings.resize(shaderImpl.uboLayoutBindings.size() + 1);
	VkDescriptorSetLayoutBinding layoutBinding = {
	    (uint32_t)bindingIndex,
	    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	    (uint32_t)1,
	    (VkShaderStageFlags)stageStageFlag[shaderType],
	    0};
	shaderImpl.uboLayoutBindings[shaderImpl.uboLayoutBindings.size() - 1] = {{ELayoutBindingType::UniformBuffer, bufferSize, stringName, bindingIndex, isArray}, layoutBinding};
	shaderImpl.uboStringBindings[stringName] = bindingIndex;
	for (uint32_t index = 0; index < 1; index++)
	{
		shaderImpl.uboStringBindings[stringName + "[" + std::to_string(index) + "]"] = bindingIndex + index;
	}
}
void VulkanRenderer::setSSBO(shaders::Shader& shader, const std::string_view name, const void* pointer, size_t size) {}
void VulkanRenderer::setTexture(shaders::Shader& shader, const std::string_view name, const textures::Texture& texture,
																const int32_t unit)
{
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
	if (!VKcheck("vkCreateShaderModule", vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule)))
	{
		throw std::runtime_error("failed to create shader module!");
	}
	return shaderModule;
}
bool VulkanRenderer::compileProgram(shaders::Shader& shader)
{
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
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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
	for (auto &uboLayoutBinding : shaderImpl.uboLayoutBindings)
	{
		layoutBindings.push_back(uboLayoutBinding.second);
	}
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
	descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo.bindingCount = layoutBindings.size();
	descriptorSetLayoutInfo.pBindings = layoutBindings.data();
	if (!VKcheck("vkCreateDescriptorSetLayout", vkCreateDescriptorSetLayout(device, &descriptorSetLayoutInfo, nullptr, &shaderImpl.descriptorSetLayout)))
	{
		throw std::runtime_error("Failed to create descriptor set layout");
	}
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &shaderImpl.descriptorSetLayout;
	if (!VKcheck("vkCreatePipelineLayout", vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &shaderImpl.pipelineLayout)))
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
	pipelineInfo.renderPass = renderPass;
	// 	if (shader.scenePointer->sceneIndex == 0)
	// 	{
	// 	}
	// 	else
	// 	{
	// 		auto &framebuffer = currentFramebuffer.pointer ? *currentFramebuffer.pointer :
	// *shader.scenePointer->framebuffers._data[0].pointer; 		auto &vulkanFramebuffer = *(VulkanFramebuffer
	// *)framebuffer.vendorData; 		pipelineInfo.renderPass = vulkanFramebuffer.renderPass; 		auto hasDepth =
	// framebuffer.attachments.reduce(Function<Boolean, FramebufferAttachment &, Boolean>([](FramebufferAttachment
	// &framebufferAttachment, Boolean hasDepth)
	// 														   {
	//    if (framebufferAttachment.type == EFramebufferAttachmentType::Depth ||
	//        framebufferAttachment.type == EFramebufferAttachmentType::DepthStencil)
	//    {
	//     hasDepth = true;
	//    }
	//    return hasDepth; }),
	// 							       false);
	// 		auto hasStencil = framebuffer.attachments.reduce(Function<Boolean, FramebufferAttachment &,
	// Boolean>([](FramebufferAttachment &framebufferAttachment, Boolean hasStencil)
	// 														     {
	//    if (framebufferAttachment.type == EFramebufferAttachmentType::DepthStencil)
	//    {
	//     hasStencil = true;
	//    }
	//    return hasStencil; }),
	// 								 false);
	// 		if (hasDepth)
	// 		{
	// 			VkPipelineDepthStencilStateCreateInfo depthStencil{};
	// 			depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	// 			depthStencil.depthTestEnable = VK_TRUE;
	// 			depthStencil.depthWriteEnable = VK_TRUE;
	// 			depthStencil.depthCompareOp = VK_COMPARE_OP_GREATER;
	// 			depthStencil.depthBoundsTestEnable = VK_FALSE;
	// 			depthStencil.stencilTestEnable = /*hasStencil ? VK_TRUE :*/ VK_FALSE;
	// 			pipelineInfo.pDepthStencilState = &depthStencil;
	// 		}
	// 	}
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	if (!VKcheck("vkCreateGraphicsPipelines", vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &shaderImpl.graphicsPipeline)))
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}
	return true;
}
bool VulkanRenderer::checkCompileErrors(const shaderc::SpvCompilationResult& module, bool isShader,
																				const char* shaderType)
{
	if (module.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		std::cerr << "SHADER_COMPILATION_ERROR[" << shaderType << "]: " << module.GetErrorMessage().c_str() << std::endl;
		return false;
	}
	return true;
}
void VulkanRenderer::deleteShader(shaders::Shader& shader) {}
void VulkanRenderer::destroyShader(shaders::Shader& shader) {}
void VulkanRenderer::bindFramebuffer(const textures::Framebuffer& framebuffer) const {}
void VulkanRenderer::unbindFramebuffer(const textures::Framebuffer& framebuffer) const {}
void VulkanRenderer::initFramebuffer(textures::Framebuffer& framebuffer) {}
void VulkanRenderer::destroyFramebuffer(textures::Framebuffer& framebuffer) {}
void VulkanRenderer::bindTexture(const textures::Texture& texture) {}
void VulkanRenderer::unbindTexture(const textures::Texture& texture) {}
void VulkanRenderer::preInitTexture(textures::Texture& texture)
{
	texture.rendererData = new VulkanTextureImpl();
	return;
}
void VulkanRenderer::midInitTexture(const textures::Texture& texture,
																		const std::vector<images::ImageLoader::ImagePair>& images)
{
}
void VulkanRenderer::postInitTexture(const textures::Texture& texture) {}
void VulkanRenderer::destroyTexture(textures::Texture& texture)
{
	auto textureImpl = (VulkanTextureImpl*)texture.rendererData;
	// do vulkan texture destroy
	delete textureImpl;
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
void VulkanRenderer::drawVAO(const vaos::VAO& vao) {}
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
	delete &vaoImpl;
}
VkCommandBuffer VulkanRenderer::beginSingleTimeCommands()
{
	VkCommandBuffer commandBuffer;
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffer!");
	}
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = nullptr;
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin command buffer!");
	}
	return commandBuffer;
}
void VulkanRenderer::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to end command buffer!");
	}
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit command buffer!");
	}
	vkQueueWaitIdle(graphicsQueue);
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}
uint32_t VulkanRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
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
			vkMapMemory(device, newBufferMemory, 0, newBufferSize, 0, (void**)&newBufferData);
			memcpy(newBufferData, bufferData, newBufferSize < bufferSize ? newBufferSize : bufferSize);
			vkDestroyBuffer(device, buffer, nullptr);
			vkFreeMemory(device, bufferMemory, nullptr);
			buffer = newBuffer;
			bufferMemory = newBufferMemory;
			bufferData = newBufferData;
		}
		else
		{
			createBuffer(newBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | extraUsageFlags,
									 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory);
			vkMapMemory(device, bufferMemory, 0, newBufferSize, 0, (void**)&bufferData);
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

	if (!VKcheck("vkCreateBuffer", vkCreateBuffer(device, &bufferInfo, nullptr, &buffer)))
	{
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (!VKcheck("vkAllocateMemory", vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory)))
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(device, buffer, bufferMemory, 0);
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
#endif
