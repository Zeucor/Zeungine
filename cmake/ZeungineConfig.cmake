include(${CMAKE_CURRENT_LIST_DIR}/PlatformSetup.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Options.cmake)
include(FindPackageHandleStandardArgs)
set(Zeungine_INCLUDE_DIR ${ZG_INC_INSTALL_PREFIX_ABS})
zg_setup_target(zeungine "${ZG_LIBRARY_TYPE}"
    "${ZG_LIB_INSTALL_PREFIX_ABS}"
    "${ZG_LIB_PREFIX}" zeungine zeungine "${SHARED_ZG_LIB_SUFFIX}")
set(Zeungine_LIBRARIES ${LAST_LIBRARY_LOCATION})
find_package_handle_standard_args(Zeungine
	REQUIRED_VARS Zeungine_INCLUDE_DIR Zeungine_LIBRARIES
	VERSION_VAR Zeungine_VERSION)