
# Dependencies
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)
set(SKIP_INSTALL_ALL ON)

if(ANDROID)
    set(SHELL bash)
elseif(WIN32)
    set(SHELL "C:\\msys64\\msys2_shell.cmd" "-defterm" "-no-start" "-mingw64" "-here" "-use-full-path" "-c")
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} /EHsc)
else()
    set(SHELL bash)
endif()

#New Dependency Declarations to the top!

# lzma
message(STATUS "FetchContent: lzma")
FetchContent_Declare(lzma
    GIT_REPOSITORY https://github.com/ZeunO8/lzma.git
    GIT_TAG chemfiles)
FetchContent_GetProperties(lzma)
if(NOT lzma_POPULATED)
    FetchContent_Populate(lzma)
endif()

set(LZMA_SOURCES
    ${lzma_SOURCE_DIR}/common/sysdefs.h
    ${lzma_SOURCE_DIR}/common/tuklib_integer.h
    ${lzma_SOURCE_DIR}/liblzma/check/check.c
    ${lzma_SOURCE_DIR}/liblzma/check/crc32_fast.c
    ${lzma_SOURCE_DIR}/liblzma/check/crc32_table.c
    ${lzma_SOURCE_DIR}/liblzma/check/crc64_fast.c
    ${lzma_SOURCE_DIR}/liblzma/check/crc64_table.c
    ${lzma_SOURCE_DIR}/liblzma/check/sha256.c
    ${lzma_SOURCE_DIR}/liblzma/common/alone_decoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/alone_encoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/auto_decoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/block_buffer_decoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/block_buffer_encoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/block_decoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/block_encoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/block_header_decoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/block_header_encoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/block_util.c
    ${lzma_SOURCE_DIR}/liblzma/common/common.c
    ${lzma_SOURCE_DIR}/liblzma/common/easy_buffer_encoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/easy_decoder_memusage.c
    ${lzma_SOURCE_DIR}/liblzma/common/easy_encoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/easy_encoder_memusage.c
    ${lzma_SOURCE_DIR}/liblzma/common/easy_preset.c
    ${lzma_SOURCE_DIR}/liblzma/common/filter_buffer_decoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/filter_buffer_encoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/filter_common.c
    ${lzma_SOURCE_DIR}/liblzma/common/filter_decoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/filter_encoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/filter_flags_decoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/filter_flags_encoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/index.c
    ${lzma_SOURCE_DIR}/liblzma/common/index_decoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/index_encoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/index_hash.c
    ${lzma_SOURCE_DIR}/liblzma/common/stream_buffer_decoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/stream_buffer_encoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/stream_decoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/stream_encoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/stream_flags_common.c
    ${lzma_SOURCE_DIR}/liblzma/common/stream_flags_decoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/stream_flags_encoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/vli_decoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/vli_encoder.c
    ${lzma_SOURCE_DIR}/liblzma/common/vli_size.c
    ${lzma_SOURCE_DIR}/liblzma/delta/delta_common.c
    ${lzma_SOURCE_DIR}/liblzma/delta/delta_decoder.c
    ${lzma_SOURCE_DIR}/liblzma/delta/delta_encoder.c
    ${lzma_SOURCE_DIR}/liblzma/lz/lz_decoder.c
    ${lzma_SOURCE_DIR}/liblzma/lz/lz_encoder.c
    ${lzma_SOURCE_DIR}/liblzma/lz/lz_encoder_mf.c
    ${lzma_SOURCE_DIR}/liblzma/lzma/fastpos_table.c
    ${lzma_SOURCE_DIR}/liblzma/lzma/lzma2_decoder.c
    ${lzma_SOURCE_DIR}/liblzma/lzma/lzma2_encoder.c
    ${lzma_SOURCE_DIR}/liblzma/lzma/lzma_decoder.c
    ${lzma_SOURCE_DIR}/liblzma/lzma/lzma_encoder.c
    ${lzma_SOURCE_DIR}/liblzma/lzma/lzma_encoder_optimum_fast.c
    ${lzma_SOURCE_DIR}/liblzma/lzma/lzma_encoder_optimum_normal.c
    ${lzma_SOURCE_DIR}/liblzma/lzma/lzma_encoder_presets.c
    ${lzma_SOURCE_DIR}/liblzma/rangecoder/price_table.c
    ${lzma_SOURCE_DIR}/liblzma/simple/arm.c
    ${lzma_SOURCE_DIR}/liblzma/simple/armthumb.c
    ${lzma_SOURCE_DIR}/liblzma/simple/ia64.c
    ${lzma_SOURCE_DIR}/liblzma/simple/powerpc.c
    ${lzma_SOURCE_DIR}/liblzma/simple/simple_coder.c
    ${lzma_SOURCE_DIR}/liblzma/simple/simple_decoder.c
    ${lzma_SOURCE_DIR}/liblzma/simple/simple_encoder.c
    ${lzma_SOURCE_DIR}/liblzma/simple/sparc.c
    ${lzma_SOURCE_DIR}/liblzma/simple/x86.c)
configure_file(${lzma_SOURCE_DIR}/config.h.in ${lzma_SOURCE_DIR}/config.h @ONLY)
add_library(lzma STATIC ${LZMA_SOURCES})
target_compile_definitions(lzma PRIVATE HAVE_CONFIG_H)
target_compile_definitions(lzma PRIVATE LZMA_API_STATIC)
target_include_directories(lzma PRIVATE
    "${lzma_SOURCE_DIR}/common"
    "${lzma_SOURCE_DIR}/liblzma/api"
    "${lzma_SOURCE_DIR}/liblzma/check"
    "${lzma_SOURCE_DIR}/liblzma/common"
    "${lzma_SOURCE_DIR}/liblzma/delta"
    "${lzma_SOURCE_DIR}/liblzma/lz"
    "${lzma_SOURCE_DIR}/liblzma/lzma"
    "${lzma_SOURCE_DIR}/liblzma/rangecoder"
    "${lzma_SOURCE_DIR}/liblzma/simple"
    "${lzma_SOURCE_DIR}")

# brotli
message(STATUS "FetchContent: brotli")
set(BROTLI_BUNDLED_MODE ON)
set(BROTLI_DISABLE_TESTS ON)
FetchContent_Declare(brotli
    GIT_REPOSITORY https://github.com/google/brotli.git
    GIT_TAG v1.1.0)
FetchContent_MakeAvailable(brotli)

# harfbuzz
message(STATUS "FetchContent: harfbuzz")
FetchContent_Declare(harfbuzz
    GIT_REPOSITORY https://github.com/harfbuzz/harfbuzz.git
    GIT_TAG 11.0.0)
FetchContent_MakeAvailable(harfbuzz)

# zlib
message(STATUS "FetchContent: zlib")
FetchContent_Declare(zlib
    GIT_REPOSITORY https://github.com/ZeunO8/zlib.git
    GIT_TAG apple-fix)
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
configure_file(${zlib_SOURCE_DIR}/zconf.h.cmakein ${zlib_SOURCE_DIR}/zconf.h)
# FetchContent_MakeAvailable(zlib)

# png
message(STATUS "FetchContent: png")
set(PNG_TESTS OFF)
FetchContent_Declare(png
    GIT_REPOSITORY https://github.com/pnggroup/libpng.git
    GIT_TAG v1.6.47)
FetchContent_GetProperties(png)
if(NOT png_POPULATED)
    FetchContent_Populate(png)
endif()

set(PNG_SOURCES
    "${png_SOURCE_DIR}/png.c"
    "${png_SOURCE_DIR}/pngerror.c"
    "${png_SOURCE_DIR}/pngget.c"
    "${png_SOURCE_DIR}/pngmem.c"
    "${png_SOURCE_DIR}/pngpread.c"
    "${png_SOURCE_DIR}/pngread.c"
    "${png_SOURCE_DIR}/pngrio.c"
    "${png_SOURCE_DIR}/pngrtran.c"
    "${png_SOURCE_DIR}/pngrutil.c"
    "${png_SOURCE_DIR}/pngset.c"
    "${png_SOURCE_DIR}/pngtrans.c"
    "${png_SOURCE_DIR}/pngwio.c"
    "${png_SOURCE_DIR}/pngwrite.c"
    "${png_SOURCE_DIR}/pngwtran.c"
    "${png_SOURCE_DIR}/pngwutil.c"
)

if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm64")
    list(APPEND PNG_SOURCES "${png_SOURCE_DIR}/arm/filter_neon_intrinsics.c" "${png_SOURCE_DIR}/arm/palette_neon_intrinsics.c" "${png_SOURCE_DIR}/arm/arm_init.c")
endif()
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "AMD64")
    list(APPEND PNG_SOURCES "${png_SOURCE_DIR}/intel/filter_sse2_intrinsics.c" "${png_SOURCE_DIR}/intel/intel_init.c")
endif()

add_library(png STATIC ${PNG_SOURCES})
target_include_directories(png PRIVATE ${png_SOURCE_DIR})
target_include_directories(png PRIVATE ${zlib_SOURCE_DIR})
configure_file(${png_SOURCE_DIR}/scripts/pnglibconf.h.prebuilt ${png_SOURCE_DIR}/pnglibconf.h)

# OpenSSL
if(NOT MACOS)
    message(STATUS "FetchContent: openssl")
    FetchContent_Declare(openssl
        GIT_REPOSITORY https://github.com/openssl/openssl.git
        GIT_TAG openssl-3.4.1)
    FetchContent_GetProperties(openssl)
    if(NOT openssl_POPULATED)
        FetchContent_Populate(openssl)
    endif()

    function(add_openssl_config VARI)
        # if(WIN32)
            # set(openssl_CONFIGURE "${openssl_CONFIGURE} ${VARI}" PARENT_SCOPE)
        # else()
            set(openssl_CONFIGURE ${openssl_CONFIGURE} ${VARI} PARENT_SCOPE)
        # endif()
    endfunction()
    if(ANDROID)
        set(openssl_BUILD_TYPE ${ANDROID_MARCH})
        add_openssl_config("./Configure")
        add_openssl_config(${openssl_BUILD_TYPE})
        set(openssl_MAKE make)
        set(openssl_MAKE_INSTALL make install_sw install_ssldirs)
    elseif(WINDOWS)
        if(${RELEASE_OR_DEBUG} STREQUAL "Debug")
            set(openssl_BUILD_TYPE debug-VC-WIN64A)
        else()
            set(openssl_BUILD_TYPE VC-WIN64A)
        endif()
        add_openssl_config(perl)
        add_openssl_config(Configure)
        add_openssl_config(${openssl_BUILD_TYPE})
        set(openssl_MAKE nmake)
        set(openssl_MAKE_INSTALL nmake install_sw install_ssldirs)
    else()
        add_openssl_config("./config")
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
    # if(WIN32)
    #     set(openssl_CONFIGURE ${openssl_CONFIGURE})
    #     set(openssl_MAKE ${openssl_MAKE})
    #     set(openssl_MAKE_INSTALL ${openssl_MAKE_INSTALL})
    # endif()
    message(STATUS "openssl-configure: \"${openssl_CONFIGURE}\"")
    execute_process(
        COMMAND ${openssl_CONFIGURE}
        WORKING_DIRECTORY ${openssl_SOURCE_DIR}
        RESULT_VARIABLE openssl_ConfigureResult)
    if(openssl_ConfigureResult)
        message(FATAL_ERROR "openssl-configure: ${openssl_ConfigureResult}")
    else()
        message(STATUS "openssl-configure: success")
    endif()
    add_custom_target(openssl ALL
        COMMAND ${openssl_MAKE}
        COMMAND ${openssl_MAKE_INSTALL}
        WORKING_DIRECTORY ${openssl_SOURCE_DIR}
        COMMENT "Building OpenSSL"
    )
endif()

# bzip2
message(STATUS "FetchContent: bzip2")
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

# FFmpeg
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
    add_ffmpeg_config(--disable-d3d12va)
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
add_custom_target(ffmpeg ALL
    COMMAND ${ffmpeg_BUILD_COMMAND}
    COMMAND ${ffmpeg_INSTALL_COMMAND}
    WORKING_DIRECTORY ${ffmpeg_SOURCE_DIR}
    COMMENT "Building ffmpeg"
)

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
