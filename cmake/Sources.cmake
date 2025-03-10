# Sources
set(ZG_SOURCES
    src/audio/AudioEngine.cpp
    src/audio/AudioPipeline.cpp
    src/audio/AudioStage.cpp
    src/audio/ISoundNode.cpp
    src/media/I1xCoder.cpp
    src/media/ReadMediaStream.cpp
    src/media/MediaStream.cpp
    src/media/AudioDecoder.cpp
    src/media/AudioEncoder.cpp
    src/media/VideoDecoder.cpp
    src/media/VideoEncoder.cpp
    src/media/entities/Video.cpp
    src/system/TabulatedIOLogger.cpp
    src/system/TerminalIO.cpp
    src/system/Budget.cpp
    src/Logger.cpp
    src/SharedLibrary.cpp
    src/crypto/vector.cpp
    src/images/ImageLoader.cpp
    src/images/SVGRasterize.cpp
    src/zgfilesystem/File.cpp
    src/zgfilesystem/Directory.cpp
    src/zgfilesystem/DirectoryWatcher.cpp
    src/editor/Hotswapper.cpp
    src/strings/HookedConsole.cpp
    src/strings/InFileProcessor.cpp
    src/interfaces/IPlatformWindow.cpp
    src/system/Command.cpp
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
    src/vp/VFBLR.cpp
    src/fonts/freetype/Freetype.cpp
    src/raytracing/BVH.cpp)
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
