# C++ Client Quick Setup Guide

## Prerequisites

Install the following dependencies on your system:

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libgl1-mesa-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libxkbcommon-dev \
    wayland-scanner++ \
    libwayland-dev
```

### Fedora/RHEL
```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    mesa-libGL-devel \
    libXrandr-devel \
    libXinerama-devel \
    libXcursor-devel \
    libXi-devel \
    libxkbcommon-devel \
    wayland-devel \
    wayland-protocols-devel
```

### macOS
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install CMake via Homebrew
brew install cmake
```

### Windows
- Install Visual Studio 2019 or later with C++ desktop development
- Install CMake from https://cmake.org/download/

## Setting Up External Dependencies

The client requires several external libraries. Run this script to download them:

```bash
cd cpp_client

# Install GLAD generator (Python package)
python3 -m pip install --user glad

# Create external directory
mkdir -p external
cd external

# Clone dependencies
git clone --depth 1 https://github.com/glfw/glfw.git
git clone --depth 1 https://github.com/g-truc/glm.git
git clone --depth 1 https://github.com/nlohmann/json.git

# Generate GLAD for OpenGL 3.3 Core
python3 -m glad --profile=core --api="gl=3.3" --out-path=glad --generator=c

cd ..
```

## Building

### Linux/macOS
```bash
cd cpp_client
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### Windows (Visual Studio)
```bash
cd cpp_client
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Windows (MinGW)
```bash
cd cpp_client
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

## Running

After building, the executable will be in `build/bin/`:

```bash
cd build/bin
./eve_client "YourCharacterName"
```

## Testing the Build

To verify everything works:

```bash
cd cpp_client/build/bin

# Should see window with starfield (if display available)
./eve_client TestPlayer

# In headless environment, should fail gracefully with message:
# "Fatal error: Failed to initialize GLFW"
```

## Troubleshooting

### "Could NOT find OpenGL"
Install OpenGL development libraries:
```bash
# Ubuntu/Debian
sudo apt-get install libgl1-mesa-dev

# Fedora
sudo dnf install mesa-libGL-devel
```

### "Failed to find wayland-scanner"
Install Wayland development packages:
```bash
# Ubuntu/Debian
sudo apt-get install wayland-scanner++ libwayland-dev

# Fedora
sudo dnf install wayland-devel wayland-protocols-devel
```

### "Package xkbcommon not found"
Install xkbcommon:
```bash
# Ubuntu/Debian
sudo apt-get install libxkbcommon-dev

# Fedora
sudo dnf install libxkbcommon-devel
```

### "Shader compilation failed"
Make sure shaders directory was copied to build directory:
```bash
ls build/bin/shaders/
# Should show: basic.frag, basic.vert, starfield.frag, starfield.vert
```

### "Error required internal CMake variable not set"
This was an issue with the CMakeLists.txt not declaring C language.
Make sure line 2 of `CMakeLists.txt` reads:
```cmake
project(EVEOfflineClient VERSION 1.0.0 LANGUAGES C CXX)
```

## Project Structure

```
cpp_client/
├── build/                  # Build directory (created by cmake)
│   └── bin/
│       ├── eve_client      # Executable
│       ├── shaders/        # GLSL shaders (auto-copied)
│       └── assets/         # Game assets (auto-copied)
│
├── include/                # Header files
│   ├── core/
│   ├── rendering/
│   ├── network/
│   └── ui/
│
├── src/                    # Implementation files
│   ├── main.cpp
│   ├── core/
│   ├── rendering/
│   ├── network/
│   └── ui/
│
├── shaders/                # GLSL shader sources
│   ├── basic.vert
│   ├── basic.frag
│   ├── starfield.vert
│   └── starfield.frag
│
├── external/               # Third-party libraries (git cloned)
│   ├── glfw/
│   ├── glm/
│   ├── glad/
│   └── json/
│
└── CMakeLists.txt          # Build configuration
```

## What's Implemented

✅ **Core Systems**
- Window management (GLFW)
- Game loop with delta time
- Input handling (keyboard, mouse)

✅ **Rendering**
- OpenGL 3.3 Core context
- Shader loading and compilation
- EVE-style orbit camera
- Starfield background (2000 procedural stars)
- Mesh rendering infrastructure

✅ **Architecture**
- Modern C++17
- RAII resource management
- Smart pointers
- Proper error handling

⏳ **Not Yet Implemented**
- Network connection to server
- Ship models
- HUD rendering
- Entity synchronization
- Textures

## Performance

- **Build time**: ~30 seconds
- **Executable size**: 1.1 MB
- **Target FPS**: 60 (with VSync)
- **RAM usage**: <100 MB (idle)

## Next Development Steps

1. **Network Integration**
   - Connect to Python server via TCP
   - Implement JSON protocol handler
   - Entity state synchronization

2. **Ship Models**
   - Procedural ship generation
   - Different factions/types
   - LOD system

3. **HUD**
   - Target info panel
   - Ship status display
   - Combat log
   - Minimap

4. **Input**
   - Camera controls (right-click drag)
   - Ship selection (left-click)
   - Keyboard shortcuts

## Documentation

- See `README.md` for full project documentation
- See `cpp_client/README.md` for detailed API documentation
- See header files in `include/` for class interfaces

## Support

For issues or questions, see the main project documentation or create an issue on GitHub.
