cmake_minimum_required(VERSION 3.22.1)
project(Zeungine VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(PkgConfig REQUIRED)

include(CTest)
enable_testing()

message(STATUS "Build type is ${CMAKE_BUILD_TYPE}")

# Set library directory
if(("${CMAKE_BUILD_TYPE}" MATCHES "Release") OR ("${CMAKE_BUILD_TYPE}" MATCHES "MinSizeRel"))
    set(RELEASE_OR_DEBUG "Release")
elseif(("${CMAKE_BUILD_TYPE}" MATCHES "Debug") OR ("${CMAKE_BUILD_TYPE}" MATCHES "RelWithDebInfo"))
    set(RELEASE_OR_DEBUG "Debug")
endif()
set(ZG_LIB_DIR "${CMAKE_SOURCE_DIR}/lib/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/${RELEASE_OR_DEBUG}")
message(STATUS "ZG_LIB_DIR: ${ZG_LIB_DIR}")

# Determine platform and set defines
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_definitions(-DLINUX)
    set(LINUX ON)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    add_definitions(-DMACOS)
    set(MACOS ON)
elseif(WIN32)
    add_definitions(-DWINDOWS)
    set(WINDOWS ON)
endif()

# Add build flags
if(LINUX OR MACOS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -fPIC")
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    endif()
    set(BUILD_SHARED_LIBS ON)
elseif(WINDOWS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4068")
endif()

# Build options
option(BUILD_GL "Whether to build OpenGL module" OFF)
option(BUILD_EGL "Whether to build EGL module" OFF)
option(BUILD_VULKAN "Whether to build Vulkan module" ON)
option(USE_SWIFTSHADER "Use SwiftShader as a software renderer" OFF)
option(BUILD_EDITOR "Whether to build the Zeungine editor" ON)

# Determine windower
if(LINUX)
    pkg_check_modules(XCB xcb)
    find_package(X11)
    find_package(Wayland)
    option(USE_X11 "Use X11 as a windower" ${X11_FOUND})
    option(USE_XCB "Use XCB as a windower" ${XCB_FOUND})
    option(USE_WAYLAND "Use Wayland as a windower" ${Wayland_FOUND})
elseif(WINDOWS)
    option(USE_WIN32 "Use Windows as a windower" ON)
elseif(MACOS)
    option(USE_MACOS "Use Mac as a windower" ON)
endif()

if(USE_SWIFTSHADER)
    message(STATUS "Compiling with SwiftShader Vulkan backend")
    add_definitions(-DUSE_VULKAN)
    add_definitions(-DUSE_SWIFTSHADER)
    set(SWIFTSHADER_INCLUDE_DIR "include/SwiftShader")
elseif(BUILD_VULKAN)
    add_definitions(-DUSE_VULKAN)
    message(STATUS "Compiling with Vulkan backend")
elseif(BUILD_GL)
    message(STATUS "Compiling with OpenGL backend")
    add_definitions(-DUSE_GL)
elseif(BUILD_EGL)
    message(STATUS "Compiling with EGL backend")
    add_definitions(-DUSE_EGL)
endif()

# Dependencies

# Freetype
add_subdirectory(vendor/freetype build/build_freetype)
set_target_properties(freetype PROPERTIES DEBUG_POSTFIX "")
set_target_properties(freetype PROPERTIES RELEASE_POSTFIX "")
set_target_properties(freetype PROPERTIES RELWITHDEBINFO_POSTFIX "")
set_target_properties(freetype PROPERTIES MINSIZEREL_POSTFIX "")
include_directories(vendor/freetype/include)

# BVH
add_subdirectory(vendor/bvh build/build_bvh)
include_directories(vendor/bvh/src)

# GLM
include_directories(vendor/glm)

# STB
include_directories(vendor/stb)

# hscpp
set(HSCPP_BUILD_EXAMPLES OFF)
set(HSCPP_BUILD_TESTS OFF)
set(HSCPP_BUILD_EXTENSION_MEM OFF)
add_subdirectory(vendor/hscpp)
include_directories(vendor/hscpp/include)

# filewatch
set(BuildTests OFF)
add_subdirectory(vendor/filewatch)
include_directories(vendor/filewatch)

# Windowing 'USE_X's
if(LINUX)
    if(USE_XCB)
        message(STATUS "Compiling with XCB windowing backend")
        add_definitions(-DUSE_XCB)
        if(NOT USE_XCB)
            set(USE_XCB ON)
        endif()
    endif()
    if(USE_X11)
        message(STATUS "Compiling with X11 windowing backend")
        add_definitions(-DUSE_X11)
        if(NOT USE_X11)
            set(USE_X11 ON)
        endif()
    endif()
    if(USE_WAYLAND)
        message(STATUS "Compiling with Wayland windowing backend")
        add_definitions(-DUSE_WAYLAND)
        if(NOT USE_WAYLAND)
            set(USE_WAYLAND ON)
        endif()
    endif()
elseif(WINDOWS)
    message(STATUS "Compiling with WIN32 windowing backend")
    if(NOT USE_WIN32)
        set(USE_WIN32 ON)
    endif()
elseif(MACOS)
    message(STATUS "Compiling with MacOS windowing backend")
    if(NOT USE_MACOS)
        set(USE_MACOS ON)
    endif()
endif()

# Includes
include_directories(include)
if(USE_SWIFTSHADER)
    include_directories(${SWIFTSHADER_INCLUDE_DIR})
endif()

# Sources
set(ZG_SOURCES
    src/interfaces/IEntity.cpp
    src/interfaces/IScene.cpp
    src/interfaces/IWindow.cpp
    src/Logger.cpp
    src/SharedLibrary.cpp
    src/crypto/vector.cpp
    src/images/ImageLoader.cpp
    src/filesystem/File.cpp
    src/filesystem/Directory.cpp
    src/editor/Hotswapper.cpp
    src/strings/HookedConsole.cpp
    src/interfaces/IPlatformWindow.cpp
)
list(APPEND ZG_SOURCES
    src/Window.cpp
    src/Entity.cpp
    src/Scene.cpp
    src/interfaces/ISizable.cpp
    src/entities/AssetBrowser.cpp
    src/entities/Button.cpp
    src/entities/Console.cpp
    src/entities/Cube.cpp
    src/entities/Dialog.cpp
    src/entities/DropdownMenu.cpp
    src/entities/Input.cpp
    src/entities/Panel.cpp
    src/entities/Plane.cpp
    src/entities/SkyBox.cpp
    src/entities/StatusText.cpp
    src/entities/Tabs.cpp
    src/entities/TextView.cpp
    src/entities/Toolbar.cpp
    src/lights/DirectionalLight.cpp
    src/lights/PointLight.cpp
    src/lights/SpotLight.cpp
    src/shaders/Shader.cpp
    src/shaders/ShaderFactory.cpp
    src/shaders/ShaderManager.cpp
    src/textures/Texture.cpp
    src/textures/TextureFactory.cpp
    src/textures/TextureLoader.cpp
    src/textures/Framebuffer.cpp
    src/textures/FramebufferFactory.cpp
    src/vaos/VAO.cpp
    src/vaos/VAOFactory.cpp
    src/vp/View.cpp
    src/vp/Projection.cpp
    src/vp/VML.cpp
    src/fonts/freetype/Freetype.cpp
    src/raytracing/BVH.cpp)
if(WIN32)
    list(APPEND ZG_SOURCES src/windows/WIN32Window.cpp)
elseif(LINUX)
    if(USE_X11)
        list(APPEND ZG_SOURCES src/windows/X11Window.cpp)
    endif()
    if(USE_XCB OR USE_X11)
        list(APPEND ZG_SOURCES src/windows/XCBWindow.cpp)
    endif()
    if(USE_WAYLAND)
        list(APPEND ZG_SOURCES
            src/windows/WaylandWindow.cpp
            src/wayland/wayland-xdg-shell-client-protocol.c
            src/wayland/xdg-decoration-unstable-v1-client-protocol.c)
    endif()
elseif(MACOS)
    list(APPEND ZG_SOURCES src/windows/MacOSWindow.mm)
endif()
if(BUILD_GL)
    list(APPEND ZG_SOURCES src/renderers/GLRenderer.cpp src/gl.c)
    if(WIN32)
        list(APPEND ZG_SOURCES src/wgl.c)
    elseif(LINUX)
        list(APPEND ZG_SOURCES src/glx.c)
    elseif(ANDROID OR IOS)
        list(APPEND ZG_SOURCES src/egl.c)
    endif()
elseif(BUILD_EGL)
    list(APPEND ZG_SOURCES src/renderers/EGLRenderer.cpp)
elseif(BUILD_VULKAN)
    list(APPEND ZG_SOURCES src/renderers/VulkanRenderer.cpp)
endif()

# Library type, Static if Windows, otherwise Shared
if(WIN32)
    set(ZG_LIBRARY_TYPE STATIC)
else()
    set(ZG_LIBRARY_TYPE SHARED)
endif()

# Add Zeungine library
add_library(zeungine ${ZG_LIBRARY_TYPE} ${ZG_SOURCES})

# Link libraries
if(BUILD_GL)
    if(WIN32)
        find_package(OpenGL REQUIRED)
        target_link_libraries(zeungine OpenGL::GL)
    elseif(LINUX)
        target_link_libraries(zeungine GL)
    endif()
endif()
target_link_libraries(zeungine freetype)
target_link_libraries(zeungine bvh)
target_link_libraries(zeungine hscpp)
if(LINUX)
    if(USE_X11)
        target_link_libraries(zeungine X11 Xrandr Xext Xfixes xkbcommon Xrender)
    endif()
    if(USE_XCB OR USE_X11)
        target_link_libraries(zeungine xcb xkbcommon xcb-util xcb-keysyms xcb-xfixes X11 Xrandr Xext Xfixes xkbcommon Xrender)
    endif()
    if(USE_WAYLAND)
        target_link_libraries(zeungine wayland-client)
    endif()
elseif(MACOS)
    if(BUILD_GL)
        find_package(OpenGL REQUIRED)
        target_link_libraries(zeungine ${OPENGL_LIBRARY})
    endif()
    target_link_libraries(zeungine "-framework Cocoa" "-framework QuartzCore" "-framework Metal")
endif()
if(BUILD_VULKAN AND NOT USE_SWIFTSHADER)
    find_package(Vulkan REQUIRED)
    include_directories(${Vulkan_INCLUDE_DIRS})
    target_link_libraries(zeungine ${Vulkan_LIBRARIES})
endif()
if(LINUX)
    if(USE_SWIFTSHADER)
        target_link_libraries(zeungine
            ${ZG_LIB_DIR}/vulkan/libvulkan.so.1
            ${ZG_LIB_DIR}/libvk_swiftshader.so)
    endif()
    target_link_libraries(zeungine
        ${ZG_LIB_DIR}/libshaderc_shared.so)
elseif(MACOS)
    if(USE_SWIFTSHADER)
        target_link_libraries(zeungine
            ${ZG_LIB_DIR}/vulkan/libvulkan.dylib
            ${ZG_LIB_DIR}/libvk_swiftshader.dylib)
    endif()
    target_link_libraries(zeungine
        ${ZG_LIB_DIR}/libshaderc_shared.dylib)
endif()

# create_test function
function(create_test TEST_NAME TEST_SOURCE TEST_ARGS LIBRARY)
    if(LIBRARY)
        add_library(${TEST_NAME} SHARED ${TEST_SOURCE})
    else()
        add_executable(${TEST_NAME} ${TEST_SOURCE})
    endif()
    target_link_libraries(${TEST_NAME} zeungine)
    if(LINUX)
        target_link_libraries(${TEST_NAME} ${X11_LIBRARIES})
    endif()
    if(NOT LIBRARY)
        add_test(NAME ${TEST_NAME} COMMAND $<TARGET_FILE_DIR:${TEST_NAME}>/${TEST_NAME}${TEST_EXT} ${TEST_ARGS})
    endif()
endfunction()

# Tests
create_test(LightingTest tests/LightingTest.cpp "" FALSE)
create_test(SimpleCubeTest tests/SimpleCubeTest.cpp "" FALSE)
create_test(SimpleWindowTest tests/SimpleWindowTest.cpp "" FALSE)

# Editor
if(BUILD_EDITOR)
    add_executable(Editor src/editor/Editor.cpp src/editor/EditorScene.cpp src/editor/CodeScene.cpp)
    target_link_libraries(Editor zeungine)
endif()

# Required test assets
set(SKYBOX_OUT_DIRECTORY ${CMAKE_BINARY_DIR}/images/skybox)
file(MAKE_DIRECTORY ${SKYBOX_OUT_DIRECTORY})
configure_file(${CMAKE_SOURCE_DIR}/images/skybox/back.jpg ${SKYBOX_OUT_DIRECTORY}/back.jpg COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/images/skybox/front.jpg ${SKYBOX_OUT_DIRECTORY}/front.jpg COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/images/skybox/left.jpg ${SKYBOX_OUT_DIRECTORY}/left.jpg COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/images/skybox/right.jpg ${SKYBOX_OUT_DIRECTORY}/right.jpg COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/images/skybox/top.jpg ${SKYBOX_OUT_DIRECTORY}/top.jpg COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/images/skybox/bottom.jpg ${SKYBOX_OUT_DIRECTORY}/bottom.jpg COPYONLY)
set(FONTS_DIRECTORY ${CMAKE_BINARY_DIR}/fonts)
set(IMAGES_DIRECTORY ${CMAKE_BINARY_DIR}/images)
file(COPY ${CMAKE_SOURCE_DIR}/fonts/Roboto/Roboto-Regular.ttf DESTINATION ${FONTS_DIRECTORY}/Roboto)
file(COPY ${CMAKE_SOURCE_DIR}/images/zeungine-icon.png DESTINATION ${IMAGES_DIRECTORY})
