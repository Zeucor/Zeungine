include(build-release/CPackConfig.cmake)

set(CPACK_INSTALL_CMAKE_PROJECTS
    "build-debug;zeungine;ALL;/"
    "build-release;zeungine;ALL;/"
    "cmake/Headers/build;zeungine-headers;ALL;/"
    "cmake/Dependencies/build-debug;zeungine-dependencies;ALL;/"
    "cmake/Dependencies/build-release;zeungine-dependencies;ALL;/"
)