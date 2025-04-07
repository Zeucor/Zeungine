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

set(ZG_TARGETS_TO_INSTALL
    boost
    freetype png harfbuzz brotlidec brotlicommon
    glm
    zlib bzip2 lzma zstd
)
set(TARGET_ARTIFACT_FILES_TO_INSTALL "")
foreach(TGT ${ZG_TARGETS_TO_INSTALL})
    if(NOT TARGET ${TGT})
        message(WARNING "Target ${TGT} listed for install does not exist at configure time.")
        continue()
    endif()

    get_target_property(TGT_TYPE ${TGT} TYPE)

    get_target_property(LOC ${TGT} LOCATION)
    if(LOC)
       if(TGT_TYPE STREQUAL "STATIC_LIBRARY" OR TGT_TYPE STREQUAL "SHARED_LIBRARY") # Shared libs have import libs sometimes via LOCATION
           if(EXISTS "${LOC}")
              list(APPEND TARGET_ARTIFACT_FILES_TO_INSTALL "${LOC}")
              message(STATUS "Configure-time path (using LOCATION) for ${TGT} ARCHIVE/IMPORT: ${LOC}")
           else()
              message(WARNING "Configure-time path (using LOCATION) for ${TGT} points to non-existent file: ${LOC}")
           endif()
       endif()
    else()
        get_target_property(ARCHIVE_DIR ${TGT} ARCHIVE_OUTPUT_DIRECTORY)
        get_target_property(ARCHIVE_NAME_PROP ${TGT} ARCHIVE_OUTPUT_NAME)
        if(NOT ARCHIVE_DIR)
             set(ARCHIVE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
        endif()
        if(ARCHIVE_NAME_PROP AND TGT_TYPE STREQUAL "STATIC_LIBRARY")
             get_target_property(PREFIX ${TGT} PREFIX)
             get_target_property(SUFFIX ${TGT} SUFFIX) # e.g. ".lib" or ".a"
             set(ARCHIVE_FILE_PATH "${ARCHIVE_DIR}/${PREFIX}${ARCHIVE_NAME_PROP}${SUFFIX}")
             message(WARNING "Trying to predict archive path for ${TGT} at configure time is fragile: ${ARCHIVE_FILE_PATH}")
        endif()
    endif()
    if(TGT_TYPE STREQUAL "SHARED_LIBRARY" OR TGT_TYPE STREQUAL "MODULE_LIBRARY" OR TGT_TYPE STREQUAL "EXECUTABLE")
        get_target_property(SUFFIX ${TGT} SUFFIX)
        if(WIN32 AND SUFFIX STREQUAL ".lib")
            message(STATUS "Configure-time path (using LOCATION) for ${TGT} appears to be import lib: ${LOC}")
        else()
            list(APPEND TARGET_ARTIFACT_FILES_TO_INSTALL "${LOC}")
            message(STATUS "Configure-time path (using LOCATION) for ${TGT} RUNTIME/LIBRARY: ${LOC}")
        endif()
        else()
            message(WARNING "Cannot reliably determine configure-time RUNTIME/LIBRARY path for ${TGT}.")
        endif()
    endif()
endforeach()

set(ZG_LIBRARIES_TO_INSTALL ${ZG_LIBRARIES_TO_INSTALL} ${TARGET_ARTIFACT_FILES_TO_INSTALL})

install(FILES ${ZG_LIBRARIES_TO_INSTALL}
    DESTINATION ${ZG_LIB_INSTALL_PREFIX}
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
    COMPONENT dependencies)

set(CPACK_COMPONENT_DEPENDENCIES_DESCRIPTION "Zeungine Dependency Libraries")
set(CPACK_COMPONENT_DEPENDENCIES_GROUP "Zeungine")

if(LINUX)
    install(CODE "execute_process(COMMAND /bin/sh -c \"echo ${ZG_LIB_INSTALL_PREFIX} > ${LD_CONF_FILE}\")")
    install(CODE "execute_process(COMMAND /bin/sh -c \"ldconfig\")")
endif()
