@echo off
cd cmake\Dependencies
:: Debug Build
cmake -G "Ninja" -B build -DCMAKE_BUILD_TYPE=Debug -DZG_PACKAGE=ON
cmake --build build
cpack --config build\CPackConfig.cmake -C Debug
:: Release Build
cmake -G "Ninja" -B build -DCMAKE_BUILD_TYPE=Release -DZG_PACKAGE=ON
cmake --build build
cpack --config build\CPackConfig.cmake -C Release
cd ..\Headers
:: Release Build
cmake -G "Ninja" -B build-headers -DCMAKE_BUILD_TYPE=Release -DZG_PACKAGE=ON
cpack --config build-headers\CPackConfig.cmake -C Release
cd ..\..
:: Debug Build
cmake -G "Ninja" -B build -DCMAKE_BUILD_TYPE=Debug -DZG_PACKAGE=ON
cmake --build build
cpack --config build\CPackConfig.cmake -C Debug
:: Release Build
cmake -G "Ninja" -B build -DCMAKE_BUILD_TYPE=Release -DZG_PACKAGE=ON
cmake --build build
cpack --config build\CPackConfig.cmake -C Release
:: List files in releases directory
dir /a /o-s releases