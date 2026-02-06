# vcpkg Setup for EVE OFFLINE C++ Client

## Problem
After installing dependencies with vcpkg, CMake was failing with errors about missing `external/imgui/imgui.cpp` and other source files.

## Solution
The CMakeLists.txt has been updated to use vcpkg-provided packages (especially ImGui) instead of requiring bundled sources in the `external/` directory.

## Complete Installation Command

Run this single command to install ALL required dependencies:

```cmd
vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows imgui[glfw-binding,opengl3-binding]:x64-windows
```

### Optional: Audio Support
```cmd
vcpkg install openal-soft:x64-windows
```

### Optional: Static Linking (No DLL dependencies)
```cmd
vcpkg install glfw3:x64-windows-static glm:x64-windows-static glew:x64-windows-static nlohmann-json:x64-windows-static imgui[glfw-binding,opengl3-binding]:x64-windows-static openal-soft:x64-windows-static
```

## Step-by-Step Setup

### 1. Install vcpkg (if not already installed)
```cmd
cd C:\
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

### 2. Install Dependencies
```cmd
cd C:\vcpkg
.\vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows imgui[glfw-binding,opengl3-binding]:x64-windows openal-soft:x64-windows
```

**Note**: This will take several minutes and use approximately 600MB of disk space.

### 3. Verify Installation
```cmd
.\vcpkg list
```

You should see output similar to:
```
glew:x64-windows
glfw3:x64-windows
glm:x64-windows
imgui[core,glfw-binding,opengl3-binding]:x64-windows
nlohmann-json:x64-windows
openal-soft:x64-windows
```

### 4. Build the Project
```cmd
cd C:\path\to\EVEOFFLINE
.\build_vs.bat
```

The build script will automatically detect vcpkg at `C:\vcpkg` and use it.

## What Changed

### Before (Broken)
- CMakeLists.txt expected bundled ImGui sources in `cpp_client/external/imgui/`
- The `external/` directory is excluded from git (in `.gitignore`)
- Build would fail: "Cannot find source file: external/imgui/imgui.cpp"

### After (Fixed)
- CMakeLists.txt now tries to use vcpkg ImGui first via `find_package(imgui CONFIG)`
- Falls back to bundled sources only if vcpkg version not found
- All test targets properly link against `imgui::imgui` when using vcpkg
- Clear error messages guide users to install correct vcpkg packages

## Package Details

| Package | Purpose | Required? | vcpkg Features |
|---------|---------|-----------|----------------|
| **glfw3** | Window/input management | ✅ Yes | - |
| **glm** | Math library (vectors, matrices) | ✅ Yes | - |
| **glew** | OpenGL extension loader | ✅ Yes | - |
| **nlohmann-json** | JSON parsing | ✅ Yes | - |
| **imgui** | User interface library | ✅ Yes | `glfw-binding`, `opengl3-binding` |
| **openal-soft** | Audio support | ⚠️ Optional | - |

### Important: ImGui Features

ImGui requires specific features for GLFW and OpenGL3 integration:
```cmd
imgui[glfw-binding,opengl3-binding]:x64-windows
```

Without these features, the build will fail. The square bracket syntax `[feature1,feature2]` tells vcpkg to enable those specific integrations.

## Troubleshooting

### "ImGui not found!" error
**Solution**: Install ImGui with the required features:
```cmd
vcpkg install imgui[glfw-binding,opengl3-binding]:x64-windows
```

### "GLEW not found, falling back to GLAD"
**Solution**: Install GLEW:
```cmd
vcpkg install glew:x64-windows
```

### CMake can't find packages even though vcpkg installed them
**Cause**: CMake isn't using the vcpkg toolchain file.

**Solution**: The `build_vs.bat` script automatically detects vcpkg at `C:\vcpkg`. If you installed vcpkg elsewhere, set the environment variable:
```cmd
set CMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
```

Or manually specify it:
```cmd
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### "Cannot open include file: 'stb_image.h'"
**Status**: STB is optional and used for texture loading. This warning can be ignored, but textures won't load.

**Solution (Optional)**: 
1. Create `cpp_client/external/stb/` directory
2. Download `stb_image.h` from https://github.com/nothings/stb/blob/master/stb_image.h
3. Place it in `cpp_client/external/stb/stb_image.h`

## Disk Space

Total vcpkg installation size: ~600MB
- Base vcpkg: ~50MB
- Installed packages: ~550MB

## Alternative: Manifest Mode (Advanced)

For automated dependency management, create `vcpkg.json` in the project root:

```json
{
  "name": "eveoffline",
  "version": "1.0.0",
  "dependencies": [
    "glfw3",
    "glm",
    "glew",
    "nlohmann-json",
    {
      "name": "imgui",
      "features": ["glfw-binding", "opengl3-binding"]
    },
    "openal-soft"
  ]
}
```

Then vcpkg will automatically install dependencies when you build.

## For More Help

- **Quick Start**: [QUICKSTART_VS2022.md](QUICKSTART_VS2022.md)
- **Complete Guide**: [VS2022_SETUP_GUIDE.md](VS2022_SETUP_GUIDE.md)
- **Troubleshooting**: [TROUBLESHOOTING_VS2022.md](TROUBLESHOOTING_VS2022.md)
- **Dependencies**: [cpp_client/DEPENDENCIES.md](cpp_client/DEPENDENCIES.md)
