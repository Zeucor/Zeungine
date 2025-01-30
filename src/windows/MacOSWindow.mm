#ifdef MACOS
#import <zg/windows/MacOSWindow.hpp>
#import <zg/entities/Plane.hpp>
#import <iostream>
#import <Cocoa/Cocoa.h>
#import <OpenGL/gl3.h>
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
void MacOSWindow::createContext()
{
#ifdef USE_GL
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
#endif
}
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
	renderWindowPointer->iVendorRenderer->destroy();
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