# OS Sources

if(MACOS)
    file(GLOB_RECURSE ZG_MAC_SOURCES "linsrc/macos/*.c" "linsrc/macos/*.cpp" "linsrc/macos/*.mm")
    set(ZG_SOURCES ${ZG_MAC_SOURCES})
elseif(LINUX)
    file(GLOB_RECURSE ZG_LNX_SOURCES "linsrc/linux/*.c" "linsrc/linux/*.cpp" "linsrc/linux/*.lx")
    set(ZG_SOURCES ${ZG_LNX_SOURCES})
elseif(ANDROID OR IOS)
    file(GLOB_RECURSE ZG_EGL_SOURCES "linsrc/egl/*.c" "linsrc/egl/*.cpp" "linsrc/egl/*.eg")
    set(ZG_SOURCES ${ZG_EGL_SOURCES})
elseif(WIN32)
    file(GLOB_RECURSE ZG_WIN_SOURCES "linsrc/windows/*.c" "linsrc/windows/*.cpp" "linsrc/windows/*.ww")
    set(ZG_SOURCES ${ZG_WIN_SOURCES})
elseif(ZUG)
    file(GLOB_RECURSE ZG_ZUG_SOURCES linsrc/coj/*.cpp linsrc/coj/*.c)
    set(ZG_SOURCES ${ZG_ZUG_SOURCES})
endif()

# ZG Sources

file(GLOB_RECURSE ZG_C_SOURCES "src/*.c")
file(GLOB_RECURSE ZG_CXX_SOURCES "src/*.cpp")
set(ZG_SOURCES ${ZG_SOURCES} ${ZG_C_SOURCES} ${ZG_CXX_SOURCES})

include(FetchContent)
#set(FETCHCONTENT_QUIET OFF)

# # protobuf
# FetchContent_Declare(protobuf
#     GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
#     GIT_TAG main)
# FetchContent_GetProperties(protobuf)
# if(NOT protobuf_POPULATED)
#     FetchContent_Populate(protobuf)
# endif()

# # absl
# FetchContent_Declare(absl
#     GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
#     GIT_TAG master)
# FetchContent_GetProperties(absl)
# if(NOT absl_POPULATED)
#     FetchContent_Populate(absl)
# endif()

# spirvheaders
FetchContent_Declare(spirvheaders
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
    GIT_TAG 54a521dd130ae1b2f38fef79b09515702d135bdd)
FetchContent_GetProperties(spirvheaders)
if(NOT spirvheaders_POPULATED)
    FetchContent_Populate(spirvheaders)
endif()

# spriv-tools
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

# lunasvg
FetchContent_Declare(lunasvg
    GIT_REPOSITORY https://github.com/ZeunO8/lunasvg.git
    GIT_TAG master)
FetchContent_GetProperties(lunasvg)
if(NOT lunasvg_POPULATED)
    FetchContent_Populate(lunasvg)
endif()

file(GLOB LUNASVG_SOURCES ${lunasvg_SOURCE_DIR}/source/*.cpp)

# plutovg
FetchContent_Declare(plutovg
    GIT_REPOSITORY https://github.com/ZeunO8/plutovg.git
    GIT_TAG main)
FetchContent_GetProperties(plutovg)
if(NOT plutovg_POPULATED)
    FetchContent_Populate(plutovg)
endif()

file(GLOB PLUTOVG_SOURCES ${plutovg_SOURCE_DIR}/source/*.c)

# freetype
# FetchContent_Declare(freetype
#     GIT_REPOSITORY https://github.com/freetype/freetype.git
#     GIT_TAG 42608f77f20749dd6ddc9e0536788eaad70ea4b5)
# FetchContent_GetProperties(freetype)
# if(NOT freetype_POPULATED)
#     FetchContent_Populate(freetype)
# endif()

# set(FT_SOURCES 
#     ${freetype_SOURCE_DIR}/src/gzip/inflate.c
# )

# miniaudio
FetchContent_Declare(
    miniaudio
    GIT_REPOSITORY https://github.com/mackron/miniaudio.git
    GIT_TAG master)
FetchContent_GetProperties(miniaudio)
if(NOT miniaudio_POPULATED)
    FetchContent_Populate(miniaudio)
endif()
set(MINIAUDIO_SOURCES ${miniaudio_SOURCE_DIR}/miniaudio.c)

set(ZG_SOURCES ${MINIAUDIO_SOURCES} ${LUNASVG_SOURCES} ${PLUTOVG_SOURCES} ${SPIRV_TOOLS_SOURCES} ${SHADERC_SOURCES} ${SHADERC_UTIL_SOURCES} ${GLSLANG_SOURCES} ${ZG_SOURCES})

option(PRINT_PYTHON_SPIRV_COMMANDS "Whether to print python3 spirv commands" OFF)
if(PRINT_PYTHON_SPIRV_COMMANDS)
    set(GRAMMAR_PROCESSING_SCRIPT "${spirvtools_SOURCE_DIR}/utils/generate_grammar_tables.py")
    set(VIMSYNTAX_PROCESSING_SCRIPT "${spirvtools_SOURCE_DIR}/utils/vim/generate_syntax.py")
    set(XML_REGISTRY_PROCESSING_SCRIPT "${spirvtools_SOURCE_DIR}/utils/generate_registry_tables.py")
    set(LANG_HEADER_PROCESSING_SCRIPT "${spirvtools_SOURCE_DIR}/utils/generate_language_headers.py")

    set(DEBUGINFO_GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/unified1/extinst.debuginfo.grammar.json")
    set(CLDEBUGINFO100_GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/unified1/extinst.opencl.debuginfo.100.grammar.json")
    set(VKDEBUGINFO100_GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/unified1/extinst.nonsemantic.shader.debuginfo.100.grammar.json")

    set(GENERATOR_INC_FILE spirv/generators.inc)
    set(SPIRV_XML_REGISTRY_FILE ${spirvheaders_SOURCE_DIR}/include/spirv/spir-v.xml)
    message(STATUS "python ${XML_REGISTRY_PROCESSING_SCRIPT} --xml=${SPIRV_XML_REGISTRY_FILE} --generator-output=${GENERATOR_INC_FILE}")

    set(SPIRV_TOOLS_BUILD_VERSION_INC spirv/build-version.inc)
    set(SPIRV_TOOLS_BUILD_VERSION_INC_GENERATOR ${spirvtools_SOURCE_DIR}/utils/update_build_version.py)
    set(SPIRV_TOOLS_CHANGES_FILE ${spirvtools_SOURCE_DIR}/CHANGES)
    message(STATUS "python ${SPIRV_TOOLS_BUILD_VERSION_INC_GENERATOR} ${SPIRV_TOOLS_CHANGES_FILE} ${SPIRV_TOOLS_BUILD_VERSION_INC}")

    macro(spvtools_core_tables CONFIG_VERSION)
        set(GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/${CONFIG_VERSION}/spirv.core.grammar.json")
        set(GRAMMAR_INSTS_INC_FILE "spirv/core.insts-${CONFIG_VERSION}.inc")
        set(GRAMMAR_KINDS_INC_FILE "spirv/operand.kinds-${CONFIG_VERSION}.inc")
        message(STATUS "python ${GRAMMAR_PROCESSING_SCRIPT} --spirv-core-grammar=${GRAMMAR_JSON_FILE} --extinst-debuginfo-grammar=${DEBUGINFO_GRAMMAR_JSON_FILE} --extinst-cldebuginfo100-grammar=${CLDEBUGINFO100_GRAMMAR_JSON_FILE} --core-insts-output=${GRAMMAR_INSTS_INC_FILE} --operand-kinds-output=${GRAMMAR_KINDS_INC_FILE} --output-language=c++")
    endmacro(spvtools_core_tables)

    macro(spvtools_enum_string_mapping CONFIG_VERSION)
        set(GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/${CONFIG_VERSION}/spirv.core.grammar.json")
        set(GRAMMAR_EXTENSION_ENUM_INC_FILE "spirv/extension_enum.inc")
        set(GRAMMAR_ENUM_STRING_MAPPING_INC_FILE "spirv/enum_string_mapping.inc")
        message(STATUS "python ${GRAMMAR_PROCESSING_SCRIPT} --spirv-core-grammar=${GRAMMAR_JSON_FILE} --extinst-debuginfo-grammar=${DEBUGINFO_GRAMMAR_JSON_FILE} --extinst-cldebuginfo100-grammar=${CLDEBUGINFO100_GRAMMAR_JSON_FILE} --extension-enum-output=${GRAMMAR_EXTENSION_ENUM_INC_FILE} --enum-string-mapping-output=${GRAMMAR_ENUM_STRING_MAPPING_INC_FILE} --output-language=c++")
    endmacro(spvtools_enum_string_mapping)

    macro(spvtools_vimsyntax CONFIG_VERSION CLVERSION)
        set(GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/${CONFIG_VERSION}/spirv.core.grammar.json")
        set(GLSL_GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/${CONFIG_VERSION}/extinst.glsl.std.450.grammar.json")
        set(OPENCL_GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/${CONFIG_VERSION}/extinst.opencl.std.100.grammar.json")
        set(VIMSYNTAX_FILE "spirv/spvasm.vim")
        message(STATUS "python ${VIMSYNTAX_PROCESSING_SCRIPT} --spirv-core-grammar=${GRAMMAR_JSON_FILE} --extinst-debuginfo-grammar=${DEBUGINFO_GRAMMAR_JSON_FILE} --extinst-glsl-grammar=${GLSL_GRAMMAR_JSON_FILE} --extinst-opencl-grammar=${OPENCL_GRAMMAR_JSON_FILE}>${VIMSYNTAX_FILE}")
    endmacro(spvtools_vimsyntax)

    macro(spvtools_glsl_tables CONFIG_VERSION)
        set(CORE_GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/${CONFIG_VERSION}/spirv.core.grammar.json")
        set(GLSL_GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/${CONFIG_VERSION}/extinst.glsl.std.450.grammar.json")
        set(GRAMMAR_INC_FILE "spirv/glsl.std.450.insts.inc")
        message(STATUS "python ${GRAMMAR_PROCESSING_SCRIPT} --extinst-glsl-grammar=${GLSL_GRAMMAR_JSON_FILE} --glsl-insts-output=${GRAMMAR_INC_FILE} --output-language=c++")
    endmacro(spvtools_glsl_tables)

    macro(spvtools_opencl_tables CONFIG_VERSION)
        set(CORE_GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/${CONFIG_VERSION}/spirv.core.grammar.json")
        set(OPENCL_GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/${CONFIG_VERSION}/extinst.opencl.std.100.grammar.json")
        set(GRAMMAR_INC_FILE "spirv/opencl.std.insts.inc")
        message(STATUS "python ${GRAMMAR_PROCESSING_SCRIPT} --extinst-opencl-grammar=${OPENCL_GRAMMAR_JSON_FILE} --opencl-insts-output=${GRAMMAR_INC_FILE}")
    endmacro(spvtools_opencl_tables)

    macro(spvtools_vendor_tables VENDOR_TABLE SHORT_NAME OPERAND_KIND_PREFIX)
        set(INSTS_FILE "spirv/${VENDOR_TABLE}.insts.inc")
        set(GRAMMAR_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/unified1/extinst.${VENDOR_TABLE}.grammar.json")
        if(NOT EXISTS ${GRAMMAR_FILE})
            set(GRAMMAR_FILE "${spirvtools_SOURCE_DIR}/source/extinst.${VENDOR_TABLE}.grammar.json")
        endif()
        message(STATUS "python ${GRAMMAR_PROCESSING_SCRIPT} --extinst-vendor-grammar=${GRAMMAR_FILE} --vendor-insts-output=${INSTS_FILE} --vendor-operand-kind-prefix=${OPERAND_KIND_PREFIX}")
    endmacro(spvtools_vendor_tables)

    macro(spvtools_extinst_lang_headers NAME GRAMMAR_FILE)
        set(OUT_H spirv/${NAME}.h)
        message(STATUS "python ${LANG_HEADER_PROCESSING_SCRIPT} --extinst-grammar=${GRAMMAR_FILE} --extinst-output-path=${OUT_H}")
    endmacro(spvtools_extinst_lang_headers)

    spvtools_core_tables("unified1")
    spvtools_enum_string_mapping("unified1")
    spvtools_opencl_tables("unified1")
    spvtools_glsl_tables("unified1")
    spvtools_vendor_tables("spv-amd-shader-explicit-vertex-parameter" "spv-amd-sevp" "")
    spvtools_vendor_tables("spv-amd-shader-trinary-minmax" "spv-amd-stm" "")
    spvtools_vendor_tables("spv-amd-gcn-shader" "spv-amd-gs" "")
    spvtools_vendor_tables("spv-amd-shader-ballot" "spv-amd-sb" "")
    spvtools_vendor_tables("debuginfo" "debuginfo" "")
    spvtools_vendor_tables("opencl.debuginfo.100" "cldi100" "CLDEBUG100_")
    spvtools_vendor_tables("nonsemantic.shader.debuginfo.100" "shdi100" "SHDEBUG100_")
    spvtools_vendor_tables("nonsemantic.clspvreflection" "clspvreflection" "")
    spvtools_vendor_tables("nonsemantic.vkspreflection" "vkspreflection" "")
    spvtools_extinst_lang_headers("DebugInfo" ${DEBUGINFO_GRAMMAR_JSON_FILE})
    spvtools_extinst_lang_headers("OpenCLDebugInfo100" ${CLDEBUGINFO100_GRAMMAR_JSON_FILE})
    spvtools_extinst_lang_headers("NonSemanticShaderDebugInfo100" ${VKDEBUGINFO100_GRAMMAR_JSON_FILE})

    set(GLSLANG_BUILD_INFO_H ${glslang_SOURCE_DIR}/glslang/build_info.h)
	message(STATUS "python ${glslang_SOURCE_DIR}/build_info.py ${glslang_SOURCE_DIR} -i ${glslang_SOURCE_DIR}/build_info.h.tmpl -o ${GLSLANG_BUILD_INFO_H}")
endif()