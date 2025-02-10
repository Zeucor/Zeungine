#include <AppKit/AppKit.h>
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
#include <ApplicationServices/ApplicationServices.h>
using namespace zg;
@interface MacOSWindowDelegate : NSObject <NSWindowDelegate>
{
    MacOSWindow* macOSWindowPointer;
}
- (instancetype)initWithWindow:(MacOSWindow*)window;
@end
@implementation MacOSWindowDelegate
- (instancetype)initWithWindow:(MacOSWindow*)window
{
    self = [super init];
    if (self)
    {
        macOSWindowPointer = window;
    }
    return self;
}
- (void)windowDidBecomeKey:(NSNotification *)notification
{
	auto& renderWindow = *macOSWindowPointer->renderWindowPointer;
	renderWindow.callFocusHandler(true);
	CGAssociateMouseAndMouseCursorPosition(YES);
}
- (void)windowDidResignKey:(NSNotification *)notification
{
	auto& renderWindow = *macOSWindowPointer->renderWindowPointer;
	renderWindow.callFocusHandler(false);
	CGAssociateMouseAndMouseCursorPosition(NO);
}
- (BOOL)windowShouldClose:(id)sender
{
	macOSWindowPointer->closed = true;
	[NSApp terminate:nil];
	return YES;
}
@end
void MacOSWindow::init(Window &renderWindow)
{
	renderWindowPointer = &renderWindow;
	windowType = WINDOW_TYPE_MACOS;
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
		[window setDelegate:[[MacOSWindowDelegate alloc] initWithWindow:this]];
		[window makeKeyAndOrderFront:nil];
		nsWindow = window;
		NSView *contentView = [(NSWindow *)nsWindow contentView];
		nsView = contentView;
		NSMenu *mainMenu = [[NSMenu alloc] initWithTitle:nsTitle];
		[NSApp setMainMenu:mainMenu];
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
		[NSApp activateIgnoringOtherApps:YES];
#endif
	}
}
bool MacOSWindow::pollMessages()
{
	if (closed)
		return false;
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
				case NSEventTypeKeyUp:
				{
					auto pressed = [event type] == NSEventTypeKeyDown;
					int32_t mod = 0; // TODO: support mod
				    NSString *characters = [event characters];
					uint32_t keycode = 0;
					if (characters.length > 0)
					{
						keycode = [characters characterAtIndex:0];
					}
					renderWindowPointer->handleKey(keycode, mod, pressed);
					break;
				}
				case NSEventTypeMouseMoved:
				{
					NSPoint location = [event locationInWindow];
					auto x = location.x;
					auto y = location.y;
					renderWindowPointer->handleMouseMove(x, y);
					break;
				}
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
#ifdef USE_GL
void GLRenderer::swapBuffers()
{
	auto& macWindow = *dynamic_cast<MacOSWindow*>(platformWindowPointer);
	[(NSOpenGLContext*)macWindow.glContext flushBuffer];
}
#endif
#if defined(USE_VULKAN) && defined(USE_SWIFTSHADER)
void VulkanRenderer::swapBuffers()
{
	if (!VKcheck("vkQueuePresentKHR", _vkQueuePresentKHR(presentQueue, &presentInfo)))
	{
		throw std::runtime_error("VulkanRenderer-vkQueuePresentKHR failed");
	}
	auto& macWindow = *dynamic_cast<MacOSWindow*>(platformWindowPointer);
	auto& vulkanRenderer = *dynamic_cast<VulkanRenderer*>(macWindow.renderWindowPointer->iRenderer);
	vulkanRenderer.getCurrentImageToBitmap();
	@autoreleasepool
	{
		unsigned char* bitmapData = (unsigned char*)vulkanRenderer.bitmap;
		for (int i = 0; i < macWindow.renderWindowPointer->windowWidth * macWindow.renderWindowPointer->windowHeight; ++i)
		{
			unsigned char temp = bitmapData[i * 4 + 0];
			bitmapData[i * 4 + 0] = bitmapData[i * 4 + 2];
			bitmapData[i * 4 + 2] = temp;
		}
		NSBitmapImageRep* bitmapRep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:&bitmapData
																					pixelsWide:macWindow.renderWindowPointer->windowWidth
																					pixelsHigh:macWindow.renderWindowPointer->windowHeight
																				bitsPerSample:8
																				samplesPerPixel:4
																					hasAlpha:YES
																					isPlanar:NO
																				colorSpaceName:NSDeviceRGBColorSpace
																					bitmapFormat:0
																					bytesPerRow:macWindow.renderWindowPointer->windowWidth * 4
																					bitsPerPixel:32];
		if (bitmapRep == nil)
		{
			NSLog(@"Failed to create NSBitmapImageRep");
			return;
		}
		NSImage *image = (NSImage *)macWindow.nsImage;
		NSArray *reps = [image representations];
		if ([reps count] > 0)
		{
			for (NSImageRep *rep in reps)
			{
				[image removeRepresentation:rep];
			}
		}
		[image addRepresentation:bitmapRep];
		[(NSImageView *)macWindow.nsImageView setImage:nil];
		[(NSImageView *)macWindow.nsImageView setImage:image]; 
		[(NSView *)macWindow.nsView displayIfNeeded];
		[(NSView *)macWindow.nsView setNeedsDisplay:YES];
	}
}
#endif
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
#endif
	if (nsWindow)
		[(NSWindow*)nsWindow release];
}
void MacOSWindow::close()
{
	auto window = (NSWindow *)nsWindow;
	[window close];
	closed = true;
}
void MacOSWindow::minimize()
{
	auto window = (NSWindow *)nsWindow;
	[window miniaturize:nil];
}
void MacOSWindow::maximize()
{
	auto window = (NSWindow *)nsWindow;
	[window setStyleMask:([nsWindow styleMask] | NSWindowStyleMaskResizable)];
	[window zoom:nil];
}
void MacOSWindow::restore()
{
	auto window = (NSWindow *)nsWindow;
	if ([window isMiniaturized])
		[window deminiaturize:nil];
	else if ([window isZoomed])
		[window zoom:nil];
}
void MacOSWindow::warpPointer(glm::vec2 coords)
{
	auto window = ((NSWindow*)nsWindow);
	auto x = window.frame.origin.x + coords.x;
	auto y = window.screen.frame.size.height - window.frame.origin.y - coords.y;
    CGPoint point = CGPointMake(x, y);
	CGWarpMouseCursorPosition(point);
	CGAssociateMouseAndMouseCursorPosition(YES);
}
void MacOSWindow::showPointer()
{
	CGDisplayShowCursor(kCGDirectMainDisplay);
}
void MacOSWindow::hidePointer()
{
	CGDisplayHideCursor(kCGDirectMainDisplay);
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
#endif