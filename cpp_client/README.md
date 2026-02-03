# EVE OFFLINE C++ OpenGL Client

A high-performance C++ client with modern OpenGL graphics for EVE OFFLINE.

## Overview

This is a cross-platform 3D client built with:
- **C++17** - Modern C++ features
- **OpenGL 3.3+** - Core profile graphics
- **GLFW** - Cross-platform windowing
- **GLM** - Mathematics library
- **GLAD** - OpenGL function loader

## Features

### Current Status: 🚧 In Development

- [x] Project structure created
- [x] Build system configured (CMake)
- [x] Header files defined
- [ ] Core implementation
- [ ] OpenGL rendering
- [ ] Network client
- [ ] Ship models
- [ ] HUD/UI

### Planned Features

**Graphics**:
- Modern OpenGL 3.3+ rendering
- EVE-style orbit camera
- Procedural ship models
- Starfield background
- Particle effects (weapons, explosions)
- Physically-based rendering (PBR)

**Networking**:
- TCP connection to dedicated server
- JSON protocol (compatible with Python server)
- Entity state synchronization
- Lag compensation

**Gameplay**:
- Full EVE mechanics
- Ship fitting and combat
- Skills and progression
- Missions and exploration

## Building

### Prerequisites

**Required**:
- C++17 compatible compiler
  - Windows: Visual Studio 2017+ or MinGW
  - Linux: GCC 7+ or Clang 5+
  - macOS: Xcode Command Line Tools
- CMake 3.15+
- OpenGL 3.3+ capable graphics card

**Dependencies** (automatically handled by CMake):
- GLFW 3.3+
- GLM
- GLAD
- nlohmann/json

### Build Steps

#### Linux/macOS

```bash
cd cpp_client
mkdir build
cd build
cmake ..
make
```

#### Windows (Visual Studio)

```bash
cd cpp_client
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

#### Windows (MinGW)

```bash
cd cpp_client
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

### Build Options

```bash
# Use system libraries instead of bundled
cmake .. -DUSE_SYSTEM_LIBS=ON

# Build without tests
cmake .. -DBUILD_TESTS=OFF
```

## Running

### Basic Usage

```bash
cd build/bin
./eve_client "CharacterName"
```

### Connecting to Server

By default, connects to `localhost:8765`. To connect to a remote server, you'll be able to:
1. Edit configuration file
2. Use command line arguments (future):
   ```bash
   ./eve_client "CharacterName" --host game.server.com --port 8765
   ```

## Project Structure

```
cpp_client/
├── CMakeLists.txt          # Build configuration
├── README.md               # This file
│
├── include/                # Header files
│   ├── core/              # Core application
│   │   ├── application.h  # Main app & game loop
│   │   └── game_client.h  # Network game client
│   │
│   ├── rendering/         # Graphics rendering
│   │   ├── window.h       # GLFW window management
│   │   ├── shader.h       # Shader programs
│   │   ├── camera.h       # EVE-style camera
│   │   ├── renderer.h     # Main renderer
│   │   ├── mesh.h         # Vertex data
│   │   ├── model.h        # 3D models
│   │   └── texture.h      # Texture loading
│   │
│   ├── network/           # Networking
│   │   ├── tcp_client.h   # TCP connection
│   │   └── protocol_handler.h  # JSON protocol
│   │
│   └── ui/                # User interface
│       ├── hud.h          # Heads-up display
│       └── input_handler.h  # Input handling
│
├── src/                   # Source files
│   ├── main.cpp           # Entry point
│   ├── core/              # Core implementations
│   ├── rendering/         # Rendering implementations
│   ├── network/           # Network implementations
│   └── ui/                # UI implementations
│
├── shaders/               # GLSL shaders
│   ├── basic.vert         # Basic vertex shader
│   ├── basic.frag         # Basic fragment shader
│   ├── starfield.vert     # Starfield vertex shader
│   └── starfield.frag     # Starfield fragment shader
│
├── assets/                # Game assets
│   ├── textures/          # Textures
│   ├── models/            # 3D models
│   └── sounds/            # Audio files
│
└── external/              # Third-party libraries
    ├── glfw/              # GLFW (git submodule)
    ├── glm/               # GLM (git submodule)
    ├── glad/              # GLAD OpenGL loader
    └── json/              # nlohmann/json
```

## Development

### Adding New Features

1. Add header file to `include/`
2. Add source file to `src/`
3. Update `CMakeLists.txt` if needed
4. Rebuild

### Code Style

- Modern C++17
- Follow existing conventions
- Use smart pointers (`unique_ptr`, `shared_ptr`)
- RAII for resource management
- Namespace everything under `eve::`

### Architecture

#### Application Flow

```
main() 
  └─> Application
        ├─> Window (GLFW)
        ├─> Renderer (OpenGL)
        │     ├─> Shaders
        │     ├─> Camera
        │     └─> Models
        ├─> GameClient (Network)
        │     ├─> TCPClient
        │     └─> ProtocolHandler
        └─> InputHandler
```

#### Rendering Pipeline

```
beginFrame()
  └─> Clear screen
renderScene(camera)
  └─> Render starfield
  └─> Render entities
  └─> Render effects
renderHUD()
  └─> Draw UI elements
endFrame()
  └─> Swap buffers
```

## Debugging

### OpenGL Debugging

Enable OpenGL debug output in debug builds:
```cpp
glEnable(GL_DEBUG_OUTPUT);
glDebugMessageCallback(debugCallback, nullptr);
```

### Network Debugging

Enable verbose logging:
```cpp
// In game_client.cpp
#define NETWORK_DEBUG 1
```

### Performance Profiling

Use built-in profiling:
```bash
./eve_client --profile
```

## Platform-Specific Notes

### Windows

- Requires Visual Studio 2017+ or MinGW-w64
- OpenGL 3.3+ drivers should be installed
- Run from build directory to find assets

### Linux

- Install OpenGL development packages:
  ```bash
  # Ubuntu/Debian
  sudo apt-get install libgl1-mesa-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
  
  # Fedora
  sudo dnf install mesa-libGL-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel
  ```

### macOS

- Xcode Command Line Tools required
- OpenGL is deprecated but still works
- Future: Consider Metal backend

## Troubleshooting

### "GLFW failed to initialize"

- Update graphics drivers
- Check OpenGL version: `glxinfo | grep "OpenGL version"` (Linux)

### "Shader compilation failed"

- Check shader syntax
- Verify GLSL version compatibility
- Look at shader info log

### "Could not connect to server"

- Verify server is running
- Check firewall settings
- Confirm correct host/port

### Black screen

- Check OpenGL context creation
- Verify shaders compiled successfully
- Enable debug output

## Performance Tips

1. **VSync**: Enabled by default (60 FPS cap)
2. **LOD System**: Distance-based detail levels
3. **Frustum Culling**: Only render visible entities
4. **Instancing**: Batch similar entities

## Contributing

See main [CONTRIBUTING.md](../CONTRIBUTING.md) in repository root.

## License

[To be determined]

## Credits

- Engine: Custom C++/OpenGL
- Math: GLM library
- Windowing: GLFW
- Inspired by EVE ONLINE (CCP Games)

## Roadmap

### Phase 1: Foundation (Current)
- [x] Project structure
- [ ] Window and OpenGL context
- [ ] Basic rendering
- [ ] Camera system

### Phase 2: Core Features
- [ ] Network client
- [ ] Entity synchronization
- [ ] Ship models
- [ ] Basic HUD

### Phase 3: Polish
- [ ] Particle effects
- [ ] PBR materials
- [ ] Audio system
- [ ] UI improvements

### Phase 4: Optimization
- [ ] LOD system
- [ ] Instanced rendering
- [ ] Multi-threading
- [ ] Performance profiling

## Status

🚧 **Under Active Development** 🚧

The C++ client is being built to replace the Python client with:
- Better performance (10-100x faster)
- Native graphics (OpenGL vs Panda3D)
- Lower memory usage
- Standalone executable (no Python runtime)

---

**Last Updated**: February 2026  
**Version**: 0.1.0-dev
