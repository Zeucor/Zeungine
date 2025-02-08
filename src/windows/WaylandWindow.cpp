#ifdef LINUX
#if defined(USE_WAYLAND)
#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland/wayland-xdg-shell-client-protocol.h>
#include <zg/windows/WaylandWindow.hpp>
using namespace zg;
static void xdgWmBasePing(void *data, xdg_wm_base *wm_base, uint32_t serial) { xdg_wm_base_pong(wm_base, serial); }
static const xdg_wm_base_listener xdgWmBaseListener = {.ping = xdgWmBasePing};
static void registry_handler(void *data, struct wl_registry *registry, uint32_t id, const char *interface,
							 uint32_t version)
{
	auto &waylandWindow = *static_cast<WaylandWindow *>(data);
	auto printInterface = [&]
	{ std::cout << interface << std::endl; };
	if (strcmp(interface, wl_compositor_interface.name) == 0)
	{
		waylandWindow.compositor = (wl_compositor *)wl_registry_bind(registry, id, &wl_compositor_interface, 1);
		printInterface();
	}
	else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
	{
		waylandWindow.xdgWm = (xdg_wm_base *)wl_registry_bind(registry, id, &xdg_wm_base_interface, 1);
		xdg_wm_base_add_listener(waylandWindow.xdgWm, &xdgWmBaseListener, data);
	}
	else if (strcmp(interface, wl_seat_interface.name) == 0)
	{
		waylandWindow.seat = (wl_seat *)wl_registry_bind(registry, id, &wl_seat_interface, 1);
		waylandWindow.seatPointer = wl_seat_get_pointer(waylandWindow.seat);
		waylandWindow.seatKeyboard = wl_seat_get_keyboard(waylandWindow.seat);
	}
	else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0)
	{
		waylandWindow.decorationManager =
			(zxdg_decoration_manager_v1 *)wl_registry_bind(registry, id, &zxdg_decoration_manager_v1_interface, 1);
	}
	else if (strcmp(interface, wl_shm_interface.name) == 0)
	{
		waylandWindow.shm = (wl_shm *)wl_registry_bind(registry, id, &wl_shm_interface, 1);
	}
}
static void registry_remove_object(void *data, struct wl_registry *registry, uint32_t name) {}
static struct wl_registry_listener registry_listener = {&registry_handler, &registry_remove_object};
static void handleShellSurfaceConfigure(void *data, struct xdg_surface *shellSurface, uint32_t serial)
{
	xdg_surface_ack_configure(shellSurface, serial);
	// if (resize)
	// {
	//     readyToResize = 1;
	// }
}
static const struct xdg_surface_listener shellSurfaceListener = {.configure = handleShellSurfaceConfigure};
static void handleToplevelConfigure(void *data, struct xdg_toplevel *toplevel, int32_t width, int32_t height,
									struct wl_array *states)
{
	if (width != 0 && height != 0)
	{
		// resize = 1;
		// newWidth = width;
		// newHeight = height;
	}
}

static void handleToplevelClose(void *data, struct xdg_toplevel *toplevel)
{
	auto &waylandWindow = *static_cast<WaylandWindow *>(data);
	waylandWindow.shouldClose = true;
}
static const struct xdg_toplevel_listener toplevelListener = {.configure = handleToplevelConfigure,
															  .close = handleToplevelClose};
static void keyboard_handle_enter(void *data, wl_keyboard *keyboard, uint32_t serial, wl_surface *surface,
								  wl_array *keys)
{
	auto &waylandWindow = *static_cast<WaylandWindow *>(data);
	waylandWindow.renderWindowPointer->callFocusHandler(true);
}
static void keyboard_handle_leave(void *data, wl_keyboard *keyboard, uint32_t serial, wl_surface *surface)
{
	auto &waylandWindow = *static_cast<WaylandWindow *>(data);
	waylandWindow.renderWindowPointer->callFocusHandler(false);
}
static void keyboard_keymap(void *data, wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size) {}
static void keyboard_key(void *data, wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key,
						 uint32_t state)
{
}
static void keyboard_modifiers(void *data, wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed,
							   uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
}
static void keyboard_repeat_info(void *data, wl_keyboard *wl_keyboard, int32_t rate, int32_t delay) {}
static const wl_keyboard_listener keyboardListener = {.keymap = keyboard_keymap,
													  .enter = keyboard_handle_enter,
													  .leave = keyboard_handle_leave,
													  .key = keyboard_key,
													  .modifiers = keyboard_modifiers,
													  .repeat_info = keyboard_repeat_info};
void pointer_handle_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface,
						  wl_fixed_t sx, wl_fixed_t sy)
{
	auto &waylandWindow = *static_cast<WaylandWindow *>(data);
	waylandWindow.serial = serial;
}

void pointer_handle_leave(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface)
{
	auto &waylandWindow = *static_cast<WaylandWindow *>(data);
	waylandWindow.serial = 0;
}

void pointer_handle_motion(void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{
	auto &waylandWindow = *static_cast<WaylandWindow *>(data);
	double sx_float = wl_fixed_to_double(sx);
	double sy_float = wl_fixed_to_double(sy);
	waylandWindow.renderWindowPointer->handleMouseMove(sx_float, sy_float);
}

void pointer_handle_button(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button,
						   uint32_t state)
{
	// Mouse button pressed or released
}

void pointer_handle_axis(void *data, struct wl_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
	// Mouse wheel scrolled
}

static struct wl_pointer_listener pointer_listener = {.enter = pointer_handle_enter,
													  .leave = pointer_handle_leave,
													  .motion = pointer_handle_motion,
													  .button = pointer_handle_button,
													  .axis = pointer_handle_axis};
void WaylandWindow::init(Window &renderWindow)
{
	renderWindowPointer = &renderWindow;
	windowType = WINDOW_TYPE_WAYLAND;
	display = wl_display_connect(nullptr);
	if (!display)
	{
		throw std::runtime_error("Failed to connect to Wayland display");
	}
	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, this);
	wl_display_dispatch(display);
	wl_display_roundtrip(display);
	if (!compositor)
	{
		throw std::runtime_error("Failed to get Wayland compositor");
	}
	surface = wl_compositor_create_surface(compositor);
	if (!surface)
	{
		throw std::runtime_error("Failed to create Wayland surface");
	}
	xdgSurface = xdg_wm_base_get_xdg_surface(xdgWm, surface);
	xdg_surface_add_listener(xdgSurface, &shellSurfaceListener, this);
	xdgToplevel = xdg_surface_get_toplevel(xdgSurface);
	xdg_toplevel_add_listener(xdgToplevel, &toplevelListener, this);
	xdg_toplevel_set_title(xdgToplevel, renderWindowPointer->title);
	xdg_toplevel_set_app_id(xdgToplevel, renderWindowPointer->title);
	if (seatPointer)
	{
		wl_pointer_add_listener(seatPointer, &pointer_listener, this);
	}
	if (seatKeyboard)
	{
		wl_keyboard_add_listener(seatKeyboard, &keyboardListener, this);
	}
	wl_surface_commit(surface);
	wl_display_roundtrip(display);
	wl_surface_commit(surface);
}
void WaylandWindow::postInit() {}
bool WaylandWindow::pollMessages()
{
	if (shouldClose)
		return false;
	while (wl_display_prepare_read(display) != 0) // If reading isn't possible, dispatch events first
	{
		wl_display_dispatch_pending(display);
	}
	wl_display_flush(display);			  // Send any queued requests
	wl_display_read_events(display);	  // Read new events (non-blocking)
	wl_display_dispatch_pending(display); // Dispatch new events
	return true;
}
void WaylandWindow::destroy()
{
	if (toplevelDecoration)
		zxdg_toplevel_decoration_v1_destroy(toplevelDecoration);
	if (xdgToplevel)
		xdg_toplevel_destroy(xdgToplevel);
	if (xdgSurface)
		xdg_surface_destroy(xdgSurface);
	if (surface)
		wl_surface_destroy(surface);
	if (display)
		wl_display_disconnect(display);
}
void WaylandWindow::close() {}
void WaylandWindow::minimize() {}
void WaylandWindow::maximize() {}
void WaylandWindow::restore() {}
void WaylandWindow::warpPointer(glm::vec2 coords)
{
}
void WaylandWindow::showPointer() {}
void WaylandWindow::hidePointer() {}
void WaylandWindow::setXY() {}
void WaylandWindow::setWidthHeight() {}
void WaylandWindow::mouseCapture(bool capture) {}
#endif
#endif
