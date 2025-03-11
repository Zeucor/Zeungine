#!/bin/bash

usage() {
    echo "  0 - Build all"
    echo "  1 - Build dependencies (shared and static) only"
    echo "  2 - Build headers only"
    echo "  3 - Build zeungine (shared and static) only"
    echo "  4 - Build dependencies static only"
    echo "  5 - Build dependencies shared only"
    echo "  4 - Build zeungine static only"
    echo "  5 - Build zeungine shared only"
    exit 1
}

# Check if an argument is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <mode>"
    usage
fi

MODE=$1

build_dependencies() {
    build_dependencies_static
    build_dependencies_shared
}

# Function to build static dependencies
build_dependencies_static() {
    cd cmake/Dependencies

    echo " -- Starting Zeungine Dependencies Debug/STATIC Configure"
    cmake -G Ninja -B build -D CMAKE_BUILD_TYPE=Debug -D ZG_PACKAGE=ON -D ZG_TYPE=STATIC
    echo " -- Starting Zeungine Dependencies Debug/STATIC Build"
    cmake --build build
    echo " -- Starting Zeungine Dependencies Debug/STATIC Install"
    sudo cmake --install build
    echo " -- Starting Zeungine Dependencies Debug/STATIC Package"
    sudo cpack --config build/CPackConfig.cmake -C Debug

    echo " -- Starting Zeungine Dependencies Release/STATIC Configure"
    cmake -G Ninja -B build -D CMAKE_BUILD_TYPE=Release -D ZG_PACKAGE=ON -D ZG_TYPE=STATIC
    echo " -- Starting Zeungine Dependencies Release/STATIC Build"
    cmake --build build
    echo " -- Starting Zeungine Dependencies Release/STATIC Install"
    sudo cmake --install build
    echo " -- Starting Zeungine Dependencies Release/STATIC Package"
    sudo cpack --config build/CPackConfig.cmake -C Release

    cd ../..
}

# Function to build shared dependencies
build_dependencies_shared() {
    cd cmake/Dependencies

    echo " -- Starting Zeungine Dependencies Debug/SHARED Configure"
    cmake -G Ninja -B build -D CMAKE_BUILD_TYPE=Debug -D ZG_PACKAGE=ON -D ZG_TYPE=SHARED
    echo " -- Starting Zeungine Dependencies Debug/SHARED Build"
    cmake --build build
    echo " -- Starting Zeungine Dependencies Debug/SHARED Install"
    sudo cmake --install build
    echo " -- Starting Zeungine Dependencies Debug/SHARED Package"
    sudo cpack --config build/CPackConfig.cmake -C Debug

    echo " -- Starting Zeungine Dependencies Release/SHARED Configure"
    cmake -G Ninja -B build -D CMAKE_BUILD_TYPE=Release -D ZG_PACKAGE=ON -D ZG_TYPE=SHARED
    echo " -- Starting Zeungine Dependencies Release/SHARED Build"
    cmake --build build
    echo " -- Starting Zeungine Dependencies Release/SHARED Install"
    sudo cmake --install build
    echo " -- Starting Zeungine Dependencies Release/SHARED Package"
    sudo cpack --config build/CPackConfig.cmake -C Release

    cd ../..
}

# Function to build headers
build_headers() {
    cd cmake/Headers

    echo " -- Starting zeungine Headers Configure"
    cmake -G Ninja -B build -D CMAKE_BUILD_TYPE=Release -D ZG_PACKAGE=ON
    echo " -- Starting zeungine Headers Install"
    sudo cmake --install build
    echo " -- Starting zeungine Headers Package"
    sudo cpack --config build/CPackConfig.cmake -C Release

    cd ../..
}

# Function to build zeungine
build_zeungine() {
    build_zeungine_static
    build_zeungine_shared
}

build_zeungine_static() {
    echo " -- Starting zeungine Debug/STATIC Configure"
    cmake -G Ninja -B build -D CMAKE_BUILD_TYPE=Debug -D ZG_PACKAGE=ON -D ZG_TYPE=STATIC
    echo " -- Starting zeungine Debug/STATIC Build"
    cmake --build build
    echo " -- Starting zeungine Debug/STATIC Install"
    sudo cmake --install build
    echo " -- Starting zeungine Debug/STATIC Package"
    sudo cpack --config build/CPackConfig.cmake -C Debug

    echo " -- Starting zeungine Release/STATIC Configure"
    cmake -G Ninja -B build -D CMAKE_BUILD_TYPE=Release -D ZG_PACKAGE=ON -D ZG_TYPE=STATIC
    echo " -- Starting zeungine Release/STATIC Build"
    cmake --build build
    echo " -- Starting zeungine Release/STATIC Install"
    sudo cmake --install build
    echo " -- Starting zeungine Release/STATIC Package"
    sudo cpack --config build/CPackConfig.cmake -C Release
}

build_zeungine_shared() {
    echo " -- Starting zeungine Debug/SHARED Configure"
    cmake -G Ninja -B build -D CMAKE_BUILD_TYPE=Debug -D ZG_PACKAGE=ON -D ZG_TYPE=SHARED
    echo " -- Starting zeungine Debug/SHARED Build"
    cmake --build build
    echo " -- Starting zeungine Debug/SHARED Install"
    sudo cmake --install build
    echo " -- Starting zeungine Debug/SHARED Package"
    sudo cpack --config build/CPackConfig.cmake -C Debug

    echo " -- Starting zeungine Release/SHARED Configure"
    cmake -G Ninja -B build -D CMAKE_BUILD_TYPE=Release -D ZG_PACKAGE=ON -D ZG_TYPE=SHARED
    echo " -- Starting zeungine Release/SHARED Build"
    cmake --build build
    echo " -- Starting zeungine Release/SHARED Install"
    sudo cmake --install build
    echo " -- Starting zeungine Release/SHARED Package"
    sudo cpack --config build/CPackConfig.cmake -C Release
}

# Execute based on the mode argument
case "$MODE" in
    0)
        build_dependencies
        build_headers
        build_zeungine
        ;;
    1)
        build_dependencies
        ;;
    2)
        build_headers
        ;;
    3)
        build_zeungine
        ;;
    4)
        build_dependencies_static
        ;;
    5)
        build_dependencies_shared
        ;;
    6)
        build_zeungine_static
        ;;
    7)
        build_zeungine_shared
        ;;
    *)
        echo "Invalid mode: $MODE"
        usage
        ;;
esac

# List the releases directory if everything was built
if [ "$MODE" -eq 0 ] || [ "$MODE" -eq 3 ]; then
    ls -lah releases
fi