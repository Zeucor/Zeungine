@echo off
setlocal enabledelayedexpansion

:: Check if an argument is provided
if "%~1"=="" (
    echo Usage: %~nx0 ^<mode^>
    echo   0 - Build all
    echo   1 - Build dependencies only
    echo   2 - Build headers only
    echo   3 - Build zeungine only
    exit /b 1
)

set MODE=%1

:: Execute based on mode
if "%MODE%"=="0" (
    call :build_dependencies
    call :build_headers
    call :build_zeungine
) else if "%MODE%"=="1" (
    call :build_dependencies
) else if "%MODE%"=="2" (
    call :build_headers
) else if "%MODE%"=="3" (
    call :build_zeungine
) else (
    echo Invalid mode: %MODE%
    echo   0 - Build all
    echo   1 - Build dependencies only
    echo   2 - Build headers only
    echo   3 - Build zeungine only
    exit /b 1
)

exit /b 0

:: Function to build dependencies
:build_dependencies
cd cmake\Dependencies

echo -- Starting zegndeps Debug Configure
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DZG_PACKAGE=ON
echo -- Starting zegndeps Debug Build
cmake --build build
echo -- Starting zegndeps Debug Install
cmake --install build
echo -- Starting zegndeps Debug Package
cpack --config build\CPackConfig.cmake -C Debug

echo -- Starting zegndeps Release Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release -DZG_PACKAGE=ON
echo -- Starting zegndeps Release Build
cmake --build build
echo -- Starting zegndeps Release Install
cmake --install build
echo -- Starting zegndeps Release Package
cpack --config build\CPackConfig.cmake -C Release

cd ..\..
goto :EOF

:: Function to build headers
:build_headers
cd cmake\Headers

echo -- Starting zeungine Headers Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release -DZG_PACKAGE=ON
echo -- Starting zeungine Headers Install
cmake --install build
echo -- Starting zeungine Headers Package
cpack --config build\CPackConfig.cmake -C Release

cd ..\..
goto :EOF

:: Function to build zeungine
:build_zeungine
echo -- Starting zeungine Debug Configure
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DZG_PACKAGE=ON
echo -- Starting zeungine Debug Build
cmake --build build
echo -- Starting zeungine Debug Install
cmake --install build
echo -- Starting zeungine Debug Package
cpack --config build\CPackConfig.cmake -C Debug

echo -- Starting zeungine Release Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release -DZG_PACKAGE=ON
echo -- Starting zeungine Release Build
cmake --build build
echo -- Starting zeungine Release Install
cmake --install build
echo -- Starting zeungine Release Package
cpack --config build\CPackConfig.cmake -C Release

:: List releases if built
if "%MODE%"=="0" (
    dir /a /o-s releases
) else if "%MODE%"=="3" (
    dir /a /o-s releases
)

goto :EOF
