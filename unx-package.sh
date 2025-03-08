#!/bin/bash
cd cmake/Dependencies
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Debug -DZG_PACKAGE=ON
cmake --build build
cpack --config build/CPackConfig.cmake -C Debug
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Release -DZG_PACKAGE=ON
cmake --build build
cpack --config build/CPackConfig.cmake -C Release
cd ../Headers
cmake -GNinja -B build-headers -DCMAKE_BUILD_TYPE=Release -DZG_PACKAGE=ON
cpack --config build-headers/CPackConfig.cmake -C Release
cd ../..
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Debug -DZG_PACKAGE=ON
cmake --build build
cpack --config build/CPackConfig.cmake -C Debug
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Release -DZG_PACKAGE=ON
cmake --build build
cpack --config build/CPackConfig.cmake -C Release
ls -lah releases