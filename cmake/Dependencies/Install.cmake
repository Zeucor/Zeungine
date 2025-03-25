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
zg_setup_target(ssl STATIC
    "${openssl_BINARY_DIR}/lib"
    "${ffmpeg_ZG_LIB_PREFIX}" ssl ssl "${STATIC_ZG_LIB_SUFFIX}" ON)
zg_setup_target(crypto STATIC
    "${openssl_BINARY_DIR}/lib"
    "${ffmpeg_ZG_LIB_PREFIX}" crypto crypto "${STATIC_ZG_LIB_SUFFIX}" ON)

install(FILES ${ZG_LIBRARIES_TO_INSTALL}
    DESTINATION ${ZG_LIB_INSTALL_PREFIX}
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

install(TARGETS
    freetype
    glm
    ARCHIVE DESTINATION ${ZG_LIB_INSTALL_PREFIX}
    LIBRARY DESTINATION ${ZG_LIB_INSTALL_PREFIX}
    RUNTIME DESTINATION ${ZG_LIB_INSTALL_PREFIX})
if(ZG_TYPE STREQUAL STATIC)
    install(TARGETS
        zlib bzip2
        ARCHIVE DESTINATION ${ZG_LIB_INSTALL_PREFIX}
        LIBRARY DESTINATION ${ZG_LIB_INSTALL_PREFIX}
        RUNTIME DESTINATION ${ZG_LIB_INSTALL_PREFIX})
endif()

if(UNIX AND NOT APPLE)
    install(CODE "execute_process(COMMAND /bin/sh -c \"echo ${ZG_LIB_INSTALL_PREFIX} > ${LD_CONF_FILE}\")")
    install(CODE "execute_process(COMMAND /bin/sh -c \"ldconfig\")")
endif()
