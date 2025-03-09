include(${CMAKE_CURRENT_LIST_DIR}/PlatformSetup.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Options.cmake)
include(FindPackageHandleStandardArgs)
set(Zeungine_INCLUDE_DIR ${ZG_INC_INSTALL_PREFIX_ABS})
if("${ZG_LIBRARY_TYPE}" MATCHES "SHARED")
    set(zeungine_LIB_SUFFIX ${SHARED_ZG_LIB_SUFFIX})
else()
    set(zeungine_LIB_SUFFIX ${STATIC_ZG_LIB_SUFFIX})
endif()
zg_setup_target(zeungine "${ZG_LIBRARY_TYPE}"
    "${ZG_LIB_INSTALL_PREFIX_ABS}"
    "${ZG_LIB_PREFIX}" zeungine zeungine "${zeungine_LIB_SUFFIX}")
include(${CMAKE_CURRENT_LIST_DIR}/Targets.cmake)
set(Zeungine_LIBRARIES "${ZG_LIBRARIES}")
find_package_handle_standard_args(Zeungine
	REQUIRED_VARS Zeungine_INCLUDE_DIR Zeungine_LIBRARIES
	VERSION_VAR Zeungine_VERSION)