zg_setup_target(glsl STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" glsl glsl "${STATIC_ZG_LIB_SUFFIX}" ON)
zg_setup_target(svg STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" svg svg "${STATIC_ZG_LIB_SUFFIX}" ON)
zg_setup_target(miniaudio STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" miniaudio miniaudio "${STATIC_ZG_LIB_SUFFIX}" ON)
zg_setup_target(ttf2mesh STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" ttf2mesh ttf2mesh "${STATIC_ZG_LIB_SUFFIX}" ON)
zg_setup_target(assimp STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" assimp assimp "${STATIC_ZG_LIB_SUFFIX}" ON)
zg_setup_target(jolt STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" jolt jolt "${STATIC_ZG_LIB_SUFFIX}" ON)
zg_setup_target(rtmidi STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" rtmidi rtmidi "${STATIC_ZG_LIB_SUFFIX}" ON)
zg_setup_target(tinyfiledialogs STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" tinyfiledialogs tinyfiledialogs "${STATIC_ZG_LIB_SUFFIX}" ON)
if(ENABLE_TRACY)
	zg_setup_target(TracyClient STATIC
		"${ZG_LIB_INSTALL_PREFIX_ABS}"
		"${ZG_LIB_PREFIX}" TracyClient TracyClient "${STATIC_ZG_LIB_SUFFIX}" ON)
endif()
zg_setup_target(freetype ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" freetype freetype "${TYPE_ZG_LIB_SUFFIX}" ON)
zg_setup_target(png STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${PNG_ZG_LIB_PREFIX}" png png "${PNG_ZG_LIB_SUFFIX}" ON)
zg_setup_target(harfbuzz STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${HB_ZG_LIB_PREFIX}" harfbuzz harfbuzz "${HB_ZG_LIB_SUFFIX}" ON)
zg_setup_target(brotlidec STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${PNG_ZG_LIB_PREFIX}" brotlidec brotlidec "${PNG_ZG_LIB_SUFFIX}" ON)
zg_setup_target(brotlicommon STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${PNG_ZG_LIB_PREFIX}" brotlicommon brotlicommon "${PNG_ZG_LIB_SUFFIX}" ON)
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
zg_setup_target(boost ${ZG_TYPE}
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" boost boost "${TYPE_ZG_LIB_SUFFIX}" ON)
zg_setup_target(zlib STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" zlib zlib "${STATIC_ZG_LIB_SUFFIX}" ON)
zg_setup_target(bzip2 STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" bzip2 bzip2 "${STATIC_ZG_LIB_SUFFIX}" ON)
zg_setup_target(lzma STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" lzma lzma "${STATIC_ZG_LIB_SUFFIX}" ON)
zg_setup_target(zstd STATIC
	"${ZG_LIB_INSTALL_PREFIX_ABS}"
	"${ZG_LIB_PREFIX}" zstd zstd "${STATIC_ZG_LIB_SUFFIX}" ON)
if(MACOS OR LINK_SYS_OPENSSL)
	find_package(OpenSSL REQUIRED)
	set(ZG_LIBRARIES
		${ZG_LIBRARIES}
		OpenSSL::SSL
		OpenSSL::Crypto)
	if(MACOS)
		set(ZG_LIBRARIES
			${ZG_LIBRARIES}
			"-framework AudioToolbox"
			"-framework CoreMedia"
			"-framework VideoToolbox"
			"-framework Security"
			iconv)
	endif()
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
		set(ZG_LIBRARIES ${ZG_LIBRARIES}
			ws2_32 secur32 crypt32 bcrypt mfplat mf mfuuid strmiids
			advapi32 iphlpapi legacy_stdio_definitions winmm Comdlg32 Ole32)
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
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-3.0)
	if(GTKMM_FOUND)
		set(ZG_LIBRARIES ${ZG_LIBRARIES} PkgConfig::GTKMM)
	endif()
elseif(MACOS)
	set(ZG_LIBRARIES ${ZG_LIBRARIES} "-framework Cocoa" "-framework QuartzCore" "-framework Metal")
endif()
find_package(Vulkan REQUIRED)
if(WIN32)
	set(ZG_LIBRARIES ${ZG_LIBRARIES} ${Vulkan_LIBRARIES})
endif()