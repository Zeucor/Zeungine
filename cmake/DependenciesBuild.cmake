
# Dependencies
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

#New Dependency Declarations to the top!

# Freetype
message(STATUS "FetchContent: freetype")
set(SKIP_INSTALL_ALL ON CACHE BOOL "Disable installation for fetched content" FORCE)
FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
    GIT_TAG master
)
FetchContent_MakeAvailable(freetype)
set_target_properties(freetype PROPERTIES DEBUG_POSTFIX "")
set_target_properties(freetype PROPERTIES RELEASE_POSTFIX "")
set_target_properties(freetype PROPERTIES RELWITHDEBINFO_POSTFIX "")
set_target_properties(freetype PROPERTIES MINSIZEREL_POSTFIX "")
include_directories(${freetype_SOURCE_DIR}/include)

# BVH
message(STATUS "FetchContent: bvh")
FetchContent_Declare(
    bvh
    GIT_REPOSITORY https://github.com/ZeunO8/bvh.git
    GIT_TAG master
)
FetchContent_MakeAvailable(bvh)
include_directories(${bvh_SOURCE_DIR}/src)

# GLM
message(STATUS "FetchContent: glm")
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG master
)
FetchContent_MakeAvailable(glm)
include_directories(${glm_SOURCE_DIR})

# STB
message(STATUS "FetchContent: stb")
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)
include_directories(${stb_SOURCE_DIR})

# lunasvg
message(STATUS "FetchContent: lunasvg")
FetchContent_Declare(
    lunasvg
    GIT_REPOSITORY https://github.com/sammycage/lunasvg.git
    GIT_TAG master
)
message(STATUS "FetchContent_MakeAvailable: lunasvg")
FetchContent_MakeAvailable(lunasvg)
include_directories(${lunasvg_SOURCE_DIR}/include)

# Includes
include_directories(include)