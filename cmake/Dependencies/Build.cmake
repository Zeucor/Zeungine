
# Dependencies
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

#New Dependency Declarations to the top!

# shaderc
message(STATUS "FetchContent: shaderc")
set(SPIRV_SKIP_TESTS ON)
set(BUILD_TESTING OFF)
set(GLSLANG_ENABLE_INSTALL OFF)
set(SHADERC_SKIP_INSTALL ON)
set(SHADERC_SKIP_TESTS ON)
set(SHADERC_SKIP_EXAMPLES ON)
set(SPIRV_SKIP_EXECUTABLES ON)
set(SKIP_SPIRV_TOOLS_INSTALL ON)
# set(SHADERC_ENABLE_SHARED_CRT OFF)
# SPIRV-Headers
FetchContent_Declare(
    SPIRV-Headers
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
    GIT_TAG vulkan-sdk-1.4.304
)
FetchContent_MakeAvailable(SPIRV-Headers)
# SPIRV-tools
FetchContent_Declare(
    SPIRV-Tools
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
    GIT_TAG vulkan-sdk-1.4.304
)
FetchContent_MakeAvailable(SPIRV-Tools)
# glslang
FetchContent_Declare(
    glslang
    GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
    GIT_TAG vulkan-sdk-1.4.304
)
FetchContent_MakeAvailable(glslang)
# shaderc
FetchContent_Declare(
    shaderc
    GIT_REPOSITORY https://github.com/google/shaderc.git
    GIT_TAG v2024.4
    GIT_SUBMODULES "")
execute_process(
    COMMAND dir util
    WORKING_DIRECTORY ${shaderc_SOURCE_DIR}
    RESULT_VARIABLE SUBMODULE_RESULT)
message("SUBMODULE_RESULT: ${SUBMODULE_RESULT}")
FetchContent_MakeAvailable(shaderc)

# FfMpeG
message(STATUS "FetchContent: ffmpeg")
FetchContent_Declare(
    ffmpeg
    GIT_REPOSITORY https://github.com/FFmpeg/FFmpeg.git
    GIT_TAG n7.1
)
FetchContent_MakeAvailable(ffmpeg)
set(ffmpeg_CONFIGURE_OPTIONS --disable-shared --enable-static --disable-programs --disable-doc --prefix=${ffmpeg_BINARY_DIR})
if(RELEASE_OR_DEBUG MATCHES "Release")
    set(ffmpeg_CONFIGURE_OPTIONS ${ffmpeg_CONFIGURE_OPTIONS} --enable-optimizations --disable-debug)
else()
    set(ffmpeg_CONFIGURE_OPTIONS ${ffmpeg_CONFIGURE_OPTIONS} --disable-optimizations)
endif()
if(ANDROID)
    set(FFMPEG_AR ${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar)
    set(FFMPEG_RANLIB ${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ranlib)
    set(FFMPEG_STRIP ${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip)
    set(ffmpeg_CONFIGURE_OPTIONS ${ffmpeg_CONFIGURE_OPTIONS} --target-os=android --arch=${CMAKE_ANDROID_ARCH} --cross-prefix=${CROSS_PREFIX} --sysroot=${CMAKE_SYSROOT} --cpu=${CPU} --ar=${FFMPEG_AR} --strip=${FFMPEG_STRIP} --ranlib=${FFMPEG_RANLIB} --enable-cross-compile)
    set(SHELL bash)
elseif(WINDOWS)
    set(ffmpeg_CONFIGURE_OPTIONS ${ffmpeg_CONFIGURE_OPTIONS} --disable-asm --disable-mmx --disable-mmxext --disable-sse2 --disable-x86asm --toolchain=msvc)
    set(SHELL sh)
else()
    set(SHELL bash)
endif()
set(ffmpeg_CONFIG_COMMAND ${SHELL} "./configure")
if(WINDOWS)
    set(ffmpeg_BUILD_COMMAND "C:\\msys64\\msys2_shell.cmd" "-defterm" "-no-start" "-mingw64" "-here" "-use-full-path" "-c" "make")
    set(ffmpeg_INSTALL_COMMAND "C:\\msys64\\msys2_shell.cmd" "-defterm" "-no-start" "-mingw64" "-here" "-use-full-path" "-c" "make install")
else()
    set(ffmpeg_BUILD_COMMAND make)
    set(ffmpeg_INSTALL_COMMAND make install)
endif()
message(STATUS "ffmpeg-configure")
execute_process(
    COMMAND ${ffmpeg_CONFIG_COMMAND} ${ffmpeg_CONFIGURE_OPTIONS}
    WORKING_DIRECTORY ${ffmpeg_SOURCE_DIR}
    RESULT_VARIABLE ffmpeg_ConfigureResult)
if(ffmpeg_ConfigureResult)
    message(FATAL_ERROR "ffmpeg-configure: ${ffmpeg_ConfigureResult}")
else()
    message(STATUS "ffmpeg-configure: success")
endif()
message(STATUS "ffmpeg-build")
execute_process(
    COMMAND ${ffmpeg_BUILD_COMMAND}
    RESULT_VARIABLE ffmpeg_BuildResult
    WORKING_DIRECTORY ${ffmpeg_SOURCE_DIR})
if(ffmpeg_BuildResult)
    message(FATAL_ERROR "ffmpeg-build: failure: ${ffmpeg_ConfigureResult}")
else()
    message(STATUS "ffmpeg-build: success")
endif()
message(STATUS "ffmpeg-install: ${ffmpeg_INSTALL_COMMAND}")
execute_process(
	COMMAND ${ffmpeg_INSTALL_COMMAND}
    RESULT_VARIABLE ffmpeg_InstallResult
    WORKING_DIRECTORY ${ffmpeg_SOURCE_DIR})
if(ffmpeg_InstallResult)
    message(FATAL_ERROR "ffmpeg-install: failure: ${ffmpeg_ConfigureResult}")
else()
    message(STATUS "ffmpeg-install: success")
endif()
# end of ffmegp(!)

# Freetype
message(STATUS "FetchContent: freetype")
set(SKIP_INSTALL_ALL ON CACHE BOOL "Disable installation for fetched content" FORCE)
FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
    GIT_TAG master
)
FetchContent_MakeAvailable(freetype)
set_target_properties(freetype PROPERTIES DEBUG_POSTFIX "")
set_target_properties(freetype PROPERTIES RELEASE_POSTFIX "")
set_target_properties(freetype PROPERTIES RELWITHDEBINFO_POSTFIX "")
set_target_properties(freetype PROPERTIES MINSIZEREL_POSTFIX "")
include_directories(${freetype_SOURCE_DIR}/include)

# BVH
message(STATUS "FetchContent: bvh")
FetchContent_Declare(
    bvh
    GIT_REPOSITORY https://github.com/ZeunO8/bvh.git
    GIT_TAG master
)
FetchContent_MakeAvailable(bvh)
include_directories(${bvh_SOURCE_DIR}/src)

# GLM
message(STATUS "FetchContent: glm")
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG master
)
FetchContent_MakeAvailable(glm)
include_directories(${glm_SOURCE_DIR})

# STB
message(STATUS "FetchContent: stb")
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)
include_directories(${stb_SOURCE_DIR})

# lunasvg
message(STATUS "FetchContent: lunasvg")
set(LUNASVG_INSTALL OFF)
set(LUNASVG_BUILD_EXAMPLES OFF)
set(PLUTOVG_INSTALL OFF)
set(PLUTOVG_BUILD_EXAMPLES OFF)
FetchContent_Declare(
    lunasvg
    GIT_REPOSITORY https://github.com/ZeunO8/lunasvg.git
    GIT_TAG master
)
message(STATUS "FetchContent_MakeAvailable: lunasvg")
FetchContent_MakeAvailable(lunasvg)
include_directories(${lunasvg_SOURCE_DIR}/include)