# C++ Client Dependencies Setup

## Overview
This document describes the C++ client dependencies and how they are configured for the EVE Offline project.

## Dependency Structure

### 1. GLAD (OpenGL Loader)
**Status**: ✅ Configured (Bundled)
- **Location**: `external/glad/src/glad.c`, `external/glad/include/glad/`
- **Files**:
  - `glad.c` (7.2K) - OpenGL function pointers loader implementation
  - `glad.h` (9.8K) - OpenGL core headers (GL 4.6)
  - `khrplatform.h` (3.3K) - Khronos platform definitions

**Why bundled**: OpenGL loader must be compiled per-platform. Bundling provides compatibility across Linux, macOS, and Windows.

### 2. nlohmann/json
**Status**: ✅ Configured (Bundled, header-only)
- **Location**: `external/json/include/json.hpp` (887K)
- **Type**: Header-only library
- **Version**: 3.11.2

**Why bundled**: Header-only libraries don't need compilation and bundling ensures version consistency.

### 3. GLFW (Window/Input Library)
**Status**: ✅ Available (System library)
- **Installed via**: apt (pre-installed)
- **Version**: 3.3+
- **Fallback**: Can use bundled version if `external/glfw` directory exists

### 4. GLM (Math Library)
**Status**: ✅ Available (System library)
- **Installed via**: apt (pre-installed)
- **Type**: Header-only library
- **Fallback**: Can use bundled version via `external/glm`

### 5. OpenGL
**Status**: ✅ Available (System library)
- **Installed via**: libgl1-mesa-dev

## CMake Configuration

### Default Setup (Recommended)
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
