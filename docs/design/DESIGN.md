# EVE OFFLINE - Design Document

## Overview
EVE OFFLINE is a PVE-focused space MMO inspired by EVE ONLINE, designed for small groups of players. This document outlines the core systems based on EVE ONLINE mechanics while simplifying for smaller-scale gameplay.

## Core Concepts from EVE ONLINE

### 1. Ship System
- **Ship Classes**: Frigates, Destroyers, Cruisers, Battlecruisers, Battleships
- **Ship Attributes**: Hull HP, Shield HP, Armor HP, Capacitor, CPU, Power Grid
- **Slot System**: High slots (weapons/mining), Mid slots (shields/tackle), Low slots (damage/tank)
- **Ship Bonuses**: Role bonuses and racial bonuses per ship type

### 2. Skills System
- **Skill Training**: Time-based passive skill training
- **Skill Levels**: 1-5 per skill, exponential training time
- **Skill Categories**: 
  - Gunnery (weapon systems)
  - Missiles (missile systems)
  - Spaceship Command (ship piloting)
  - Engineering (capacitor, power, CPU)
  - Shields/Armor (defensive systems)
  - Navigation (speed, agility)
  - Targeting (lock range, speed, count)

### 3. PVE Combat System
- **NPC Types**: Pirates (Serpentis, Guristas, Blood Raiders, Sansha, Angel Cartel)
- **NPC Behaviors**: 
  - Frigates: Fast, orbit close, low HP
  - Cruisers: Medium range, balanced stats
  - Battleships: Long range, high HP, slow
- **Damage Types**: EM, Thermal, Kinetic, Explosive
- **Resistances**: Ships and NPCs have resistance profiles per damage type

### 4. Mission System
- **Mission Levels**: 1-4 (simplified from EVE's 1-5)
- **Mission Types**:
  - Combat: Destroy specific NPCs
  - Mining: Collect ore
  - Courier: Transport items
  - Exploration: Scan and retrieve items
- **Mission Agents**: NPCs that provide missions
- **Rewards**: ISK (currency), LP (loyalty points), items, standings

### 5. Fitting System
- **Modules**: Weapons, shields, armor, engineering, navigation
- **Fitting Constraints**: CPU and Power Grid requirements
- **Meta Levels**: T1 (basic), T2 (advanced), Faction, Deadspace
- **Ammo Types**: Different ammo for different situations

### 6. Economy & Items
- **Currency**: ISK (InterStellar Kredits)
- **Item Types**: Ships, modules, ammo, ore, salvage
- **Market**: Buy/sell orders (simplified player market)
- **Manufacturing**: Blueprint-based crafting (optional for v1)

### 7. Universe Structure
- **Solar Systems**: Connected via stargates
- **Security Levels**: High-sec (0.5-1.0), Low-sec (0.1-0.4), Null-sec (0.0)
- **Locations**:
  - Stations: Docking, market, missions
  - Asteroid Belts: Mining locations
  - Mission Deadspaces: Instanced PVE areas
  - Gates: System travel

## Technical Architecture

### Engine Architecture (Custom Python-based)

```
┌─────────────────────────────────────────────────────────┐
│                     EVE OFFLINE Engine                   │
├─────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐    │
│  │   Client    │  │   Server    │  │   Shared    │    │
│  │             │  │             │  │             │    │
│  │ - Rendering │  │ - World Sim │  │ - ECS       │    │
│  │ - UI        │  │ - Physics   │  │ - Network   │    │
│  │ - Input     │  │ - AI        │  │ - Data      │    │
│  │ - Network   │  │ - Network   │  │ - Utils     │    │
│  └─────────────┘  └─────────────┘  └─────────────┘    │
└─────────────────────────────────────────────────────────┘
```

### Entity Component System (ECS)

**Components**:
- `Position`: x, y, z coordinates, rotation
- `Velocity`: speed, direction, angular velocity
- `Health`: hull, armor, shield, capacitor
- `Ship`: ship type, bonuses, attributes
- `Fitting`: equipped modules, cargo
- `Skills`: trained skills and levels
- `AI`: behavior type, state machine
- `Target`: targeting info, locked targets
- `Weapon`: weapon stats, ammo, range
- `Player`: player ID, connection, input

**Systems**:
- `MovementSystem`: Updates positions based on velocity
- `CombatSystem`: Handles weapon firing, damage application
- `TargetingSystem`: Manages target locking/unlocking
- `AISystem`: Controls NPC behavior
- `CapacitorSystem`: Manages capacitor consumption/recharge
- `SkillSystem`: Processes skill training
- `NetworkSystem`: Syncs state between client/server

### Network Protocol

**Transport**: TCP for reliability (UDP optional for position updates)

**Message Types**:
- `CONNECT`: Client connection handshake
- `DISCONNECT`: Client disconnect
- `INPUT`: Player input commands
- `STATE_UPDATE`: World state synchronization
- `SPAWN_ENTITY`: Create new entity
- `DESTROY_ENTITY`: Remove entity
- `DAMAGE`: Damage application
- `CHAT`: Text messages

**State Synchronization**:
- Server authoritative model
- Client-side prediction for local player
- Snapshot interpolation for other entities
- Delta compression for bandwidth optimization

### Data Storage

**Format**: JSON for game data, SQLite for persistence

**Data Files**:
- `ships.json`: Ship definitions
- `modules.json`: Module definitions
- `skills.json`: Skill definitions
- `npcs.json`: NPC definitions
- `missions.json`: Mission templates
- `universe.json`: Solar systems and locations

**Database Tables**:
- `players`: Player accounts and metadata
- `characters`: Character data, skills, ISK
- `inventory`: Items owned by characters
- `missions`: Active and completed missions

## Implementation Phases

### Phase 1: Core Engine (MVP)
1. Basic ECS implementation
2. Simple 2D rendering (top-down view)
3. Server with single solar system
4. Basic ship movement and controls
5. Simple NPC spawning

### Phase 2: Combat System
1. Weapon systems (turrets, missiles)
2. Damage calculation and resistances
3. Targeting system
4. NPC AI (basic behaviors)
5. Ship destruction and respawning

### Phase 3: Progression
1. Skills system implementation
2. Multiple ship types
3. Module fitting system
4. Inventory management
5. Basic economy (NPC buy/sell)

### Phase 4: Content
1. Mission system
2. Multiple solar systems
3. Mining mechanics
4. More NPC types and behaviors
5. Rewards and loot

### Phase 5: Multiplayer
1. Fleet system (group play)
2. Chat system
3. Shared PVE encounters
4. Trading between players
5. Corporation/guild basics

### Phase 6: Polish
1. Better graphics/UI
2. Sound effects and music
3. More content (ships, modules, missions)
4. Balance tuning
5. Performance optimization

## Technology Stack

**Core Language**: Python 3.11+
**Graphics**: Pygame (2D) or Pyglet (2D/3D capable)
**Networking**: asyncio + websockets or TCP sockets
**Data**: JSON for config, SQLite for persistence
**Math**: NumPy for vector operations
**Optional**: 
- Cython for performance-critical code
- Rust/C++ modules for physics engine
- Three.js for web-based client

## Minimum System Requirements

**Server**:
- CPU: 2+ cores
- RAM: 2GB
- Storage: 1GB
- Network: 1Mbps upload per 10 players

**Client**:
- CPU: Dual-core 2GHz+
- RAM: 4GB
- GPU: Any with OpenGL 2.0+
- Storage: 500MB
- Network: 1Mbps download

## Success Metrics

1. Supports 10-50 concurrent players on single server
2. Smooth 60 FPS gameplay on modest hardware
3. < 100ms latency for responsive controls
4. Complete mission loop (accept, complete, reward)
5. Satisfying PVE combat similar to EVE ONLINE experience
6. Fun and engaging for small groups without PVP
