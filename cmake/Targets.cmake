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
if(ZG_TYPE STREQUAL STATIC)
	set(TYPE_ZG_LIB_SUFFIX ${STATIC_ZG_LIB_SUFFIX})
elseif(ZG_TYPE STREQUAL SHARED)
	set(TYPE_ZG_LIB_SUFFIX ${SHARED_ZG_LIB_SUFFIX})
endif()
zg_setup_target(avformat ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" ${avformat_NAME} avformat "${TYPE_ZG_LIB_SUFFIX}" ON)
zg_setup_target(avdevice ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" ${avdevice_NAME} avdevice "${TYPE_ZG_LIB_SUFFIX}" ON)
zg_setup_target(avcodec ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" ${avcodec_NAME} avcodec "${TYPE_ZG_LIB_SUFFIX}" ON)
zg_setup_target(avfilter ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" ${avfilter_NAME} avfilter "${TYPE_ZG_LIB_SUFFIX}" ON)
zg_setup_target(avutil ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" ${avutil_NAME} avutil "${TYPE_ZG_LIB_SUFFIX}" ON)
zg_setup_target(swresample ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" ${swresample_NAME} swresample "${TYPE_ZG_LIB_SUFFIX}" ON)
zg_setup_target(swscale ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" ${swscale_NAME} swscale "${TYPE_ZG_LIB_SUFFIX}" ON)
if(ZG_TYPE STREQUAL SHARED)
	zg_setup_target(shaderc SHARED
		"${ZG_LIB_INSTALL_PREFIX_ABS}"
		"${ZG_LIB_PREFIX}" shaderc_shared shaderc_shared "${SHARED_ZG_LIB_SUFFIX}" ON)
	zg_setup_target(SPIRV-Tools SHARED
		"${ZG_LIB_INSTALL_PREFIX_ABS}"
		"${ZG_LIB_PREFIX}" SPIRV-Tools-shared SPIRV-Tools-shared "${SHARED_ZG_LIB_SUFFIX}" ON)
elseif(ZG_TYPE STREQUAL STATIC)
	zg_setup_target(zlib STATIC
		"${ZG_LIB_INSTALL_PREFIX_ABS}"
		"${ZG_LIB_PREFIX}" zlib zlib "${STATIC_ZG_LIB_SUFFIX}" ON)
	zg_setup_target(bzip2 STATIC
		"${ZG_LIB_INSTALL_PREFIX_ABS}"
		"${ZG_LIB_PREFIX}" bzip2 bzip2 "${STATIC_ZG_LIB_SUFFIX}" ON)
endif()
zg_setup_target(lunasvg ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" lunasvg lunasvg "${TYPE_ZG_LIB_SUFFIX}" ON)
zg_setup_target(plutovg ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" plutovg plutovg "${TYPE_ZG_LIB_SUFFIX}" ON)
zg_setup_target(glm ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" glm glm "${TYPE_ZG_LIB_SUFFIX}" ON)
zg_setup_target(freetype ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" freetype freetype "${TYPE_ZG_LIB_SUFFIX}" ON)
zg_setup_target(vk_device ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" vk_device vk_device "${TYPE_ZG_LIB_SUFFIX}" ON)
zg_setup_target(vk_wsi ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" vk_wsi vk_wsi "${TYPE_ZG_LIB_SUFFIX}" ON)
zg_setup_target(vk_system ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" vk_system vk_system "${TYPE_ZG_LIB_SUFFIX}" ON)
zg_setup_target(vk_pipeline ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" vk_pipeline vk_pipeline "${TYPE_ZG_LIB_SUFFIX}" ON)
zg_setup_target(marl ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" marl marl "${TYPE_ZG_LIB_SUFFIX}" ON)
zg_setup_target(miniaudio ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" miniaudio miniaudio "${TYPE_ZG_LIB_SUFFIX}" ON)
if(WIN32)
	find_package(OpenGL REQUIRED)
	set(ZG_LIBRARIES ${ZG_LIBRARIES} OpenGL::GL)
elseif(LINUX)
	set(ZG_LIBRARIES ${ZG_LIBRARIES} GL)
endif()
# set(ZG_LIBRARIES ${ZG_LIBRARIES} freetype)
# set(ZG_LIBRARIES ${ZG_LIBRARIES} bvh)
if(LINUX)
	if(USE_X11)
		set(ZG_LIBRARIES ${ZG_LIBRARIES} X11 Xrandr Xext Xfixes xkbcommon Xrender)
	endif()
	if(USE_XCB OR USE_X11)
		set(ZG_LIBRARIES ${ZG_LIBRARIES} xcb xkbcommon xcb-util xcb-keysyms xcb-xfixes X11 Xrandr Xext Xfixes xkbcommon Xrender)
	endif()
	if(USE_WAYLAND)
		set(ZG_LIBRARIES ${ZG_LIBRARIES} wayland-client)
	endif()
	set(ZG_LIBRARIES ${ZG_LIBRARIES} drm)
elseif(MACOS)
	set(ZG_LIBRARIES ${ZG_LIBRARIES} "-framework Cocoa" "-framework QuartzCore" "-framework Metal")
endif()
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
if(WIN32)
	set(ZG_LIBRARIES ${ZG_LIBRARIES} ${Vulkan_LIBRARIES})
endif()