#ifdef MACOS
#import <zg/windows/MacOSWindow.hpp>
#import <zg/entities/Plane.hpp>
#import <iostream>
#import <Cocoa/Cocoa.h>
#ifdef USE_GL
#import <OpenGL/gl3.h>
#include <zg/renderers/GLRenderer.hpp>
#elif defined(USE_EGL)
#include <zg/renderers/EGLRenderer.hpp>
#elif defined(USE_VULKAN)
#include <zg/renderers/VulkanRenderer.hpp>
#endif
#import <QuartzCore/CAMetalLayer.h>
#include <Metal/Metal.h>
using namespace zg;
@interface MacOSWindowDelegate : NSObject <NSWindowDelegate>
@end
@implementation MacOSWindowDelegate
- (BOOL)windowShouldClose:(id)sender
{
	[NSApp terminate:nil];
	return YES;
}
@end
void MacOSWindow::init(Window &renderWindow)
{
	renderWindowPointer = &renderWindow;
	@autoreleasepool
	{
		if (NSApp == nil)
		{
			[NSApplication sharedApplication];
			[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
		}
		int32_t windowX = renderWindow.windowX == -1 ? 128 : renderWindow.windowX,
				windowY = renderWindow.windowY == -1 ? 128 : renderWindow.windowY;
		NSRect rect = NSMakeRect(windowX, windowY, renderWindow.windowWidth, renderWindow.windowHeight);
		NSWindow *window = [[NSWindow alloc] initWithContentRect:rect
							styleMask:(NSWindowStyleMaskTitled |
										NSWindowStyleMaskClosable |
										NSWindowStyleMaskResizable |
										NSWindowStyleMaskMiniaturizable)
							backing:NSBackingStoreBuffered
							defer:NO];
		NSString *nsTitle = [NSString stringWithUTF8String:renderWindow.title];
		[window setTitle:nsTitle];
		[window setDelegate:[[MacOSWindowDelegate alloc] init]];
		[window makeKeyAndOrderFront:nil];
		nsWindow = window;
		NSView *contentView = [(NSWindow *)nsWindow contentView];
		nsView = contentView;
	}
	nsImage = [[NSImage alloc] initWithSize:NSMakeSize(renderWindow.windowWidth, renderWindow.windowHeight)];
	NSRect rect = NSMakeRect(0, 0, renderWindow.windowWidth, renderWindow.windowHeight);
	nsImageView = [[NSImageView alloc] initWithFrame:rect];
	[(NSView*)nsView addSubview:(NSImageView *)nsImageView];
}
#ifdef USE_GL
void GLRenderer::createContext(IPlatformWindow* platformWindowPointer)
{
	this->platformWindowPointer = platformWindowPointer;
	NSOpenGLPixelFormatAttribute attributes[] =
	{
		NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
		NSOpenGLPFAColorSize, 24,
		NSOpenGLPFAAlphaSize, 8,
		NSOpenGLPFADepthSize, 24,
		NSOpenGLPFAStencilSize, 8,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFADoubleBuffer,
		0
	};
	NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
	NSOpenGLContext *context = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
	[context setView:nsView];
	glContext = context;
}
#elif defined(USE_EGL)
void EGLRenderer::createContext(IPlatformWindow* platformWindowPointer)
{
	this->platformWindowPointer = platformWindowPointer;
	auto macWindow = dynamic_cast<MacOSWindow*>(platformWindowPointer);
	NSView *contentView = [macWindow->nsWindow contentView];
	if (![contentView wantsLayer])
	{
		[contentView setWantsLayer:YES];
	}
	macWindow->nsView = contentView;
	auto nativeWindow = (void *)contentView.layer;
    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, nativeWindow, nullptr);
    if (eglSurface == EGL_NO_SURFACE)
    {
        throw std::runtime_error("EGL_NO_SURFACE");
    }
    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 0, EGL_NONE };
    eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttribs);
    if (eglContext == EGL_NO_CONTEXT)
    {
        throw std::runtime_error("EGL_NO_CONTEXT");
    }
    if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext))
    {
        throw std::runtime_error("eglMakeCurrent failed!");
    }
}
#elif defined(USE_VULKAN)
void VulkanRenderer::createSurface()
{
	auto& macWindow = *dynamic_cast<MacOSWindow*>(platformWindowPointer);
	VkHeadlessSurfaceCreateInfoEXT surfaceCreateInfo{};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT;
	if (!VKcheck("vkCreateHeadlessSurfaceEXT", vkCreateHeadlessSurfaceEXT(instance, &surfaceCreateInfo, nullptr, &surface)))
	{
		throw std::runtime_error("Failed to create Vulkan headless surface!");
	}
    // nsView.wantsLayer = YES;
    // if (![nsView.layer isKindOfClass:[CAMetalLayer class]])
    // {
    //     nsView.layer = [CAMetalLayer layer];
    // }
    // CAMetalLayer *metalLayer = (CAMetalLayer *)nsView.layer;
    // id<MTLDevice> metalDevice = MTLCreateSystemDefaultDevice();
    // if (!metalDevice)
    // {
    //     throw std::runtime_error("VulkanRenderer-createSurface: No available Metal device");
    // }
    // metalLayer.device = metalDevice;
    // metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    // metalLayer.contentsScale = [NSScreen mainScreen].backingScaleFactor;
	// VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo{};
	// surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
	// surfaceCreateInfo.pView = macWindow.nsView;
	// if (!VKcheck("vkCreateMacOSSurfaceMVK", vkCreateMacOSSurfaceMVK(instance, &surfaceCreateInfo, nullptr, &surface)))
	// {
	// 	throw std::runtime_error("VulkanRenderer-createSurface: failed to create MacOS surface");
	// }
}
#endif
void MacOSWindow::postInit()
{
	@autoreleasepool
	{
#ifdef USE_GL
		[(NSOpenGLContext*)glContext makeCurrentContext];
		GLint swapInterval = renderWindowPointer->vsync ? 1 : 0;
		[(NSOpenGLContext*)glContext setValues:&swapInterval forParameter:NSOpenGLContextParameterSwapInterval];
#elif defined(USE_EGL)
		auto &eglRenderer = *std::dynamic_pointer_cast<EGLRenderer>(renderWindowPointer->iRenderer);
		EGLint swapInterval = renderWindowPointer->vsync ? 1 : 0;
		if (!eglSwapInterval(eglRenderer.eglDisplay, swapInterval))
		{
			throw std::runtime_error("Failed to set swap interval");
		}
#elif defined(MACOS)
		auto& vulkanRenderer = *std::dynamic_pointer_cast<VulkanRenderer>(renderWindowPointer->iRenderer);
		vkMapMemory(vulkanRenderer.device, vulkanRenderer.stagingBufferMemory, 0, VK_WHOLE_SIZE, 0, &bitmap);
#endif
		[NSApp activateIgnoringOtherApps:YES];
	}
}
bool MacOSWindow::pollMessages()
{
	@autoreleasepool
	{
		NSEvent *event;
		while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
								untilDate:nil
								inMode:NSDefaultRunLoopMode
								dequeue:YES]))
		{
			switch ([event type])
			{
				case NSEventTypeKeyDown:
					std::cout << "Key Down" << std::endl;
					break;
				case NSEventTypeKeyUp:
					std::cout << "Key Up" << std::endl;
					break;
				case NSEventTypeMouseMoved:
					std::cout << "Mouse Moved" << std::endl;
					break;
				case NSEventTypeLeftMouseDragged:
				case NSEventTypeRightMouseDragged:
				case NSEventTypeOtherMouseDragged:
					std::cout << "Mouse Dragged" << std::endl;
					break;
				case NSEventTypeApplicationDefined:
					return false;
				default:
					break;
			}
			[NSApp sendEvent:event];
			[NSApp updateWindows];
		}
	}
	return true;
}
void MacOSWindow::swapBuffers()
{
#ifdef USE_GL
	[(NSOpenGLContext*)glContext flushBuffer];
#elif defined(USE_EGL)
	auto &eglRenderer = *std::dynamic_pointer_cast<EGLRenderer>(renderWindowPointer->iRenderer);
    if (!eglSwapBuffers(eglRenderer.eglDisplay, eglRenderer.eglSurface))
    {
        throw std::runtime_error("eglSwapBuffers failed");
    }
#elif defined(USE_VULKAN)
	auto& renderWindow = *renderWindowPointer;
	auto& vulkanRenderer = *std::dynamic_pointer_cast<VulkanRenderer>(renderWindow.iRenderer);
	auto currentFrame = vulkanRenderer.currentFrame;
	VkCommandBuffer commandBuffer = vulkanRenderer.beginSingleTimeCommands();
	VkImage image = vulkanRenderer.swapChainImages[currentFrame];
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
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	vulkanRenderer.endSingleTimeCommands(commandBuffer);
	commandBuffer = vulkanRenderer.beginSingleTimeCommands();
	VkMappedMemoryRange range{};
	range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	range.memory = vulkanRenderer.stagingBufferMemory;
	range.offset = 0;
	range.size = VK_WHOLE_SIZE;
	vkInvalidateMappedMemoryRanges(vulkanRenderer.device, 1, &range);
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
	vkCmdCopyImageToBuffer(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vulkanRenderer.stagingBuffer, 1, &region);
	vulkanRenderer.endSingleTimeCommands(commandBuffer);
	commandBuffer = vulkanRenderer.beginSingleTimeCommands();
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    vulkanRenderer.endSingleTimeCommands(commandBuffer);
	vkQueueWaitIdle(vulkanRenderer.graphicsQueue);
	@autoreleasepool
	{
		unsigned char* bitmapData = (unsigned char*)bitmap;
		for (int i = 0; i < renderWindow.windowWidth * renderWindow.windowHeight; ++i)
		{
			unsigned char temp = bitmapData[i * 4 + 0];
			bitmapData[i * 4 + 0] = bitmapData[i * 4 + 2];
			bitmapData[i * 4 + 2] = temp;
		}
		NSBitmapImageRep* bitmapRep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:(unsigned char**)&bitmap
																					pixelsWide:renderWindow.windowWidth
																					pixelsHigh:renderWindow.windowHeight
																				bitsPerSample:8
																				samplesPerPixel:4
																					hasAlpha:YES
																					isPlanar:NO
																				colorSpaceName:NSDeviceRGBColorSpace
																					bitmapFormat:0
																					bytesPerRow:renderWindow.windowWidth * 4
																					bitsPerPixel:32];
		if (bitmapRep == nil)
		{
			NSLog(@"Failed to create NSBitmapImageRep");
			return;
		}
		NSImage *image = (NSImage *)nsImage;
		NSArray *reps = [image representations];
		if ([reps count] > 0)
		{
			for (NSImageRep *rep in reps)
			{
				[image removeRepresentation:rep];
			}
		}
		[image addRepresentation:bitmapRep];
		[(NSImageView *)nsImageView setImage:nil];
		[(NSImageView *)nsImageView setImage:image]; 
		[(NSView *)nsView displayIfNeeded];
		[(NSView*)nsView setNeedsDisplay:YES];
	}
#endif
}
void MacOSWindow::destroy()
{
#ifdef USE_GL
	if (glContext)
		[(NSOpenGLContext*)glContext release];
#elif defined(USE_EGL)
	auto &eglRenderer = *std::dynamic_pointer_cast<EGLRenderer>(renderWindowPointer->iRenderer);
    if (eglRenderer.eglContext != EGL_NO_CONTEXT)
    {
        if (!eglDestroyContext(eglRenderer.eglDisplay, eglRenderer.eglContext))
        {
            throw std::runtime_error("Failed to destroy EGL context");
        }
        eglRenderer.eglContext = EGL_NO_CONTEXT;
    }
#elif defined(MACOS)
	auto& vulkanRenderer = *std::dynamic_pointer_cast<VulkanRenderer>(renderWindowPointer->iRenderer);
	vkUnmapMemory(vulkanRenderer.device, vulkanRenderer.stagingBufferMemory);
#endif
	if (nsWindow)
		[(NSWindow*)nsWindow release];
	renderWindowPointer->iRenderer->destroy();
}
void MacOSWindow::close()
{
}
void MacOSWindow::minimize()
{
}
void MacOSWindow::maximize()
{
}
void MacOSWindow::restore()
{
}
void MacOSWindow::warpPointer(glm::vec2 coords)
{
}
void MacOSWindow::setXY()
{
}
void MacOSWindow::setWidthHeight()
{
}
void MacOSWindow::mouseCapture(bool capture)
{
	if (capture)
	{}
	else
	{}
}
std::shared_ptr<IPlatformWindow> zg::createPlatformWindow()
{
	return std::shared_ptr<IPlatformWindow>(new MacOSWindow());
}
#endif