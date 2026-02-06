# VS2022 Build Troubleshooting Guide

This document provides solutions to common issues when building EVE OFFLINE in Visual Studio 2022.

## Table of Contents
- [Script Errors](#script-errors)
- [CMake Issues](#cmake-issues)
- [Dependency Problems](#dependency-problems)
- [Build Failures](#build-failures)
- [Runtime Issues](#runtime-issues)
- [Performance Issues](#performance-issues)

---

## Script Errors

### "\Microsoft was unexpected at this time"

**Symptoms**: Running `build_vs.bat` fails immediately with this error.

**Cause**: Batch file syntax issue with paths containing parentheses (e.g., "Program Files (x86)").

**Solution**: This has been fixed in the latest version. Update your files:
```cmd
git pull origin main
```

If still experiencing issues, the problem might be with your local modifications. Verify the batch file has proper syntax:
- All `set` commands should use quotes: `set "VAR=value"`
- IF statements with SET should use parentheses: `if condition (set "VAR=value")`

---

### "The system cannot find the path specified"

**Symptoms**: Script runs but fails to find cpp_client directory.

**Cause**: Script is being run from the wrong directory.

**Solution**: Always run from the repository root:
```cmd
cd C:\path\to\EVEOFFLINE
.\build_vs.bat
```

---

## CMake Issues

### "CMake not found" or "cmake is not recognized"

**Symptoms**: Script reports CMake is not installed.

**Solution 1 - Add VS CMake to PATH**:
```cmd
set PATH=%PATH%;C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin
```

**Solution 2 - Install standalone CMake**:
1. Download from https://cmake.org/download/
2. During installation, select "Add CMake to system PATH"
3. Restart command prompt

**Solution 3 - Install via Visual Studio Installer**:
1. Open Visual Studio Installer
2. Modify VS 2022
3. Check "C++ CMake tools for Windows"
4. Apply changes

---

### "Could NOT find Visual Studio generator"

**Symptoms**: CMake can't find VS 2022 generator.

**Solution**: Ensure VS 2022 is properly installed:
1. Open Visual Studio Installer
2. Verify "Desktop development with C++" is installed
3. Check that MSVC v143 build tools are included

If VS 2022 is installed, try specifying the generator manually:
```cmd
cd cpp_client\build_vs
cmake .. -G "Visual Studio 17 2022" -A x64
```

---

### "Does not match the generator used previously"

**Symptoms**: CMake fails when falling back to VS 2019 with error:
```
Error: generator : Visual Studio 16 2019
Does not match the generator used previously: Visual Studio 17 2022
Either remove the CMakeCache.txt file and CMakeFiles directory or choose a different binary directory.
```

**Cause**: CMake cache from a previous run with a different generator wasn't cleaned.

**Solution**: This has been fixed in the latest version. The build scripts now automatically clean the CMake cache before switching generators. Update your files:
```cmd
git pull origin main
```

If you need to manually fix it:
```cmd
cd build_vs
del /f CMakeCache.txt
rmdir /s /q CMakeFiles
cd ..
.\generate_solution.bat
```

---

### "The C compiler identification is unknown"

**Symptoms**: CMake fails to detect the C/C++ compiler.

**Solution**:
1. Open Visual Studio Installer
2. Modify your VS 2022 installation
3. Ensure these are checked:
   - MSVC v143 - VS 2022 C++ x64/x86 build tools
   - Windows 10 SDK or Windows 11 SDK
4. Apply and restart command prompt

---

## Dependency Problems

### "Could NOT find glfw3"

**Symptoms**: CMake configuration fails with missing GLFW.

**Solution**: Install via vcpkg:
```cmd
cd C:\vcpkg
.\vcpkg install glfw3:x64-windows
```

Then configure with toolchain file:
```cmd
cd cpp_client\build_vs
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

---

### "Cannot open include file: 'GL/glew.h'"

**Symptoms**: Build fails with missing GLEW header.

**Solution**: Install GLEW via vcpkg:
```cmd
.\vcpkg install glew:x64-windows
```

Then reconfigure CMake with the toolchain file (see above).

---

### "Could NOT find GLM"

**Symptoms**: CMake can't find GLM library.

**Solution**: Install via vcpkg:
```cmd
.\vcpkg install glm:x64-windows
```

GLM is header-only, so no linking is required.

---

### "Could NOT find nlohmann_json"

**Symptoms**: CMake warns about missing nlohmann/json.

**Note**: This is usually not critical as the project includes a bundled version.

**Solution (Optional)**: Install system version via vcpkg:
```cmd
.\vcpkg install nlohmann-json:x64-windows
```

---

### "OpenAL not found" warning

**Symptoms**: CMake warns that OpenAL is not found.

**Impact**: Audio support will be disabled. The client will build and run fine without it.

**Solution (Optional)**: To enable audio:
```cmd
.\vcpkg install openal-soft:x64-windows
```

Then reconfigure CMake.

---

### vcpkg packages installed but CMake still can't find them

**Symptoms**: Packages are installed but CMake doesn't detect them.

**Solution**: Always specify the toolchain file:
```cmd
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

Or set it as an environment variable:
```cmd
set CMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

---

## Build Failures

### "LINK : fatal error LNK1104: cannot open file 'glfw3.lib'"

**Symptoms**: Linker can't find library files.

**Solution**: 
1. Verify vcpkg packages are installed correctly
2. Ensure you specified the toolchain file in CMake configuration
3. Check the architecture matches (x64-windows vs x86-windows)
4. Try rebuilding:
```cmd
cd cpp_client\build_vs
cmake --build . --config Release --clean-first
```

---

### "error C2039: identifier not found"

**Symptoms**: Compiler errors about missing identifiers or types.

**Solution**:
1. Ensure all dependencies are installed
2. Clean and rebuild:
```cmd
cd cpp_client
rmdir /s /q build_vs
mkdir build_vs
cd build_vs
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Release
```

---

### "MSB8066: Custom build for ... exited with code 1"

**Symptoms**: Build fails with MSBuild error.

**Solution**:
1. Check the Output window in Visual Studio for the actual error
2. Common causes:
   - Missing dependencies (install via vcpkg)
   - Shader files not found (check cpp_client/shaders/ exists)
   - Asset files missing (check cpp_client/assets/ exists)

---

### Build succeeds but with many warnings

**Impact**: Usually not critical, but should be investigated.

**Solution**:
- Warnings about deprecated functions: Can be ignored for now
- Warnings about unused variables: Safe to ignore
- Warnings about potential security issues: Should be reviewed
- Conversion warnings: Usually safe but review if many occur

---

## Runtime Issues

### Executable crashes immediately on startup

**Symptoms**: eve_client.exe crashes with no error message or "Application Error".

**Solution 1 - Missing DLLs**:
If using dynamic linking, copy DLLs to exe directory:
```cmd
copy C:\vcpkg\installed\x64-windows\bin\*.dll cpp_client\build_vs\bin\Release\
```

Or use static linking instead:
```cmd
.\vcpkg install glfw3:x64-windows-static glew:x64-windows-static
# Reconfigure CMake
cmake .. -G "Visual Studio 17 2022" -A x64 -DVCPKG_TARGET_TRIPLET=x64-windows-static -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

**Solution 2 - Wrong working directory**:
In Visual Studio:
1. Right-click eve_client project
2. Properties → Debugging
3. Set Working Directory to: `$(TargetDir)` or `$(ProjectDir)..\bin\$(Configuration)`

**Solution 3 - Missing shaders/assets**:
Verify these directories exist next to the .exe:
- shaders/
- assets/

CMake should copy these automatically, but if not:
```cmd
xcopy /E /I cpp_client\shaders cpp_client\build_vs\bin\Release\shaders
xcopy /E /I cpp_client\assets cpp_client\build_vs\bin\Release\assets
```

---

### "Failed to initialize GLFW"

**Symptoms**: Application starts but fails to create window.

**Cause**: OpenGL/graphics drivers issue or running in headless environment.

**Solution**:
1. Update graphics drivers
2. Verify OpenGL support: Download and run OpenGL Extensions Viewer
3. If on a VM or remote desktop, ensure GPU passthrough is enabled

---

### Application runs but shows black screen

**Symptoms**: Window opens but displays nothing.

**Solution**:
1. Check shader files are in the correct location
2. Update graphics drivers
3. Try running in Debug mode to see error messages
4. Check Visual Studio Output window for OpenGL errors

---

### "Shader compilation failed"

**Symptoms**: Console shows shader compilation errors.

**Solution**:
1. Verify shader files exist in the shaders/ directory
2. Check shader syntax is valid GLSL
3. Ensure shaders are compatible with your OpenGL version
4. Try running a test program to verify OpenGL works:
```cmd
cd cpp_client\build_vs\bin\Release
test_lighting.exe
```

---

## Performance Issues

### Low framerate (< 30 FPS)

**Symptoms**: Game runs slowly even with few objects.

**Solution 1 - Use Release build**:
Debug builds are 5-10x slower. Always use Release for testing:
```cmd
.\build_vs.bat --release
```

Or in Visual Studio: Select "Release" from the configuration dropdown.

**Solution 2 - Update graphics drivers**:
Ensure you have the latest drivers for your GPU.

**Solution 3 - Verify VSync settings**:
VSync limits FPS to monitor refresh rate (usually 60 Hz), which is normal.

**Solution 4 - Check power settings**:
- Ensure laptop is plugged in
- Set Windows power plan to "High Performance"
- Check NVIDIA/AMD control panel for power-saving settings

---

### High CPU usage

**Symptoms**: CPU usage is very high even when idle.

**Solution**:
1. Use Release build (Debug has more overhead)
2. Check for infinite loops in the console output
3. Verify frame rate limiting is working (should be ~60 FPS with VSync)

---

### Memory leak

**Symptoms**: Memory usage grows over time.

**Solution**:
1. Use Debug build and check Visual Studio's diagnostic tools
2. Enable memory leak detection in Visual Studio
3. Report the issue on GitHub with details

---

## Additional Help

If none of these solutions work:

1. **Check the documentation**:
   - [VS2022_SETUP_GUIDE.md](VS2022_SETUP_GUIDE.md)
   - [QUICKSTART_VS2022.md](QUICKSTART_VS2022.md)
   - [docs/development/VISUAL_STUDIO_BUILD.md](docs/development/VISUAL_STUDIO_BUILD.md)

2. **Search GitHub Issues**:
   https://github.com/shifty81/EVEOFFLINE/issues

3. **Create a new issue** with:
   - Windows version
   - Visual Studio version (Help → About)
   - CMake version (`cmake --version`)
   - vcpkg version (if used)
   - Full error message
   - Steps to reproduce
   - Build command used

4. **Diagnostic info to include**:
```cmd
REM Collect this info when reporting issues:
echo Windows Version:
ver

echo Visual Studio Version:
dir "C:\Program Files\Microsoft Visual Studio\2022"

echo CMake Version:
cmake --version

echo vcpkg packages:
vcpkg list
```

---

**Last Updated**: 2024
**For**: Visual Studio 2022 build system
