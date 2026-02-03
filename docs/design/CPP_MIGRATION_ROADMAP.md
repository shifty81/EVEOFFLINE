# C++ Client Migration - Implementation Roadmap

## Executive Summary

This document provides a detailed, actionable roadmap for migrating the EVEOFFLINE client from Python to C++ with embedded server capabilities.

**Timeline**: 15-25 weeks  
**Complexity**: High  
**Risk**: Medium-High  
**Value**: High (better performance, easier distribution, professional product)

## Immediate Next Steps (Week 1)

### 1. Environment Setup

```bash
# Install required tools
# Windows:
- Visual Studio 2019/2022 with C++ tools
- CMake 3.15+
- vcpkg (package manager)

# Linux:
sudo apt install build-essential cmake libgl1-mesa-dev libx11-dev

# macOS:
xcode-select --install
brew install cmake
```

### 2. Install Dependencies

```bash
# Using vcpkg (recommended)
vcpkg install panda3d boost-asio nlohmann-json spdlog

# Or manual installation
# Panda3D: https://www.panda3d.org/download/
# Boost: https://www.boost.org/
# nlohmann/json: https://github.com/nlohmann/json
```

### 3. Create Initial Project Structure

```bash
cd /path/to/EVEOFFLINE
mkdir -p cpp_client/{src,include,tests,assets,external}
cd cpp_client
```

Create `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.15)
project(EVEOfflineClient VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find packages
find_package(Panda3D REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

# Executable
add_executable(eve_offline ${SOURCES} ${HEADERS})

target_include_directories(eve_offline 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${Panda3D_INCLUDE_DIRS}
)

target_link_libraries(eve_offline 
    PRIVATE 
        ${Panda3D_LIBRARIES}
        Boost::system
        Boost::thread
        nlohmann_json::nlohmann_json
        spdlog::spdlog
)

# Copy assets to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
```

### 4. Create Minimal "Hello World" Client

`src/main.cpp`:

```cpp
#include <iostream>
#include <pandaFramework.h>
#include <pandaSystem.h>
#include <load_prc_file.h>

int main(int argc, char *argv[]) {
    std::cout << "EVE OFFLINE - C++ Client\n";
    std::cout << "Version: 0.1.0\n";
    
    // Initialize Panda3D
    PandaFramework framework;
    framework.open_framework(argc, argv);
    framework.set_window_title("EVE OFFLINE");
    
    // Open window
    WindowFramework *window = framework.open_window();
    if (!window) {
        std::cerr << "Failed to open window!\n";
        return 1;
    }
    
    std::cout << "Panda3D initialized successfully!\n";
    std::cout << "Press Ctrl+C to exit\n";
    
    // Main loop
    framework.main_loop();
    framework.close_framework();
    
    return 0;
}
```

### 5. Build and Test

```bash
mkdir build && cd build
cmake ..
cmake --build .
./eve_offline  # or eve_offline.exe on Windows
```

**Expected Output**: Window opens with empty scene

## Phase 1: Core Client (Weeks 2-4)

### Week 2: Rendering Foundation

**Goal**: Render a star field and basic camera controls

#### Tasks

1. **Create Render System** (`src/client/rendering/renderer.h/cpp`)

```cpp
class RenderSystem {
private:
    PandaFramework& framework;
    WindowFramework* window;
    NodePath render;
    NodePath camera;
    
public:
    RenderSystem(PandaFramework& fw);
    ~RenderSystem();
    
    void initialize();
    void createStarField(int num_stars);
    void update(float dt);
    
    NodePath& getRender() { return render; }
    NodePath& getCamera() { return camera; }
};
```

2. **Create Camera System** (`src/client/rendering/camera.h/cpp`)

```cpp
class CameraSystem {
private:
    NodePath camera;
    LVector3 target_pos;
    float distance;
    float yaw, pitch;
    
public:
    CameraSystem(NodePath cam);
    
    void setTarget(const LVector3& pos);
    void setDistance(float dist);
    void rotate(float dyaw, float dpitch);
    void zoom(float delta);
    void update(float dt);
};
```

3. **Port Star Field** (`src/client/rendering/starfield.h/cpp`)
   - Copy logic from Python `starfield.py`
   - Use Panda3D's GeomNode API in C++
   - Create point sprites for stars

**Deliverable**: Executable with star field and camera controls

### Week 3: Entity System

**Goal**: Render ships and other entities

#### Tasks

1. **Create Entity Manager** (`src/shared/entity_manager.h/cpp`)

```cpp
struct Entity {
    uint64_t id;
    std::string name;
    LVector3 position;
    LVector3 velocity;
    LQuaternion rotation;
    std::string model_type;
    
    // Health
    float shield_cur, shield_max;
    float armor_cur, armor_max;
    float hull_cur, hull_max;
};

class EntityManager {
private:
    std::unordered_map<uint64_t, Entity> entities;
    
public:
    void addEntity(const Entity& entity);
    void removeEntity(uint64_t id);
    void updateEntity(uint64_t id, const Entity& data);
    const Entity* getEntity(uint64_t id) const;
    
    std::vector<const Entity*> getAllEntities() const;
};
```

2. **Create Ship Renderer** (`src/client/rendering/ship_renderer.h/cpp`)
   - Load ship models
   - Port procedural ship generation
   - Render entities in 3D space

3. **Integration Testing**
   - Create test scene with multiple ships
   - Verify rendering performance

**Deliverable**: Ships rendered in 3D space

### Week 4: Input and Controls

**Goal**: Keyboard and mouse input handling

#### Tasks

1. **Input Manager** (`src/client/input/input_manager.h/cpp`)

```cpp
class InputManager {
private:
    PandaFramework& framework;
    std::unordered_map<std::string, bool> key_states;
    LVector2 mouse_pos;
    
public:
    InputManager(PandaFramework& fw);
    
    void initialize();
    void update();
    
    bool isKeyPressed(const std::string& key) const;
    bool isKeyDown(const std::string& key) const;
    LVector2 getMousePos() const;
    LVector2 getMouseDelta();
};
```

2. **Camera Controls Integration**
   - W/A/S/D for camera movement
   - Mouse drag for rotation
   - Mouse wheel for zoom

**Deliverable**: Fully interactive 3D scene

## Phase 2: Networking (Weeks 5-7)

### Week 5: Network Client

**Goal**: Connect to Python server

#### Tasks

1. **Protocol Definition** (`src/network/protocol/messages.h`)

```cpp
enum class MessageType {
    CONNECT,
    DISCONNECT,
    ENTITY_UPDATE,
    PLAYER_COMMAND,
    CHAT_MESSAGE,
    // ... more types
};

struct Message {
    MessageType type;
    std::string payload;  // JSON string
    
    nlohmann::json toJson() const;
    static Message fromJson(const nlohmann::json& j);
};
```

2. **TCP Client** (`src/network/client/network_client.h/cpp`)

```cpp
class NetworkClient {
private:
    asio::io_context io_context;
    asio::ip::tcp::socket socket;
    std::thread network_thread;
    std::atomic<bool> connected;
    
    std::queue<Message> incoming_messages;
    std::queue<Message> outgoing_messages;
    std::mutex message_mutex;
    
public:
    NetworkClient();
    ~NetworkClient();
    
    void connect(const std::string& host, uint16_t port);
    void disconnect();
    void send(const Message& msg);
    
    bool hasMessages() const;
    Message popMessage();
    
private:
    void networkThread();
    void asyncRead();
    void asyncWrite(const Message& msg);
};
```

3. **Connection Testing**
   - Test with existing Python server
   - Verify message serialization
   - Handle disconnection gracefully

**Deliverable**: Client can connect and exchange messages

### Week 6: State Synchronization

**Goal**: Receive entity updates and display them

#### Tasks

1. **Message Handler** (`src/client/message_handler.h/cpp`)

```cpp
class MessageHandler {
private:
    EntityManager& entities;
    RenderSystem& renderer;
    
public:
    MessageHandler(EntityManager& em, RenderSystem& rs);
    
    void handleMessage(const Message& msg);
    void handleEntityUpdate(const nlohmann::json& data);
    void handlePlayerCommand(const nlohmann::json& data);
};
```

2. **Entity Interpolation**
   - Smooth entity movement between updates
   - Predictive positioning
   - Lag compensation

3. **Integration**
   - Main loop processes messages
   - Updates entities
   - Renders scene

**Deliverable**: Multiplayer works with Python server

### Week 7: Player Input

**Goal**: Send player commands to server

#### Tasks

1. **Command System** (`src/client/command_system.h/cpp`)

```cpp
class CommandSystem {
private:
    NetworkClient& network;
    
public:
    CommandSystem(NetworkClient& net);
    
    void moveCommand(const LVector3& direction);
    void targetCommand(uint64_t entity_id);
    void fireCommand(uint64_t target_id);
    void warpCommand(const LVector3& destination);
};
```

2. **Input to Command Mapping**
   - Keyboard/mouse → game commands
   - Send to server
   - Await server response

**Deliverable**: Full client-server gameplay

## Phase 3: UI System (Weeks 8-11)

### Weeks 8-9: Choose and Implement UI Framework

**Decision Point**: ImGui vs Custom

#### Option A: Dear ImGui (Faster)

```cpp
class UISystem {
private:
    ImGuiContext* context;
    
public:
    void initialize(WindowFramework* window);
    void render();
    void shutdown();
    
    // EVE-styled panels
    void renderShipStatus(const Entity& ship);
    void renderTargetInfo(const Entity* target);
    void renderCombatLog(const std::vector<std::string>& messages);
};
```

#### Option B: Custom EVE UI (More Authentic)

```cpp
class EVEUISystem {
private:
    NodePath ui_root;
    std::vector<std::unique_ptr<UIPanel>> panels;
    
public:
    void initialize(NodePath aspect2d);
    void createPanel(const PanelConfig& config);
    void render(float dt);
    
    // Specific panels
    ShipStatusPanel* getShipStatus();
    TargetInfoPanel* getTargetInfo();
};
```

### Weeks 10-11: Implement All UI Panels

1. Ship Status Panel
2. Target Info Panel
3. Navigation Panel
4. Combat Log
5. Overview Panel
6. Neocom Sidebar
7. Capacitor Display (with geometry)
8. Health Rings (with geometry)

**Deliverable**: Complete EVE-styled UI in C++

## Phase 4: Embedded Server (Weeks 12-15)

### Week 12: Server Core

**Goal**: Game logic runs in separate thread

#### Tasks

1. **Game World** (`src/server/game_world.h/cpp`)

```cpp
class GameWorld {
private:
    EntityManager entities;
    std::vector<uint64_t> players;
    float tick_rate = 30.0f;
    
public:
    void initialize();
    void tick(float dt);
    void addPlayer(uint64_t player_id);
    void removePlayer(uint64_t player_id);
    
    // Game systems
    void updateMovement(float dt);
    void updateCombat(float dt);
    void updateAI(float dt);
};
```

2. **Threading Architecture**

```cpp
class EmbeddedServer {
private:
    GameWorld world;
    NetworkServer network;
    std::thread server_thread;
    std::atomic<bool> running;
    
public:
    void start(uint16_t port);
    void stop();
    
private:
    void serverLoop();  // Runs in separate thread
};
```

**Deliverable**: Server runs in background

### Week 13: Network Server

**Goal**: Accept client connections

#### Tasks

1. **TCP Server** (`src/network/server/network_server.h/cpp`)

```cpp
class NetworkServer {
private:
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor;
    std::vector<std::shared_ptr<ClientSession>> clients;
    
public:
    void start(uint16_t port);
    void stop();
    void broadcast(const Message& msg);
    void sendTo(uint64_t client_id, const Message& msg);
    
private:
    void asyncAccept();
};

class ClientSession {
private:
    asio::ip::tcp::socket socket;
    uint64_t player_id;
    
public:
    void start();
    void send(const Message& msg);
    void asyncRead();
};
```

**Deliverable**: Server accepts connections

### Week 14: Integration

**Goal**: Client hosts server and plays

#### Tasks

1. **UI for Hosting**
   - "Host Server" button in main menu
   - Display server status
   - Show connected players
   - Stop server button

2. **Local Player Integration**
   - Client connects to localhost
   - Seamless host+play experience
   - No network lag for host

3. **Testing**
   - Multiple clients connect to hosted server
   - Verify gameplay works
   - Test disconnect/reconnect

**Deliverable**: Working host+play mode

### Week 15: Server Systems

**Goal**: Port game logic from Python

#### Tasks

1. **Combat System**
   - Damage calculations
   - Weapon firing
   - Target locking

2. **Movement System**
   - Velocity updates
   - Rotation
   - Warp mechanics

3. **AI System** (basic)
   - NPC behavior
   - Simple combat AI

**Deliverable**: Full game logic in C++

## Phase 5: Polish & Release (Weeks 16-20)

### Week 16-17: Optimization

1. **Profiling**
   - Use profiler (Visual Studio, gprof, Instruments)
   - Identify bottlenecks
   - Optimize hot paths

2. **Memory**
   - Check for leaks (Valgrind, AddressSanitizer)
   - Optimize allocations
   - Use object pools

3. **Network**
   - Message batching
   - Compression
   - Prediction improvements

### Week 18: Testing

1. **Unit Tests** (Catch2 or Google Test)
2. **Integration Tests**
3. **Multiplayer Tests** (8+ players)
4. **Stress Tests**

### Week 19: Packaging

1. **Windows**: Create installer with WiX or NSIS
2. **Linux**: Create .deb and .rpm packages
3. **macOS**: Create .app bundle and .dmg

### Week 20: Documentation

1. **User Manual**
2. **Developer Documentation**
3. **API Documentation** (Doxygen)
4. **Build Instructions**

## Risk Mitigation

### Technical Risks

| Risk | Mitigation |
|------|------------|
| Panda3D C++ API difficulties | Prototype early, read docs, join forums |
| Performance issues | Profile early and often, optimize incrementally |
| Threading bugs | Use thread sanitizer, careful design, extensive testing |
| Cross-platform issues | Test on all platforms continuously |

### Project Risks

| Risk | Mitigation |
|------|------------|
| Scope creep | Stick to MVP, document "nice-to-haves" for later |
| Time overrun | Weekly reviews, adjust scope if needed |
| Team capacity | Realistic estimates, allow buffer time |
| Python code drift | Keep protocol in sync, integration tests |

## Success Metrics

### MVP (Week 15)
- [ ] Runs on Windows and Linux
- [ ] Connects to Python server
- [ ] Renders at 60 FPS
- [ ] Basic UI functional
- [ ] Can host 2-4 players

### Full Release (Week 20)
- [ ] All features from Python client
- [ ] Complete EVE UI
- [ ] Hosts 8+ players smoothly
- [ ] Cross-platform packages
- [ ] < 100MB download size
- [ ] < 5 second startup

## Resources Needed

### Development Tools
- IDEs: Visual Studio, CLion, or VS Code
- Debuggers: GDB, LLDB, Visual Studio Debugger
- Profilers: VTune, Instruments, gprof
- Version Control: Git

### Team Skills
- C++ (intermediate to advanced)
- Panda3D (or willing to learn)
- Network programming
- 3D graphics basics
- CMake

### Time Commitment
- Full-time: 15-20 weeks
- Part-time: 30-40 weeks
- Casual: 50+ weeks

## Next Actions

1. **This Week**:
   - [ ] Review architecture document
   - [ ] Approve roadmap
   - [ ] Set up development environment
   - [ ] Install dependencies

2. **Next Week**:
   - [ ] Create CMake project
   - [ ] Build "Hello World" client
   - [ ] Begin rendering system

3. **Month 1**:
   - [ ] Complete Phase 1 (Core Client)
   - [ ] Validate architecture decisions
   - [ ] Adjust timeline if needed

## Conclusion

This roadmap provides a clear path from the current Python client to a professional C++ executable with embedded server capabilities. The phased approach allows for validation at each step and graceful handling of unexpected issues.

**Key Success Factors**:
- Start small (MVP first)
- Test continuously
- Maintain Python compatibility initially
- Document as you go
- Regular progress reviews

**Estimated Effort**: 400-500 hours of development time

Ready to begin Phase 1!
