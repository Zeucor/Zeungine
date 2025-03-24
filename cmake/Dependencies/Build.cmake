
# Dependencies
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

if(ANDROID)
    set(SHELL bash)
elseif(WIN32)
    set(SHELL "C:\\msys64\\msys2_shell.cmd" "-defterm" "-no-start" "-mingw64" "-here" "-use-full-path" "-c")
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} /EHsc)
else()
    set(SHELL bash)
endif()

#New Dependency Declarations to the top!

# OpenSSL
FetchContent_Declare(openssl
    GIT_REPOSITORY https://github.com/openssl/openssl.git
    GIT_TAG openssl-3.5)
FetchContent_GetProperties(openssl)
if(NOT openssl_POPULATED)
    FetchContent_Populate(openssl)
endif()

function(add_openssl_config VARI)
    if(WIN32)
        set(openssl_CONFIGURE "${openssl_CONFIGURE} ${VARI}" PARENT_SCOPE)
    else()
        set(openssl_CONFIGURE ${openssl_CONFIGURE} ${VARI} PARENT_SCOPE)
    endif()
endfunction()
if(ANDROID)
    set(openssl_BUILD_TYPE ${ANDROID_MARCH})
    add_openssl_config("./Configure")
    set(openssl_MAKE make)
    set(openssl_MAKE_INSTALL make install_sw install_ssldirs)
elseif(WINDOWS)
    add_openssl_config("perl Configure")
    set(openssl_MAKE "make")
    set(openssl_MAKE_INSTALL "make install_sw install_ssldirs")
    if(${RELEASE_OR_DEBUG} STREQUAL "Debug")
        set(openssl_BUILD_TYPE debug-VC-WIN64A)
    else()
        set(openssl_BUILD_TYPE VC-WIN64A)
    endif()
else()
    add_openssl_config("./Configure")
    set(openssl_MAKE make)
    set(openssl_MAKE_INSTALL make install_sw install_ssldirs)
endif()
add_openssl_config(no-shared)
add_openssl_config(--prefix=${openssl_BINARY_DIR})
add_openssl_config(--openssldir=${openssl_BINARY_DIR})
if(ANDROID)
    set(openssl_CONFIGURE
        env
        CC=${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android34-clang
        CXX=${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android34-clang++
        AR=${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar
        LD=${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/ld.lld
        ${openssl_CONFIGURE}
        --sysroot=${CMAKE_SYSROOT}
    )
endif()
message(STATUS "openssl-configure: \"${openssl_CONFIGURE}\"")
execute_process(
    COMMAND ${SHELL} ${openssl_CONFIGURE}
    WORKING_DIRECTORY ${openssl_SOURCE_DIR}
    RESULT_VARIABLE openssl_ConfigureResult)
if(openssl_ConfigureResult)
    message(FATAL_ERROR "openssl-configure: ${openssl_ConfigureResult}")
else()
    message(STATUS "openssl-configure: success")
endif()
add_custom_target(openssl ALL
    COMMAND ${SHELL} ${openssl_MAKE}
    COMMAND ${SHELL} ${openssl_MAKE_INSTALL}
    WORKING_DIRECTORY ${openssl_SOURCE_DIR}
    COMMENT "Building OpenSSL"
)

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
FetchContent_Declare(
    miniaudio
    GIT_REPOSITORY https://github.com/mackron/miniaudio.git
    GIT_TAG master)
FetchContent_GetProperties(miniaudio)
if(NOT miniaudio_POPULATED)
    FetchContent_Populate(miniaudio)
endif()

# # swiftshader
# set(SWIFTSHADER_BUILD_TESTS OFF)
# set(BUILD_TESTING OFF)

# ##
# set(SPIRV_SKIP_TESTS ON)
# set(SKIP_GLSLANG_INSTALL ON)
# set(SHADERC_SKIP_INSTALL ON)
# set(SHADERC_SKIP_TESTS ON)
# set(SHADERC_SKIP_EXAMPLES ON)
# set(SHADERC_COMPILE_GLSLC OFF)
# set(SPIRV_SKIP_EXECUTABLES ON)
# set(SKIP_SPIRV_TOOLS_INSTALL ON)
# set(GLSLANG_ENABLE_INSTALL OFF)
# set(ENABLE_GLSLANG_BINARIES OFF)
# ##

# cmake_policy(SET CMP0097 NEW)
# include(ExternalProject)
# message(STATUS "FetchContent_Declare: swiftshader")
# FetchContent_Declare(swiftshader
#     GIT_REPOSITORY https://github.com/ZeunO8/swiftshader.git
#     GIT_TAG win_x86_64)
# FetchContent_MakeAvailable(swiftshader)

# FfMpeG
message(STATUS "|FetchContent&Build&buildInstall|OneTime: ffmpeg")
FetchContent_Declare(ffmpeg
    GIT_REPOSITORY https://github.com/FFmpeg/FFmpeg.git
    GIT_TAG n7.1)
FetchContent_MakeAvailable(ffmpeg)
function(add_ffmpeg_config VARI)
    if(WIN32)
        set(ffmpeg_CONFIGURE "${ffmpeg_CONFIGURE} ${VARI}" PARENT_SCOPE)
    else()
        set(ffmpeg_CONFIGURE ${ffmpeg_CONFIGURE} ${VARI} PARENT_SCOPE)
    endif()
endfunction()
add_ffmpeg_config(--disable-programs)
add_ffmpeg_config(--disable-doc)
add_ffmpeg_config(--prefix=${ffmpeg_BINARY_DIR})
if(ZG_TYPE STREQUAL STATIC)
    add_ffmpeg_config(--enable-static)
    add_ffmpeg_config(--disable-shared)
    add_ffmpeg_config(--enable-decoder=aac)
    add_ffmpeg_config(--enable-parser=aac)
elseif(ZG_TYPE STREQUAL SHARED)
    add_ffmpeg_config(--enable-shared)
    add_ffmpeg_config(--disable-static)
endif()
if(RELEASE_OR_DEBUG STREQUAL Release)
    add_ffmpeg_config(--enable-optimizations)
    add_ffmpeg_config(--disable-debug)
else()
    add_ffmpeg_config(--disable-optimizations)
endif()
if(ANDROID)
    set(FFMPEG_AR ${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar)
    set(FFMPEG_RANLIB ${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ranlib)
    set(FFMPEG_STRIP ${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip)
    add_ffmpeg_config(--target-os=android)
    add_ffmpeg_config(--arch=${CMAKE_ANDROID_ARCH})
    add_ffmpeg_config(--cross-prefix=${CROSS_PREFIX})
    add_ffmpeg_config(--sysroot=${CMAKE_SYSROOT})
    add_ffmpeg_config(--cpu=${CPU})
    add_ffmpeg_config(--ar=${FFMPEG_AR})
    add_ffmpeg_config(--strip=${FFMPEG_STRIP})
    add_ffmpeg_config(--ranlib=${FFMPEG_RANLIB})
    add_ffmpeg_config(--enable-cross-compile)
elseif(WIN32)
    add_ffmpeg_config(--disable-asm)
    add_ffmpeg_config(--disable-mmx)
    add_ffmpeg_config(--disable-mmxext)
    add_ffmpeg_config(--disable-sse2)
    add_ffmpeg_config(--disable-x86asm)
    add_ffmpeg_config(--toolchain=msvc)
endif()
if(WIN32)
    set(ffmpeg_BUILD_COMMAND "make")
    set(ffmpeg_INSTALL_COMMAND "make install")
    set(ffmpeg_CONFIGURE "./configure ${ffmpeg_CONFIGURE}")
    set(ffmpeg_BUILD_COMMAND ${SHELL} ${ffmpeg_BUILD_COMMAND})
    set(ffmpeg_INSTALL_COMMAND ${SHELL} ${ffmpeg_INSTALL_COMMAND})
else()
    set(ffmpeg_BUILD_COMMAND make)
    set(ffmpeg_INSTALL_COMMAND make install)
    set(ffmpeg_CONFIGURE "./configure" ${ffmpeg_CONFIGURE})
    set(ffmpeg_BUILD_COMMAND ${ffmpeg_BUILD_COMMAND})
    set(ffmpeg_INSTALL_COMMAND ${ffmpeg_INSTALL_COMMAND})
endif()
message(STATUS "ffmpeg-dos2unix")
execute_process(COMMAND dos2unix configure WORKING_DIRECTORY ${ffmpeg_SOURCE_DIR})
message(STATUS "ffmpeg-configure: ${SHELL} \"./configure ${ffmpeg_CONFIGURE}\"")
execute_process(
    COMMAND ${SHELL} ${ffmpeg_CONFIGURE}
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
FetchContent_Declare(freetype
    GIT_REPOSITORY https://github.com/freetype/freetype.git
    GIT_TAG 42608f77f20749dd6ddc9e0536788eaad70ea4b5)
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

# glm
message(STATUS "FetchContent: glm")
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/icaven/glm.git
    GIT_TAG master)
FetchContent_MakeAvailable(glm)

# STB
message(STATUS "FetchContent: stb")
FetchContent_Declare(stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master)
FetchContent_MakeAvailable(stb)
