# Options
#
option(BUILD_GL             "Whether to build OpenGL module"        OFF)
option(BUILD_EGL            "Whether to build EGL module"           OFF)
option(BUILD_VULKAN         "Whether to build Vulkan module"        ON)
option(BUILD_EDITOR         "Whether to build the Zeungine editor"  ON)
option(ZG_TYPE              "STATIC or SHARED"                      "STATIC")
option(ZG_INSTALL           "Whether to install Zeungine"           ON)
option(ZG_INSTALL_TESTS     "Whether to install Zeungine tests"     ON)
option(ZG_PACKAGE           "Whether to package Zeungine"           ON)
set   (ZG_SRC_ABS           ${CMAKE_CURRENT_LIST_DIR}/..) # valid in, in source configures only
get_filename_component(ZG_SRC_ABS "${ZG_SRC_ABS}" ABSOLUTE)