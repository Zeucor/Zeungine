# ffmpeg includes

if(WIN32)
    set(ffmpeg_ZG_LIB_PREFIX "")
    set(ffmpeg_ZG_LIB_SUFFIX "dll")
elseif(APPLE)
    set(ffmpeg_ZG_LIB_PREFIX "lib")
    set(ffmpeg_ZG_LIB_SUFFIX "dylib")
else()
    set(ffmpeg_ZG_LIB_PREFIX "lib")
    set(ffmpeg_ZG_LIB_SUFFIX "so")
endif()
if(UNIX)
    install(FILES
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avcodec.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avdevice.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avfilter.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avformat.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avutil.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}swresample.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}swscale.${ffmpeg_ZG_LIB_SUFFIX}"
        DESTINATION ${ZG_LIB_INSTALL_PREFIX}
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
elseif(WIN32)
    install(FILES
        "${ffmpeg_BINARY_DIR}/bin/${ffmpeg_ZG_LIB_PREFIX}avcodec-61.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/bin/${ffmpeg_ZG_LIB_PREFIX}avdevice-61.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/bin/${ffmpeg_ZG_LIB_PREFIX}avfilter-10.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/bin/${ffmpeg_ZG_LIB_PREFIX}avformat-61.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/bin/${ffmpeg_ZG_LIB_PREFIX}avutil-59.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/bin/${ffmpeg_ZG_LIB_PREFIX}swresample-5.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/bin/${ffmpeg_ZG_LIB_PREFIX}swscale-8.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/bin/avcodec.lib"
        "${ffmpeg_BINARY_DIR}/bin/avdevice.lib"
        "${ffmpeg_BINARY_DIR}/bin/avfilter.lib"
        "${ffmpeg_BINARY_DIR}/bin/avformat.lib"
        "${ffmpeg_BINARY_DIR}/bin/avutil.lib"
        "${ffmpeg_BINARY_DIR}/bin/swresample.lib"
        "${ffmpeg_BINARY_DIR}/bin/swscale.lib"
        DESTINATION ${ZG_LIB_INSTALL_PREFIX}
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
endif()
IF(APPLE)
    install(FILES
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avcodec.61.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avcodec.61.19.100.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avdevice.61.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avdevice.61.3.100.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avfilter.10.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avfilter.10.4.100.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avformat.61.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avformat.61.7.100.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avutil.59.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avutil.59.39.100.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}swresample.5.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}swresample.5.3.100.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}swscale.8.${ffmpeg_ZG_LIB_SUFFIX}"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}swscale.8.3.100.${ffmpeg_ZG_LIB_SUFFIX}"
        DESTINATION ${ZG_LIB_INSTALL_PREFIX}
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
elseif(UNIX AND NOT APPLE)
    install(FILES
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avcodec.${ffmpeg_ZG_LIB_SUFFIX}.61"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avcodec.${ffmpeg_ZG_LIB_SUFFIX}.61.19.100"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avdevice.${ffmpeg_ZG_LIB_SUFFIX}.61"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avdevice.${ffmpeg_ZG_LIB_SUFFIX}.61.3.100"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avfilter.${ffmpeg_ZG_LIB_SUFFIX}.10"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avfilter.${ffmpeg_ZG_LIB_SUFFIX}.10.4.100"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avformat.${ffmpeg_ZG_LIB_SUFFIX}.61"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avformat.${ffmpeg_ZG_LIB_SUFFIX}.61.7.100"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avutil.${ffmpeg_ZG_LIB_SUFFIX}.59"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}avutil.${ffmpeg_ZG_LIB_SUFFIX}.59.39.100"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}swresample.${ffmpeg_ZG_LIB_SUFFIX}.5"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}swresample.${ffmpeg_ZG_LIB_SUFFIX}.5.3.100"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}swscale.${ffmpeg_ZG_LIB_SUFFIX}.8"
        "${ffmpeg_BINARY_DIR}/lib/${ffmpeg_ZG_LIB_PREFIX}swscale.${ffmpeg_ZG_LIB_SUFFIX}.8.3.100"
        DESTINATION ${ZG_LIB_INSTALL_PREFIX}
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
endif()
message(STATUS "ZG_LIB_INSTALL_PREFIX: ${ZG_LIB_INSTALL_PREFIX}")

# datsit frrom ffmpeg

#add new targets to enj
install(TARGETS
    freetype lunasvg plutovg
    glm
    shaderc_shared
    glslang SPIRV-Tools-shared
    vk_swiftshader vk_device vk_wsi vk_system vk_pipeline
    marl
    miniaudio
    ARCHIVE DESTINATION ${ZG_LIB_INSTALL_PREFIX}
    LIBRARY DESTINATION ${ZG_LIB_INSTALL_PREFIX})

if(WIN32)
    install(FILES
        ${CMAKE_BINARY_DIR}/vk_swiftshader.dll
        DESTINATION ${ZG_LIB_INSTALL_PREFIX}
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
endif()

if(UNIX AND NOT APPLE)
    install(CODE "execute_process(COMMAND /bin/sh -c \"echo ${ZG_LIB_INSTALL_PREFIX} > ${LD_CONF_FILE}\")")
    install(CODE "execute_process(COMMAND /bin/sh -c \"ldconfig\")")
endif()