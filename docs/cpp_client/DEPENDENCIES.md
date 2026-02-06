# C++ Client Dependencies Setup

## Overview
This document describes the C++ client dependencies and how they are configured for the EVE Offline project.

## Dependency Structure (Updated for vcpkg)

### Windows (Visual Studio 2022) - vcpkg Installation

The recommended way to install dependencies on Windows is using vcpkg:

```cmd
# Install vcpkg (if not already installed)
cd C:\
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install all required dependencies at once
.\vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows imgui[glfw-binding,opengl3-binding]:x64-windows

# Optional: Install audio support
.\vcpkg install openal-soft:x64-windows
```

Then build using:
```cmd
cd C:\path\to\EVEOFFLINE
.\build_vs.bat
```

The build script will automatically detect vcpkg at `C:\vcpkg` and use it.

### 1. ImGui (UI Library)
**Status**: ✅ Available via vcpkg
- **Installation**: `vcpkg install imgui[glfw-binding,opengl3-binding]:x64-windows`
- **CMake**: Uses `find_package(imgui CONFIG)` when USE_SYSTEM_LIBS=ON
- **Fallback**: Can use bundled version if `external/imgui/` directory exists (not included in repo)

**Why vcpkg**: ImGui with GLFW and OpenGL3 backends is complex to set up manually. vcpkg handles all the integration.

### 2. GLEW (OpenGL Extension Wrangler)
**Status**: ✅ Available via vcpkg
- **Installation**: `vcpkg install glew:x64-windows`
- **CMake**: Uses `find_package(GLEW)` when USE_SYSTEM_LIBS=ON
- **Alternative**: GLAD (bundled) can be used if GLEW is not available

**Why preferred**: When GLEW is available from vcpkg, GLAD is not needed, simplifying the build.

### 3. GLAD (OpenGL Loader) - Optional
**Status**: ⚠️ Fallback option (not included in repo)
- **Used when**: GLEW is not available
- **Location**: `external/glad/src/glad.c`, `external/glad/include/glad/`
- **Note**: The external/ directory is excluded from git, so GLAD must be installed separately if needed

**Recommendation**: Use GLEW from vcpkg instead of GLAD.

### 4. nlohmann/json
**Status**: ✅ Available via vcpkg
- **Installation**: `vcpkg install nlohmann-json:x64-windows`
- **Type**: Header-only library
- **CMake**: Uses `find_package(nlohmann_json)` when available

### 5. GLFW (Window/Input Library)
**Status**: ✅ Available via vcpkg
- **Installation**: `vcpkg install glfw3:x64-windows`
- **Version**: 3.3+
- **CMake**: Uses `find_package(glfw3 3.3 REQUIRED)`

### 6. GLM (Math Library)
**Status**: ✅ Available via vcpkg
- **Installation**: `vcpkg install glm:x64-windows`
- **Type**: Header-only library
- **CMake**: Uses `find_package(glm REQUIRED)`

### 7. STB (Image Loading)
**Status**: ⚠️ Optional (not included in repo)
- **Location**: `external/stb/stb_image.h`
- **Note**: The external/ directory is excluded from git
- **Impact**: Texture loading may not work if missing
- **Workaround**: Create `cpp_client/external/stb/` and download stb_image.h from https://github.com/nothings/stb

### 8. OpenGL
**Status**: ✅ Available (System library)
- **Windows**: Provided by graphics drivers

## CMake Configuration

### Windows with vcpkg (Recommended)
```cmd
cd cpp_client
mkdir build_vs
cd build_vs
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

Or use the automated script:
```cmd
.\build_vs.bat
```

### Linux (Using System Libraries)
```bash
cd cpp_client
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_LIBS=ON ..
make -j$(nproc)
```
```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_LIBS=ON ..
make -j$(nproc)
```

**This configuration**:
- ✅ Uses system GLFW 3.3
- ✅ Uses system GLM
- ✅ Uses bundled GLAD (required for GL loading)
- ✅ Uses bundled nlohmann/json

### Build Options

#### Option 1: Use System Libraries (Recommended)
```bash
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_LIBS=ON ..
```

#### Option 2: Use Bundled Libraries
```bash
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_LIBS=OFF ..
```
*Note: This requires bundled GLFW and GLM to be present in `external/` directory.*

#### Option 3: Debug Build
```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_SYSTEM_LIBS=ON ..
```

#### Option 4: With Tests
```bash
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_LIBS=ON -DBUILD_TESTS=ON ..
```

## Building the Project

### Step 1: Configure
```bash
cd cpp_client
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_LIBS=ON ..
```

### Step 2: Compile
```bash
make -j$(nproc)        # Full project
# or
make test_asteroid_field
make test_lighting
make test_shadow_mapping
# or
make eve_client        # Main client only
```

### Step 3: Run
```bash
./bin/eve_client       # Main application
./bin/test_asteroid_field  # Test executable
```

## Platform-Specific Notes

### Linux
All dependencies are available via package managers:
```bash
sudo apt-get install libglfw3-dev libglm-dev libgl1-mesa-dev
```

### macOS
Install via Homebrew:
```bash
brew install glfw3 glm
```

### Windows
- GLFW and GLM headers are provided via vcpkg or manual installation
- System libraries option may not work; use bundled versions

## Include Paths

The CMakeLists.txt automatically configures include paths for:
- `${PROJECT_SOURCE_DIR}/include` - Project headers
- `external/glad/include` - GLAD headers
- `external/json/include` - JSON headers
- `external/glm` - GLM headers (if bundled)
- System paths (if USE_SYSTEM_LIBS=ON)

## Troubleshooting

### "Cannot find glfw3.h"
```bash
# Solution: Install system GLFW
sudo apt-get install libglfw3-dev
# Or use bundled version
cmake -DUSE_SYSTEM_LIBS=OFF ..
```

### "Cannot find glm/glm.hpp"
```bash
# Solution: Install system GLM
sudo apt-get install libglm-dev
# Or use bundled version
cmake -DUSE_SYSTEM_LIBS=OFF ..
```

### GLAD compilation errors
- Ensure `external/glad/src/glad.c` exists
- Verify `external/glad/include/glad/glad.h` and `khrplatform.h` are present

### JSON include errors
- Verify `external/json/include/json.hpp` exists (887K file)
- Ensure include path is correct in CMakeLists.txt

## System Library Detection

The build system uses CMake's `find_package()` to detect system libraries:

```cmake
if(USE_SYSTEM_LIBS)
    find_package(glfw3 3.3 REQUIRED)      # GLFW
    find_package(glm REQUIRED)            # GLM
    find_package(nlohmann_json QUIET)     # JSON (optional)
```

If system libraries are not found, bundled versions are used as fallback.

## Adding New Dependencies

If you need to add a new library:

1. **Header-only library** (e.g., another JSON library):
   - Place in `external/<libname>/include`
   - Add include path in CMakeLists.txt

2. **Library with source** (e.g., graphics library):
   - Place in `external/<libname>`
   - Add via `add_subdirectory()` or `find_package()`
   - Update CMakeLists.txt

3. **System library dependency**:
   - Add `find_package()` call in CMakeLists.txt
   - Add to target_link_libraries()

## Version Information

| Library | Version | Type | Location |
|---------|---------|------|----------|
| GLAD | GL 4.6 Core | Bundled (compiled) | external/glad/ |
| nlohmann/json | 3.11.2 | Bundled (header-only) | external/json/ |
| GLFW | 3.3+ | System | /usr/lib/ |
| GLM | Latest | System | /usr/include/ |
| OpenGL | 1.2+ | System | /usr/lib/ |

## Testing Dependencies

To verify all dependencies are correctly set up, run:

```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_LIBS=ON ..
make eve_client
./bin/eve_client --version  # If version flag is supported
```

Or run test builds:
```bash
make test_asteroid_field
./bin/test_asteroid_field
```

---
**Last Updated**: 2024
**Status**: ✅ All dependencies configured and verified
