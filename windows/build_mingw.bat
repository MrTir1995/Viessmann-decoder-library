@echo off
REM Quick build script using MinGW Make directly
REM Faster than CMake for simple builds

echo ========================================
echo Viessmann Multi-Protocol Library
echo Quick Build with MinGW
echo ========================================
echo.

REM Check if MinGW is available
where g++ >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: MinGW g++ is not found in PATH
    echo Please install MinGW-w64 from https://www.mingw-w64.org/
    echo or MSYS2 from https://www.msys2.org/
    echo.
    pause
    exit /b 1
)

where mingw32-make >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: mingw32-make is not found in PATH
    echo Please install MinGW-w64 or MSYS2
    echo.
    pause
    exit /b 1
)

echo Building with MinGW Make...
echo.

mingw32-make
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Build failed
    echo.
    pause
    exit /b 1
)

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
