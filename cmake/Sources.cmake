# OS Sources

if(MACOS)
    set(OS_SRC macos)
elseif(LINUX)
    set(OS_SRC linux)
elseif(ANDROID OR IOS)
    set(OS_SRC egl)
elseif(WIN32)
    set(OS_SRC windows)
elseif(ZUG)
    set(OS_SRC coj)
endif()

# ZG Sources

file(GLOB_RECURSE ZG_SOURCES "src/*.c" "src/*.cpp" "os_src/${OS_SRC}/*.cpp" "os_src/${OS_SRC}/*.c" "os_src/${OS_SRC}/*.mm")
set(ZG_SOURCES ${ZG_SOURCES})
include(FetchContent)
set(ZG_SOURCES  ${ZG_SOURCES})