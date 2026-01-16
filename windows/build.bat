@echo off
REM Build script for Viessmann Multi-Protocol Library (Windows)
REM This script builds the library and example using CMake

echo ========================================
echo Viessmann Multi-Protocol Library
echo Windows Build Script (CMake)
echo ========================================
echo.

REM Check if CMake is available
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake is not found in PATH
    echo Please install CMake from https://cmake.org/download/
    echo.
    pause
    exit /b 1
)

REM Check for build directory
if exist build (
    echo Cleaning previous build...
    rmdir /S /Q build
)

REM Create build directory
echo Creating build directory...
mkdir build
cd build

REM Detect Visual Studio or MinGW
echo.
echo Detecting compiler...
echo.

REM Try to find Visual Studio
where cl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Found Visual Studio compiler
    echo Configuring with Visual Studio...
    cmake .. -G "NMake Makefiles"
    if %ERRORLEVEL% NEQ 0 (
        echo ERROR: CMake configuration failed
        cd ..
        pause
        exit /b 1
    )
    
    echo.
    echo Building with NMake...
    nmake
    if %ERRORLEVEL% NEQ 0 (
        echo ERROR: Build failed
        cd ..
        pause
        exit /b 1
    )
    goto success
)

REM Try MinGW
where g++ >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Found MinGW compiler
    echo Configuring with MinGW...
    cmake .. -G "MinGW Makefiles"
    if %ERRORLEVEL% NEQ 0 (
        echo ERROR: CMake configuration failed
        cd ..
        pause
        exit /b 1
    )
    
    echo.
    echo Building with MinGW Make...
    mingw32-make
    if %ERRORLEVEL% NEQ 0 (
        echo ERROR: Build failed
        cd ..
        pause
        exit /b 1
    )
    goto success
)

echo ERROR: No suitable compiler found
echo Please install either:
echo   - Visual Studio 2017 or later with C++ support
echo   - MinGW-w64 from https://www.mingw-w64.org/
echo.
cd ..
pause
exit /b 1

:success
cd ..
echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo.
echo Output files:
echo   - Library: build\lib\
echo   - Example: build\bin\vbusdecoder_windows.exe
echo.
echo To run the example:
echo   build\bin\vbusdecoder_windows.exe -h
echo.
pause
