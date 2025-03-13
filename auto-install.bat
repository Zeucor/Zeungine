@echo off
setlocal enabledelayedexpansion
:: Define version and targets
set TARGETS=ZeunDependencies-debug-static  ZeungineDependencies-static ZeungineDependencies-debug ZeungineDependencies ZeungineHeaders Zeungine Zeungine-debug Zeungine-static Zeungine-debug-static
:: Iterate over targets and run them
for %%T in (%TARGETS%) do (
    set EXECUTABLE=%%T.exe
    if exist !EXECUTABLE! (
        echo Installing !EXECUTABLE! /S
        !EXECUTABLE! /S
    ) else (
        echo Installer !EXECUTABLE! not found.
    )
)
endlocal