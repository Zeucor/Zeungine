if(WIN32)
	set(avcodec_NAME avcodec-61)
	set(avdevice_NAME avdevice-61)
	set(avfilter_NAME avfilter-10)
	set(avformat_NAME avformat-61)
	set(avutil_NAME avutil-59)
	set(swresample_NAME swresample-5)
	set(swscale_NAME swscale-8)
else()
	set(avcodec_NAME avcodec)
	set(avdevice_NAME avdevice)
	set(avfilter_NAME avfilter)
	set(avformat_NAME avformat)
	set(avutil_NAME avutil)
	set(swresample_NAME swresample)
	set(swscale_NAME swscale)
endif()
zg_setup_target(avcodec SHARED
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" ${avcodec_NAME} avcodec "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(avdevice SHARED
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" ${avdevice_NAME} avdevice "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(avfilter SHARED
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" ${avfilter_NAME} avfilter "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(avformat SHARED
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" ${avformat_NAME} avformat "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(avutil SHARED
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" ${avutil_NAME} avutil "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(swresample SHARED
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" ${swresample_NAME} swresample "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(swscale SHARED
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" ${swscale_NAME} swscale "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(shaderc SHARED
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" shaderc_shared shaderc_shared "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(glslang STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" glslang glslang "${STATIC_ZG_LIB_SUFFIX}")
zg_setup_target(SPIRV-Tools-shared SHARED
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" SPIRV-Tools-shared SPIRV-Tools-shared "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(lunasvg SHARED
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" lunasvg lunasvg "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(plutovg SHARED
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" plutovg plutovg "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(glm STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" glm glm "${STATIC_ZG_LIB_SUFFIX}")
zg_setup_target(freetype SHARED
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" freetype freetype "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(vk_swiftshader SHARED
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" vk_swiftshader vk_swiftshader "${SHARED_ZG_LIB_SUFFIX}")
zg_setup_target(vk_device STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" vk_device vk_device "${STATIC_ZG_LIB_SUFFIX}")
zg_setup_target(vk_wsi STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" vk_wsi vk_wsi "${STATIC_ZG_LIB_SUFFIX}")
zg_setup_target(vk_system STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" vk_system vk_system "${STATIC_ZG_LIB_SUFFIX}")
zg_setup_target(vk_pipeline STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" vk_pipeline vk_pipeline "${STATIC_ZG_LIB_SUFFIX}")
zg_setup_target(marl STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" marl marl "${STATIC_ZG_LIB_SUFFIX}")
zg_setup_target(miniaudio STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" miniaudio miniaudio "${STATIC_ZG_LIB_SUFFIX}")