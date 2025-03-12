# Sources
set(ZG_SOURCES
    src/fonts/SystemFonts.cpp
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

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

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

set(ZG_SOURCES ${SPIRV_TOOLS_SOURCES} ${SHADERC_SOURCES} ${SHADERC_UTIL_SOURCES} ${GLSLANG_SOURCES} ${ZG_SOURCES})

option(PRINT_PYTHON_SPIRV_COMMANDS "Whether to print python3 spirv commands" ON)
if(PRINT_PYTHON_SPIRV_COMMANDS)
    set(GRAMMAR_PROCESSING_SCRIPT "${spirvtools_SOURCE_DIR}/utils/generate_grammar_tables.py")
    set(VIMSYNTAX_PROCESSING_SCRIPT "${spirvtools_SOURCE_DIR}/utils/vim/generate_syntax.py")
    set(XML_REGISTRY_PROCESSING_SCRIPT "${spirvtools_SOURCE_DIR}/utils/generate_registry_tables.py")
    set(LANG_HEADER_PROCESSING_SCRIPT "${spirvtools_SOURCE_DIR}/utils/generate_language_headers.py")

    set(DEBUGINFO_GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/unified1/extinst.debuginfo.grammar.json")
    set(CLDEBUGINFO100_GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/unified1/extinst.opencl.debuginfo.100.grammar.json")
    set(VKDEBUGINFO100_GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/unified1/extinst.nonsemantic.shader.debuginfo.100.grammar.json")

    set(GENERATOR_INC_FILE spirv/${GL_OS}/generators.inc)
    set(SPIRV_XML_REGISTRY_FILE ${spirvheaders_SOURCE_DIR}/include/spirv/spir-v.xml)
    message(STATUS "python ${XML_REGISTRY_PROCESSING_SCRIPT} --xml=${SPIRV_XML_REGISTRY_FILE} --generator-output=${GENERATOR_INC_FILE}")

    set(SPIRV_TOOLS_BUILD_VERSION_INC spirv/${GL_OS}/build-version.inc)
    set(SPIRV_TOOLS_BUILD_VERSION_INC_GENERATOR ${spirvtools_SOURCE_DIR}/utils/update_build_version.py)
    set(SPIRV_TOOLS_CHANGES_FILE ${spirvtools_SOURCE_DIR}/CHANGES)
    message(STATUS "python ${SPIRV_TOOLS_BUILD_VERSION_INC_GENERATOR} ${SPIRV_TOOLS_CHANGES_FILE} ${SPIRV_TOOLS_BUILD_VERSION_INC}")

    macro(spvtools_core_tables CONFIG_VERSION)
        set(GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/${CONFIG_VERSION}/spirv.core.grammar.json")
        set(GRAMMAR_INSTS_INC_FILE "spirv/${GL_OS}/core.insts-${CONFIG_VERSION}.inc")
        set(GRAMMAR_KINDS_INC_FILE "spirv/${GL_OS}/operand.kinds-${CONFIG_VERSION}.inc")
        message(STATUS "python ${GRAMMAR_PROCESSING_SCRIPT} --spirv-core-grammar=${GRAMMAR_JSON_FILE} --extinst-debuginfo-grammar=${DEBUGINFO_GRAMMAR_JSON_FILE} --extinst-cldebuginfo100-grammar=${CLDEBUGINFO100_GRAMMAR_JSON_FILE} --core-insts-output=${GRAMMAR_INSTS_INC_FILE} --operand-kinds-output=${GRAMMAR_KINDS_INC_FILE} --output-language=c++")
    endmacro(spvtools_core_tables)

    macro(spvtools_enum_string_mapping CONFIG_VERSION)
        set(GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/${CONFIG_VERSION}/spirv.core.grammar.json")
        set(GRAMMAR_EXTENSION_ENUM_INC_FILE "spirv/${GL_OS}/extension_enum.inc")
        set(GRAMMAR_ENUM_STRING_MAPPING_INC_FILE "spirv/${GL_OS}/enum_string_mapping.inc")
        message(STATUS "python ${GRAMMAR_PROCESSING_SCRIPT} --spirv-core-grammar=${GRAMMAR_JSON_FILE} --extinst-debuginfo-grammar=${DEBUGINFO_GRAMMAR_JSON_FILE} --extinst-cldebuginfo100-grammar=${CLDEBUGINFO100_GRAMMAR_JSON_FILE} --extension-enum-output=${GRAMMAR_EXTENSION_ENUM_INC_FILE} --enum-string-mapping-output=${GRAMMAR_ENUM_STRING_MAPPING_INC_FILE} --output-language=c++")
    endmacro(spvtools_enum_string_mapping)

    macro(spvtools_vimsyntax CONFIG_VERSION CLVERSION)
        set(GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/${CONFIG_VERSION}/spirv.core.grammar.json")
        set(GLSL_GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/${CONFIG_VERSION}/extinst.glsl.std.450.grammar.json")
        set(OPENCL_GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/${CONFIG_VERSION}/extinst.opencl.std.100.grammar.json")
        set(VIMSYNTAX_FILE "spirv/${GL_OS}/spvasm.vim")
        message(STATUS "python ${VIMSYNTAX_PROCESSING_SCRIPT} --spirv-core-grammar=${GRAMMAR_JSON_FILE} --extinst-debuginfo-grammar=${DEBUGINFO_GRAMMAR_JSON_FILE} --extinst-glsl-grammar=${GLSL_GRAMMAR_JSON_FILE} --extinst-opencl-grammar=${OPENCL_GRAMMAR_JSON_FILE}>${VIMSYNTAX_FILE}")
    endmacro(spvtools_vimsyntax)

    macro(spvtools_glsl_tables CONFIG_VERSION)
        set(CORE_GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/${CONFIG_VERSION}/spirv.core.grammar.json")
        set(GLSL_GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/${CONFIG_VERSION}/extinst.glsl.std.450.grammar.json")
        set(GRAMMAR_INC_FILE "spirv/${GL_OS}/glsl.std.450.insts.inc")
        message(STATUS "python ${GRAMMAR_PROCESSING_SCRIPT} --extinst-glsl-grammar=${GLSL_GRAMMAR_JSON_FILE} --glsl-insts-output=${GRAMMAR_INC_FILE} --output-language=c++")
    endmacro(spvtools_glsl_tables)

    macro(spvtools_opencl_tables CONFIG_VERSION)
        set(CORE_GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/${CONFIG_VERSION}/spirv.core.grammar.json")
        set(OPENCL_GRAMMAR_JSON_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/${CONFIG_VERSION}/extinst.opencl.std.100.grammar.json")
        set(GRAMMAR_INC_FILE "spirv/${GL_OS}/opencl.std.insts.inc")
        message(STATUS "python ${GRAMMAR_PROCESSING_SCRIPT} --extinst-opencl-grammar=${OPENCL_GRAMMAR_JSON_FILE} --opencl-insts-output=${GRAMMAR_INC_FILE}")
    endmacro(spvtools_opencl_tables)

    macro(spvtools_vendor_tables VENDOR_TABLE SHORT_NAME OPERAND_KIND_PREFIX)
        set(INSTS_FILE "spirv/${GL_OS}/${VENDOR_TABLE}.insts.inc")
        set(GRAMMAR_FILE "${spirvheaders_SOURCE_DIR}/include/spirv/unified1/extinst.${VENDOR_TABLE}.grammar.json")
        if(NOT EXISTS ${GRAMMAR_FILE})
            set(GRAMMAR_FILE "${spirvtools_SOURCE_DIR}/source/extinst.${VENDOR_TABLE}.grammar.json")
        endif()
        message(STATUS "python ${GRAMMAR_PROCESSING_SCRIPT} --extinst-vendor-grammar=${GRAMMAR_FILE} --vendor-insts-output=${INSTS_FILE} --vendor-operand-kind-prefix=${OPERAND_KIND_PREFIX}")
    endmacro(spvtools_vendor_tables)

    macro(spvtools_extinst_lang_headers NAME GRAMMAR_FILE)
        set(OUT_H spirv/${GL_OS}/${NAME}.h)
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
endif()