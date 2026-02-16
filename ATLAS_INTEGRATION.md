# Atlas Engine Integration

This document explains the relationship between the **EVEOFFLINE** game project and the **Atlas Engine**, and how the two repositories work together.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    EVEOFFLINE Repository                    │
│                  (This Repo — Game Project)                 │
│                                                             │
│  ┌───────────────────────────────────────────────────────┐  │
│  │              Atlas Engine (engine/)                    │  │
│  │  Generic, game-agnostic engine core                   │  │
│  │  • ECS Framework        • Graph VM + Compiler         │  │
│  │  • Asset System          • Networking (CS + P2P)       │  │
│  │  • Tick Scheduler        • World Layouts               │  │
│  │  • Hot Reload            • Lockstep + Rollback         │  │
│  └───────────────────────────────────────────────────────┘  │
│                          ▲                                  │
│                          │ links                            │
│              ┌───────────┼───────────┐                      │
│              │           │           │                      │
│  ┌───────────┴──┐  ┌─────┴────┐  ┌──┴──────────┐           │
│  │  cpp_client/  │  │ editor/  │  │ cpp_server/  │           │
│  │  Game Client  │  │  Editor  │  │ Game Server  │           │
│  │  (OpenGL)     │  │  (Dev)   │  │ (Dedicated)  │           │
│  └──────────────┘  └──────────┘  └─────────────┘           │
│                                                             │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐    │
│  │  data/   │  │  docs/   │  │  tools/  │  │ archive/ │    │
│  │  (JSON)  │  │          │  │          │  │ (Legacy) │    │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘    │
└─────────────────────────────────────────────────────────────┘
```

## Dual-Repository Strategy

### Current State (This Repository)

The Atlas Engine lives **inside** this repository under the `engine/` directory. This allows:

- Rapid iteration on both engine and game code
- Single build system for everything
- Easy debugging across engine/game boundaries

### Future State (Two Repositories)

The plan is to eventually split into two repositories:

| Repository | Purpose |
|------------|---------|
| **[Atlas](https://github.com/shifty81/Atlas)** | Standalone engine — generic, game-agnostic |
| **EVEOFFLINE** | Game project — uses Atlas as a dependency |

When the split happens:
1. The `engine/` and `editor/` directories move to the Atlas repo
2. This repo pulls Atlas as a submodule or external dependency
3. Game-specific code (`cpp_client/`, `cpp_server/`, `data/`) stays here

## Keeping EVEOFFLINE "Pure" While Atlas Evolves

If you want day-to-day EVEOFFLINE work to stay focused on the game project while Atlas evolves separately, use this workflow:

1. Treat `cpp_client/`, `cpp_server/`, `data/`, `docs/`, and `tools/` as EVEOFFLINE-owned scope
2. Make Atlas engine/framework changes in the Atlas repo first, then sync intentionally
3. Build EVEOFFLINE-only targets when you do not need Atlas engine/editor work:

```bash
mkdir -p build && cd build
cmake .. -DBUILD_CLIENT=ON -DBUILD_SERVER=ON \
         -DBUILD_ATLAS_ENGINE=OFF -DBUILD_ATLAS_TESTS=OFF -DBUILD_ATLAS_EDITOR=OFF
cmake --build . --config Release
```

This keeps EVEOFFLINE gameplay progress moving without coupling every change to Atlas development.

## Directory Structure

```
EVEOFFLINE/
├── engine/                 # ★ Atlas Engine (will move to Atlas repo)
│   ├── core/               #   Engine bootstrap, logging, config
│   ├── ecs/                #   Entity/Component/System framework
│   ├── graphvm/            #   Deterministic Graph VM + compiler
│   ├── assets/             #   Asset registry, binary format, hot reload
│   ├── net/                #   Networking (CS + P2P + lockstep/rollback)
│   ├── sim/                #   Tick scheduler, deterministic simulation
│   └── world/              #   World layouts (cube-sphere, voxel grid)
│
├── editor/                 # ★ Atlas Editor (will move to Atlas repo)
│   ├── ui/                 #   Docking, layout, panel system
│   ├── panels/             #   ECS Inspector, Net Inspector, Console
│   ├── tools/              #   Game Packager, Asset Cooker
│   └── ai/                 #   AI Aggregator for asset generation
│
├── atlas_tests/            # ★ Atlas Engine unit tests
│
├── cpp_client/             # EVEOFFLINE game client (stays here)
├── cpp_server/             # EVEOFFLINE game server (stays here)
├── data/                   # Game data — moddable JSON (stays here)
├── docs/                   # Documentation (stays here)
├── tools/                  # Modding tools (stays here)
└── archive/                # Legacy code (stays here)
```

## Building

### Build Everything

```bash
./build.sh
```

### Build Atlas Engine Only

```bash
make build-engine
```

### Run Atlas Engine Tests

```bash
make test-engine
```

### Build Game Client + Server (without Atlas Editor)

```bash
make build
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_ATLAS_ENGINE` | `ON` | Build the Atlas Engine static library |
| `BUILD_ATLAS_EDITOR` | `OFF` | Build the Atlas Editor executable |
| `BUILD_ATLAS_TESTS` | `ON` | Build and register Atlas Engine tests |
| `BUILD_CLIENT` | `ON` | Build the EVEOFFLINE game client |
| `BUILD_SERVER` | `ON` | Build the EVEOFFLINE game server |

## Atlas Engine Design Principles

1. **Engine Is Not a Game** — The engine contains no gameplay logic; all game-specific code is in `cpp_client/` and `cpp_server/`
2. **Everything Is Data** — All behavior is authored as assets and executable graphs
3. **One Engine, Three Roles** — Editor, Client, and Server share the same engine core
4. **Determinism First** — All simulation is deterministic for networking and replay
5. **Hot-Reloadable Assets** — Zero-rebuild iteration during development

## Atlas Engine Components

### ECS Framework (`engine/ecs/`)
- Entity creation/destruction
- Type-safe component management (add, get, has, remove)
- Tick callbacks for system updates

### Graph VM (`engine/graphvm/`)
- Stack-based bytecode virtual machine
- Opcodes: arithmetic, comparison, branching, events
- Graph compiler: converts visual graph IR to bytecode
- Deterministic execution for networking

### Asset System (`engine/assets/`)
- Binary asset format (`.atlasb`) with magic number validation
- Asset registry with filesystem scanning
- Hot reload with file change detection
- Serialization utilities

### Networking (`engine/net/`)
- Client-Server and Peer-to-Peer modes
- Packet send/receive with type-tagged messages
- Peer management (add, remove, RTT tracking)
- Lockstep/rollback support with world snapshots
- Authority model (server/host authoritative)

### Simulation (`engine/sim/`)
- Fixed tick-rate scheduler
- Deterministic delta time
- Tick counting

### World Generation (`engine/world/`)
- Abstract WorldLayout interface
- Cube-sphere layout for planetary surfaces
- Voxel grid layout for block worlds
- LOD support, chunk neighbor queries, streaming hints

## Syncing with Atlas Repository

The `engine/` and `editor/` directories in this repo mirror the Atlas repo structure. To sync changes:

```bash
# From Atlas repo, copy engine changes
cp -r Atlas/engine/* EVEOFFLINE/engine/
cp -r Atlas/editor/* EVEOFFLINE/editor/

# Or from EVEOFFLINE to Atlas
cp -r EVEOFFLINE/engine/* Atlas/engine/
cp -r EVEOFFLINE/editor/* Atlas/editor/
```

Once the split is complete, this will be replaced with a git submodule or CMake `FetchContent`.

## Related Documentation

- [Atlas Engine README](https://github.com/shifty81/Atlas/blob/main/README.md)
- [Atlas Development Roadmap](https://github.com/shifty81/Atlas/blob/main/docs/09_DEVELOPMENT_ROADMAP.md)
- [Atlas Future Plans](https://github.com/shifty81/EVEOFFLINE/commit/033f2f8e222e7bfa0f853123ed902c23d98c307a) — Detailed design discussion covering WorldGraph APIs, API cleanup, editor tools, and documentation tracks
- [Atlas Contributing Guide](https://github.com/shifty81/Atlas/blob/main/CONTRIBUTING.md)
- [EVEOFFLINE Roadmap](docs/ROADMAP.md)
- [EVEOFFLINE Modding Guide](docs/MODDING_GUIDE.md)
- [EVEOFFLINE Development Guidance](docs/DEVELOPMENT_GUIDANCE.md)
