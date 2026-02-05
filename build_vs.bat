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

REM Check for Visual Studio
where msbuild >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Visual Studio not found!
    echo Please install Visual Studio 2019 or later with C++ desktop development
    pause
    exit /b 1
)

REM Parse command line arguments
set BUILD_TYPE=Release
set CLEAN_BUILD=0
set OPEN_VS=0

:parse_args
if "%~1"=="" goto end_parse
if /i "%~1"=="--debug" set BUILD_TYPE=Debug
if /i "%~1"=="--release" set BUILD_TYPE=Release
if /i "%~1"=="--clean" set CLEAN_BUILD=1
if /i "%~1"=="--open" set OPEN_VS=1
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

cmake .. ^
    -G "Visual Studio 17 2022" ^
    -A x64 ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DUSE_SYSTEM_LIBS=ON ^
    -DBUILD_TESTS=ON

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: CMake configuration failed!
    echo.
    echo Possible issues:
    echo   1. Missing dependencies - run: vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows openal-soft:x64-windows
    echo   2. Visual Studio version mismatch - try using "Visual Studio 16 2019" generator
    echo   3. Missing Visual Studio C++ tools
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
