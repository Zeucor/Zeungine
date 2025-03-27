if(ZG_TYPE STREQUAL STATIC)
	set(TYPE_ZG_LIB_SUFFIX ${STATIC_ZG_LIB_SUFFIX})
elseif(ZG_TYPE STREQUAL SHARED)
	set(TYPE_ZG_LIB_SUFFIX ${SHARED_ZG_LIB_SUFFIX})
endif()
zg_setup_target(freetype ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" freetype freetype "${TYPE_ZG_LIB_SUFFIX}" ON)
zg_setup_target(png STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ffmpeg_ZG_LIB_PREFIX}" ${PNG_LIBRARY_REAL} ${PNG_LIBRARY_REAL} "${STATIC_ZG_LIB_SUFFIX}" ON)
zg_setup_target(harfbuzz STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ffmpeg_ZG_LIB_PREFIX}" harfbuzz harfbuzz "${STATIC_ZG_LIB_SUFFIX}" ON)
zg_setup_target(brotlidec STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ffmpeg_ZG_LIB_PREFIX}" brotlidec brotlidec "${STATIC_ZG_LIB_SUFFIX}" ON)
zg_setup_target(avformat ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ffmpeg_ZG_LIB_PREFIX}" ${avformat_NAME} avformat ".${ffmpeg_ZG_LIB_SUFFIX}" ON)
zg_setup_target(avdevice ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ffmpeg_ZG_LIB_PREFIX}" ${avdevice_NAME} avdevice ".${ffmpeg_ZG_LIB_SUFFIX}" ON)
zg_setup_target(avcodec ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ffmpeg_ZG_LIB_PREFIX}" ${avcodec_NAME} avcodec ".${ffmpeg_ZG_LIB_SUFFIX}" ON)
zg_setup_target(avfilter ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ffmpeg_ZG_LIB_PREFIX}" ${avfilter_NAME} avfilter ".${ffmpeg_ZG_LIB_SUFFIX}" ON)
zg_setup_target(avutil ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ffmpeg_ZG_LIB_PREFIX}" ${avutil_NAME} avutil ".${ffmpeg_ZG_LIB_SUFFIX}" ON)
zg_setup_target(swresample ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ffmpeg_ZG_LIB_PREFIX}" ${swresample_NAME} swresample ".${ffmpeg_ZG_LIB_SUFFIX}" ON)
zg_setup_target(swscale ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ffmpeg_ZG_LIB_PREFIX}" ${swscale_NAME} swscale ".${ffmpeg_ZG_LIB_SUFFIX}" ON)
if(ZG_TYPE STREQUAL STATIC)
	zg_setup_target(zlib STATIC
		"${ZG_LIB_INSTALL_PREFIX_ABS}"
		"${ZG_LIB_PREFIX}" zlib zlib "${STATIC_ZG_LIB_SUFFIX}" ON)
	zg_setup_target(bzip2 STATIC
		"${ZG_LIB_INSTALL_PREFIX_ABS}"
		"${ZG_LIB_PREFIX}" bzip2 bzip2 "${STATIC_ZG_LIB_SUFFIX}" ON)
endif()
if(MACOS)
	find_package(OpenSSL REQUIRED)
	set(ZG_LIBRARIES
		${ZG_LIBRARIES}
		OpenSSL::SSL
		OpenSSL::Crypto
		"-framework AudioToolbox"
		"-framework CoreMedia"
		"-framework VideoToolbox"
		"-framework Security"
		iconv)
else()
	zg_setup_target(ssl STATIC
		"${ZG_LIB_INSTALL_PREFIX_ABS}"
		"${ffmpeg_ZG_LIB_PREFIX}" ssl ssl "${STATIC_ZG_LIB_SUFFIX}" ON)
	zg_setup_target(crypto STATIC
		"${ZG_LIB_INSTALL_PREFIX_ABS}"
		"${ffmpeg_ZG_LIB_PREFIX}" crypto crypto "${STATIC_ZG_LIB_SUFFIX}" ON)
endif()
if(WIN32)
	if(ZG_TYPE STREQUAL STATIC)
	# ucrt
		set(ZG_LIBRARIES ${ZG_LIBRARIES} ws2_32 secur32 crypt32 bcrypt mfplat mf mfuuid strmiids advapi32  iphlpapi legacy_stdio_definitions)
		# if (RELEASE_OR_DEBUG STREQUAL Release)
		# 	set(ZG_LIBRARIES ${ZG_LIBRARIES} msvcrt)
		# else()
		# 	set(ZG_LIBRARIES ${ZG_LIBRARIES} msvcrtd)
		# endif()
	endif()
endif()
# zg_setup_target(lunasvg ${ZG_TYPE}
# 	"${ZG_LIB_INSTALL_PREFIX_ABS}"
# 	"${ZG_LIB_PREFIX}" lunasvg lunasvg "${TYPE_ZG_LIB_SUFFIX}" ON)
# zg_setup_target(plutovg ${ZG_TYPE}
# 	"${ZG_LIB_INSTALL_PREFIX_ABS}"
# 	"${ZG_LIB_PREFIX}" plutovg plutovg "${TYPE_ZG_LIB_SUFFIX}" ON)
zg_setup_target(glm ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" glm glm "${TYPE_ZG_LIB_SUFFIX}" ON)
# zg_setup_target(vk_device ${ZG_TYPE}
# 	"${ZG_LIB_INSTALL_PREFIX_ABS}"
# 	"${ZG_LIB_PREFIX}" vk_device vk_device "${TYPE_ZG_LIB_SUFFIX}" ON)
# zg_setup_target(vk_wsi ${ZG_TYPE}
# 	"${ZG_LIB_INSTALL_PREFIX_ABS}"
# 	"${ZG_LIB_PREFIX}" vk_wsi vk_wsi "${TYPE_ZG_LIB_SUFFIX}" ON)
# zg_setup_target(vk_system ${ZG_TYPE}
# 	"${ZG_LIB_INSTALL_PREFIX_ABS}"
# 	"${ZG_LIB_PREFIX}" vk_system vk_system "${TYPE_ZG_LIB_SUFFIX}" ON)
# zg_setup_target(vk_pipeline ${ZG_TYPE}
# 	"${ZG_LIB_INSTALL_PREFIX_ABS}"
# 	"${ZG_LIB_PREFIX}" vk_pipeline vk_pipeline "${TYPE_ZG_LIB_SUFFIX}" ON)
# zg_setup_target(marl ${ZG_TYPE}
# 	"${ZG_LIB_INSTALL_PREFIX_ABS}"
# 	"${ZG_LIB_PREFIX}" marl marl "${TYPE_ZG_LIB_SUFFIX}" ON)
# zg_setup_target(miniaudio ${ZG_TYPE}
# 	"${ZG_LIB_INSTALL_PREFIX_ABS}"
# 	"${ZG_LIB_PREFIX}" miniaudio miniaudio "${TYPE_ZG_LIB_SUFFIX}" ON)
if(WIN32)
	find_package(OpenGL REQUIRED)
	set(ZG_LIBRARIES ${ZG_LIBRARIES} OpenGL::GL)
elseif(LINUX)
	set(ZG_LIBRARIES ${ZG_LIBRARIES} GL)
endif()
# set(ZG_LIBRARIES ${ZG_LIBRARIES} freetype)
# set(ZG_LIBRARIES ${ZG_LIBRARIES} bvh)
if(LINUX)
	set(ZG_LIBRARIES ${ZG_LIBRARIES} xcb xkbcommon xcb-util xcb-keysyms xcb-xfixes X11 Xrandr Xext Xfixes xkbcommon Xrender)
	if(USE_WAYLAND)
		set(ZG_LIBRARIES ${ZG_LIBRARIES} wayland-client)
	endif()
	set(ZG_LIBRARIES ${ZG_LIBRARIES} drm)
elseif(MACOS)
	set(ZG_LIBRARIES ${ZG_LIBRARIES} "-framework Cocoa" "-framework QuartzCore" "-framework Metal")
endif()
find_package(Vulkan REQUIRED)
if(WIN32)
	set(ZG_LIBRARIES ${ZG_LIBRARIES} ${Vulkan_LIBRARIES})
endif()