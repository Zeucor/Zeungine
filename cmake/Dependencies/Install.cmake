# ffmpeg includes
zg_setup_target(avcodec ${ZG_TYPE}
    "${ffmpeg_BINARY_DIR}/lib"
    "${ffmpeg_ZG_LIB_PREFIX}" avcodec avcodec ".${ffmpeg_ZG_LIB_SUFFIX}" ON)
zg_setup_target(avdevice ${ZG_TYPE}
    "${ffmpeg_BINARY_DIR}/lib"
    "${ffmpeg_ZG_LIB_PREFIX}" avdevice avdevice ".${ffmpeg_ZG_LIB_SUFFIX}" ON)
zg_setup_target(avfilter ${ZG_TYPE}
    "${ffmpeg_BINARY_DIR}/lib"
    "${ffmpeg_ZG_LIB_PREFIX}" avfilter avfilter ".${ffmpeg_ZG_LIB_SUFFIX}" ON)
zg_setup_target(avformat ${ZG_TYPE}
    "${ffmpeg_BINARY_DIR}/lib"  
    "${ffmpeg_ZG_LIB_PREFIX}" avformat avformat ".${ffmpeg_ZG_LIB_SUFFIX}" ON)
zg_setup_target(avutil ${ZG_TYPE}
    "${ffmpeg_BINARY_DIR}/lib"
    "${ffmpeg_ZG_LIB_PREFIX}" avutil avutil ".${ffmpeg_ZG_LIB_SUFFIX}" ON)
zg_setup_target(swresample ${ZG_TYPE}
    "${ffmpeg_BINARY_DIR}/lib"
    "${ffmpeg_ZG_LIB_PREFIX}" swresample swresample ".${ffmpeg_ZG_LIB_SUFFIX}" ON)
zg_setup_target(swscale ${ZG_TYPE}
    "${ffmpeg_BINARY_DIR}/lib"
    "${ffmpeg_ZG_LIB_PREFIX}" swscale swscale ".${ffmpeg_ZG_LIB_SUFFIX}" ON)
if(WIN32)
    set(openssl_LIB_DIR "${openssl_BINARY_DIR}/lib")
elseif(UNIX)
    set(openssl_LIB_DIR "${openssl_BINARY_DIR}/lib64")
endif()
if(NOT MACOS AND NOT LINK_SYS_OPENSSL)
    zg_setup_target(ssl STATIC
        "${openssl_LIB_DIR}"
        "${ffmpeg_ZG_LIB_PREFIX}" ssl ssl "${STATIC_ZG_LIB_SUFFIX}" ON)
    zg_setup_target(crypto STATIC
        "${openssl_LIB_DIR}"
        "${ffmpeg_ZG_LIB_PREFIX}" crypto crypto "${STATIC_ZG_LIB_SUFFIX}" ON)
endif()
if(ENABLE_TRACY)
    if(WINDOWS)
        set(TRACY_EXT .exe)
    endif()
    install(FILES ${CMAKE_BINARY_DIR}/tracyserver/tracy-profiler${TRACY_EXT}
        DESTINATION ${ZG_BIN_INSTALL_PREFIX}
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
        COMPONENT Dependencies)
endif()
set(ZG_TARGETS_TO_INSTALL
    boost
    freetype png harfbuzz brotlidec brotlicommon
    glm jolt
    zlib bzip2 lzma zstd
    miniaudio glsl svg
    ttf2mesh assimp
    rtmidi
    tinyfiledialogs
    mc33 msdf_atlas_gen
)
if(ENABLE_TRACY)
    set(ZG_TARGET_TO_INSTALL
        ${ZG_TARGET_TO_INSTALL}
        TracyClient)
endif()
set(TARGET_ARTIFACT_FILES_TO_INSTALL "")
foreach(TGT ${ZG_TARGETS_TO_INSTALL})
    if(NOT TARGET ${TGT})
        continue()
    endif()
    set(TARGET_ARTIFACT_FILES_TO_INSTALL ${TARGET_ARTIFACT_FILES_TO_INSTALL} "${CMAKE_BINARY_DIR}/${ZG_LIB_PREFIX}${TGT}${TYPE_ZG_LIB_SUFFIX}")
endforeach()

set(ZG_LIBRARIES_TO_INSTALL ${ZG_LIBRARIES_TO_INSTALL} ${TARGET_ARTIFACT_FILES_TO_INSTALL})

message(STATUS "ZG_LIBRARIES_TO_INSTALL: ${ZG_LIBRARIES_TO_INSTALL}")

install(FILES ${ZG_LIBRARIES_TO_INSTALL}
    DESTINATION ${ZG_LIB_INSTALL_PREFIX}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    COMPONENT Dependencies)

set(CPACK_COMPONENT_DEPENDENCIES_DESCRIPTION "Zeungine Dependency Libraries")
set(CPACK_COMPONENT_DEPENDENCIES_GROUP "Zeungine")

if(LINUX)
    install(CODE "execute_process(COMMAND /bin/sh -c \"echo ${ZG_LIB_INSTALL_PREFIX} > ${LD_CONF_FILE}\")")
    install(CODE "execute_process(COMMAND /bin/sh -c \"ldconfig\")")
endif()
