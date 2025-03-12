
# Dependencies
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

#New Dependency Declarations to the top!
# zlib
FetchContent_Declare(zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG 0f51fb4933fc9ce18199cb2554dacea8033e7fd3)
FetchContent_GetProperties(zlib)
if(NOT zlib_POPULATED)
    FetchContent_Populate(zlib)
endif()

set(ZLIB_SOURCES
    "${zlib_SOURCE_DIR}/zutil.c"
    "${zlib_SOURCE_DIR}/uncompr.c"
    "${zlib_SOURCE_DIR}/trees.c"
    "${zlib_SOURCE_DIR}/inftrees.c"
    "${zlib_SOURCE_DIR}/inflate.c"
    "${zlib_SOURCE_DIR}/inffast.c"
    "${zlib_SOURCE_DIR}/infback.c"
    "${zlib_SOURCE_DIR}/gzwrite.c"
    "${zlib_SOURCE_DIR}/gzread.c"
    "${zlib_SOURCE_DIR}/gzlib.c"
    "${zlib_SOURCE_DIR}/gzclose.c"
    "${zlib_SOURCE_DIR}/deflate.c"
    "${zlib_SOURCE_DIR}/crc32.c"
    "${zlib_SOURCE_DIR}/compress.c"
    "${zlib_SOURCE_DIR}/adler32.c"
)

add_library(zlib STATIC ${ZLIB_SOURCES})
target_include_directories(zlib PRIVATE ${zlib_SOURCE_DIR})

# bzip2
FetchContent_Declare(bzip2
    GIT_REPOSITORY https://github.com/centricular/bzip2.git
    GIT_TAG 928fd716ecffa87f47d47585a9e09ff364c7689a)
FetchContent_MakeAvailable(bzip2)
FetchContent_GetProperties(bzip2)
if(NOT bzip2_POPULATED)
    FetchContent_Populate(bzip2)
endif()

set(BZIP2_SOURCES
    "${bzip2_SOURCE_DIR}/blocksort.c"
    "${bzip2_SOURCE_DIR}/bzip2.c"
    "${bzip2_SOURCE_DIR}/bzlib.c"
    "${bzip2_SOURCE_DIR}/compress.c"
    "${bzip2_SOURCE_DIR}/crctable.c"
    "${bzip2_SOURCE_DIR}/decompress.c"
    "${bzip2_SOURCE_DIR}/dlltest.c"
    "${bzip2_SOURCE_DIR}/huffman.c"
    "${bzip2_SOURCE_DIR}/mk251.c"
    "${bzip2_SOURCE_DIR}/randtable.c"
    "${bzip2_SOURCE_DIR}/spewG.c"
    "${bzip2_SOURCE_DIR}/unzcrash.c"
)

add_library(bzip2 STATIC ${BZIP2_SOURCES})
target_include_directories(bzip2 PRIVATE ${bzip2_SOURCE_DIR})

# miniaudio
message(STATUS "FetchContent_Declare: miniaudio")
FetchContent_Declare(
    miniaudio
    GIT_REPOSITORY https://github.com/mackron/miniaudio.git
    GIT_TAG master)
FetchContent_MakeAvailable(miniaudio)

# swiftshader
set(SWIFTSHADER_BUILD_TESTS OFF)
set(BUILD_TESTING OFF)

##
set(SPIRV_SKIP_TESTS ON)
set(SKIP_GLSLANG_INSTALL ON)
set(SHADERC_SKIP_INSTALL ON)
set(SHADERC_SKIP_TESTS ON)
set(SHADERC_SKIP_EXAMPLES ON)
set(SHADERC_COMPILE_GLSLC OFF)
set(SPIRV_SKIP_EXECUTABLES ON)
set(SKIP_SPIRV_TOOLS_INSTALL ON)
set(GLSLANG_ENABLE_INSTALL OFF)
set(ENABLE_GLSLANG_BINARIES OFF)
##

cmake_policy(SET CMP0097 NEW)
include(ExternalProject)
message(STATUS "FetchContent_Declare: swiftshader")
FetchContent_Declare(swiftshader
    GIT_REPOSITORY https://github.com/ZeunO8/swiftshader.git
    GIT_TAG win_x86_64)
FetchContent_MakeAvailable(swiftshader)

# FfMpeG
message(STATUS "|FetchContent&Build&buildInstall|OneTime: ffmpeg")
FetchContent_Declare(ffmpeg
    GIT_REPOSITORY https://github.com/FFmpeg/FFmpeg.git
    GIT_TAG n7.1)
FetchContent_MakeAvailable(ffmpeg)
set(ffmpeg_CONFIGURE_OPTIONS --disable-programs --disable-doc --prefix=${ffmpeg_BINARY_DIR})
if(ZG_TYPE STREQUAL STATIC)
    set(ffmpeg_CONFIGURE_OPTIONS ${ffmpeg_CONFIGURE_OPTIONS} --enable-static --disable-shared)
    set(ffmpeg_CONFIGURE_OPTIONS ${ffmpeg_CONFIGURE_OPTIONS} --enable-decoder=aac --enable-parser=aac)
elseif(ZG_TYPE STREQUAL SHARED)
    set(ffmpeg_CONFIGURE_OPTIONS ${ffmpeg_CONFIGURE_OPTIONS} --enable-shared --disable-static)
endif()
if(RELEASE_OR_DEBUG STREQUAL Release)
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
elseif(WIN32)
    set(ffmpeg_CONFIGURE_OPTIONS ${ffmpeg_CONFIGURE_OPTIONS} --disable-asm --disable-mmx --disable-mmxext --disable-sse2 --disable-x86asm --toolchain=msvc)
    set(SHELL sh)
else()
    set(SHELL bash)
endif()
set(ffmpeg_CONFIG_COMMAND ${SHELL} "./configure")
if(WIN32)
    set(ffmpeg_BUILD_COMMAND "C:\\msys64\\msys2_shell.cmd" "-defterm" "-no-start" "-mingw64" "-here" "-use-full-path" "-c" "make")
    set(ffmpeg_INSTALL_COMMAND "C:\\msys64\\msys2_shell.cmd" "-defterm" "-no-start" "-mingw64" "-here" "-use-full-path" "-c" "make install")
else()
    set(ffmpeg_BUILD_COMMAND make)
    set(ffmpeg_INSTALL_COMMAND make install)
endif()
message(STATUS "ffmpeg-configure: ${ffmpeg_SOURCE_DIR} ${ffmpeg_CONFIG_COMMAND}")
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
set(FT_DISABLE_ZLIB ON)
set(FT_DISABLE_BZIP2 ON)
set(FT_DISABLE_PNG ON)
set(FT_DISABLE_HARFBUZZ ON)
set(FT_DISABLE_BROTLI ON)
set(SKIP_INSTALL_ALL ON CACHE BOOL "Disable installation for fetched content" FORCE)
FetchContent_Declare(freetype
    GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
    GIT_TAG master)
FetchContent_MakeAvailable(freetype)
set_target_properties(freetype PROPERTIES DEBUG_POSTFIX "")
set_target_properties(freetype PROPERTIES RELEASE_POSTFIX "")
set_target_properties(freetype PROPERTIES RELWITHDEBINFO_POSTFIX "")
set_target_properties(freetype PROPERTIES MINSIZEREL_POSTFIX "")

# BVH
message(STATUS "FetchContent: bvh")
FetchContent_Declare(bvh
    GIT_REPOSITORY https://github.com/ZeunO8/bvh.git
    GIT_TAG master)
FetchContent_MakeAvailable(bvh)

# GLM
message(STATUS "FetchContent: glm")
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG master)
FetchContent_MakeAvailable(glm)

# STB
message(STATUS "FetchContent: stb")
FetchContent_Declare(stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master)
FetchContent_MakeAvailable(stb)

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
