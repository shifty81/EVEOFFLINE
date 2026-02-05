# EVE OFFLINE C++ OpenGL Client

A high-performance C++ client with modern OpenGL graphics for EVE OFFLINE.

## Overview

This is a cross-platform 3D client built with:
- **C++17** - Modern C++ features
- **OpenGL 3.3+** - Core profile graphics
- **GLFW** - Cross-platform windowing
- **GLM** - Mathematics library
- **GLEW** - OpenGL extension loader
- **ImGui** - Immediate mode GUI library
- **OpenAL** - 3D audio (optional)

## Features

### Current Status: 🎉 Phase 3 - Complete! 🎉

Phase 3 is now 100% complete with all advanced rendering features implemented!

**Phase 1: Core Rendering (Complete) ✅**
- [x] Project structure created
- [x] Build system configured (CMake)
- [x] Window management (GLFW)
- [x] OpenGL 3.3+ rendering pipeline
- [x] Shader system
- [x] Camera system (EVE-style orbit)
- [x] Mesh and model system
- [x] Procedural ship models (46 ships × 7 factions)
- [x] PBR materials system
- [x] Particle system (10,000 particles)
- [x] Visual effects (weapons, explosions)
- [x] Health bar rendering
- [x] LOD manager (4 levels)
- [x] Frustum culling
- [x] Instanced rendering
- [x] Texture loading (STB_image)

**Phase 2: Advanced Rendering (Complete) ✅**
- [x] Asteroid field rendering
  - Procedural icosphere-based meshes
  - 4 asteroid sizes (SMALL, MEDIUM, LARGE, HUGE)
  - 2 layout patterns (SEMICIRCLE, SPHERICAL)
  - Efficient instanced rendering
- [x] Dynamic lighting system
  - Multiple light types (directional, point, spot)
  - Up to 16 lights total
  - EVE-style preset lighting
  - Real-time light management

**Phase 3: Shadow Mapping & Post-Processing (Complete) ✅**
- [x] Shadow mapping for directional lights
  - ShadowMap class with configurable resolution (2048x2048)
  - Depth texture framebuffer management
  - Light space matrix calculation
  - PCF (Percentage Closer Filtering) for soft shadows
  - Shadow bias for preventing shadow acne
  - Enhanced shaders (multi_light_shadow.vert/frag)
- [x] Deferred rendering pipeline ✅
  - GBuffer class with 3 color attachments + depth
  - Two-pass rendering (geometry + lighting)
  - Efficient multi-light support: O(lights) instead of O(lights × objects)
  - Full integration with existing LightManager
  - Complete documentation (DEFERRED_RENDERING.md)
- [x] Post-processing effects (bloom, HDR) ✅
  - PostProcessingBuffer class with HDR-capable framebuffers
  - HDR rendering with RGB16F textures
  - Bloom effect with multi-pass gaussian blur
  - Tone mapping (Reinhard, ACES, Uncharted 2)
  - Exposure and gamma correction
  - Complete documentation (POST_PROCESSING.md)
- [x] UI system integration ✅
  - ImGui integration with GLFW + OpenGL3 backend
  - UIManager class for managing UI state and rendering
  - EVE-styled theme and color scheme
  - 4 core HUD panels:
    - Ship Status Panel (shields, armor, hull, capacitor)
    - Target Info Panel (target health, distance, status)
    - Speed Panel (velocity display)
    - Combat Log Panel (scrolling message list)
  - EVE color scheme (dark blue-black backgrounds, teal accents)
  - Test program: test_ui_system
- [x] Audio system (OpenAL) ✅
  - AudioManager class with full 3D spatial audio
  - OpenAL integration (device, context, listener)
  - WAV file loading and playback
  - Sound categories: weapons, explosions, engines, UI, music
  - 3D positional audio with distance attenuation
  - Doppler effect for moving objects
  - Volume controls (master, SFX, music, UI)
  - Source pooling and resource management
  - AudioGenerator for procedural test sounds
  - Complete documentation (AUDIO_SYSTEM.md)
  - Test program: test_audio_system
  - Optional dependency (works without OpenAL)

**Phase 4: Gameplay Integration (In Progress) 🚀**
- [x] **Phase 4.1: Network Client Integration** ✅
  - TCP client with JSON protocol (compatible with Python server)
  - NetworkManager for high-level game integration
  - Message serialization/deserialization (nlohmann/json)
  - Connection state management
  - Test program: test_network
  - Cross-platform (Windows, Linux, macOS)
  - Thread-safe message queue
  - See [PHASE4_NETWORK.md](PHASE4_NETWORK.md) for details
- [x] **Phase 4.2: Entity State Synchronization** ✅
  - Entity class with smooth interpolation (cubic ease-out)
  - EntityManager for entity lifecycle management
  - EntityMessageParser for JSON protocol parsing
  - GameClient integration (NetworkManager + EntityManager)
  - Automatic entity cleanup
  - Event callback system for rendering integration
  - Test program: test_entity_sync (4 test suites, 100% pass rate)
  - See [PHASE4_ENTITY_SYNC.md](PHASE4_ENTITY_SYNC.md) for details
- [x] **Phase 4.3: Renderer Integration** ✅
  - Integrated EntityManager with Renderer
  - Visual entity creation/destruction
  - EVE-style target list UI with circular icons
  - Arc-based health indicators (shield/armor/hull)
  - Ship model selection and rendering
  - See [PHASE4_RENDERER_INTEGRATION.md](PHASE4_RENDERER_INTEGRATION.md) for details
- [x] **Phase 4.4: Game Input** ✅
  - Entity picking via 3D raycasting
  - Click-to-target and CTRL+Click multi-target
  - Tab key for target cycling
  - ESC to clear targets
  - F1-F8 module activation (ready for server integration)
  - W/A/S/D movement keys (placeholders)
  - Full keyboard state tracking and modifiers
  - See [PHASE4_INPUT_SYSTEM.md](PHASE4_INPUT_SYSTEM.md) for details
- [x] **Phase 4.5: Enhanced UI** ✅
  - Inventory management panel (cargo/hangar with transfer/jettison)
  - Ship fitting window (module slots, CPU/PG management)
  - Mission tracker panel (objectives, rewards, completion)
  - See [PHASE4.5_ENHANCED_UI.md](PHASE4.5_ENHANCED_UI.md) for details
- [x] **Phase 4.6: Advanced Features** ✅
  - Drag-and-drop item management (cargo ↔ hangar, jettison)
  - Module browser with search/filter/sort
  - Market interface (browse, order book, quick trade)
  - See [PHASE4.6_ADVANCED_FEATURES.md](PHASE4.6_ADVANCED_FEATURES.md) for details
- [x] **Phase 4.7: Network & Gameplay Integration** ✅
  - Protocol extensions for inventory, fitting, and market
  - NetworkManager methods for all gameplay operations
  - UI callback integration (inventory drag-drop, module activation)
  - Ready for server-side implementation
  - See [PHASE4.7_NETWORK_INTEGRATION.md](PHASE4.7_NETWORK_INTEGRATION.md) for details
- [ ] **Phase 4.8: Server Response Handling** (Next)
  - Implement response message handlers
  - Update UI based on server confirmation
  - Error handling and user feedback
  - Transaction confirmation dialogs

### Planned Features

**Graphics**:
- Modern OpenGL 3.3+ rendering ✅
- EVE-style orbit camera ✅
- Procedural ship models ✅
- Procedural asteroid fields ✅
- Starfield background ✅
- Particle effects (weapons, explosions) ✅
- Physically-based rendering (PBR) ✅
- Dynamic multi-light system ✅
- Shadow mapping ✅
- Deferred rendering ✅

**Networking**:
- TCP connection to dedicated server ✅
- JSON protocol (compatible with Python server) ✅
- Entity state synchronization (in progress)
- Lag compensation (planned)

**Gameplay**:
- Full EVE mechanics (in progress)
- Ship fitting and combat (in progress)
- Skills and progression (planned)
- Missions and exploration (planned)

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
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \     # Debug or Release
  -DBUILD_TESTS=ON \                # Build test suite (default: ON)
  -DUSE_SYSTEM_LIBS=OFF            # Use system libraries instead of bundled (default: OFF)
```

### Running Tests

The project includes several test programs to verify functionality:

#### Asteroid Field Rendering Test
```bash
./cpp_client/build_test_asteroid.sh
./cpp_client/build_test_asteroid/bin/test_asteroid_field
```

Features tested:
- Procedural asteroid generation
- Instanced rendering performance
- Different field layouts (semicircle, spherical)
- Multiple asteroid sizes

#### Dynamic Lighting Test
```bash
./cpp_client/build_test_lighting.sh
./cpp_client/build_test_lighting/bin/test_lighting
```

Features tested:
- Directional lights (sun/star)
- Point lights (omnidirectional)
- Spot lights (cone-shaped)
- Multiple light configurations
- EVE-style lighting preset

Controls:
- `Right Mouse`: Rotate camera
- `Middle Mouse`: Pan camera
- `Mouse Wheel`: Zoom in/out
- `1-5`: Switch between lighting tests
- `ESC`: Exit

#### Other Tests
```bash
./test_frustum_culling      # Frustum culling system
./test_instanced_rendering  # Instanced rendering performance
./test_texture_loading      # Texture loading with STB_image
```

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
