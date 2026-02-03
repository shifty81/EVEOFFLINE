@echo off
REM EVE OFFLINE - Windows Build and Test Script
REM Quick access script for Windows users

echo ================================================
echo EVE OFFLINE - Build and Test
echo ================================================
echo.

REM Check if Python is available
python --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Python not found!
    echo Please install Python 3.11 or higher
    pause
    exit /b 1
)

REM Run the build script
python build_and_test.py %*

REM Check result
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
