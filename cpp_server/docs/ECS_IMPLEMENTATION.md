# C++ ECS Implementation

## Overview

The C++ dedicated server now includes a fully functional Entity Component System (ECS) architecture, ported from the Python implementation. The ECS provides the foundation for all game logic, including movement, combat, AI, and targeting.

## Architecture

### Core ECS Classes

- **Entity**: Game object with attached components
- **Component**: Pure data containers 
- **System**: Game logic that processes entities
- **World**: Manages all entities and systems

## Game Components

10 core components implemented:
1. Position - 3D position and rotation
2. Velocity - Movement and speed
3. Health - Shield/Armor/Hull with resistances  
4. Capacitor - Energy management
5. Ship - Ship stats and fitting
6. Weapon - Weapon stats and state
7. Target - Target locking
8. AI - NPC behavior
9. Player - Player data
10. Faction - Faction affiliation

## Game Systems

4 core systems implemented:

1. **MovementSystem** - Position updates
2. **CombatSystem** - Damage, weapons, recharge
3. **AISystem** - NPC AI states and behaviors
4. **TargetingSystem** - Progressive target locking

## Performance

- 30 Hz tick rate
- O(1) component access
- Template-based entity queries
- ~10-50x faster than Python

## Usage

```cpp
// Create entity
auto* npc = world->createEntity("npc_1");

// Add components
npc->addComponent(std::make_unique<Position>());
npc->addComponent(std::make_unique<AI>());

// Systems process automatically each tick
world->update(delta_time);
```

For more details on components and systems, see the header files in `include/components/` and `include/systems/`.
