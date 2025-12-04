@echo off
REM Build script for AWK Interpreter (Windows)
REM Usage: scripts\build_all.bat [Debug|Release]

setlocal enabledelayedexpansion

set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

echo ========================================
echo AWK Interpreter Build Script
echo Build type: %BUILD_TYPE%
echo ========================================

cd /d "%~dp0\.."

REM Clean previous build
if exist build (
    echo Cleaning previous build...
    rmdir /s /q build
)

REM Configure
echo.
echo Configuring...
cmake -B build ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DBUILD_TESTS=ON ^
    -DAWK_ENABLE_LTO=ON

if errorlevel 1 (
    echo Configuration failed!
    exit /b 1
)

REM Build
echo.
echo Building...
cmake --build build --config %BUILD_TYPE%

if errorlevel 1 (
    echo Build failed!
    exit /b 1
)

REM Run tests
echo.
echo Running unit tests...
build\bin\%BUILD_TYPE%\awk_tests.exe

if errorlevel 1 (
    echo Unit tests failed!
    exit /b 1
)

REM Run integration tests
echo.
echo Running integration tests...
cd integration_tests
call run_tests.bat
cd ..

echo.
echo ========================================
echo Build complete!
echo.
echo Executable: build\bin\%BUILD_TYPE%\awk.exe
echo Library: build\lib\%BUILD_TYPE%\awk.lib
echo ========================================

endlocal
