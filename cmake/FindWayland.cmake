# FindWayland.cmake

find_path(WAYLAND_INCLUDE_DIR NAMES wayland-client.h)
find_library(WAYLAND_LIBRARIES NAMES wayland-client)

if(WAYLAND_INCLUDE_DIR AND WAYLAND_LIBRARIES)
    set(WAYLAND_FOUND TRUE)
else()
    set(WAYLAND_FOUND FALSE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Wayland DEFAULT_MSG WAYLAND_LIBRARIES WAYLAND_INCLUDE_DIR)
