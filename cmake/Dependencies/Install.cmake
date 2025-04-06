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
if(NOT MACOS)
    zg_setup_target(ssl STATIC
        "${openssl_LIB_DIR}"
        "${ffmpeg_ZG_LIB_PREFIX}" ssl ssl "${STATIC_ZG_LIB_SUFFIX}" ON)
    zg_setup_target(crypto STATIC
        "${openssl_LIB_DIR}"
        "${ffmpeg_ZG_LIB_PREFIX}" crypto crypto "${STATIC_ZG_LIB_SUFFIX}" ON)
endif()

install(FILES ${ZG_LIBRARIES_TO_INSTALL}
    DESTINATION ${ZG_LIB_INSTALL_PREFIX}
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
    COMPONENT dependencies)

install(TARGETS
    boost
    freetype png harfbuzz brotlidec brotlicommon
    glm
    zlib bzip2 lzma zstd
    ARCHIVE DESTINATION ${ZG_LIB_INSTALL_PREFIX}
    LIBRARY DESTINATION ${ZG_LIB_INSTALL_PREFIX}
    RUNTIME DESTINATION ${ZG_LIB_INSTALL_PREFIX}
    COMPONENT dependencies)

set(CPACK_COMPONENT_DEPENDENCIES_DESCRIPTION "Zeungine Dependency Libraries")
set(CPACK_COMPONENT_DEPENDENCIES_GROUP "Zeungine")

if(LINUX)
    install(CODE "execute_process(COMMAND /bin/sh -c \"echo ${ZG_LIB_INSTALL_PREFIX} > ${LD_CONF_FILE}\")")
    install(CODE "execute_process(COMMAND /bin/sh -c \"ldconfig\")")
endif()
