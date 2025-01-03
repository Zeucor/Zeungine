cmake_minimum_required(VERSION 3.30)
project(AbstractNexus VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(vendor/fenster)
include_directories(vendor/fenster/include)
include_directories(vendor/glm)
include_directories(include)

option(BUILD_FENSTER "Whether to build Fenster module" ON)
option(BUILD_OPENGL "Whether to build OpenGL module" ON)

set(ANEX_SOURCES
    src/IEntity.cpp
    src/IScene.cpp
    src/IWindow.cpp
    src/Logger.cpp
    src/crypto/vector.cpp)

if(BUILD_FENSTER)
    list(APPEND ANEX_SOURCES src/modules/fenster/Fenster.cpp)
endif()
if(BUILD_OPENGL)
    list(APPEND ANEX_SOURCES
        src/modules/gl/GL.cpp
        src/modules/gl/GLEntity.cpp
        src/modules/gl/GLScene.cpp
        src/modules/gl/lights/DirectionalLight.cpp
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
        src/modules/gl/glad.c)
endif()

add_library(abstractnexus STATIC ${ANEX_SOURCES})

if(BUILD_FENSTER)
    target_link_libraries(abstractnexus fenster)
endif()
if(BUILD_OPENGL)
    if(WIN32)
        find_package(OpenGL REQUIRED)
        target_link_libraries(abstractnexus OpenGL::GL)
    endif()
endif()

function(create_test TEST_NAME TEST_SOURCE TEST_ARGS)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} abstractnexus)
    if(UNIX AND NOT APPLE)
        target_link_libraries(${TEST_NAME} ${X11_LIBRARIES})
    endif()
    add_test(NAME ${TEST_NAME} COMMAND $<TARGET_FILE_DIR:${TEST_NAME}>/${TEST_NAME}${TEST_EXT} ${TEST_ARGS})
endfunction()

include(CTest)
enable_testing()
if(BUILD_FENSTER)
    create_test(FensterWindow tests/FensterWindow.cpp "")
endif()
if(BUILD_OPENGL)
    create_test(GLWindow tests/GLWindow.cpp "")
endif()