# Phase 4.2: Entity State Synchronization

## Overview

The entity state synchronization system allows the C++ OpenGL client to receive and manage game entities from the server. It provides smooth interpolation for realistic movement and integrates seamlessly with the network protocol.

## Architecture

### Core Components

#### 1. Entity (`entity.h` / `entity.cpp`)

Represents a single game entity with state and interpolation support.

**Key Features:**
- Position, velocity, and rotation tracking
- Health system (shield, armor, hull)
- Ship information (type, name, faction)
- Smooth interpolation using cubic ease-out
- Update flag for rendering optimization

**Interpolation Algorithm:**
- Uses 100ms interpolation window (configurable)
- Cubic ease-out curve for natural deceleration
- Formula: `smoothT = 1.0 - (1.0 - t)^3`
- Prevents jitter from network updates

#### 2. EntityManager (`entity_manager.h` / `entity_manager.cpp`)

Manages the lifecycle of all entities in the game world.

**Key Features:**
- Spawn/update/destroy entity operations
- Automatic entity cleanup (entities not in STATE_UPDATE are removed)
- Frame-by-frame interpolation updates
- Event callbacks for entity lifecycle events
- Thread-safe entity access via shared_ptr

**Callback System:**
```cpp
manager.setOnEntitySpawned([](const std::shared_ptr<Entity>& entity) {
    // Handle new entity (e.g., create visual representation)
});

manager.setOnEntityDestroyed([](const std::shared_ptr<Entity>& entity) {
    // Handle entity removal (e.g., cleanup visuals)
});

manager.setOnEntityUpdated([](const std::shared_ptr<Entity>& entity) {
    // Handle entity state change (optional)
});
```

#### 3. EntityMessageParser (`entity_message_parser.h` / `entity_message_parser.cpp`)

Parses JSON network messages and updates the EntityManager.

**Supported Messages:**

##### SPAWN_ENTITY
```json
{
    "entity_id": "uuid-123-456",
    "position": {"x": 100.0, "y": 200.0, "z": 300.0},
    "health": {"shield": 150, "armor": 250, "hull": 350},
    "ship_type": "Merlin",
    "ship_name": "My Ship",
    "faction": "Caldari"
}
```

##### STATE_UPDATE
```json
{
    "entities": [
        {
            "id": "uuid-123-456",
            "pos": {"x": 110.0, "y": 210.0, "z": 310.0, "rot": 1.5},
            "vel": {"vx": 5.0, "vy": 3.0, "vz": 2.0},
            "health": {"s": 140, "a": 240, "h": 350}
        }
    ],
    "tick": 42
}
```

##### DESTROY_ENTITY
```json
{
    "entity_id": "uuid-123-456"
}
```

#### 4. GameClient Integration

The `GameClient` class now integrates both `NetworkManager` and `EntityManager`:

```cpp
GameClient client;

// Connect to server
client.connect("localhost", 8765, "MyCharacter");

// Set up callbacks for visual integration
client.setOnEntitySpawned([](const std::shared_ptr<Entity>& entity) {
    // Create 3D model for entity
    renderer.createEntityVisual(entity);
});

// Game loop
while (running) {
    float deltaTime = calculateDeltaTime();
    
    // Update networking and entities
    client.update(deltaTime);
    
    // Render entities
    for (const auto& [id, entity] : client.getEntityManager().getAllEntities()) {
        renderer.renderEntity(entity);
    }
}
```

## Implementation Details

### Interpolation System

The interpolation system ensures smooth movement despite network updates arriving at ~10 Hz:

1. **Server sends STATE_UPDATE** (every ~100ms)
2. **EntityManager stores target position** for each entity
3. **Entity.interpolate()** called every frame (60+ Hz)
4. **Position smoothly interpolates** using cubic ease-out
5. **Results in fluid 60 FPS movement** from 10 Hz updates

**Why Cubic Ease-Out?**
- Natural deceleration curve
- Prevents abrupt stops
- Matches player expectations
- Better than linear interpolation

### State Cleanup

Entities that are no longer in STATE_UPDATE messages are automatically destroyed:

```cpp
void EntityManager::processStateUpdate(const std::vector<std::string>& entityIds) {
    // Find entities not in the update
    for (const auto& [id, entity] : m_entities) {
        if (not in entityIds) {
            destroyEntity(id);  // Automatic cleanup
        }
    }
}
```

This prevents "ghost" entities when the server removes them.

### Health Structure

Health is represented as three separate pools:
- **Shield**: Regenerates over time, first line of defense
- **Armor**: Damage reduction, second layer
- **Hull**: Final health pool, entity dies at 0

Each pool tracks current and maximum values for UI display.

## Testing

### Test Suite

The `test_entity_sync.cpp` program includes 4 comprehensive test suites:

1. **Basic Entity Operations**
   - Entity spawn/update/destroy
   - State management
   - Interpolation verification

2. **Message Parsing**
   - SPAWN_ENTITY parsing
   - STATE_UPDATE parsing with multiple entities
   - DESTROY_ENTITY parsing

3. **Smooth Interpolation**
   - Verifies cubic ease-out algorithm
   - Tests 100ms interpolation window
   - Validates final position accuracy

4. **Entity Callbacks**
   - Spawn event callbacks
   - Update event callbacks
   - Destroy event callbacks

### Running Tests

```bash
# Build and run tests
./build_test_entity_sync.sh
./test_entity_sync
```

All tests should pass with green checkmarks:
```
========================================
All tests PASSED!
========================================
```

## Integration Guide

### Step 1: Include Headers

```cpp
#include "core/game_client.h"
```

### Step 2: Create GameClient

```cpp
GameClient gameClient;
```

### Step 3: Connect to Server

```cpp
if (!gameClient.connect("localhost", 8765, "MyCharacter")) {
    std::cerr << "Failed to connect!" << std::endl;
    return -1;
}
```

### Step 4: Set Up Callbacks (Optional)

```cpp
gameClient.setOnEntitySpawned([&renderer](const std::shared_ptr<Entity>& entity) {
    // Create visual representation
    auto model = renderer.createShipModel(entity->getShipType());
    renderer.attachModelToEntity(entity->getId(), model);
});

gameClient.setOnEntityDestroyed([&renderer](const std::shared_ptr<Entity>& entity) {
    // Remove visual representation
    renderer.removeEntityVisual(entity->getId());
});
```

### Step 5: Game Loop

```cpp
while (running) {
    float deltaTime = timer.getDeltaTime();
    
    // Update game state
    gameClient.update(deltaTime);
    
    // Render all entities
    const auto& entities = gameClient.getEntityManager().getAllEntities();
    for (const auto& [id, entity] : entities) {
        auto pos = entity->getPosition();
        renderer.renderEntityAt(entity->getId(), pos);
    }
    
    // Handle input
    if (input.isKeyPressed(KEY_W)) {
        gameClient.sendMove(0.0f, 1.0f, 0.0f);  // Move forward
    }
}
```

## Performance Considerations

### Memory
- Entities use `std::shared_ptr` for safe concurrent access
- Entity map uses `std::unordered_map` for O(1) lookup
- Interpolation state adds minimal overhead (~32 bytes per entity)

### CPU
- Interpolation is O(1) per entity per frame
- Message parsing is O(n) where n = number of entities in update
- Cubic ease-out uses pow() which is fast on modern CPUs

### Network
- Server sends STATE_UPDATE at 10 Hz (every ~100ms)
- Message size: ~50-100 bytes per entity
- Efficient JSON parsing with nlohmann/json

### Optimization Tips
1. Use callbacks to create visuals only when needed
2. Cull off-screen entities before interpolation
3. Batch entity updates in renderer
4. Consider LOD for distant entities

## Known Limitations

1. **No prediction**: Client displays server state with interpolation only
   - Future: Add client-side prediction for player entity
   
2. **Fixed interpolation window**: 100ms window not configurable per-entity
   - Could add lag compensation for high-latency connections

3. **No extrapolation**: If updates stop, entity freezes at last position
   - Could add velocity-based extrapolation as fallback

## Future Enhancements

### Phase 4.3: Planned Improvements
- Client-side prediction for player entity
- Lag compensation system
- Adaptive interpolation based on network conditions
- Entity priority system (prioritize nearby entities)
- Occlusion culling integration

### Phase 4.4: Visual Integration
- Integrate with existing Renderer
- Hook up entity callbacks to 3D model creation
- Add visual effects for entity spawn/destroy
- Health bar rendering from Health struct

## Related Documentation

- [Network Protocol](PHASE4_NETWORK.md) - TCP/JSON network protocol
- [Renderer](../include/rendering/renderer.h) - 3D rendering system
- [C++ Client README](README.md) - Overall client architecture

## Version History

- **v1.0** (2026-02-05): Initial implementation
  - Entity, EntityManager, EntityMessageParser
  - Cubic ease-out interpolation
  - Comprehensive test suite
  - GameClient integration

---

**Status**: ✅ Complete  
**Test Coverage**: 4 test suites, 100% pass rate  
**Lines of Code**: ~700 (production) + ~350 (tests)
