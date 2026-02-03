# C++ Client Architecture Design

## Overview

This document outlines the architecture for transitioning the EVEOFFLINE client from Python to C++, including:
1. Standalone executable client
2. Embedded server capability (optional in-game toggle)
3. Dedicated server remains standalone
4. Support for player-hosted multiplayer sessions

## Current Architecture

### Python-Based System
```
┌─────────────────────┐     ┌─────────────────────┐
│  Python 3D Client   │────▶│  Python Server      │
│  (Panda3D)          │     │  (asyncio)          │
│  - Rendering        │     │  - Game Logic       │
│  - UI (EVE-styled)  │     │  - State Management │
│  - Network Client   │     │  - Player Sessions  │
└─────────────────────┘     └─────────────────────┘
```

**Pros:**
- Rapid development
- Easy prototyping
- Good for testing

**Cons:**
- Performance limitations
- Distribution requires Python runtime
- Harder to protect assets
- No native executable

## Proposed C++ Architecture

### Three-Tier System

```
┌──────────────────────────────────────────────────────┐
│              C++ Client Executable                   │
│  ┌────────────────────┐  ┌──────────────────────┐  │
│  │   Client Mode      │  │   Host Mode          │  │
│  │   - Rendering      │  │   - Rendering        │  │
│  │   - UI             │  │   - UI               │  │
│  │   - Network Client │  │   - Network Client   │  │
│  │                    │  │   - Embedded Server  │  │
│  └────────────────────┘  └──────────────────────┘  │
└───────────────┬──────────────────┬──────────────────┘
                │                  │
                ▼                  ▼
       ┌─────────────────┐  ┌─────────────────┐
       │ Dedicated Server│  │  Other Players  │
       │ (Python/C++)    │  │  (C++ Clients)  │
       └─────────────────┘  └─────────────────┘
```

## Technology Stack

### Core Technologies

#### 1. Rendering Engine: Panda3D (C++ API)
- **Why**: Already using Panda3D, has native C++ support
- **Features**:
  - Scene graph management
  - 3D rendering
  - Shader support
  - Physics integration
  - Cross-platform

#### 2. UI Framework: Dear ImGui or Custom
- **Option A: Dear ImGui**
  - Pros: Immediate mode, easy to integrate, performant
  - Cons: Different from current DirectGui style
  
- **Option B: Custom EVE-styled UI**
  - Pros: Maintain exact EVE look
  - Cons: More development time

#### 3. Networking: Boost.Asio or ASIO
- **Why**: High-performance async I/O, similar to Python's asyncio
- **Features**:
  - TCP/UDP support
  - Async operations
  - Cross-platform
  - Well-documented

#### 4. Serialization: JSON (nlohmann/json) or MessagePack
- **Option A: nlohmann/json**
  - Pros: Easy to use, compatible with Python server
  - Cons: Text-based (larger messages)
  
- **Option B: MessagePack**
  - Pros: Binary, faster, smaller
  - Cons: Need to update Python server too

#### 5. Build System: CMake
- **Why**: Industry standard, cross-platform
- **Features**:
  - Multi-platform builds
  - Dependency management
  - Package support (vcpkg, Conan)

#### 6. Threading: std::thread or Boost.Thread
- **Why**: Server runs in separate thread
- **Features**:
  - Game logic thread
  - Network thread
  - Render thread

### Additional Libraries

```cmake
# Core
- Panda3D (3D engine)
- Boost or standalone ASIO (networking)
- nlohmann/json (JSON parsing)

# Optional
- spdlog (logging)
- fmt (string formatting)
- catch2 (unit testing)
- ImGui (UI)
- SDL2 (input, if not using Panda3D's)
```

## Architecture Components

### 1. Client Application

```cpp
class EVEOfflineClient {
private:
    // Core systems
    std::unique_ptr<RenderSystem> renderer;
    std::unique_ptr<UISystem> ui;
    std::unique_ptr<NetworkClient> network;
    std::unique_ptr<AudioSystem> audio;
    
    // Optional embedded server
    std::unique_ptr<EmbeddedServer> embedded_server;
    bool hosting_mode = false;
    
public:
    void initialize();
    void run();
    void shutdown();
    
    // Server hosting
    void startHosting(uint16_t port);
    void stopHosting();
    bool isHosting() const;
};
```

### 2. Embedded Server

```cpp
class EmbeddedServer {
private:
    std::unique_ptr<GameWorld> world;
    std::unique_ptr<NetworkServer> network;
    std::thread server_thread;
    std::atomic<bool> running;
    
public:
    void start(uint16_t port);
    void stop();
    void tick(float dt);  // Called from server thread
    
    // Integration with client
    GameState getState() const;
    void addLocalPlayer(PlayerID id);
};
```

### 3. Network Layer

```cpp
// Client networking
class NetworkClient {
private:
    asio::io_context io_context;
    asio::ip::tcp::socket socket;
    std::thread network_thread;
    
public:
    void connect(const std::string& host, uint16_t port);
    void disconnect();
    void sendMessage(const Message& msg);
    void update();  // Process incoming messages
};

// Server networking (embedded or dedicated)
class NetworkServer {
private:
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor;
    std::vector<std::shared_ptr<ClientSession>> clients;
    
public:
    void start(uint16_t port);
    void stop();
    void broadcast(const Message& msg);
    void tick();
};
```

### 4. Game State Management

```cpp
// Shared between client and embedded server
class GameWorld {
private:
    EntityManager entities;
    std::unordered_map<EntityID, Entity> entity_map;
    
public:
    void update(float dt);
    void addEntity(const Entity& entity);
    void removeEntity(EntityID id);
    const Entity* getEntity(EntityID id) const;
    
    // Serialization for networking
    json serialize() const;
    void deserialize(const json& data);
};
```

## Directory Structure

```
EVEOFFLINE_CPP/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── client/
│   │   ├── main.cpp
│   │   ├── client_app.h/cpp
│   │   ├── rendering/
│   │   │   ├── renderer.h/cpp
│   │   │   ├── camera.h/cpp
│   │   │   ├── effects.h/cpp
│   │   │   └── models.h/cpp
│   │   ├── ui/
│   │   │   ├── ui_system.h/cpp
│   │   │   ├── eve_hud.h/cpp
│   │   │   ├── panels.h/cpp
│   │   │   └── styles.h/cpp
│   │   ├── audio/
│   │   │   └── audio_system.h/cpp
│   │   └── input/
│   │       └── input_manager.h/cpp
│   ├── network/
│   │   ├── client/
│   │   │   └── network_client.h/cpp
│   │   ├── server/
│   │   │   ├── network_server.h/cpp
│   │   │   └── client_session.h/cpp
│   │   ├── protocol/
│   │   │   ├── messages.h/cpp
│   │   │   └── serialization.h/cpp
│   │   └── common/
│   │       └── connection.h/cpp
│   ├── server/
│   │   ├── embedded_server.h/cpp
│   │   ├── game_world.h/cpp
│   │   ├── entity_manager.h/cpp
│   │   └── systems/
│   │       ├── combat_system.h/cpp
│   │       ├── movement_system.h/cpp
│   │       └── ai_system.h/cpp
│   ├── shared/
│   │   ├── entity.h/cpp
│   │   ├── components.h/cpp
│   │   ├── math_utils.h/cpp
│   │   └── types.h
│   └── utils/
│       ├── logger.h/cpp
│       └── config.h/cpp
├── include/
│   └── eve_offline/
│       └── (public headers if needed)
├── tests/
│   ├── test_client.cpp
│   ├── test_server.cpp
│   └── test_network.cpp
├── assets/
│   ├── models/
│   ├── textures/
│   ├── sounds/
│   └── shaders/
├── data/
│   ├── ships/
│   ├── modules/
│   └── universe/
├── external/
│   └── (third-party dependencies)
└── build/
    └── (generated build files)
```

## Implementation Phases

### Phase 1: Foundation (2-3 weeks)
**Goal**: Basic C++ client that can render a scene

1. **Week 1: Project Setup**
   - ✅ Create CMake build system
   - ✅ Integrate Panda3D C++ API
   - ✅ Set up dependencies (Boost.Asio, nlohmann/json)
   - ✅ Create basic project structure
   - ✅ Set up logging system

2. **Week 2: Core Systems**
   - ✅ Implement basic rendering system
   - ✅ Create camera system
   - ✅ Port starfield rendering
   - ✅ Basic entity rendering

3. **Week 3: Testing & Validation**
   - ✅ Create test scenes
   - ✅ Verify Panda3D integration
   - ✅ Test cross-platform builds

**Deliverable**: Executable that renders a 3D scene with camera controls

### Phase 2: Networking (2-3 weeks)
**Goal**: Client can connect to existing Python server

1. **Week 4: Network Client**
   - ✅ Implement TCP client with Boost.Asio
   - ✅ Create message serialization/deserialization
   - ✅ Port protocol from Python
   - ✅ Handle connection/disconnection

2. **Week 5: State Synchronization**
   - ✅ Receive entity updates from server
   - ✅ Update local game state
   - ✅ Handle player input
   - ✅ Send commands to server

3. **Week 6: Testing**
   - ✅ Test with Python dedicated server
   - ✅ Verify multiplayer functionality
   - ✅ Performance testing

**Deliverable**: C++ client that can connect to Python server and play

### Phase 3: UI System (3-4 weeks)
**Goal**: Port EVE-styled UI to C++

1. **Week 7-8: Core UI**
   - ✅ Choose UI framework (ImGui or custom)
   - ✅ Implement EVE color scheme
   - ✅ Create panel system
   - ✅ Port basic HUD elements

2. **Week 9-10: Advanced UI**
   - ✅ Port capacitor display
   - ✅ Port health rings
   - ✅ Complete all panels
   - ✅ Polish and styling

**Deliverable**: Full EVE-styled UI in C++

### Phase 4: Embedded Server (3-4 weeks)
**Goal**: Client can host its own server

1. **Week 11-12: Server Core**
   - ✅ Implement game world logic
   - ✅ Create entity management system
   - ✅ Port game systems (combat, movement, AI)
   - ✅ Threading architecture

2. **Week 13-14: Integration**
   - ✅ Integrate embedded server with client
   - ✅ In-game UI for hosting
   - ✅ Local player handling
   - ✅ Testing with multiple clients

**Deliverable**: Client can host server and allow others to connect

### Phase 5: Polish & Optimization (2-3 weeks)
**Goal**: Production-ready executable

1. **Week 15-16: Optimization**
   - ✅ Profile and optimize performance
   - ✅ Memory leak detection
   - ✅ Network optimization
   - ✅ Threading improvements

2. **Week 17: Packaging**
   - ✅ Create installer/package
   - ✅ Asset bundling
   - ✅ Distribution preparation
   - ✅ Documentation

**Deliverable**: Polished, distributable executable

## Key Design Decisions

### 1. Threading Model

```
Main Thread (60 FPS):
- Rendering
- UI updates
- Input handling
- Audio

Network Thread:
- Message sending/receiving
- Connection management
- Non-blocking I/O

Server Thread (Optional, 30 TPS):
- Game world updates
- Physics simulation
- AI processing
- State synchronization
```

### 2. Memory Management

- **Smart Pointers**: Use `std::unique_ptr` and `std::shared_ptr`
- **RAII**: All resources managed via RAII
- **Pool Allocators**: For frequently created/destroyed objects (entities, messages)
- **Asset Management**: Reference counting for models, textures, sounds

### 3. Cross-Platform Support

**Primary Targets**:
- Windows (MSVC, MinGW)
- Linux (GCC, Clang)
- macOS (Clang) - optional

**Build System**:
```cmake
cmake_minimum_required(VERSION 3.15)
project(EVEOffline CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific settings
if(WIN32)
    # Windows settings
elseif(UNIX AND NOT APPLE)
    # Linux settings
elseif(APPLE)
    # macOS settings
endif()
```

### 4. Compatibility with Python Server

**During Transition**:
- C++ client can connect to Python dedicated server
- Use same JSON protocol
- Same entity IDs and types
- Gradual migration

**Long Term**:
- Option to port dedicated server to C++ too
- Or keep Python server for flexibility
- Protocol remains compatible

## Migration Strategy

### Gradual Migration Path

1. **Phase 1**: C++ client + Python server
   - Lowest risk
   - Validate C++ client works
   - Python server unchanged

2. **Phase 2**: Add embedded server
   - C++ client with optional hosting
   - Python dedicated server still available
   - Players choose which to use

3. **Phase 3**: Optional C++ dedicated server
   - Port server to C++ if needed
   - Better performance
   - Easier deployment

### Maintaining Python Code

- Keep Python codebase for:
  - Modding tools
  - Content creation
  - Testing
  - Rapid prototyping
  
- Use Python for:
  - Data file generation
  - Asset pipeline
  - Development tools

## User Experience

### Client Startup Flow

```
┌─────────────────────┐
│  Launch Executable  │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│   Main Menu         │
│  - Play (Client)    │◄─┐
│  - Host Server      │  │
│  - Settings         │  │
│  - Exit             │  │
└──────────┬──────────┘  │
           │             │
   ┌───────┴────────┐    │
   ▼                ▼    │
┌─────────┐    ┌──────────────┐
│ Client  │    │ Host Server  │
│ Mode    │    │ - Start on   │
│         │    │   port XXXX  │
│- Connect│    │- Play locally│
│  to     │    │- Display IP  │
│  server │    │- Player list │
└─────────┘    └──────────────┘
                      │
                      └──────────┘
```

### In-Game Server Hosting UI

```
┌──────────────────────────────┐
│  Server Hosting Panel        │
├──────────────────────────────┤
│ Status: [HOSTING]  [STOP]    │
│                              │
│ Server IP: 192.168.1.100     │
│ Port: 8765                   │
│                              │
│ Connected Players: 3/8       │
│ - Player1 (You)              │
│ - Player2                    │
│ - Player3                    │
│                              │
│ [ Invite Link ]              │
└──────────────────────────────┘
```

## Performance Targets

### Client Performance
- **Frame Rate**: 60 FPS minimum (1080p)
- **Memory**: < 1GB RAM usage
- **Startup**: < 5 seconds cold start
- **Network**: < 100ms latency handling

### Embedded Server Performance
- **Tick Rate**: 30 TPS
- **Players**: Support 8-16 concurrent players
- **Memory**: < 512MB additional for server
- **CPU**: < 25% additional CPU usage

## Security Considerations

### Client-Side
- Input validation
- Cheat prevention (basic)
- Secure connection options (TLS)
- Asset protection

### Server-Side (Embedded)
- Authority model (server is authoritative)
- Player validation
- Rate limiting
- Anti-cheat measures

## Distribution & Packaging

### Windows
```
EVEOffline-Setup.exe
├── eve_offline.exe
├── panda3d.dll
├── boost_asio.dll
├── assets/
└── data/
```

### Linux
```
eve-offline.tar.gz or .deb/.rpm
├── bin/eve_offline
├── lib/
│   ├── libpanda3d.so
│   └── libboost_asio.so
├── share/eve_offline/
│   ├── assets/
│   └── data/
└── eve_offline.desktop
```

### macOS
```
EVEOffline.app
└── Contents/
    ├── MacOS/eve_offline
    ├── Frameworks/
    ├── Resources/
    │   ├── assets/
    │   └── data/
    └── Info.plist
```

## Risks & Mitigations

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| Panda3D C++ API issues | High | Medium | Prototype early, have fallback plan |
| Performance worse than Python | Medium | Low | Profile continuously, optimize early |
| Cross-platform issues | High | Medium | Test on all platforms regularly |
| Scope creep | High | High | Stick to phases, MVP first |
| Team C++ expertise | Medium | Varies | Training, code reviews, documentation |

## Success Criteria

### Minimum Viable Product (MVP)
- ✅ Executable runs on Windows/Linux
- ✅ Renders game world at 60 FPS
- ✅ Can connect to Python server
- ✅ Basic EVE-styled UI
- ✅ Can host embedded server (optional)
- ✅ 2-4 players can connect

### Full Release
- ✅ All features from Python client
- ✅ Complete EVE-styled UI
- ✅ Stable embedded server (8+ players)
- ✅ Cross-platform support
- ✅ Installer/packaging
- ✅ Performance better than Python

## Conclusion

This architecture provides:
1. **Clear migration path** from Python to C++
2. **Flexible deployment** (client-only or client+server)
3. **Backward compatibility** with Python server
4. **Professional executable** for distribution
5. **Player-hosted multiplayer** support

**Estimated Total Time**: 15-20 weeks for MVP, 20-25 weeks for full release

**Next Steps**:
1. Review and approve architecture
2. Set up development environment
3. Create CMake project structure
4. Begin Phase 1 implementation
