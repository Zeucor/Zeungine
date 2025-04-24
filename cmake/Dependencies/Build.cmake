
# Dependencies
include(FetchContent)
# set(FETCHCONTENT_QUIET OFF)
set(SKIP_INSTALL_ALL ON)

if(ANDROID)
    set(SHELL bash)
elseif(WIN32)
    set(SHELL "C:\\msys64\\msys2_shell.cmd" "-defterm" "-no-start" "-mingw64" "-here" "-use-full-path" "-c")
else()
    set(SHELL bash)
endif()

#New Dependency Declarations to the top!

# miniaudio
message(STATUS "FetchContent: miniaudio")
FetchContent_Declare(
    miniaudio
    GIT_REPOSITORY https://github.com/mackron/miniaudio.git
    GIT_TAG master)
FetchContent_GetProperties(miniaudio)
if(NOT miniaudio_POPULATED)
    FetchContent_Populate(miniaudio)
endif()

set(MINIAUDIO_SOURCES ${miniaudio_SOURCE_DIR}/miniaudio.c)
add_library(miniaudio STATIC ${MINIAUDIO_SOURCES})
target_include_directories(miniaudio PRIVATE ${miniaudio_SOURCE_DIR})

# lunasvg & plutovg combined lib
message(STATUS "FetchContent: lunasvg")
FetchContent_Declare(lunasvg
    GIT_REPOSITORY https://github.com/ZeunO8/lunasvg.git
    GIT_TAG master)
FetchContent_GetProperties(lunasvg)
if(NOT lunasvg_POPULATED)
    FetchContent_Populate(lunasvg)
endif()

file(GLOB LUNASVG_SOURCES ${lunasvg_SOURCE_DIR}/source/*.cpp)

message(STATUS "FetchContent: plutovg")
FetchContent_Declare(plutovg
    GIT_REPOSITORY https://github.com/ZeunO8/plutovg.git
    GIT_TAG main)
FetchContent_GetProperties(plutovg)
if(NOT plutovg_POPULATED)
    FetchContent_Populate(plutovg)
endif()

file(GLOB PLUTOVG_SOURCES ${plutovg_SOURCE_DIR}/source/*.c)

add_library(svg STATIC ${LUNASVG_SOURCES} ${PLUTOVG_SOURCES})
target_include_directories(svg PRIVATE ${lunasvg_SOURCE_DIR}/include)
target_include_directories(svg PRIVATE ${plutovg_SOURCE_DIR}/include)
target_compile_definitions(svg PRIVATE LUNASVG_BUILD_STATIC)
target_compile_definitions(svg PRIVATE PLUTOVG_BUILD_STATIC)

# combined spirvheaders, spirv-tools, glslang & shaderc
message(STATUS "FetchContent: sprivheaders")
FetchContent_Declare(spirvheaders
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
    GIT_TAG 54a521dd130ae1b2f38fef79b09515702d135bdd)
FetchContent_GetProperties(spirvheaders)
if(NOT spirvheaders_POPULATED)
    FetchContent_Populate(spirvheaders)
endif()

# spriv-tools
message(STATUS "FetchContent: spirvtools")
FetchContent_Declare(spirvtools
    GIT_REPOSITORY https://github.com/ZeunO8/SPIRV-Tools.git
    GIT_TAG fixes)
FetchContent_GetProperties(spirvtools)
if(NOT spirvtools_POPULATED)
    FetchContent_Populate(spirvtools)
endif()

file(GLOB SPIRV_TOOLS_SOURCES
    "${spirvtools_SOURCE_DIR}/source/diff/*.cpp"
    "${spirvtools_SOURCE_DIR}/source/link/*.cpp"
    "${spirvtools_SOURCE_DIR}/source/lint/*.cpp"
    "${spirvtools_SOURCE_DIR}/source/opt/*.cpp"
    "${spirvtools_SOURCE_DIR}/source/reduce/*.cpp"
    "${spirvtools_SOURCE_DIR}/source/util/*.cpp"
    "${spirvtools_SOURCE_DIR}/source/val/*.cpp"
    "${spirvtools_SOURCE_DIR}/source/*.cpp")

# glslang
message(STATUS "FetchContent: glslang")
FetchContent_Declare(glslang
    GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
    GIT_TAG 2b2523fb951f63f072cfba514c26f2feea5f4329)
FetchContent_GetProperties(glslang)
if(NOT glslang_POPULATED)
    FetchContent_Populate(glslang)
endif()

if(LINUX OR MACOS)
    set(GL_OS Unix)
elseif(WINDOWS)
    set(GL_OS Windows)
elseif(EMSCRIPTEN)
    set(GL_OS Web)
endif()
file(GLOB GLSLANG_SOURCES
    "${glslang_SOURCE_DIR}/glslang/CInterface/*.cpp"
    "${glslang_SOURCE_DIR}/glslang/HLSL/*.cpp"
    "${glslang_SOURCE_DIR}/glslang/GenericCodeGen/*.cpp"
    "${glslang_SOURCE_DIR}/glslang/MachineIndependent/*.cpp"
    "${glslang_SOURCE_DIR}/glslang/MachineIndependent/preprocessor/*.cpp"
    "${glslang_SOURCE_DIR}/glslang/HLSL/*.cpp"
    "${glslang_SOURCE_DIR}/glslang/OSDependent/${GL_OS}/*.cpp"
    "${glslang_SOURCE_DIR}/OGLCompilersDLL/*.cpp"
    "${glslang_SOURCE_DIR}/SPIRV/*.cpp"
    "${glslang_SOURCE_DIR}/SPIRV/CInterface*.cpp")

# shaderc
message(STATUS "FetchContent: shaderc")
FetchContent_Declare(shaderc
    GIT_REPOSITORY https://github.com/ZeunO8/shaderc.git
    GIT_TAG win_x86_64
    GIT_SUBMODULES "")
FetchContent_GetProperties(shaderc)
if(NOT shaderc_POPULATED)
    FetchContent_Populate(shaderc)
endif()

file(GLOB SHADERC_SOURCES "${shaderc_SOURCE_DIR}/libshaderc/src/*.c" "${shaderc_SOURCE_DIR}/libshaderc/src/*.cc")
list(REMOVE_ITEM SHADERC_SOURCES "${shaderc_SOURCE_DIR}/libshaderc/src/shaderc_test.cc")
list(REMOVE_ITEM SHADERC_SOURCES "${shaderc_SOURCE_DIR}/libshaderc/src/shaderc_c_smoke_test.c")
list(REMOVE_ITEM SHADERC_SOURCES "${shaderc_SOURCE_DIR}/libshaderc/src/shaderc_cpp_test.cc")
list(REMOVE_ITEM SHADERC_SOURCES "${shaderc_SOURCE_DIR}/libshaderc/src/shaderc_private_test.cc")

file(GLOB SHADERC_UTIL_SOURCES "${shaderc_SOURCE_DIR}/libshaderc_util/src/*.c" "${shaderc_SOURCE_DIR}/libshaderc_util/src/*.cc")
list(REMOVE_ITEM SHADERC_UTIL_SOURCES "${shaderc_SOURCE_DIR}/libshaderc_util/src/compiler_test.cc")
list(REMOVE_ITEM SHADERC_UTIL_SOURCES "${shaderc_SOURCE_DIR}/libshaderc_util/src/counting_includer_test.cc")
list(REMOVE_ITEM SHADERC_UTIL_SOURCES "${shaderc_SOURCE_DIR}/libshaderc_util/src/file_finder_test.cc")
list(REMOVE_ITEM SHADERC_UTIL_SOURCES "${shaderc_SOURCE_DIR}/libshaderc_util/src/format_test.cc")
list(REMOVE_ITEM SHADERC_UTIL_SOURCES "${shaderc_SOURCE_DIR}/libshaderc_util/src/io_shaderc_test.cc")
list(REMOVE_ITEM SHADERC_UTIL_SOURCES "${shaderc_SOURCE_DIR}/libshaderc_util/src/message_test.cc")
list(REMOVE_ITEM SHADERC_UTIL_SOURCES "${shaderc_SOURCE_DIR}/libshaderc_util/src/mutex_test.cc")
list(REMOVE_ITEM SHADERC_UTIL_SOURCES "${shaderc_SOURCE_DIR}/libshaderc_util/src/string_piece_test.cc")
list(REMOVE_ITEM SHADERC_UTIL_SOURCES "${shaderc_SOURCE_DIR}/libshaderc_util/src/version_profile_test.cc")

add_library(glsl STATIC ${SPIRV_TOOLS_SOURCES} ${SHADERC_SOURCES} ${SHADERC_UTIL_SOURCES} ${GLSLANG_SOURCES})
target_include_directories(glsl PRIVATE ${shaderc_SOURCE_DIR}/libshaderc/include)
target_include_directories(glsl PRIVATE ${shaderc_SOURCE_DIR}/libshaderc_util/include)
target_include_directories(glsl PRIVATE ${spirvtools_SOURCE_DIR}/include)
target_include_directories(glsl PRIVATE ${spirvtools_SOURCE_DIR})
target_include_directories(glsl PRIVATE ${spirvheaders_SOURCE_DIR}/include)
target_include_directories(glsl PRIVATE ${glslang_SOURCE_DIR})
target_include_directories(glsl PRIVATE ${glslang_SOURCE_DIR}/OGLCompilersDLL)
target_include_directories(glsl PRIVATE include)
target_include_directories(glsl PRIVATE include/spirv)
target_compile_definitions(glsl PRIVATE ENABLE_HLSL)
target_compile_definitions(glsl PRIVATE ENABLE_OPT)

# Jolt
message(STATUS "FetchContent: jolt")
FetchContent_Declare(jolt
    GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics.git
    GIT_TAG v5.3.0)
FetchContent_GetProperties(jolt)
if(NOT jolt_POPULATED)
    FetchContent_Populate(jolt)
endif()
file(GLOB_RECURSE JOLT_SOURCES ${jolt_SOURCE_DIR}/Jolt/*.c ${jolt_SOURCE_DIR}/Jolt/*.cpp)

add_library(jolt STATIC ${JOLT_SOURCES})

target_include_directories(jolt PRIVATE ${jolt_SOURCE_DIR})

# zstd
message(STATUS "FetchContent: zstd")
FetchContent_Declare(zstd
    GIT_REPOSITORY https://github.com/facebook/zstd.git
    GIT_TAG v1.5.7)
FetchContent_GetProperties(zstd)
if(NOT zstd_POPULATED)
    FetchContent_Populate(zstd)
endif()

file(GLOB ZSTD_COMMON_SOURCES ${zstd_SOURCE_DIR}/lib/common/*.c)
file(GLOB ZSTD_COMPRESS_SOURCES ${zstd_SOURCE_DIR}/lib/compress/*.c)
file(GLOB ZSTD_DECOMPRESS_SOURCES ${zstd_SOURCE_DIR}/lib/decompress/*.c)
file(GLOB ZSTD_DICTBUILDER_SOURCES ${zstd_SOURCE_DIR}/lib/dictBuilder/*.c)

add_library(zstd STATIC ${ZSTD_COMMON_SOURCES} ${ZSTD_COMPRESS_SOURCES} ${ZSTD_DECOMPRESS_SOURCES} ${ZSTD_DICTBUILDER_SOURCES})

target_include_directories(zstd PRIVATE ${zstd_SOURCE_DIR}/lib)

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
)

add_library(bzip2 STATIC ${BZIP2_SOURCES})
target_include_directories(bzip2 PRIVATE ${bzip2_SOURCE_DIR})

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

# Boost
message(STATUS "FetchContent: boost")
FetchContent_Declare(boost
    GIT_REPOSITORY https://github.com/boostorg/boost
    GIT_TAG boost-1.87.0)
FetchContent_GetProperties(boost)
if(NOT boost_POPULATED)
    FetchContent_Populate(boost)
endif()

file(GLOB BOOST_LIB_SOURCES
    "${boost_SOURCE_DIR}/libs/*/src/*.cpp"
    "${boost_SOURCE_DIR}/libs/*/src/*.c"
)

# Building Boost without support for: filesystem, atomic, python, graph_parallel, stacktrace, mpi, container (dlmalloc)

list(FILTER BOOST_LIB_SOURCES EXCLUDE REGEX "^${boost_SOURCE_DIR}/libs/filesystem/src/.*\\.cpp$")
list(FILTER BOOST_LIB_SOURCES EXCLUDE REGEX "^${boost_SOURCE_DIR}/libs/filesystem/src/.*\\.c$")
list(FILTER BOOST_LIB_SOURCES EXCLUDE REGEX "^${boost_SOURCE_DIR}/libs/atomic/src/.*\\.cpp$")
list(FILTER BOOST_LIB_SOURCES EXCLUDE REGEX "^${boost_SOURCE_DIR}/libs/atomic/src/.*\\.c$")
list(FILTER BOOST_LIB_SOURCES EXCLUDE REGEX "^${boost_SOURCE_DIR}/libs/python/src/.*\\.cpp$")
list(FILTER BOOST_LIB_SOURCES EXCLUDE REGEX "^${boost_SOURCE_DIR}/libs/python/src/.*\\.c$")
list(FILTER BOOST_LIB_SOURCES EXCLUDE REGEX "^${boost_SOURCE_DIR}/libs/graph_parallel/src/.*\\.cpp$")
list(FILTER BOOST_LIB_SOURCES EXCLUDE REGEX "^${boost_SOURCE_DIR}/libs/graph_parallel/src/.*\\.c$")
list(FILTER BOOST_LIB_SOURCES EXCLUDE REGEX "^${boost_SOURCE_DIR}/libs/stacktrace/src/.*\\.cpp$")
list(FILTER BOOST_LIB_SOURCES EXCLUDE REGEX "^${boost_SOURCE_DIR}/libs/stacktrace/src/.*\\.c$")
list(FILTER BOOST_LIB_SOURCES EXCLUDE REGEX "^${boost_SOURCE_DIR}/libs/mpi/src/.*\\.cpp$")
list(FILTER BOOST_LIB_SOURCES EXCLUDE REGEX "^${boost_SOURCE_DIR}/libs/mpi/src/.*\\.c$")
list(FILTER BOOST_LIB_SOURCES EXCLUDE REGEX "^${boost_SOURCE_DIR}/libs/context/src/untested.cpp$")
list(FILTER BOOST_LIB_SOURCES EXCLUDE REGEX "^${boost_SOURCE_DIR}/libs/container/src/dlmalloc_2_8_6.c$")
list(FILTER BOOST_LIB_SOURCES EXCLUDE REGEX "^${boost_SOURCE_DIR}/libs/container/src/dlmalloc_ext_2_8_6.c$")
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm64")
    list(FILTER BOOST_LIB_SOURCES EXCLUDE REGEX "^${boost_SOURCE_DIR}/libs/log/src/dump_avx2.cpp$")
    list(FILTER BOOST_LIB_SOURCES EXCLUDE REGEX "^${boost_SOURCE_DIR}/libs/log/src/dump_ssse3.cpp$")
endif()
if(WINDOWS)
    list(FILTER BOOST_LIB_SOURCES EXCLUDE REGEX "^${boost_SOURCE_DIR}/libs/stacktrace/src/addr2line.cpp$")
elseif(UNIX)
    list(FILTER BOOST_LIB_SOURCES EXCLUDE REGEX "^${boost_SOURCE_DIR}/libs/stacktrace/src/windbg.cpp$")
    list(FILTER BOOST_LIB_SOURCES EXCLUDE REGEX "^${boost_SOURCE_DIR}/libs/stacktrace/src/windbg_cached.cpp$")
endif()

# message(STATUS "BOOST_LIB_SOURCES: ${BOOST_LIB_SOURCES}")

add_library(boost STATIC ${BOOST_LIB_SOURCES})

file(GLOB BOOST_INCLUDES "${boost_SOURCE_DIR}/libs/*/include" "${boost_SOURCE_DIR}/libs/*/include/boost")

file(GLOB BOOST_NUMERIC_INCLUDES
    "${boost_SOURCE_DIR}/libs/numeric/conversion/include"
    "${boost_SOURCE_DIR}/libs/numeric/conversion/include/boost"
    "${boost_SOURCE_DIR}/libs/numeric/interval/include"
    "${boost_SOURCE_DIR}/libs/numeric/interval/include/boost"
    "${boost_SOURCE_DIR}/libs/numeric/odeint/include"
    "${boost_SOURCE_DIR}/libs/numeric/odeint/include/boost"
    "${boost_SOURCE_DIR}/libs/numeric/ublas/include"
    "${boost_SOURCE_DIR}/libs/numeric/ublas/include/boost"
)

# message(STATUS "BOOST_INCLUDES: ${BOOST_INCLUDES}")

target_include_directories(boost PRIVATE ${BOOST_INCLUDES})
target_include_directories(boost PRIVATE ${BOOST_NUMERIC_INCLUDES})
target_include_directories(boost PRIVATE ${bzip2_SOURCE_DIR})
target_include_directories(boost PRIVATE ${lzma_SOURCE_DIR}/liblzma/api)
target_include_directories(boost PRIVATE ${zlib_SOURCE_DIR})
target_include_directories(boost PRIVATE ${zstd_SOURCE_DIR}/lib)
if(UNIX)
    target_compile_definitions(boost PRIVATE BOOST_USE_UCONTEXT)
elseif(WINDOWS)
    target_compile_definitions(boost PRIVATE BOOST_USE_WINFIB)
endif()

# find_package(MPI REQUIRED)
# target_include_directories(boost PRIVATE ${MPI_C_INCLUDE_DIRS})

# ExprTK
message(STATUS "FetchContent: exprtk")
FetchContent_Declare(exprtk
    GIT_REPOSITORY https://github.com/ArashPartow/exprtk.git
    GIT_TAG master)
FetchContent_GetProperties(exprtk)
if(NOT exprtk_POPULATED)
    FetchContent_Populate(exprtk)
endif()

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
message(STATUS "ffmpeg-configure: ${SHELL} \"${ffmpeg_CONFIGURE}\"")
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
