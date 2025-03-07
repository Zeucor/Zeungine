@echo off
setlocal enabledelayedexpansion
:: Define version and targets
set VERSION=0.2.3.5
set TARGETS=ZeungineDependencies ZeungineDependencies-debug ZeungineHeaders Zeungine Zeungine-debug
:: Iterate over targets and run them
for %%T in (%TARGETS%) do (
    set EXECUTABLE=%%T-%VERSION%.exe
    if exist !EXECUTABLE! (
        echo Running !EXECUTABLE! /S
        !EXECUTABLE! /S
    ) else (
        echo Warning: !EXECUTABLE! not found.
    )
)
endlocal