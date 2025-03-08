@echo off
setlocal enabledelayedexpansion
:: Define version and targets
set VERSION=0.2.3.5
set TARGETS=ZeungineDependencies-debug ZeungineDependencies ZeungineHeaders Zeungine Zeungine-debug
:: Iterate over targets and run them
for %%T in (%TARGETS%) do (
    set EXECUTABLE=%%T-%VERSION%.exe
    if exist !EXECUTABLE! (
        echo Quietly installing !EXECUTABLE! /S
        !EXECUTABLE! /S
    ) else (
        echo Installer !EXECUTABLE! not found.
    )
)
endlocal