cmake_minimum_required(VERSION 3.29.4)
project(Zeungine VERSION 0.1.0)


set(CMAKE_CXX_STANDARD 20)

#Dependencies

# Fenster
add_subdirectory(vendor/fenster build/build_fenster)
include_directories(vendor/fenster/include)

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
add_subdirectory(vendor/hscpp)
include_directories(vendor/hscpp/include)

# filewatch
add_subdirectory(vendor/filewatch)
include_directories(vendor/filewatch)

include_directories(include)

option(BUILD_FENSTER "Whether to build Fenster module" ON)
option(BUILD_OPENGL "Whether to build OpenGL module" ON)

set(ZG_SOURCES
    src/IEntity.cpp
    src/IScene.cpp
    src/IWindow.cpp
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
    list(APPEND ZG_SOURCES
        src/modules/gl/RenderWindow.cpp
        src/modules/gl/GLEntity.cpp
        src/modules/gl/GLScene.cpp
        src/modules/gl/ISizable.cpp
        src/modules/gl/entities/AssetBrowser.cpp
        src/modules/gl/entities/Button.cpp
        src/modules/gl/entities/Console.cpp
        src/modules/gl/entities/Cube.cpp
        src/modules/gl/entities/Dialog.cpp
        src/modules/gl/entities/DropdownMenu.cpp
        src/modules/gl/entities/Input.cpp
        src/modules/gl/entities/Panel.cpp
        src/modules/gl/entities/Plane.cpp
        src/modules/gl/entities/SkyBox.cpp
        src/modules/gl/entities/Status.cpp
        src/modules/gl/entities/Tabs.cpp
        src/modules/gl/entities/TextView.cpp
        src/modules/gl/entities/Toolbar.cpp
        src/modules/gl/lights/DirectionalLight.cpp
        src/modules/gl/lights/PointLight.cpp
        src/modules/gl/lights/SpotLight.cpp
        src/modules/gl/shaders/Shader.cpp
        src/modules/gl/shaders/ShaderFactory.cpp
        src/modules/gl/shaders/ShaderManager.cpp
        src/modules/gl/textures/Texture.cpp
        src/modules/gl/textures/TextureFactory.cpp
        src/modules/gl/textures/TextureLoader.cpp
        src/modules/gl/textures/Framebuffer.cpp
        src/modules/gl/textures/FramebufferFactory.cpp
        src/modules/gl/vaos/VAO.cpp
        src/modules/gl/vaos/VAOFactory.cpp
        src/modules/gl/vp/View.cpp
        src/modules/gl/vp/Projection.cpp
        src/modules/gl/vp/VML.cpp
        src/modules/gl/gl.c
        src/modules/gl/fonts/freetype/Freetype.cpp
        src/modules/gl/raytracing/BVH.cpp)
    if(WIN32)
        list(APPEND ZG_SOURCES src/modules/gl/wgl.c src/modules/gl/windows/WIN32Window.cpp)
    elseif(UNIX AND NOT APPLE)
        list(APPEND ZG_SOURCES src/modules/gl/glx.c)
    elseif(ANDROID OR IOS)
        list(APPEND ZG_SOURCES src/modules/gl/egl.c)
    endif()
endif()

add_library(zeungine STATIC ${ZG_SOURCES})

if(BUILD_FENSTER)
    target_link_libraries(zeungine fenster)
endif()
if(BUILD_OPENGL)
    if(WIN32)
        find_package(OpenGL REQUIRED)
        target_link_libraries(zeungine OpenGL::GL)
    endif()
endif()
target_link_libraries(zeungine freetype)
target_link_libraries(zeungine bvh)
target_link_libraries(zeungine hscpp)

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
if(BUILD_FENSTER)
    create_test(FensterWindow tests/FensterWindow.cpp "" FALSE)
endif()
if(BUILD_OPENGL)
    create_test(GLTest tests/GLTest.cpp "" FALSE)
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