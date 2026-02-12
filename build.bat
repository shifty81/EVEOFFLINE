@echo off
REM Atlas - Windows Build Script
REM Builds the C++ client and server using CMake

echo ================================================
echo Atlas - Build
echo ================================================
echo.

REM Check for CMake
where cmake >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake not found!
    echo Please install CMake from https://cmake.org/download/
    pause
    exit /b 1
)

set "BUILD_TYPE=Release"
if /i "%~1"=="--debug" (set "BUILD_TYPE=Debug")

REM Create build directory
if not exist build mkdir build
cd build

REM Configure
echo Configuring CMake (%BUILD_TYPE%)...
cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DUSE_SYSTEM_LIBS=ON

if errorlevel 1 (
    echo.
    echo ================================================
    echo CMAKE CONFIGURATION FAILED
    echo ================================================
    echo.
    echo Install dependencies via vcpkg:
    echo   vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows
    echo   vcpkg install imgui[glfw-binding,opengl3-binding]:x64-windows
    pause
    exit /b 1
)

REM Build
echo Building...
cmake --build . --config %BUILD_TYPE%

if errorlevel 1 (
    echo.
    echo ================================================
    echo BUILD FAILED
    echo ================================================
    pause
    exit /b 1
) else (
    echo.
    echo ================================================
    echo BUILD SUCCESSFUL
    echo ================================================
    pause
    exit /b 0
)
