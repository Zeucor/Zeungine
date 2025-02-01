#ifdef MACOS
#import <zg/windows/MacOSWindow.hpp>
#import <zg/entities/Plane.hpp>
#import <iostream>
#import <Cocoa/Cocoa.h>
#ifdef USE_GL
#import <OpenGL/gl3.h>
#include <zg/renderers/GLRenderer.hpp>
#endif
#ifdef USE_EGL
#include <zg/renderers/EGLRenderer.hpp>
#endif
#import <QuartzCore/CAMetalLayer.h>
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
	}
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
	NSView *contentView = [(NSWindow *)nsWindow contentView];
	[context setView:contentView];
	nsView = contentView;
	glContext = context;
}
#endif
#ifdef USE_EGL
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
    EGLint contextAttribs[] = { EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 0, EGL_NONE };
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
		[(NSOpenGLContext*)glContext makeCurrentContext];
		GLint swapInterval = renderWindowPointer->vsync ? 1 : 0;
		[(NSOpenGLContext*)glContext setValues:&swapInterval forParameter:NSOpenGLContextParameterSwapInterval];
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
	[(NSOpenGLContext*)glContext flushBuffer];
}
void MacOSWindow::destroy()
{
	if (glContext)
		[(NSOpenGLContext*)glContext release];
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