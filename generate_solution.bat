@echo off
REM EVE OFFLINE - Generate Root-Level Visual Studio Solution
REM This script creates a solution that includes both C++ client and server

echo ================================================
echo EVE OFFLINE - Visual Studio Solution Generator
echo ================================================
echo.

REM Check for CMake
where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake not found!
    echo Please install CMake from https://cmake.org/download/
    pause
    exit /b 1
)

REM Parse command line arguments
set "BUILD_TYPE=Release"
set "CLEAN_BUILD=0"
set "OPEN_VS=0"

:parse_args
if "%~1"=="" goto end_parse
if /i "%~1"=="--debug" (set "BUILD_TYPE=Debug")
if /i "%~1"=="--release" (set "BUILD_TYPE=Release")
if /i "%~1"=="--clean" (set "CLEAN_BUILD=1")
if /i "%~1"=="--open" (set "OPEN_VS=1")
shift
goto parse_args
:end_parse

echo Build Type: %BUILD_TYPE%
echo.

REM Clean build if requested
if %CLEAN_BUILD% EQU 1 (
    echo Cleaning previous builds...
    if exist build_vs rmdir /s /q build_vs
    echo.
)

REM Create build directory
if not exist build_vs mkdir build_vs
cd build_vs

REM Configure CMake for Visual Studio
echo Configuring root solution with CMake...
echo.

REM Check for vcpkg and set toolchain file if found
set "VCPKG_TOOLCHAIN="
set "VCPKG_FOUND=0"

REM Check common vcpkg locations
REM Note: Using backslashes for Windows file checks, forward slashes for CMake (CMake prefers forward slashes)
if exist "C:\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set "VCPKG_TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
    set "VCPKG_FOUND=1"
    echo Found vcpkg at C:\vcpkg
    echo.
) else if exist "%USERPROFILE%\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set "VCPKG_TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=%USERPROFILE%/vcpkg/scripts/buildsystems/vcpkg.cmake"
    set "VCPKG_FOUND=1"
    echo Found vcpkg at %USERPROFILE%\vcpkg
    echo.
) else if defined VCPKG_ROOT (
    if exist "%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" (
        set "VCPKG_TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake"
        set "VCPKG_FOUND=1"
        echo Found vcpkg at %VCPKG_ROOT%
        echo.
    )
)

if %VCPKG_FOUND% EQU 0 (
    echo WARNING: vcpkg not found. Dependencies ^(GLEW, GLFW, GLM^) must be installed manually.
    echo          Or install vcpkg and dependencies first - see docs/guides/VS2022_SETUP_GUIDE.md
    echo.
)

REM Always clean CMake cache before configuring to avoid generator mismatch errors
if exist CMakeCache.txt (
    echo Cleaning old CMake cache to avoid generator conflicts...
    del /f CMakeCache.txt
    if exist CMakeFiles rmdir /s /q CMakeFiles
    echo.
)

REM Detect which Visual Studio generator to use
set "VS_GENERATOR="

REM Check VS 2022 first
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" (set "VS_GENERATOR=Visual Studio 17 2022")
if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" (set "VS_GENERATOR=Visual Studio 17 2022")
if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" (set "VS_GENERATOR=Visual Studio 17 2022")
if exist "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (set "VS_GENERATOR=Visual Studio 17 2022")

REM If VS 2022 not found, check VS 2019
if "%VS_GENERATOR%"=="" (
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" (set "VS_GENERATOR=Visual Studio 16 2019")
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" (set "VS_GENERATOR=Visual Studio 16 2019")
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" (set "VS_GENERATOR=Visual Studio 16 2019")
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (set "VS_GENERATOR=Visual Studio 16 2019")
)

if "%VS_GENERATOR%"=="" (
    echo ERROR: Could not detect Visual Studio installation.
    echo Please install Visual Studio 2019 or 2022 with "Desktop development with C++" workload.
    pause
    exit /b 1
)

echo Using generator: %VS_GENERATOR%
echo.

cmake .. ^
    -G "%VS_GENERATOR%" ^
    -A x64 ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    %VCPKG_TOOLCHAIN%

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: CMake configuration failed!
    echo.
    echo Generator used: %VS_GENERATOR%
    echo.
    echo Most likely cause: Missing dependencies.
    echo.
    echo SOLUTION: Install ALL required dependencies using vcpkg:
    echo.
    echo   Step 1 - Install vcpkg ^(if not already installed^):
    echo      cd C:\
    echo      git clone https://github.com/microsoft/vcpkg.git
    echo      cd vcpkg
    echo      .\bootstrap-vcpkg.bat
    echo.
    echo   Step 2 - Install required dependencies:
    echo      .\vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows

    echo.
    echo   Step 3 - Optional audio support:
    echo      .\vcpkg install openal-soft:x64-windows
    echo.
    echo   Step 4 - Run this script again
    echo.
    echo For more information, see: docs/guides/VS2022_SETUP_GUIDE.md
    echo.
    pause
    exit /b 1
)

echo.
echo CMake configuration successful!
echo.

REM Open Visual Studio if requested
if %OPEN_VS% EQU 1 (
    echo Opening Visual Studio...
    for %%f in (*.sln) do (
        start "" "%%f"
        goto :found
    )
    echo Warning: No solution file found
    :found
)

echo.
echo ================================================
echo Solution Generated Successfully!
echo ================================================
echo.
echo Solution file location: build_vs\EVEOffline.sln
echo.
echo To open in Visual Studio:
echo   start build_vs\EVEOffline.sln
echo.
echo Or use individual component solutions:
echo   cpp_client\build_vs\EVEOfflineClient.sln
echo   cpp_server\build\EVEOfflineDedicatedServer.sln
echo.

pause
