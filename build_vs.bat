@echo off
REM EVE OFFLINE C++ Client - Visual Studio Build Script
REM Generates Visual Studio solution and builds the project

echo ================================================
echo EVE OFFLINE C++ Client - Visual Studio Build
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

REM Check for Visual Studio (VS 2022, 2019, or 2017)
set "VS_FOUND=0"

REM Check VS 2022
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" (set "VS_FOUND=1")
if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" (set "VS_FOUND=1")
if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" (set "VS_FOUND=1")

REM Check VS 2019
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" (set "VS_FOUND=1")
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" (set "VS_FOUND=1")
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" (set "VS_FOUND=1")

REM Check VS 2017
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" (set "VS_FOUND=1")
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvarsall.bat" (set "VS_FOUND=1")
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" (set "VS_FOUND=1")

REM Also check if msbuild is in PATH
where msbuild >nul 2>&1
if %ERRORLEVEL% EQU 0 (set "VS_FOUND=1")

if %VS_FOUND% EQU 0 (
    echo ERROR: Visual Studio not found!
    echo Please install Visual Studio 2017/2019/2022 with C++ desktop development
    echo.
    echo Installation paths checked:
    echo   - C:\Program Files\Microsoft Visual Studio\2022\
    echo   - C:\Program Files ^(x86^)\Microsoft Visual Studio\2019\
    echo   - C:\Program Files ^(x86^)\Microsoft Visual Studio\2017\
    echo.
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

REM Navigate to cpp_client directory
cd /d "%~dp0cpp_client"

REM Clean build if requested
if %CLEAN_BUILD% EQU 1 (
    echo Cleaning previous build...
    if exist build_vs rmdir /s /q build_vs
    echo.
)

REM Create build directory
if not exist build_vs mkdir build_vs
cd build_vs

REM Configure CMake for Visual Studio
echo Configuring CMake for Visual Studio...
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
REM (e.g., switching between VS 2022 and VS 2019 in the same build directory)
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
    -DUSE_SYSTEM_LIBS=ON ^
    -DBUILD_TESTS=ON ^
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
    echo CMake version:
    cmake --version
    echo.
    pause
    exit /b 1
)

echo.
echo CMake configuration successful!
echo.

REM Build the project
echo Building project...
echo.

cmake --build . --config %BUILD_TYPE% -- /m

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo ================================================
echo BUILD SUCCESSFUL
echo ================================================
echo.
echo Executable location: build_vs\bin\%BUILD_TYPE%\eve_client.exe
echo.

REM Open Visual Studio if requested
if %OPEN_VS% EQU 1 (
    echo Opening Visual Studio...
    start EVEOfflineClient.sln
)

REM List built executables
echo Built files:
dir /b bin\%BUILD_TYPE%\*.exe 2>nul

echo.
echo To open the project in Visual Studio, run:
echo   build_vs\EVEOfflineClient.sln
echo.
echo Or rebuild using this script with:
echo   build_vs.bat --clean          # Clean rebuild
echo   build_vs.bat --debug          # Debug build
echo   build_vs.bat --open           # Open in Visual Studio after build
echo.

pause
