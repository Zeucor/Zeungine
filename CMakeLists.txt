cmake_minimum_required(VERSION 3.22.1)
project(Zeungine VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

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

if(LINUX OR MACOS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -fPIC")
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    endif()
    set(BUILD_SHARED_LIBS ON)
elseif(WINDOWS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4068")
endif()

#Dependencies

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

include_directories(include)

option(BUILD_OPENGL "Whether to build OpenGL module" ON)

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
)

if(BUILD_FENSTER)
    list(APPEND ZG_SOURCES src/modules/fenster/Fenster.cpp)
endif()
if(BUILD_OPENGL)
    add_definitions(-DUSE_GL)
    list(APPEND ZG_SOURCES
        src/Window.cpp
        src/renderers/GLRenderer.cpp
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
        src/gl.c
        src/fonts/freetype/Freetype.cpp
        src/raytracing/BVH.cpp)
    if(WIN32)
        list(APPEND ZG_SOURCES src/wgl.c src/windows/WIN32Window.cpp)
    elseif(LINUX)
        list(APPEND ZG_SOURCES src/glx.c src/windows/X11Window.cpp)
    elseif(MACOS)
    list(APPEND ZG_SOURCES src/windows/MacOSWindow.cpp)
    elseif(ANDROID OR IOS)
        list(APPEND ZG_SOURCES src/egl.c)
    endif()
endif()

if(WIN32)
    set(ZG_LIBRARY_TYPE STATIC)
else()
    set(ZG_LIBRARY_TYPE SHARED)
endif()

add_library(zeungine ${ZG_LIBRARY_TYPE} ${ZG_SOURCES})

if(BUILD_OPENGL)
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
    target_link_libraries(zeungine X11 Xrandr Xext Xfixes xkbcommon Xrender)
endif()

function(create_test TEST_NAME TEST_SOURCE TEST_ARGS LIBRARY)
    if(LIBRARY)
        add_library(${TEST_NAME} SHARED ${TEST_SOURCE})
    else()
        add_executable(${TEST_NAME} ${TEST_SOURCE})
    endif()
    target_link_libraries(${TEST_NAME} zeungine)
    if(UNIX AND NOT APPLE)
        target_link_libraries(${TEST_NAME} ${X11_LIBRARIES})
    endif()
    if(NOT LIBRARY)
        add_test(NAME ${TEST_NAME} COMMAND $<TARGET_FILE_DIR:${TEST_NAME}>/${TEST_NAME}${TEST_EXT} ${TEST_ARGS})
    endif()
endfunction()

include(CTest)
enable_testing()
if(BUILD_OPENGL)
    create_test(LightingTest tests/LightingTest.cpp "" FALSE)
    create_test(SimpleCubeTest tests/SimpleCubeTest.cpp "" FALSE)
    add_executable(Editor src/editor/Editor.cpp src/editor/EditorScene.cpp src/editor/CodeScene.cpp)
    target_link_libraries(Editor zeungine)
    set(SKYBOX_OUT_DIRECTORY ${CMAKE_BINARY_DIR}/images/skybox)
    file(MAKE_DIRECTORY ${SKYBOX_OUT_DIRECTORY})
    configure_file(${CMAKE_SOURCE_DIR}/images/skybox/back.jpg ${SKYBOX_OUT_DIRECTORY}/back.jpg COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/images/skybox/front.jpg ${SKYBOX_OUT_DIRECTORY}/front.jpg COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/images/skybox/left.jpg ${SKYBOX_OUT_DIRECTORY}/left.jpg COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/images/skybox/right.jpg ${SKYBOX_OUT_DIRECTORY}/right.jpg COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/images/skybox/top.jpg ${SKYBOX_OUT_DIRECTORY}/top.jpg COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/images/skybox/bottom.jpg ${SKYBOX_OUT_DIRECTORY}/bottom.jpg COPYONLY)
endif()
set(FONTS_DIRECTORY ${CMAKE_BINARY_DIR}/fonts)
set(IMAGES_DIRECTORY ${CMAKE_BINARY_DIR}/images)
file(COPY ${CMAKE_SOURCE_DIR}/fonts/Roboto/Roboto-Regular.ttf DESTINATION ${FONTS_DIRECTORY}/Roboto)
file(COPY ${CMAKE_SOURCE_DIR}/images/zeungine-icon.png DESTINATION ${IMAGES_DIRECTORY})