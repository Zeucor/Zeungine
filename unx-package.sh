#!/bin/bash

# Check if an argument is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <mode>"
    echo "  0 - Build all"
    echo "  1 - Build dependencies only"
    echo "  2 - Build headers only"
    echo "  3 - Build zeungine only"
    exit 1
fi

MODE=$1

# Function to build dependencies
build_dependencies() {
    cd cmake/Dependencies || exit

    echo " -- Starting zegndeps Debug Configure"
    cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug -DZG_PACKAGE=ON
    echo " -- Starting zegndeps Debug Build"
    cmake --build build
    echo " -- Starting zegndeps Debug Install"
    sudo cmake --install build
    echo " -- Starting zegndeps Debug Package"
    sudo cpack --config build/CPackConfig.cmake -C Debug

    echo " -- Starting zegndeps Release Configure"
    cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release -DZG_PACKAGE=ON
    echo " -- Starting zegndeps Release Build"
    cmake --build build
    echo " -- Starting zegndeps Release Install"
    sudo cmake --install build
    echo " -- Starting zegndeps Release Package"
    sudo cpack --config build/CPackConfig.cmake -C Release

    cd ../..
}

# Function to build headers
build_headers() {
    cd cmake/Headers || exit

    echo " -- Starting zeungine Headers Configure"
    cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release -DZG_PACKAGE=ON
    echo " -- Starting zeungine Headers Install"
    sudo cmake --install build
    echo " -- Starting zeungine Headers Package"
    sudo cpack --config build/CPackConfig.cmake -C Release

    cd ../..
}

# Function to build zeungine
build_zeungine() {
    echo " -- Starting zeungine Debug Configure"
    cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug -DZG_PACKAGE=ON
    echo " -- Starting zeungine Debug Build"
    cmake --build build
    echo " -- Starting zeungine Debug Install"
    sudo cmake --install build
    echo " -- Starting zeungine Debug Package"
    sudo cpack --config build/CPackConfig.cmake -C Debug

    echo " -- Starting zeungine Release Configure"
    cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release -DZG_PACKAGE=ON
    echo " -- Starting zeungine Release Build"
    cmake --build build
    echo " -- Starting zeungine Release Install"
    sudo cmake --install build
    echo " -- Starting zeungine Release Package"
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
    *)
        echo "Invalid mode: $MODE"
        echo "  0 - Build all"
        echo "  1 - Build dependencies only"
        echo "  2 - Build headers only"
        echo "  3 - Build zeungine only"
        exit 1
        ;;
esac

# List the releases directory if everything was built
if [ "$MODE" -eq 0 ] || [ "$MODE" -eq 3 ]; then
    ls -lah releases
fi