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
	//
	std::vector<const char*> extensions;
	extensions.push_back("VK_KHR_surface");
#if defined(LINUX)
	extensions.push_back("VK_KHR_xlib_surface");
#elif defined(ANDROID)
	extensions.push_back("VK_KHR_android_surface");
#endif
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
		throw std::runtime_error("VulkanDriver-setupDebugMessenger: failed to set up debug messenger!");
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
		throw std::runtime_error("VulkanDriver-createSurface: failed to create Xlib surface");
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
		throw std::runtime_error("VulkanDriver-createSurface: failed to create Xlib surface");
	}
#endif
}
#endif
void VulkanRenderer::pickPhysicalDevice() {}
void VulkanRenderer::createLogicalDevice() {}
void VulkanRenderer::createSwapChain() {}
void VulkanRenderer::createImageViews() {}
void VulkanRenderer::createRenderPass() {}
void VulkanRenderer::createFramebuffers() {}
void VulkanRenderer::createCommandPool() {}
void VulkanRenderer::createCommandBuffers() {}
void VulkanRenderer::createSyncObjects() {}
void VulkanRenderer::init() {}
void VulkanRenderer::destroy() {}
std::shared_ptr<IRenderer> zg::createRenderer() { return std::shared_ptr<IRenderer>(new VulkanRenderer()); }
void VulkanRenderer::clearColor(glm::vec4 color) {}
void VulkanRenderer::clear() {}
void VulkanRenderer::viewport(glm::ivec4 vp) const {}
void VulkanRenderer::setUniform(shaders::Shader& shader, const std::string_view name, const void* pointer,
																uint32_t size, enums::EUniformType uniformType)
{
}
void VulkanRenderer::setBlock(shaders::Shader& shader, const std::string_view name, const void* pointer, size_t size) {}
void VulkanRenderer::deleteBuffer(uint32_t id) {}
void VulkanRenderer::bindShader(const shaders::Shader& shader) {}
void VulkanRenderer::unbindShader(const shaders::Shader& shader) {}
void VulkanRenderer::addSSBO(shaders::Shader& shader, const std::string_view name, uint32_t bindingIndex) {}
void VulkanRenderer::addUBO(shaders::Shader& shader, const std::string_view name, uint32_t bindingIndex) {}
void VulkanRenderer::setSSBO(shaders::Shader& shader, const std::string_view name, const void* pointer, size_t size) {}
void VulkanRenderer::setTexture(shaders::Shader& shader, const std::string_view name, const textures::Texture& texture,
																const int32_t unit)
{
}
bool VulkanRenderer::compileShader(shaders::Shader& shader, shaders::ShaderType shaderType,
																	 shaders::ShaderPair& shaderPair)
{
	return false;
}
bool VulkanRenderer::compileProgram(shaders::Shader& shader, const shaders::ShaderMap& shaderMap) { return false; }
bool VulkanRenderer::checkCompileErrors(shaders::Shader& shader, const uint32_t& id, bool isShader,
																				const char* shaderType)
{
	return false;
}
void VulkanRenderer::deleteShader(shaders::Shader& shader) {}
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
void VulkanRenderer::updateIndicesVAO(const vaos::VAO& vao, const std::vector<uint32_t>& indices) {}
void VulkanRenderer::updateElementsVAO(const vaos::VAO& vao, const std::string_view constant, uint8_t* elementsAsChar)
{
}
void VulkanRenderer::drawVAO(const vaos::VAO& vao) {}
void VulkanRenderer::generateVAO(vaos::VAO& vao) {}
void VulkanRenderer::destroyVAO(vaos::VAO& vao) {}
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
