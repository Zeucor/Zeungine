# setup target helper
function(zg_setup_target
	TARGET_NAME
	LIBRARY_TYPE
	LIB_DIR
	LIBRARY_PREFIX LIBRARY_NAME LIBRARY_SUFFIX
	IMPLIB_NAME)
	add_library(${TARGET_NAME} ${LIBRARY_TYPE} IMPORTED)
	if(WINDOWS)
		if(${LIBRARY_TYPE} STREQUAL "STATIC")
			set(LIBRARY_LOCATION "${LIB_DIR}/${LIBRARY_PREFIX}${LIBRARY_NAME}.${LIBRARY_SUFFIX}")
		elseif(${LIBRARY_TYPE} STREQUAL "SHARED")
			set(LIBRARY_LOCATION "${DLL_DIR}/${LIBRARY_NAME}.dll")
			set(IMPLIB_LOCATION "${LIB_DIR}/${IMPLIB_NAME}.lib")
			set_target_properties(${TARGET_NAME} PROPERTIES IMPORTED_IMPLIB ${IMPLIB_LOCATION})
		endif()
	else()
		set(LIBRARY_LOCATION "${LIB_DIR}/${LIBRARY_PREFIX}${LIBRARY_NAME}${LIBRARY_SUFFIX}")
	endif()
	set_target_properties(${TARGET_NAME} PROPERTIES IMPORTED_LOCATION ${LIBRARY_LOCATION})
endfunction()
zg_setup_target(avcodec SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} avcodec ${SHARED_LIB_SUFFIX}
	"")
zg_setup_target(avdevice SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} avdevice ${SHARED_LIB_SUFFIX}
	"")
zg_setup_target(avfilter SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} avfilter ${SHARED_LIB_SUFFIX}
	"")
zg_setup_target(avformat SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} avformat ${SHARED_LIB_SUFFIX}
	"")
zg_setup_target(avutil SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} avutil ${SHARED_LIB_SUFFIX}
	"")
zg_setup_target(swresample SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} swresample ${SHARED_LIB_SUFFIX}
	"")
zg_setup_target(swscale SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} swscale ${SHARED_LIB_SUFFIX}
	"")
zg_setup_target(shaderc SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} shaderc_shared ${SHARED_LIB_SUFFIX}
	"")
zg_setup_target(glslang STATIC
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} glslang ${STATIC_LIB_SUFFIX}
	"")
zg_setup_target(SPIRV-Tools-shared SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} SPIRV-Tools-shared ${SHARED_LIB_SUFFIX}
	"")
zg_setup_target(lunasvg SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} lunasvg ${SHARED_LIB_SUFFIX}
	"")
zg_setup_target(glm STATIC
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} glm ${STATIC_LIB_SUFFIX}
	"")
zg_setup_target(freetype SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} freetype ${SHARED_LIB_SUFFIX}
	"")
zg_setup_target(vk_swiftshader SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} vk_swiftshader ${SHARED_LIB_SUFFIX}
	"")
zg_setup_target(vk_device STATIC
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} vk_device ${STATIC_LIB_SUFFIX}
	"")
zg_setup_target(vk_wsi STATIC
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} vk_wsi ${STATIC_LIB_SUFFIX}
	"")
zg_setup_target(vk_system STATIC
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} vk_system ${STATIC_LIB_SUFFIX}
	"")
zg_setup_target(vk_pipeline STATIC
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} vk_pipeline ${STATIC_LIB_SUFFIX}
	"")
zg_setup_target(marl STATIC
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} marl ${STATIC_LIB_SUFFIX}
	"")
zg_setup_target(miniaudio STATIC
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} miniaudio ${STATIC_LIB_SUFFIX}
	"")