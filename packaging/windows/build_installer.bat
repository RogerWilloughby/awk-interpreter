@echo off
REM AWK Interpreter Windows Installer Build Script
REM Requires: NSIS (https://nsis.sourceforge.io/) or WiX Toolset

setlocal enabledelayedexpansion

set VERSION=1.0.0
set BUILD_DIR=..\..\build
set SCRIPT_DIR=%~dp0

echo ========================================
echo AWK Interpreter Installer Builder
echo Version: %VERSION%
echo ========================================

REM Check if build exists
if not exist "%BUILD_DIR%\bin\awk.exe" (
    echo ERROR: Build not found. Please build the project first.
    echo Run: cmake -B build -DCMAKE_BUILD_TYPE=Release
    echo      cmake --build build --config Release
    exit /b 1
)

REM Method 1: Using CPack (recommended)
echo.
echo Building with CPack...
pushd %BUILD_DIR%
cpack -G NSIS -C Release
cpack -G ZIP -C Release
if exist "*.msi" del *.msi
cpack -G WIX -C Release 2>nul
popd

REM Method 2: Using custom NSIS script
if exist "%PROGRAMFILES(X86)%\NSIS\makensis.exe" (
    echo.
    echo Building with custom NSIS script...
    "%PROGRAMFILES(X86)%\NSIS\makensis.exe" /DVERSION=%VERSION% installer.nsi
) else (
    echo.
    echo NSIS not found. Skipping custom installer.
    echo Install NSIS from: https://nsis.sourceforge.io/
)

echo.
echo ========================================
echo Build complete!
echo.
echo Installers created:
dir /b %BUILD_DIR%\*.exe %BUILD_DIR%\*.zip %BUILD_DIR%\*.msi 2>nul
echo ========================================

endlocal
