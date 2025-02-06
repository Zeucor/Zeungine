#ifdef LINUX
#if defined(USE_WAYLAND)
#include <zg/windows/WaylandWindow.hpp>
#include <iostream>
using namespace zg;
static void registry_handler(void* data, struct wl_registry* registry, uint32_t id, const char* interface,
														 uint32_t version)
{
	auto& waylandWindow = *static_cast<WaylandWindow*>(data);
	std::cout << interface << std::endl;
	if (strcmp(interface, wl_compositor_interface.name) == 0)
	{
		waylandWindow.compositor = (wl_compositor*)wl_registry_bind(registry, id, &wl_compositor_interface, 1);
	}
	else if (strcmp(interface, wl_shell_interface.name) == 0)
	{
		waylandWindow.shell = (wl_shell*)wl_registry_bind(registry, id, &wl_shell_interface, 1);
	}
	else if (strcmp(interface, wl_shm_interface.name) == 0)
	{
		waylandWindow.shm = (wl_shm*)wl_registry_bind(registry, id, &wl_shm_interface, 1);
	}
	else if (strcmp(interface, wl_seat_interface.name) == 0)
	{
		// Binding to the wl_seat interface to get a valid seat
		waylandWindow.seat = (wl_seat*)wl_registry_bind(registry, id, &wl_seat_interface, 1);
	}
}
static void registry_remove_object(void* data, struct wl_registry* registry, uint32_t name) { /* no-op */ }
static struct wl_registry_listener registry_listener = {&registry_handler, &registry_remove_object};
void WaylandWindow::init(Window& renderWindow)
{
	renderWindowPointer = &renderWindow;
	windowType = WINDOW_TYPE_WAYLAND;
	display = wl_display_connect(nullptr);
	if (!display)
	{
		throw std::runtime_error("Failed to connect to Wayland display");
	}
	struct wl_registry* registry = wl_display_get_registry(display);
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
	seatPointer = wl_seat_get_pointer(seat);
	wl_shell_surface* shell_surface = wl_shell_get_shell_surface(shell, surface);
	wl_shell_surface_set_toplevel(shell_surface);
	wl_surface_commit(surface);
}
void WaylandWindow::postInit() {}
bool WaylandWindow::pollMessages() { wl_display_dispatch(display); }
void WaylandWindow::destroy() {}
void WaylandWindow::close() {}
void WaylandWindow::minimize() {}
void WaylandWindow::maximize() {}
void WaylandWindow::restore() {}
void WaylandWindow::warpPointer(glm::vec2 coords) {}
void WaylandWindow::showPointer() {}
void WaylandWindow::hidePointer() {}
void WaylandWindow::setXY() {}
void WaylandWindow::setWidthHeight() {}
void WaylandWindow::mouseCapture(bool capture) {}
#endif
#endif
