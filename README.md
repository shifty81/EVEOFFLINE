# EVE OFFLINE

A PVE-focused space MMO heavily inspired by EVE Online, designed for small groups of players (2-20) or solo play with AI companions. Built with C++ and OpenGL. Features fleet/party systems supporting both AI and human players in cooperative PVE content.

> **Note**: This project is heavily based on EVE Online's game mechanics and systems but uses entirely original naming conventions for all in-game content (ships, factions, resources, systems, etc.). All gameplay focuses on PVE and cooperative fleet content — no PVP.

> **Status**: In active R&D and development — actively testing until further notice.

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
- **Sci-Fi Styled UI**: ImGui-based interface inspired by EVE Online's Photon UI
- **Audio**: OpenAL spatial audio for weapons, explosions, and engines
- **Networking**: TCP client connecting to the dedicated server
- **Entity System**: Synchronized entity management with interpolation

### C++ Server
- **ECS Architecture**: Entity Component System for game logic
- **Multiplayer**: TCP server supporting multiple concurrent clients
- **AI Systems**: NPC behavior, combat, movement, and targeting — AI pilots can fill fleet roles
- **Steam Integration**: Optional Steam authentication and server browser
- **Cross-Platform**: Windows, Linux, macOS

### Game Content (data/)
All game content is moddable via JSON files:
- 102+ ships (frigates to titans, Tech I and Tech II, plus capitals)
  - Frigates, Destroyers, Cruisers, Battlecruisers, Battleships
  - Interceptors, Covert Ops, Assault Frigates, Stealth Bombers, Marauders
  - Interdictors, Command Ships, Logistics Cruisers, Recon Ships
  - Carriers, Dreadnoughts, Titans
  - Industrial haulers, Mining Barges, Exhumers
  - Procedural 3D models with faction-specific designs
- 159+ modules (weapons, defenses, utilities, Tech II, Faction, Officer)
- 137 skills across 20 categories with attribute-based training
- Missions across 5 levels with 7 types (combat, mining, courier, trade, scenario, exploration, storyline)
- 4 playable factions: Solari, Veyren, Aurelian, Keldari
- Character creation with races, bloodlines, and attributes
- Clone system, implants, and Learning skills
- AEGIS security enforcement and insurance
- Corporation system with NPC and player corps
- Contract/escrow system
- Deadspace complexes with 5 difficulty tiers
- Mining, manufacturing, market, and exploration systems
- Fleet/party system with AI or player wingmates
- Stations and asteroids with visual variety

## 🔧 Modding

Edit JSON files in `data/` to customize game content:
```
data/
├── character_creation/ # Races, bloodlines, clones, implants
├── ships/              # Ship definitions
├── modules/            # Module definitions
├── skills/             # Skill definitions
├── npcs/               # NPC definitions
├── missions/           # Mission templates
├── universe/           # Solar system data
├── security/           # AEGIS security, insurance
├── corporations/       # NPC and player corps
├── contracts/          # Contract/escrow system
├── exploration/        # Signatures, deadspace complexes
├── industry/           # Blueprints, manufacturing
├── market/             # Pricing system
├── asteroid_fields/    # Mining belt data
└── planetary_interaction/ # PI resources
```

## 📚 Documentation

All documentation is in [docs/](docs/):

**Getting Started**
- [Tutorial](docs/TUTORIAL.md) — New player guide: controls, combat, skills, ISK making
- [Modding Guide](docs/MODDING_GUIDE.md) — Create custom ships, modules, and missions
- [Build Guides](docs/guides/) — VS2022, vcpkg, build automation

**Development**
- [Roadmap](docs/ROADMAP.md) — Development progress and plans
- [Next Tasks](docs/NEXT_TASKS.md) — Recommendations for upcoming work
- [Design Document](docs/design/DESIGN.md) — Game systems design

**Technical**
- [C++ Client Docs](docs/cpp_client/) — Rendering, UI, audio, networking
- [Ship Modeling](docs/SHIP_MODELING.md) — Procedural ship generation system
- [Standings System](docs/STANDINGS_SYSTEM.md) — NPC relationships and faction standings

## 🤝 Contributing

Contributions are welcome! See [CONTRIBUTING.md](docs/CONTRIBUTING.md).

## 📝 License

[To be determined]

---

**Note**: This is an indie PVE space MMO project heavily inspired by EVE Online's game mechanics. It is not affiliated with or endorsed by CCP Games. All in-game content (ships, factions, resources, systems, etc.) uses original naming conventions. The game focuses exclusively on PVE and cooperative fleet content with AI or player wingmates.
