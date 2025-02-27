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
	${LIB_PREFIX} avcodec ${LIB_SUFFIX}
	"")
zg_setup_target(avdevice SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} avdevice ${LIB_SUFFIX}
	"")
zg_setup_target(avfilter SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} avfilter ${LIB_SUFFIX}
	"")
zg_setup_target(avformat SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} avformat ${LIB_SUFFIX}
	"")
zg_setup_target(avutil SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} avutil ${LIB_SUFFIX}
	"")
zg_setup_target(swresample SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} swresample ${LIB_SUFFIX}
	"")
zg_setup_target(swscale SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} swscale ${LIB_SUFFIX}
	"")
zg_setup_target(shaderc SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} shaderc_shared ${LIB_SUFFIX}
	"")
zg_setup_target(glslang SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} glslang ${LIB_SUFFIX}
	"")
zg_setup_target(SPIRV-Tools-shared SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} SPIRV-Tools-shared ${LIB_SUFFIX}
	"")
zg_setup_target(lunasvg SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} lunasvg ${LIB_SUFFIX}
	"")
zg_setup_target(glm SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} glm ${LIB_SUFFIX}
	"")
zg_setup_target(freetype SHARED
	${ZG_LIB_INSTALL_PREFIX}
	${LIB_PREFIX} freetype ${LIB_SUFFIX}
	"")