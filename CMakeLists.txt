cmake_minimum_required(VERSION 3.22.1)
include(cmake/InstallPrefix.cmake)
project(zeungine VERSION ${ZG_VERSION_MAJOR}.${ZG_VERSION_MINOR}.${ZG_VERSION_PATCH}.${ZG_VERSION_TWEAK})
set(CMAKE_CXX_STANDARD 20)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(cmake/Options.cmake)
include(cmake/PlatformSetup.cmake)
include(cmake/Windower.cmake)
include(cmake/Sources.cmake)
include(cmake/Targets.cmake)
include(cmake/Library.cmake)
include(cmake/Tests.cmake)
set(CPACK_COMPONENT_TESTS_DESCRIPTION "Zeungine Tests")
set(CPACK_COMPONENT_TESTS_GROUP "Zeungine")
include(cmake/Editor.cmake)
if(ZG_INSTALL)
    include(cmake/Install.cmake)
endif()
if(ZG_PACKAGE)
    include(CPackComponent)
    cpack_add_component(core)
    # cpack_add_component(zedit)
    cpack_add_component(headers)
    cpack_add_component(cmakeconfig)
    cpack_add_component(dependencies)
    cpack_add_component(tests)
    include(cmake/Package.cmake)
    crossplatform_cpack_helper(
        ${CMAKE_SOURCE_DIR}/releases
        Zeungine
        zeungine
        ${ZG_VERSION_MAJOR}
        ${ZG_VERSION_MINOR}
        ${ZG_VERSION_PATCH}
        ${ZG_VERSION_TWEAK}
        ${CMAKE_SOURCE_DIR}/LICENSE
        "Zeungine: an engine allowing developers to build cross platform games apps and tools"
        "Contains various structs and methods to help build cross platform games apps and tools quickly"
        ""
        ""
        "Zeucor"
        "Zeun"
        "Zeun"
        "libgl1-mesa-dri,libx11-6,libvulkan1,libxrandr2,libxfixes3,libxkbcommon-x11-0,libsecret-1-0,libglib2.0-dev,libdrm-dev"
        ""
        "Development/Tools"
        "Program Files\\\\Zeungine"
        "C:"
        "zeungine-uninstaller"
        "dev.zeucor.zeungine"
        "core;zedit;headers;cmakeconfig;dependencies;tests")
    # set(CPACK_NSIS_MENU_LINKS "Release/STATIC/zedit.exe" "ZEdit")
endif()
