# C++ Client Quick Start Guide

This guide helps you quickly build and run the C++ OpenGL client on any platform.

## 🚀 Super Quick Start

### One-Command Build (All Platforms)

```bash
python3 build_cpp_client.py
```

That's it! The script will:
1. Check for required tools (CMake, compiler)
2. Guide you through dependency installation
3. Configure and build the project
4. Run basic tests
5. Show you how to run the client

## Platform-Specific Quick Start

### Linux

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install build-essential cmake libgl1-mesa-dev libglew-dev \
    libglfw3-dev libglm-dev nlohmann-json3-dev libopenal-dev \
    libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev

# Build
python3 build_cpp_client.py

# Run
cd cpp_client/build/bin
./eve_client "YourName"
```

### Windows

#### Option 1: Visual Studio (Recommended)

```batch
REM Install dependencies with vcpkg first:
REM vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows openal-soft:x64-windows

REM Generate Visual Studio solution and build
build_vs.bat

REM Or open in Visual Studio
build_vs.bat --open
```

#### Option 2: Python Script

```batch
REM Install dependencies with vcpkg first (see above)

REM Build
python build_cpp_client.py --vcpkg C:\path\to\vcpkg

REM Run
cd cpp_client\build\bin\Release
eve_client.exe "YourName"
```

### macOS

```bash
# Install dependencies with Homebrew
brew install cmake glfw glm glew nlohmann-json openal-soft

# Build
python3 build_cpp_client.py

# Run
cd cpp_client/build/bin
./eve_client "YourName"
```

## Build Options

### Debug Build

```bash
python3 build_cpp_client.py --debug
```

### Clean Rebuild

```bash
python3 build_cpp_client.py --clean
```

### Skip Tests

```bash
python3 build_cpp_client.py --skip-tests
```

### Verbose Output

```bash
python3 build_cpp_client.py --verbose
```

### Use Specific CMake Generator

```bash
python3 build_cpp_client.py --generator "Unix Makefiles"
```

### All Options

```bash
python3 build_cpp_client.py --help
```

## Manual Build (Advanced Users)

If you prefer to build manually:

```bash
cd cpp_client
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_LIBS=ON

# Build
make -j$(nproc)           # Linux/macOS
cmake --build . --config Release  # Windows

# Run
cd bin
./eve_client "YourName"
```

## Troubleshooting

### "CMake not found"

**Install CMake:**
- Linux: `sudo apt-get install cmake` or `sudo dnf install cmake`
- macOS: `brew install cmake`
- Windows: Download from https://cmake.org/download/

### "Compiler not found"

**Install compiler:**
- Linux: `sudo apt-get install build-essential`
- macOS: `xcode-select --install`
- Windows: Install Visual Studio with C++ tools

### "Library not found" errors

**Install dependencies** using your platform's package manager (see platform-specific sections above).

### Build succeeds but exe crashes

**On Windows:**
1. Make sure DLLs are in the same directory as the .exe
2. Or install dependencies as static libraries: `vcpkg install glfw3:x64-windows-static ...`

### Black screen when running

**Common causes:**
1. GPU doesn't support OpenGL 3.3+
2. Graphics drivers need updating
3. Working directory is wrong (should be in `bin/` directory with `shaders/` and `assets/`)

## What Gets Built

After building, you'll have:

```
cpp_client/build/
├── bin/
│   ├── eve_client          # Main executable
│   ├── test_network        # Test programs
│   ├── test_entity_sync
│   ├── test_lighting
│   ├── test_asteroid_field
│   ├── shaders/           # GLSL shaders
│   └── assets/            # Game assets
└── lib/                   # Libraries
```

## Running the Client

### Basic Usage

```bash
cd cpp_client/build/bin
./eve_client "CharacterName"
```

### Connect to Server

The client will try to connect to `localhost:8765` by default.

**Start Python server first:**
```bash
python server/server.py
```

**Then start C++ client:**
```bash
./eve_client "YourName"
```

### Running Tests

```bash
# Network test (tests TCP and JSON protocol)
./test_network

# Entity synchronization test
./test_entity_sync

# Visual tests (require display)
./test_lighting
./test_asteroid_field
./test_shadow_mapping
```

## Next Steps

1. **Read the README**: [../../cpp_client/README.md](../../cpp_client/README.md)
2. **Learn about features**: [../../cpp_client/README.md#features](../../cpp_client/README.md#features)
3. **Visual Studio setup**: [VISUAL_STUDIO_BUILD.md](VISUAL_STUDIO_BUILD.md)
4. **Development guide**: [DOCUMENTATION.md](DOCUMENTATION.md)

## CI/CD

The C++ client builds automatically on every push via GitHub Actions:

- ✅ Linux (Ubuntu)
- ✅ Windows (MSVC)
- ✅ macOS

See [CI_CD.md](CI_CD.md) for details.

## Getting Help

If you encounter issues:

1. Check this guide's troubleshooting section
2. Read [../../cpp_client/SETUP.md](../../cpp_client/SETUP.md)
3. Check GitHub Issues
4. Create a new issue with:
   - Your platform and version
   - CMake and compiler versions
   - Full error message
   - Build command used

## Quick Reference

```bash
# Install dependencies
# (see platform-specific section)

# Build
python3 build_cpp_client.py

# Or with options
python3 build_cpp_client.py --debug --clean --verbose

# Run
cd cpp_client/build/bin
./eve_client "YourName"

# Visual Studio (Windows)
build_vs.bat --open
```

**That's it!** The automated build script handles everything else.
