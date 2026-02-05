# Building EVE OFFLINE C++ Client with Visual Studio

This guide covers building the C++ client using Visual Studio on Windows.

## Prerequisites

### Required Software

1. **Visual Studio 2019 or later**
   - Download from: https://visualstudio.microsoft.com/downloads/
   - During installation, select "Desktop development with C++"
   - Required components:
     - MSVC v142+ (or latest)
     - Windows 10/11 SDK
     - C++ CMake tools for Windows

2. **CMake 3.15 or later**
   - Download from: https://cmake.org/download/
   - Add to PATH during installation
   - Or install via Visual Studio Installer

3. **vcpkg** (recommended for dependencies)
   - Clone and bootstrap:
     ```cmd
     git clone https://github.com/microsoft/vcpkg.git
     cd vcpkg
     .\bootstrap-vcpkg.bat
     ```
   - Add to PATH or note the location

### Installing Dependencies with vcpkg

```cmd
vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows openal-soft:x64-windows
```

Or for static linking:
```cmd
vcpkg install glfw3:x64-windows-static glm:x64-windows-static glew:x64-windows-static nlohmann-json:x64-windows-static openal-soft:x64-windows-static
```

## Quick Start - Automated Build

### Using the Build Script

The easiest way to build is using the provided batch script:

```cmd
# From repository root
build_vs.bat
```

This will:
1. Generate Visual Studio solution files
2. Build the Release configuration
3. Show the location of the executable

### Build Script Options

```cmd
build_vs.bat --clean      # Clean rebuild (deletes build directory)
build_vs.bat --debug      # Build Debug configuration instead of Release
build_vs.bat --open       # Open Visual Studio after generating solution
```

### Example Usage

```cmd
# Clean Debug build and open in Visual Studio
build_vs.bat --clean --debug --open

# Quick Release build
build_vs.bat --release
```

## Manual Visual Studio Setup

If you prefer to set up manually or the script doesn't work:

### Step 1: Generate Visual Studio Solution

```cmd
cd cpp_client
mkdir build_vs
cd build_vs

# Configure CMake with Visual Studio generator
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
```

**Note**: Adjust the generator for your Visual Studio version:
- Visual Studio 2022: `"Visual Studio 17 2022"`
- Visual Studio 2019: `"Visual Studio 16 2019"`

### Step 2: Open Solution in Visual Studio

```cmd
start EVEOfflineClient.sln
```

Or navigate to `cpp_client/build_vs/` and double-click `EVEOfflineClient.sln`

### Step 3: Build in Visual Studio

1. Select build configuration (Debug or Release) from the toolbar
2. Right-click `eve_client` project → **Set as Startup Project**
3. Press **F7** or **Build → Build Solution**

### Step 4: Run the Client

- Press **F5** to run with debugging
- Or press **Ctrl+F5** to run without debugging
- Executable location: `build_vs/bin/Release/eve_client.exe` (or Debug)

## Configuration Options

### Using System Libraries vs. Bundled

By default, the build system will try to use system-installed libraries:

```cmd
cmake .. -DUSE_SYSTEM_LIBS=ON
```

To use bundled libraries instead:

```cmd
cmake .. -DUSE_SYSTEM_LIBS=OFF
```

### Building Tests

Tests are built by default. To disable:

```cmd
cmake .. -DBUILD_TESTS=OFF
```

### Build Configuration

Choose between Debug and Release:

```cmd
# Debug build (with debugging symbols)
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Release build (optimized)
cmake .. -DCMAKE_BUILD_TYPE=Release
```

## Project Structure in Visual Studio

After generating the solution, you'll see these projects:

```
Solution 'EVEOfflineClient'
├── eve_client (Main executable - Set as Startup Project)
├── test_asteroid_field
├── test_lighting
├── test_shadow_mapping
├── test_deferred_rendering
├── test_ui_system
├── test_post_processing
├── test_audio_system (if OpenAL found)
├── test_network
├── test_entity_sync
└── test_enhanced_ui
```

## Running Tests in Visual Studio

1. Right-click a test project (e.g., `test_lighting`)
2. Select **Set as Startup Project**
3. Press **F5** to run the test

Or from command line after building:
```cmd
cd build_vs\bin\Release
test_lighting.exe
test_asteroid_field.exe
test_network.exe
```

## Troubleshooting

### "CMake Error: Could not find CMAKE_C_COMPILER"

**Solution**: Install "C++ CMake tools for Windows" in Visual Studio Installer:
1. Open Visual Studio Installer
2. Modify your installation
3. Check "C++ CMake tools for Windows"

### "Cannot open include file: 'GL/glew.h'"

**Solution**: Dependencies not found. Install via vcpkg:
```cmd
vcpkg install glew:x64-windows
```

Then configure CMake with vcpkg toolchain:
```cmd
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
```

### "LINK : fatal error LNK1104: cannot open file 'glfw3.lib'"

**Solution**: Dependencies not linked properly. Make sure:
1. vcpkg packages are installed
2. CMake is configured with vcpkg toolchain file
3. Run cmake configuration again

### "Could not find OpenAL"

OpenAL is optional. The client will build without audio support. To enable:
```cmd
vcpkg install openal-soft:x64-windows
```

### Build succeeds but exe crashes on start

**Common causes**:
1. **DLL not found**: Copy required DLLs from vcpkg\installed\x64-windows\bin\ to your exe directory
2. **Working directory wrong**: Set working directory in Visual Studio:
   - Right-click eve_client → Properties → Debugging → Working Directory
   - Set to: `$(TargetDir)` or `$(ProjectDir)\..\bin\$(Configuration)`

## Advanced Options

### Static vs Dynamic Linking

For static linking (single .exe with no external DLLs):
```cmd
vcpkg install glfw3:x64-windows-static glew:x64-windows-static
cmake .. -DVCPKG_TARGET_TRIPLET=x64-windows-static
```

### Multi-threaded Builds

CMake will use multiple cores by default with MSBuild. To manually control:
```cmd
cmake --build . --config Release -- /m:4  # Use 4 cores
```

### Custom Build Directory

```cmd
# Use any directory name you want
cmake -B my_custom_build -S .
cmake --build my_custom_build --config Release
```

## Integration with Visual Studio Code

If you prefer VS Code:

1. Install extensions:
   - C/C++ (Microsoft)
   - CMake Tools (Microsoft)

2. Open cpp_client folder in VS Code

3. Configure CMake with vcpkg:
   ```json
   // .vscode/settings.json
   {
       "cmake.configureSettings": {
           "CMAKE_TOOLCHAIN_FILE": "C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"
       }
   }
   ```

4. Select kit (Visual Studio compiler)

5. Build and run with CMake extension

## Performance Notes

### Release vs Debug

- **Debug**: Slow, but enables debugging, ~10-20 FPS
- **Release**: Optimized, 60+ FPS with hundreds of entities
- **RelWithDebInfo**: Fast with some debug info (good compromise)

### Recommended Settings for Development

```cmd
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTS=ON
```

This gives you:
- Good performance for testing
- Debugging information when needed
- All test programs built

## Next Steps

After building successfully:

1. **Run the client**: See [cpp_client/README.md](../cpp_client/README.md) for usage
2. **Connect to server**: Start Python server first, then run client
3. **Try tests**: Run individual test programs to see features
4. **Modify code**: Edit in Visual Studio and rebuild

## Getting Help

If you encounter issues:

1. Check [cpp_client/DEPENDENCIES.md](../cpp_client/DEPENDENCIES.md) for dependency info
2. Review [cpp_client/SETUP.md](../cpp_client/SETUP.md) for general setup
3. Search GitHub Issues for similar problems
4. Create a new issue with:
   - Visual Studio version
   - CMake version
   - Build command used
   - Full error message

## See Also

- [CMakeLists.txt](../cpp_client/CMakeLists.txt) - Build configuration
- [README.md](../cpp_client/README.md) - C++ client overview
- [SETUP.md](../cpp_client/SETUP.md) - General setup guide
- [docs/development/](../docs/development/) - Development documentation
