# EVE OFFLINE

A PVE-focused space MMO inspired by EVE ONLINE, designed for small groups of players (2-20). Built with C++ and OpenGL.

## Project Structure

```
EVEOFFLINE/
├── cpp_client/          # C++ OpenGL game client
│   ├── src/             #   Source code (core, rendering, network, ui, audio)
│   ├── include/         #   Header files
│   ├── shaders/         #   GLSL shader files
│   ├── assets/          #   Game assets (models, textures)
│   └── external/        #   Third-party libraries (stb, etc.)
├── cpp_server/          # C++ dedicated game server
│   ├── src/             #   Source code (ECS, network, systems)
│   ├── include/         #   Header files
│   └── config/          #   Server configuration
├── data/                # Game data (JSON - ships, modules, missions, etc.)
├── docs/                # Documentation
│   ├── guides/          #   Build & setup guides
│   ├── cpp_client/      #   C++ client documentation
│   └── sessions/        #   Development session notes
├── archive/             # Legacy Python prototype (for reference only)
├── CMakeLists.txt       # Root CMake build configuration
├── build.sh             # Unix/macOS build script
├── build.bat            # Windows build script
├── build_vs.bat         # Visual Studio solution generator
└── generate_solution.bat # Root VS solution generator
```

## 🚀 Quick Start

### Prerequisites

- **CMake** 3.15+
- **C++17** compiler (GCC 9+, Clang 10+, or Visual Studio 2019+)
- **Dependencies**: GLFW3, GLM, GLEW, nlohmann-json, OpenAL (optional)

### Building (Linux/macOS)

```bash
# Install dependencies
# Ubuntu/Debian:
sudo apt-get install build-essential cmake libgl1-mesa-dev libglew-dev libglfw3-dev libglm-dev nlohmann-json3-dev libopenal-dev

# macOS:
brew install cmake glfw glm glew nlohmann-json openal-soft

# Build
./build.sh
# or
./build.sh Debug

# Run
cd build/bin
./eve_client "YourName"
```

### Building (Windows - Visual Studio)

**Install dependencies with vcpkg first:**
```cmd
cd C:\
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows
.\vcpkg install imgui[glfw-binding,opengl3-binding]:x64-windows
```

**Build:**
```cmd
build_vs.bat
```

**Or open in Visual Studio:**
```cmd
build_vs.bat --open
```

The executable will be at: `cpp_client\build_vs\bin\Release\eve_client.exe`

For detailed setup instructions, see:
- [Quick Start](docs/guides/QUICKSTART_VS2022.md)
- [Full VS2022 Guide](docs/guides/VS2022_SETUP_GUIDE.md)
- [Troubleshooting](docs/guides/TROUBLESHOOTING_VS2022.md)

### Building (CMake directly)

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## 🎮 Features

### C++ Client (OpenGL)
- **3D Rendering**: OpenGL 3.3+ with deferred rendering, shadow mapping, post-processing
- **EVE-Styled UI**: ImGui-based interface matching EVE Online's Photon UI
- **Audio**: OpenAL spatial audio for weapons, explosions, and engines
- **Networking**: TCP client connecting to the dedicated server
- **Entity System**: Synchronized entity management with interpolation

### C++ Server
- **ECS Architecture**: Entity Component System for game logic
- **Multiplayer**: TCP server supporting multiple concurrent clients
- **AI Systems**: NPC behavior, combat, movement, and targeting
- **Steam Integration**: Optional Steam authentication and server browser
- **Cross-Platform**: Windows, Linux, macOS

### Game Content (data/)
All game content is moddable via JSON files:
- 58+ ships (frigates to titans, Tech I and Tech II, plus capitals)
  - Frigates, Destroyers, Cruisers, Battlecruisers, Battleships
  - Carriers, Dreadnoughts, Titans
  - Procedural 3D models with faction-specific designs
- 70+ modules (weapons, defenses, utilities)
- 47+ skills with training system
- 28 missions across 4 difficulty levels
- Mining, manufacturing, market, and exploration systems
- Stations and asteroids with visual variety

## 🔧 Modding

Edit JSON files in `data/` to customize game content:
```
data/
├── ships/          # Ship definitions
├── modules/        # Module definitions
├── skills/         # Skill definitions
├── npcs/           # NPC definitions
├── missions/       # Mission templates
└── universe/       # Solar system data
```

## 📚 Documentation

All documentation is in [docs/](docs/):
- [Build Guides](docs/guides/) — VS2022, vcpkg, build automation
- [C++ Client Docs](docs/cpp_client/) — Rendering, UI, audio, networking
- [Ship Modeling](docs/SHIP_MODELING.md) — Procedural ship generation system
- [EVE Ship Reference](docs/EVE_SHIP_REFERENCE.md) — Design inspiration from EVE Online
- [API & Design](docs/) — Architecture, roadmap, contributing

## 🤝 Contributing

Contributions are welcome! See [CONTRIBUTING.md](docs/CONTRIBUTING.md).

## 📝 License

[To be determined]

---

**Note**: This is an indie project inspired by EVE ONLINE. It is not affiliated with or endorsed by CCP Games.
