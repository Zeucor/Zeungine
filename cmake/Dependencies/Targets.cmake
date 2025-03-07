# setup target helper
function(zg_setup_target
	TARGET_NAME
	LIBRARY_TYPE
	LIBRARY_DIR
	LIBRARY_PREFIX LIBRARY_NAME IMPLIB_NAME LIBRARY_SUFFIX)
	add_library(${TARGET_NAME} ${LIBRARY_TYPE} IMPORTED)
	if(WIN32)
		if(${LIBRARY_TYPE} STREQUAL "STATIC")
			set(LIBRARY_LOCATION "${LIBRARY_DIR}/${LIBRARY_PREFIX}${LIBRARY_NAME}${LIBRARY_SUFFIX}")
		elseif(${LIBRARY_TYPE} STREQUAL "SHARED")
			set(LIBRARY_LOCATION "${LIBRARY_DIR}/${LIBRARY_NAME}.dll")
			set(IMPLIB_LOCATION "${LIBRARY_DIR}/${IMPLIB_NAME}.lib")
			set_target_properties(${TARGET_NAME} PROPERTIES IMPORTED_IMPLIB ${IMPLIB_LOCATION})
		endif()
	else()
		set(LIBRARY_LOCATION "${LIBRARY_DIR}/${LIBRARY_PREFIX}${LIBRARY_NAME}${LIBRARY_SUFFIX}")
	endif()
	message(STATUS "LIBRARY_LOCATION: ${LIBRARY_LOCATION}")
	message(STATUS "TARGET_NAME: ${TARGET_NAME}")
	set_target_properties(${TARGET_NAME} PROPERTIES IMPORTED_LOCATION ${LIBRARY_LOCATION})
endfunction()
zg_setup_target(avcodec SHARED
	"${ZG_ABS_LIB_INSTALL_PREFIX}"
	"${ZG_LIB_PREFIX}" avcodec avcodec "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(avdevice SHARED
	"${ZG_ABS_LIB_INSTALL_PREFIX}"
	"${ZG_LIB_PREFIX}" avdevice avdevice "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(avfilter SHARED
	"${ZG_ABS_LIB_INSTALL_PREFIX}"
	"${ZG_LIB_PREFIX}" avfilter avfilter "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(avformat SHARED
	"${ZG_ABS_LIB_INSTALL_PREFIX}"
	"${ZG_LIB_PREFIX}" avformat avformat "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(avutil SHARED
	"${ZG_ABS_LIB_INSTALL_PREFIX}"
	"${ZG_LIB_PREFIX}" avutil avutil "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(swresample SHARED
	"${ZG_ABS_LIB_INSTALL_PREFIX}"
	"${ZG_LIB_PREFIX}" swresample swresample "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(swscale SHARED
	"${ZG_ABS_LIB_INSTALL_PREFIX}"
	"${ZG_LIB_PREFIX}" swscale swscale "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(shaderc SHARED
	"${ZG_ABS_LIB_INSTALL_PREFIX}"
	"${ZG_LIB_PREFIX}" shaderc_shared shaderc_shared "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(glslang STATIC
	"${ZG_ABS_LIB_INSTALL_PREFIX}"
	"${ZG_LIB_PREFIX}" glslang glslang "${STATIC_ZG_LIB_SUFFIX}")
zg_setup_target(SPIRV-Tools-shared SHARED
	"${ZG_ABS_LIB_INSTALL_PREFIX}"
	"${ZG_LIB_PREFIX}" SPIRV-Tools-shared SPIRV-Tools-shared "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(lunasvg SHARED
	"${ZG_ABS_LIB_INSTALL_PREFIX}"
	"${ZG_LIB_PREFIX}" lunasvg lunasvg "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(plutovg SHARED
	"${ZG_ABS_LIB_INSTALL_PREFIX}"
	"${ZG_LIB_PREFIX}" plutovg plutovg "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(glm STATIC
	"${ZG_ABS_LIB_INSTALL_PREFIX}"
	"${ZG_LIB_PREFIX}" glm glm "${STATIC_ZG_LIB_SUFFIX}")
zg_setup_target(freetype SHARED
	"${ZG_ABS_LIB_INSTALL_PREFIX}"
	"${ZG_LIB_PREFIX}" freetype freetype "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(vk_swiftshader SHARED
	"${ZG_ABS_LIB_INSTALL_PREFIX}"
	"${ZG_LIB_PREFIX}" vk_swiftshader vk_swiftshader "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(vk_device STATIC
	"${ZG_ABS_LIB_INSTALL_PREFIX}"
	"${ZG_LIB_PREFIX}" vk_device vk_device "${STATIC_ZG_LIB_SUFFIX}")
zg_setup_target(vk_wsi STATIC
	"${ZG_ABS_LIB_INSTALL_PREFIX}"
	"${ZG_LIB_PREFIX}" vk_wsi vk_wsi "${STATIC_ZG_LIB_SUFFIX}")
zg_setup_target(vk_system STATIC
	"${ZG_ABS_LIB_INSTALL_PREFIX}"
	"${ZG_LIB_PREFIX}" vk_system vk_system "${STATIC_ZG_LIB_SUFFIX}")
zg_setup_target(vk_pipeline STATIC
	"${ZG_ABS_LIB_INSTALL_PREFIX}"
	"${ZG_LIB_PREFIX}" vk_pipeline vk_pipeline "${STATIC_ZG_LIB_SUFFIX}")
zg_setup_target(marl STATIC
	"${ZG_ABS_LIB_INSTALL_PREFIX}"
	"${ZG_LIB_PREFIX}" marl marl "${STATIC_ZG_LIB_SUFFIX}")
zg_setup_target(miniaudio STATIC
	"${ZG_ABS_LIB_INSTALL_PREFIX}"
	"${ZG_LIB_PREFIX}" miniaudio miniaudio "${STATIC_ZG_LIB_SUFFIX}")