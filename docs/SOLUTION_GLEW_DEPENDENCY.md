# Solution: GLEW Dependency Error

## Problem
Users were encountering a CMake configuration error when running `generate_solution.bat` or `build_vs.bat`:

```
CMake Error at C:/Program Files/CMake/share/cmake-4.2/Modules/FindPackageHandleStandardArgs.cmake:290 (message):
  Could NOT find GLEW (missing: GLEW_INCLUDE_DIRS GLEW_LIBRARIES)
```

Users were confused about:
1. Whether to install dependencies first or run the build script first
2. How to install the dependencies
3. Where the error was coming from

## Root Cause
The C++ client requires several external libraries:
- GLFW (window/input management)
- GLM (math library)
- GLEW (OpenGL extension loader)
- nlohmann/json (JSON parsing)

These libraries are not included in the repository (the `cpp_client/external/` directory is gitignored). The CMakeLists.txt was configured with `USE_SYSTEM_LIBS=ON` by default, which means it expects these libraries to be installed on the system via vcpkg.

However, the build scripts did not:
1. Automatically detect and use vcpkg
2. Provide clear error messages when dependencies were missing
3. Guide users on the correct installation order

## Solution Implemented

### 1. Enhanced CMakeLists.txt
- Changed GLEW from `REQUIRED` to `QUIET` with proper error handling
- Added detailed warning messages when GLEW is not found
- Made the error message clearly explain how to install dependencies via vcpkg
- Added conditional linking for GLEW (only link if found)

### 2. Improved build_vs.bat
- Added automatic detection of vcpkg in multiple common locations:
  - `C:\vcpkg`
  - `%USERPROFILE%\vcpkg`
  - `%VCPKG_ROOT%` (environment variable)
- Automatically passes the vcpkg toolchain file to CMake if found
- Enhanced error messages with step-by-step vcpkg installation instructions
- Added warning when vcpkg is not detected

### 3. Improved generate_solution.bat
- Same vcpkg detection logic as build_vs.bat
- Enhanced error messages with installation instructions
- Clearer guidance on what to do when CMake fails

### 4. Updated Documentation
- **README.md**: Added prominent warning to install dependencies FIRST
- **QUICKSTART_VS2022.md**: Reorganized to show dependency installation as step 1
- Added "Could NOT find GLEW" to the common issues list
- Emphasized the importance of dependency installation order

## What Users Should Do Now

### New Users (First Time Setup)
1. Install vcpkg:
   ```cmd
   cd C:\
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   ```

2. Install dependencies:
   ```cmd
   .\vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows
   ```

3. Run the build script:
   ```cmd
   cd C:\path\to\EVEOFFLINE
   build_vs.bat
   ```

The build script will now automatically detect vcpkg and use it!

### Existing Users Experiencing the Error
If you already have vcpkg installed at `C:\vcpkg`:
```cmd
cd C:\vcpkg
.\vcpkg install glew:x64-windows glfw3:x64-windows glm:x64-windows nlohmann-json:x64-windows
cd C:\path\to\EVEOFFLINE
build_vs.bat
```

The script will automatically find and use your vcpkg installation.

## Technical Details

### Vcpkg Auto-Detection
The scripts now check these locations in order:
1. `C:\vcpkg\scripts\buildsystems\vcpkg.cmake`
2. `%USERPROFILE%\vcpkg\scripts\buildsystems\vcpkg.cmake`
3. `%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake`

If found, the script automatically adds:
```cmd
-DCMAKE_TOOLCHAIN_FILE=<vcpkg-path>/scripts/buildsystems/vcpkg.cmake
```

### CMake Error Handling
When GLEW is not found, CMake now:
1. Shows a WARNING message (not just silently failing)
2. Provides the exact vcpkg command to run
3. Explains how to configure CMake with the toolchain file
4. Calls FATAL_ERROR with a clear message

### Backwards Compatibility
The changes maintain full backwards compatibility:
- If dependencies are already installed, everything works as before
- If vcpkg is at `C:\vcpkg`, it's automatically detected
- Manual CMake configuration still works for advanced users

## Files Modified
1. `cpp_client/CMakeLists.txt` - Enhanced error handling and conditional GLEW linking
2. `build_vs.bat` - Added vcpkg auto-detection and better error messages
3. `generate_solution.bat` - Added vcpkg auto-detection and better error messages
4. `README.md` - Emphasized dependency installation order
5. `QUICKSTART_VS2022.md` - Reorganized to show correct order

## Testing
The changes have been tested to ensure:
- ✅ Clear error messages appear when GLEW is missing
- ✅ vcpkg is automatically detected when present
- ✅ CMake toolchain file is automatically passed to CMake
- ✅ Build succeeds when dependencies are installed
- ✅ Error messages provide actionable guidance

## References
- [VS2022_SETUP_GUIDE.md](VS2022_SETUP_GUIDE.md) - Complete setup guide
- [TROUBLESHOOTING_VS2022.md](TROUBLESHOOTING_VS2022.md) - Troubleshooting guide
- [QUICKSTART_VS2022.md](QUICKSTART_VS2022.md) - Quick reference
