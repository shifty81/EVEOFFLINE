# EVE Offline C++ Client - Dependencies Setup Guide

## Overview

The C++ client for EVE Offline has been configured with all required dependencies for successful compilation and execution. This guide documents the setup and provides quick start instructions.

## What Was Done

### 1. **GLAD (OpenGL Loader) - Downloaded & Configured** ✅

GLAD is a multi-language OpenGL loader generator that provides cross-platform OpenGL function loading.

- **Files Created:**
  - `external/glad/src/glad.c` - Implementation file (7.2K)
  - `external/glad/include/glad/glad.h` - Header with GL constants and function pointers (7.8K)
  - `external/glad/include/glad/khrplatform.h` - Khronos platform definitions (3.3K)

- **Why Bundled:** OpenGL loaders must be compiled per-platform for compatibility

### 2. **nlohmann/json - Downloaded & Configured** ✅

A popular, header-only JSON library for C++.

- **File:** `external/json/include/json.hpp` (887K, v3.11.2)
- **Why Bundled:** Header-only libraries benefit from bundling for version consistency

### 3. **System Libraries - Verified** ✅

The following libraries are available as system packages:

| Library | Version | Status | Command |
|---------|---------|--------|---------|
| GLFW | 3.3.10 | ✅ Installed | `pkg-config --modversion glfw3` |
| GLM | 0.9.9.8 | ✅ Installed | `pkg-config --modversion glm` |
| OpenGL | 1.2+ | ✅ Installed | `glxinfo \| grep "OpenGL version"` |

### 4. **CMakeLists.txt - Updated** ✅

The main build configuration has been enhanced with intelligent library detection:

- Default: `USE_SYSTEM_LIBS=ON` (uses system GLFW, GLM, and OpenGL)
- Fallback: Automatic bundled version detection if system libraries unavailable
- Always bundled: GLAD and JSON (for compatibility and consistency)

### 5. **Documentation - Created** ✅

- `DEPENDENCIES.md` - Detailed dependency documentation
- `SETUP_COMPLETE.md` - Complete setup guide with troubleshooting
- `verify_dependencies.sh` - Automated verification script
- `README_DEPENDENCIES.md` - This file

## Directory Structure

```
cpp_client/
├── external/
│   ├── glad/
│   │   ├── src/
│   │   │   └── glad.c
│   │   └── include/glad/
│   │       ├── glad.h
│   │       └── khrplatform.h
│   └── json/include/
│       └── json.hpp
├── CMakeLists.txt (updated)
├── DEPENDENCIES.md
├── SETUP_COMPLETE.md
├── verify_dependencies.sh
└── README_DEPENDENCIES.md (this file)
```

## Quick Start

### Prerequisites

Ensure you have the following tools installed:

```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake git
sudo apt-get install libglfw3-dev libglm-dev libgl1-mesa-dev

# macOS
brew install cmake glfw3 glm

# Verify installation
cmake --version
make --version
g++ --version
pkg-config --list-all | grep -E "glfw|glm"
```

### Build Steps

#### 1. Create Build Directory
```bash
cd cpp_client
mkdir -p build
cd build
```

#### 2. Configure with CMake
```bash
# Recommended: Use system libraries
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_LIBS=ON ..

# Alternative: Debug build
cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_SYSTEM_LIBS=ON ..

# Alternative: Use bundled libraries
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_LIBS=OFF ..

# With tests enabled (default)
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_LIBS=ON -DBUILD_TESTS=ON ..
```

#### 3. Compile
```bash
# Full project
make -j$(nproc)

# Or specific targets
make eve_client                  # Main executable
make test_asteroid_field         # Asteroid field test
make test_lighting               # Lighting test
make test_shadow_mapping         # Shadow mapping test
```

#### 4. Run
```bash
# Main application
./bin/eve_client

# Tests
./bin/test_asteroid_field
./bin/test_lighting
./bin/test_shadow_mapping
```

## Verification

To verify all dependencies are correctly installed, run the verification script:

```bash
cd cpp_client
./verify_dependencies.sh
```

Expected output:
```
✅ GLAD Source
✅ GLAD Header
✅ KHR Platform Header
✅ nlohmann/json
✅ GLFW (v3.3.10)
✅ GLM (v0.9.9.8)
✅ CMake
✅ Make
✅ G++
✅ Dependencies compile successfully

🎉 All dependencies verified successfully!
```

## CMake Configuration Options

```bash
# Display all options
cmake -LA ..

# Common combinations
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_LIBS=ON ..     # Production
cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_SYSTEM_LIBS=ON ..       # Development
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_LIBS=OFF ..    # Bundled only
cmake -DBUILD_TESTS=ON ..                                     # With tests
cmake -DBUILD_TESTS=OFF ..                                    # Without tests
```

## Troubleshooting

### Issue: "Could not find package GLFW"

**Solution 1:** Install system GLFW
```bash
sudo apt-get install libglfw3-dev  # Ubuntu/Debian
brew install glfw3                  # macOS
```

**Solution 2:** Use bundled libraries
```bash
cmake -DUSE_SYSTEM_LIBS=OFF ..
```

### Issue: "Could not find package GLM"

**Solution 1:** Install system GLM
```bash
sudo apt-get install libglm-dev   # Ubuntu/Debian
brew install glm                   # macOS
```

**Solution 2:** Use bundled libraries
```bash
cmake -DUSE_SYSTEM_LIBS=OFF ..
```

### Issue: OpenGL errors during compilation

**Solution:** Ensure OpenGL is installed
```bash
# Ubuntu/Debian
sudo apt-get install libgl1-mesa-dev

# Verify
glxinfo | grep "direct rendering"
```

### Issue: GLAD compilation errors

**Check:**
1. `external/glad/src/glad.c` exists and is not empty
2. `external/glad/include/glad/glad.h` exists and is not empty
3. `external/glad/include/glad/khrplatform.h` exists
4. CMake found all files

**Debug:**
```bash
find ./external/glad -type f -exec ls -lh {} \;
```

### Issue: Build still references old cache

**Solution:** Clean and reconfigure
```bash
rm -rf build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_LIBS=ON ..
make -j$(nproc)
```

## Advanced Options

### Static Linking
```bash
cmake -DCMAKE_EXE_LINKER_FLAGS="-static" ..
```

### Optimized Build
```bash
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS_RELEASE="-O3 -march=native" ..
```

### Cross-Compilation
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/toolchain.cmake ..
```

### Custom Installation Path
```bash
cmake -DCMAKE_INSTALL_PREFIX=/custom/path ..
make install
```

## Environment Variables

```bash
# Compiler selection
export CC=gcc
export CXX=g++

# Optimization
export CXXFLAGS="-O3 -march=native"
export LDFLAGS="-Wl,-O1"

# pkg-config path (if needed)
export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH"
```

## Performance Tips

1. **Use Release builds:**
   ```bash
   cmake -DCMAKE_BUILD_TYPE=Release ..
   ```

2. **Parallel compilation:**
   ```bash
   make -j$(nproc)  # Uses all CPU cores
   ```

3. **Ccache for faster rebuilds:**
   ```bash
   sudo apt-get install ccache
   cmake -DCMAKE_C_COMPILER_LAUNCHER=ccache \
         -DCMAKE_CXX_COMPILER_LAUNCHER=ccache ..
   ```

4. **Link-time optimization:**
   ```bash
   cmake -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON ..
   ```

## Documentation

- **DEPENDENCIES.md** - Detailed dependency information
- **SETUP_COMPLETE.md** - Complete setup documentation
- **CMakeLists.txt** - Build configuration
- **verify_dependencies.sh** - Automated verification

## System Requirements

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| CMake | 3.15 | 3.30+ |
| C++ Standard | C++17 | C++20 |
| GCC/Clang | 7.0 | 13.0+ |
| RAM | 2GB | 8GB |
| Disk Space | 500MB | 2GB |

## Platform Status

| Platform | Support | Notes |
|----------|---------|-------|
| Linux (x86_64) | ✅ Tested | Fully supported |
| macOS (Intel) | ✅ Ready | Requires Xcode |
| macOS (ARM64) | ✅ Ready | Requires Xcode |
| Windows (MSVC) | ⚠️ Partial | Requires MSVC 2019+ |
| Windows (MinGW) | ✅ Ready | Requires MinGW |

## Next Steps

1. **Verify Setup:**
   ```bash
   ./verify_dependencies.sh
   ```

2. **Build the Project:**
   ```bash
   cd build
   cmake -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_LIBS=ON ..
   make -j$(nproc)
   ```

3. **Run Tests:**
   ```bash
   ./bin/test_asteroid_field
   ```

4. **Launch Application:**
   ```bash
   ./bin/eve_client
   ```

## Support Resources

- **CMake Documentation:** https://cmake.org/documentation/
- **GLFW Documentation:** https://www.glfw.org/documentation.html
- **GLM Documentation:** https://glm.g-truc.net/0.9.9/
- **GLAD Generator:** https://glad.dav1d.de/
- **nlohmann/json:** https://github.com/nlohmann/json

## Contributing

When adding new dependencies:

1. **Header-only libraries:** Add to `external/<lib>/include`
2. **Libraries with sources:** Add to `external/<lib>` with CMakeLists.txt
3. **System libraries:** Add `find_package()` call in CMakeLists.txt
4. **Update documentation** with new dependency info

## License & Attribution

- **GLAD:** MIT License
- **nlohmann/json:** MIT License
- **GLFW:** zlib License
- **GLM:** Happy Bunny License or MIT License
- **OpenGL:** Khronos License

---

**Setup Date:** 2024
**Status:** ✅ Complete and Verified
**Configuration:** USE_SYSTEM_LIBS=ON (Recommended)

For detailed information, see DEPENDENCIES.md and SETUP_COMPLETE.md
