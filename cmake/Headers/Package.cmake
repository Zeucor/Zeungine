include(../Package.cmake)
crossplatform_cpack_helper(
    ${CMAKE_SOURCE_DIR}/../../releases               # OUTPUT_DIRECTORY
    "zeungine-headers"                               # DISPLAY_NAME
    "zeungine-headers"                               # TARGET_NAME
    ${ZG_VERSION_MAJOR}                              # M
    ${ZG_VERSION_MINOR}                              # m
    ${ZG_VERSION_PATCH}                              # p
    ${ZG_VERSION_TWEAK}                              # t
    ${CMAKE_SOURCE_DIR}/../Dependencies/DEPS_LICENSE # LICENSE
    ""                                               # SUMMARY
    ""                                               # DESCRIPTION
    ""                                               # HOMEPAGE_URL
    ""                                               # PACKAGE_ICON
    "Zeucor"                                         # VENDOR
    "Zeun"                                           # CONTACT
    "Zeun"                                           # MAINTAINER
    ""                                               # DEB_DEPENDS
    ""                                               # RPM_DEPENDS
    "Development/Libraries"                          # RPM_GROUP                           # fedora group (Development/Debug, Development/Languages, Development/Libraries, Development/System, Development/Tools, System Environment/Base, System Environment/Daemons, System Environment/Kernel, System Environment/Libraries, System Environment/Shells, Networking/Daemons, Networking/Utilities, Networking/Clients, Networking/Servers, User Interface/Desktops, User Interface/X, User Interface/Printing, Applications/Multimedia, Applications/Graphics, Games/Action, Games/Strategy, Games/Tools, Security/Authentication, Security/Encryption, Scientific/Mathematics, Scientific/Engineering)
    "Program Files\\\\Zeungine"                      # WINDOWS_PREFERRED_INSTALL_DIRECTORY
    "C:"                                             # WINDOWS_PREFERRED_INSTALL_ROOT
    "zeungine-headers-uninstaller"                   # WINDOWS_UNINSTALL_NAME
    ""                                               # MACOS_BUNDLE_ID
    "headers cmakeconfig"
)