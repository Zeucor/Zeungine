# todo make function

set(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_SOURCE_DIR}/releases")
if(LINUX OR MACOS)
    set(CPACK_PACKAGE_NAME "zeungine${BUILD_POSTFIX}")
elseif(WINDOWS)
    set(CPACK_PACKAGE_NAME "Zeungine${BUILD_POSTFIX}")
endif()
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Zeungine: an engine allowing developers to build cross platform games apps and tools")
set(CPACK_PACKAGE_DESCRIPTION "Contains various structs and methods to help build cross platform games apps and tools quickly")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://")
if(LINUX OR MACOS)
    # set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/images/zeungine.dev-app-icon.bmp")
elseif(WINDOWS)
    # set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}\\\\images\\\\zeungine.dev-app-icon.bmp")
endif()
set(CPACK_PACKAGE_VENDOR "")
set(CPACK_PACKAGE_CONTACT "")

if(DEB)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Zeun")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "zeungine-dependencies${BUILD_POSTFIX},zeungine-headers")
elseif(WINDOWS)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_CMAKE_GENERATOR "Ninja")
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_MENU_LINKS "https://" " *.****:****|Web Site")
    set(CPACK_NSIS_DISPLAY_NAME Zeungine)
    set(CPACK_NSIS_PACKAGE_NAME Zeungine)
    SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Program Files\\\\Zeungine")
    SET(CPACK_NSIS_INSTALL_ROOT "C:")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
    # set(CPACK_NSIS_MUI_ICON "${Zeungine_SRC_DIR}\\\\images\\\\zeungine-logo-new.ico")
    set(CPACK_NSIS_UNINSTALL_NAME "Zeungine_Uninstaller")
    set(CPACK_NSIS_BRANDING_TEXT " ")
endif()

set(CPACK_PACKAGE_VERSION_MAJOR ${ZG_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${ZG_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${ZG_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION_TWEAK ${ZG_VERSION_TWEAK})
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}.${CPACK_PACKAGE_VERSION_TWEAK}")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}")
include(CPack)