# call this function once per project
function(crossplatform_cpack_helper
    OUTPUT_DIRECTORY                    # i.e. ${CMAKE_SOURCE_DIR}/releases
    DISPLAY_NAME                        # installer display name
    TARGET_NAME                         # package target name
    VERSION_MAJOR                       # M
    VERSION_MINOR                       # m
    VERSION_PATCH                       # p
    VERSION_TWEAK                       # t
    LICENSE                             # i.e. ${CMAKE_SOURCE_DIR}/LICENSE
    SUMMARY                             # a short summary of the package
    DESCRIPTION                         # a longer description of the package
    HOMEPAGE_URL                        # url to projects homepage
    PACKAGE_ICON                        # icon used for package and installer
    VENDOR                              # The vendor of the package
    CONTACT                             # contact info for the vendor
    MAINTAINER                          # the maintainer of the package
    DEB_DEPENDS                         # debian dependencies (comma separated)
    RPM_DEPENDS                         # fedora dependencies (comma separated)
    RPM_GROUP                           # fedora group (Development/Debug, Development/Languages, Development/Libraries, Development/System, Development/Tools, System Environment/Base, System Environment/Daemons, System Environment/Kernel, System Environment/Libraries, System Environment/Shells, Networking/Daemons, Networking/Utilities, Networking/Clients, Networking/Servers, User Interface/Desktops, User Interface/X, User Interface/Printing, Applications/Multimedia, Applications/Graphics, Games/Action, Games/Strategy, Games/Tools, Security/Authentication, Security/Encryption, Scientific/Mathematics, Scientific/Engineering)
    # WINDOWS_START_MENU_LINKS            # windows start menu links (pairs: binarypath/entryname)
    WINDOWS_PREFERRED_INSTALL_DIRECTORY # i.e. Program Files\\\\ProjectName
    WINDOWS_PREFERRED_INSTALL_ROOT      # i.e. C:
    WINDOWS_UNINSTALL_NAME              # i.e. ${TARGET_NAME}-uninstaller
    MACOS_BUNDLE_ID                     # i.e. com.projectorg.projectname
    COMPONENTS_ALL                      # i.e. core tool-is headers CMakeConfig dependencies
)
    set(CPACK_OUTPUT_FILE_PREFIX "${OUTPUT_DIRECTORY}")
    if(DEB)
        set(OS_LOWER "debian")
    elseif(RPM)
        set(OS_LOWER "fedora")
    elseif(WIN32)
        set(OS_LOWER "windows")
    elseif(MACOS)
        set(OS_LOWER "macos")
    endif()
    set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
    set(CPACK_PACKAGE_VERSION_TWEAK ${VERSION_TWEAK})
    set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}.${CPACK_PACKAGE_VERSION_TWEAK}")
    set(CPACK_PACKAGE_NAME "${TARGET_NAME}-v${CPACK_PACKAGE_VERSION}-${OS_LOWER}-${CMAKE_SYSTEM_PROCESSOR}")
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${SUMMARY}")
    set(CPACK_PACKAGE_DESCRIPTION "${DESCRIPTION}")
    set(CPACK_PACKAGE_HOMEPAGE_URL "${HOMEPAGE_URL}")
    set(CPACK_PACKAGE_ICON "${PACKAGE_ICON}")
    set(CPACK_PACKAGE_VENDOR "${VENDOR}")
    set(CPACK_PACKAGE_CONTACT "${CONTACT}")
    if(DEB)
        set(CPACK_GENERATOR "DEB")
        set(CPACK_PACKAGE_INSTALL_DIRECTORY "/")
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${MAINTAINER}")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${DEB_DEPENDS}")
        set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")
    elseif(RPM)
        set(CPACK_GENERATOR "RPM")
        set(CPACK_RPM_PACKAGE_DEPENDS "${RPM_DEPENDS}")
        set(CPACK_RPM_PACKAGE_LICENSE "${LICENSE}")
        set(CPACK_RPM_PACKAGE_GROUP "${RPM_GROUP}")
        set(CPACK_RPM_PACKAGE_URL "${HOMEPAGE_URL}")
        set(CPACK_RPM_PACKAGE_SUMMARY "${SUMMARY}")
        set(CPACK_PACKAGE_INSTALL_DIRECTORY "/")
    elseif(WINDOWS)
        set(CPACK_GENERATOR "NSIS")
        set(CPACK_CMAKE_GENERATOR "Ninja")
        set(CPACK_NSIS_MODIFY_PATH ON)
        set(CPACK_NSIS_DISPLAY_NAME ${DISPLAY_NAME})
        set(CPACK_NSIS_PACKAGE_NAME ${TARGET_NAME})
        set(CPACK_RESOURCE_FILE_LICENSE "${LICENSE}")
        set(CPACK_NSIS_MUI_ICON "${PACKAGE_ICON}")
        set(CPACK_NSIS_UNINSTALL_NAME "${WINDOWS_UNINSTALL_NAME}")
        set(CPACK_PACKAGE_INSTALL_DIRECTORY "${WINDOWS_PREFERRED_INSTALL_DIRECTORY}")
        set(CPACK_NSIS_INSTALL_ROOT "${WINDOWS_PREFERRED_INSTALL_ROOT}")
    elseif(MACOS)
        set(CPACK_GENERATOR "productbuild")
        set(CPACK_PRODUCTBUILD_IDENTIFIER "${MACOS_BUNDLE_ID}")
        set(CPACK_RESOURCE_FILE_LICENSE_NOPATH "${LICENSE}")
        set(CPACK_PRODUCTBUILD_DOMAINS_ROOT TRUE)
        set(CPACK_PACKAGE_INSTALL_DIRECTORY "${ZG_OSX_INSTALL_PREFIX}")
    elseif(ZUG)
        set(CPACK_PACKAGE_INSTALL_DIRECTORY "${ZG_ZUG_INSTALL_PREFIX}")
    endif()
    set(CPACK_PRODUCTBUILD_COMPONENT_INSTALL ON)
    set(CPACK_COMPONENTS_ALL ${COMPONENTS_ALL})
    set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)
    set(CPACK_MONOLITHIC_INSTALL OFF)
    include(CPack)
endfunction()