@echo off
setlocal enabledelayedexpansion

:: Check if an argument is provided
if "%~1"=="" (
    echo Usage: %~nx0 ^<mode^>
    call :usage
    exit /b 1
)

set MODE=%1

:: Execute based on mode
if "%MODE%"=="0" (
    call :build_dependencies_static
    call :build_headers
    call :build_zeungine_static
    call :bundle
) else if "%MODE%"=="1" (
    call :build_dependencies_static
) else if "%MODE%"=="2" (
    call :build_headers
) else if "%MODE%"=="3" (
    call :build_zeungine_static
) else if "%MODE%"=="4" (
    call :build_dependencies_static
) else if "%MODE%"=="5" (
    call :build_dependencies_shared
) else if "%MODE%"=="6" (
    call :build_zeungine_static
) else if "%MODE%"=="7" (
    call :build_zeungine_shared
) else if "%MODE%"=="8" (
    call "C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\VC\\Auxiliary\\Build\\vcvars64.bat" x64
    call :build_dependencies_static
    call :build_headers
    call :build_zeungine_static
    call :bundle
) else (
    echo Invalid mode: %MODE%
    call :usage
    exit /b 1
)

if "%MODE%"=="0" (
    dir /a /o-s releases
) else if "%MODE%"=="3" (
    dir /a /o-s releases
)
exit /b 0

:usage
echo   0 - Build static all and bundle
echo   1 - Build dependencies static only
echo   2 - Build headers only
echo   3 - Build zeungine static only
echo   4 - Build dependencies static only
echo   5 - Build dependencies shared only
echo   6 - Build zeungine static only
echo   7 - Build zeungine shared only
echo   8 - Build all (enterprise vcvars64.bat) and bundle
goto :EOF

:bundle
cpack --config cmake/MultiCPackConfig.cmake -C Release
goto :EOF

:build_dependencies_static
cd cmake\Dependencies
echo -- Starting zegndeps Debug/STATIC Configure
cmake -B build-debug -D CMAKE_BUILD_TYPE=Debug -G Ninja -D ZG_TYPE=STATIC
echo -- Starting zegndeps Debug/STATIC Build
cmake --build build-debug --config Debug
echo -- Starting zegndeps Debug/STATIC Install
cmake --install build-debug --config Debug
echo -- Starting zegndeps Release/STATIC Configure
cmake -B build-release -D CMAKE_BUILD_TYPE=Release -G Ninja -D ZG_TYPE=STATIC
echo -- Starting zegndeps Release/STATIC Build
cmake --build build-release --config Release
echo -- Starting zegndeps Release/STATIC Install
cmake --install build-release --config Release
cd ..\..
goto :EOF

:build_dependencies_shared
cd cmake\Dependencies
echo -- Starting zegndeps Debug/SHARED Configure
cmake -B build-debug -D CMAKE_BUILD_TYPE=Debug -G Ninja -D ZG_TYPE=SHARED
echo -- Starting zegndeps Debug/SHARED Build
cmake --build build-debug --config Debug
echo -- Starting zegndeps Debug/SHARED Install
cmake --install build-debug --config Debug
echo -- Starting zegndeps Release/SHARED Configure
cmake -B build-release -D CMAKE_BUILD_TYPE=Release -G Ninja -D ZG_TYPE=SHARED
echo -- Starting zegndeps Release/SHARED Build
cmake --build build-release --config Release
echo -- Starting zegndeps Release/SHARED Install
cmake --install build-release --config Release
cd ..\..
goto :EOF

:build_headers
cd cmake\Headers
echo -- Starting zeungine Headers Configure
cmake -B build -D CMAKE_BUILD_TYPE=Release -G Ninja -D ZG_TYPE=SHARED
echo -- Starting zeungine Headers Install
cmake --install build --config Release
cd ..\..
goto :EOF

:build_zeungine_static
echo -- Starting zeungine Debug/STATIC Configure
cmake -B build-debug -D CMAKE_BUILD_TYPE=Debug -G Ninja -D ZG_TYPE=STATIC
echo -- Starting zeungine Debug/STATIC Build
cmake --build build-debug --config Debug
echo -- Starting zeungine Debug/STATIC Install
cmake --install build-debug --config Debug
echo -- Starting zeungine Release/STATIC Configure
cmake -B build-release -D CMAKE_BUILD_TYPE=Release -G Ninja -D ZG_TYPE=STATIC
echo -- Starting zeungine Release/STATIC Build
cmake --build build-release --config Release
echo -- Starting zeungine Release/STATIC Install
cmake --install build-release --config Release
goto :EOF

:build_zeungine_shared
echo -- Starting zeungine Debug/SHARED Configure
cmake -B build-debug -D CMAKE_BUILD_TYPE=Debug -G Ninja -D ZG_TYPE=SHARED
echo -- Starting zeungine Debug/SHARED Build
cmake --build build-debug --config Debug
echo -- Starting zeungine Debug/SHARED Install
cmake --install build-debug --config Debug
echo -- Starting zeungine Release/SHARED Configure
cmake -B build-release -D CMAKE_BUILD_TYPE=Release -G Ninja -D ZG_TYPE=SHARED
echo -- Starting zeungine Release/SHARED Build
cmake --build build-release --config Release
echo -- Starting zeungine Release/SHARED Install
cmake --install build-release --config Release
goto :EOF