# Atlas — Design Document

> **Status**: In active R&D and development. Actively testing until further notice.

## Overview

EVE OFFLINE is a PVE-focused space MMO inspired by EVE ONLINE, designed for small groups of players (2-20). Built with C++ and OpenGL, this project recreates the core EVE ONLINE experience while simplifying for smaller-scale cooperative gameplay. This design document is structured around the systems described in the EVE ONLINE game manual.

## Core Game Systems

### 1. Character Creation & Races
*(Based on EVE Manual Chapters 1, 3)*

- **Four Playable Races**: Amarr, Caldari, Gallente, Minmatar
  - Each race has distinct bloodlines with attribute modifiers
  - Starting skills, ship, and system determined by race choice
- **Character Attributes**: Perception, Memory, Willpower, Intelligence, Charisma
  - Attributes affect skill training speed
  - Modified by bloodline, Learning skills, and implants
- **No Classes**: Characters advance through skills — no class is superior to another
- **Data**: `data/character_creation/races.json`

### 2. Ship System
*(Based on EVE Manual Chapters 1, 8)*

- **Ship Classes**: Frigates, Destroyers, Cruisers, Battlecruisers, Battleships, Mining Barges, Exhumers, Capitals
- **Ship Variants**: Tech I, Tech II (Assault Ships, Heavy Assault Cruisers, Interceptors, Logistics, Covert Ops, Recon, Heavy Interdiction)
- **Ship Attributes**: Hull HP, Shield HP, Armor HP, Capacitor, CPU, Power Grid, Signature Radius
- **Slot System**: High slots (weapons/mining), Mid slots (shields/tackle/EWAR), Low slots (damage/armor/utility)
- **Hardpoints**: Turret slots and Launcher slots limiting weapon types per ship
- **Ship Bonuses**: Role bonuses and racial bonuses per ship type
- **Racial Specialties**:
  - Amarr: Energy turrets (lasers), armor tanking
  - Caldari: Hybrid turrets & missiles, shield tanking
  - Gallente: Hybrid turrets & drones, armor tanking
  - Minmatar: Projectile turrets, speed-based defense (shields or armor)
- **Data**: `data/ships/`

### 3. Skills System
*(Based on EVE Manual Chapter 7)*

- **Skill Training**: Real-time passive skill training (continues even when offline)
- **Skill Levels**: 1-5 per skill, with rank-based training multiplier
- **Skill Attributes**: Each skill has primary and secondary attributes that determine training speed
- **Skill Categories**:
  - Gunnery (perception/willpower) — weapon turret skills
  - Missiles (perception/willpower) — missile launcher skills
  - Spaceship Command (perception/willpower) — ship piloting skills
  - Engineering (intelligence/memory) — capacitor, power, CPU
  - Electronics (intelligence/memory) — sensor, CPU management
  - Shields (intelligence/memory) — shield defensive systems
  - Armor (intelligence/memory) — armor defensive systems
  - Navigation (intelligence/perception) — speed, agility, warp
  - Targeting (intelligence/memory) — lock range, speed, count
  - Drones (memory/perception) — drone operation
  - Learning (memory/intelligence) — attribute enhancement
  - Social (charisma/intelligence) — agent standings, mission bonuses
  - Trade (willpower/charisma) — market orders, fees
  - Leadership (willpower/charisma) — fleet command
  - Corporation Management (memory/charisma) — corp administration
  - Mechanic (intelligence/memory) — hull upgrades, repair
  - Science (intelligence/memory) — research, cybernetics
  - Resource Processing (memory/intelligence) — refining
- **No Skill Limit**: No limit to total skills a character can train
- **Data**: `data/skills/skills.json`, `data/skills/science_skills.json`

### 4. PVE Combat System
*(Based on EVE Manual Chapters 9, 10)*

- **NPC Pirate Factions**: Serpentis, Guristas, Blood Raiders, Sansha's Nation, Angel Cartel, Rogue Drones
- **NPC Behaviors**:
  - Frigates: Fast, orbit close, low HP
  - Cruisers: Medium range, balanced stats
  - Battlecruisers: Heavy firepower with good mobility
  - Battleships: Long range, high HP, slow tracking
  - Commanders/Officers: Enhanced loot drops
- **Damage Types**: EM, Thermal, Kinetic, Explosive
- **Weapon Types**:
  - Turrets: Projectile (kinetic/explosive), Hybrid (kinetic/thermal), Energy (EM/thermal)
  - Missiles: Rockets, Light Missiles, Heavy Missiles, Cruise Missiles, Torpedoes
- **Damage Application**: Tracking speed, signature radius, optimal range, falloff
- **Resistances**: Ships and NPCs have per-type resistance profiles
- **Pirate Hunting**: NPC pirates in asteroid belts and anomalies with ISK bounties
- **Data**: `data/npcs/pirates.json`, `data/modules/`

### 5. Fitting System
*(Based on EVE Manual Chapter 8)*

- **Module Categories**: Weapons, shields, armor, engineering, navigation, EWAR, drones
- **Fitting Constraints**: CPU and Power Grid requirements per module
- **Slot Types**: High (turrets, launchers, mining lasers), Mid (shields, EWAR, propulsion), Low (armor, damage mods)
- **Meta Levels**: T1 (basic), T2 (advanced), Faction, Deadspace
- **Ammo Types**: Race-specific ammunition with different damage profiles and range modifiers
- **Turret Sub-Types**:
  - Projectile: Auto-Cannons (short range, rapid fire) and Artillery (long range, high alpha)
  - Hybrid: Blasters (short range, high damage) and Railguns (long range, good accuracy)
  - Energy: Pulse Lasers (medium range, no ammo) and Beam Lasers (long range, no ammo)
- **Launcher Sub-Types**: Rocket Launchers (frigate), Standard/Assault Launchers (cruiser), Cruise/Siege Launchers (battleship)
- **Data**: `data/modules/`

### 6. Mission System
*(Based on EVE Manual Chapter 11)*

- **Mission Levels**: 1-5
- **Mission Types**:
  - Combat (Kill): Destroy specific NPC targets
  - Mining: Deliver ore or minerals to agent
  - Courier: Transport items between stations
  - Trade: Acquire specific items and deliver to agent
  - Scenario: Retrieve items from combat zones
  - Exploration: Scan and retrieve items from signatures
  - Storyline/Important: Special missions affecting faction standing
- **Agent System**:
  - Agents belong to NPC corporations and divisions
  - Agent quality and level determine rewards
  - Standing requirements to access higher-level agents
  - Research agents for scientific research
- **Rewards**: ISK, Loyalty Points (LP), items, standings (personal, corporation, faction)
- **Standings**:
  - Personal standing with agents
  - Corporation standing
  - Faction standing (affected by Important/Storyline missions)
  - Skills (Social, Connections, Negotiation) affect standing gains
- **Data**: `data/missions/`

### 7. Economy & Items
*(Based on EVE Manual Chapters 6, 13, 15)*

- **Currency**: ISK (InterStellar Kredits)
- **Item Types**: Ships, modules, ammo, ore, minerals, salvage, blueprints, skill books, implants
- **Market**: Buy/sell orders with regional pricing
- **Manufacturing**: Blueprint-based crafting with Material Efficiency and Time Efficiency research
- **Mining**:
  - Ore types: Veldspar, Scordite, Pyroxeres, Plagioclase, Kernite, and more
  - Mining lasers (civilian, Miner I), Strip Miners for barges
  - Refining ore into minerals at stations
- **Contracts & Escrow**: Item exchange, courier contracts, and auctions with escrow protection
- **Insurance**: Ship hull insurance with multiple coverage levels
- **Data**: `data/market/`, `data/industry/`, `data/contracts/`, `data/asteroid_fields/`

### 8. Universe Structure
*(Based on EVE Manual Chapters 4, 5)*

- **Solar Systems**: Connected via stargates
- **Security Levels**: High-sec (0.5-1.0), Low-sec (0.1-0.4), Null-sec (0.0)
- **CONCORD**: Security enforcement in high-sec space with response times based on security level
- **Locations**:
  - Stations: Docking, market, missions, repair, fitting, cloning, insurance
  - Asteroid Belts: Mining locations with ore spawns
  - Deadspace Complexes: Multi-room instanced PVE areas with acceleration gates
  - Gates: Stargate travel between systems
- **Data**: `data/universe/`, `data/security/`

### 9. Character Progression
*(Based on EVE Manual Chapters 7, 3)*

- **Clone System**: Medical clones retain skill points on pod death
  - Clone grades from Alpha (free) to Omega (maximum)
  - Jump clones for instant travel (24-hour cooldown)
  - Implant destruction on pod death
- **Implants**: Cybernetic implants that boost character attributes
  - One implant per attribute slot (5 slots)
  - Grades: Limited (+1), Limited-Beta (+2), Basic (+3), Standard (+4)
  - Requires Cybernetics skill
- **Learning Skills**: Dedicated skill category for boosting attributes
  - Instant Recall (Memory), Analytical Mind (Intelligence), Spatial Awareness (Perception)
  - Iron Will (Willpower), Empathy (Charisma), Learning (general reduction)
- **Data**: `data/character_creation/clones.json`, `data/character_creation/implants.json`

### 10. Corporation System
*(Based on EVE Manual Chapter 12)*

- **NPC Corporations**: Military, industrial, and research corporations for each faction
- **Player Corporations**: Player-created organizations with roles, hangars, and wallets
- **Research Corporations**: Provide research agents for scientific research
  - Amarr: Carthum Conglomerate, Viziam
  - Caldari: Ishukone, Kalaakiota, Lai Dai
  - Gallente: Creodron, Duvolle, Roden Shipyards
  - Minmatar: Core Complexion, Boundless Creation, Thukker Mix
- **Corporation Standings**: Affect agent access, broker fees, and NPC behavior
- **Data**: `data/corporations/corporations.json`

### 11. Security & Legal System
*(Based on EVE Manual Chapters 5, 17)*

- **Security Status**: Personal security rating (-10.0 to +10.0) affecting access to high-sec
- **Criminal Flags**: Suspect, criminal, and weapons timer mechanics
- **CONCORD Response**: Automatic enforcement in high-sec with response times based on system security
- **Data**: `data/security/concord_and_insurance.json`

### 12. Deadspace Complexes
*(Based on EVE Manual Chapter 14)*

- **Difficulty Levels**: 5 tiers from Minor Annex (level 1) to Pirate Stronghold (level 5)
- **Multi-Room Design**: 2-6 rooms connected by acceleration gates
- **Faction-Specific**: Each pirate faction has unique complex templates
- **Escalation Mechanics**: Chance to escalate to harder site in nearby system
- **Loot Tiers**: Basic, Standard, Advanced, Rare, Faction (including officer/deadspace modules)
- **Data**: `data/exploration/deadspace_complexes.json`

## Technical Architecture

### Engine Architecture (C++ / OpenGL)

```
┌─────────────────────────────────────────────────────────┐
│                     EVE OFFLINE Engine                   │
├─────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐    │
│  │   Client    │  │   Server    │  │   Shared    │    │
│  │  (C++/GL)   │  │    (C++)    │  │   (JSON)    │    │
│  │             │  │             │  │             │    │
│  │ - OpenGL 3D │  │ - ECS World │  │ - Data      │    │
│  │ - ImGui UI  │  │ - AI        │  │ - Network   │    │
│  │ - OpenAL    │  │ - Network   │  │ - Protocol  │    │
│  │ - Input     │  │ - Systems   │  │ - Config    │    │
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

**Format**: JSON for all game content (100% moddable)

**Data Directories**:
- `data/character_creation/`: Races, bloodlines, clones, implants
- `data/ships/`: Ship definitions by class
- `data/modules/`: Module definitions (weapons, defense, utility, drones)
- `data/skills/`: Skill definitions with attribute-based training
- `data/npcs/`: NPC pirates and hireable pilots
- `data/missions/`: Mission templates by level
- `data/universe/`: Solar systems, stations, stargates
- `data/security/`: CONCORD, insurance
- `data/corporations/`: NPC and player corporation mechanics
- `data/contracts/`: Contract and escrow system
- `data/exploration/`: Deadspace complexes and exploration sites
- `data/industry/`: Blueprints and manufacturing
- `data/market/`: Pricing data
- `data/asteroid_fields/`: Mining belt data

**Database Tables** (for persistence):
- `players`: Player accounts and metadata
- `characters`: Character data, skills, ISK, attributes, clones
- `inventory`: Items owned by characters
- `missions`: Active and completed missions

## Implementation Phases

> **Note**: All phases are part of ongoing R&D and development. Actively testing until further notice.

### Phase 1: Core Engine (Complete)
1. ECS architecture implementation
2. C++ OpenGL 3D rendering with deferred rendering pipeline
3. Server with solar system simulation
4. Ship movement, docking, and warping
5. NPC spawning and basic AI

### Phase 2: Combat & Content (Complete)
1. Weapon systems (turrets, missiles, drones)
2. Damage calculation with resistances and tracking
3. Targeting system with locking mechanics
4. NPC AI with faction-based behaviors
5. Ship destruction, loot, and bounties

### Phase 3: Progression & Fitting (Complete)
1. Skills system with attribute-based training
2. 58+ ship types across all classes (T1, T2, capitals)
3. Module fitting system with CPU/PG constraints
4. 70+ modules (weapons, defense, utility)
5. Mining, manufacturing, and market systems

### Phase 4: Missions & Universe (Complete)
1. Mission system with agents and standings
2. Multiple solar systems connected via stargates
3. Mining mechanics with ore types and refining
4. Exploration with signatures and anomalies
5. Rewards, loot tables, and loyalty points

### Phase 5: Multiplayer & Polish (Complete)
1. TCP multiplayer with entity synchronization
2. ImGui-based EVE-styled UI
3. OpenAL spatial audio
4. Shadow mapping and post-processing
5. Procedural ship model generation

### Phase 6: Advanced Content (Complete)
1. Tech II ships (HAC, HIC, Recon, Logistics)
2. Capital ships (Carriers, Dreadnoughts, Titans)
3. Advanced mission content (28 missions, 5 levels)
4. Mining barges and exhumers
5. Ice mining and planetary interaction

### Phase 7: Manual-Aligned Systems (In Progress)
1. Character creation with races and bloodlines
2. Clone system and implants
3. CONCORD security enforcement
4. Corporation system
5. Contract/escrow system
6. Deadspace complexes
7. Learning, Social, Trade, Leadership skill categories
8. Trade, scenario, and storyline mission types
9. Insurance system

## Technology Stack

> **Note**: The project has migrated from the original Python prototype to a full C++/OpenGL implementation.

**Core Language**: C++17
**Graphics**: OpenGL 3.3+ with deferred rendering, shadow mapping, post-processing
**UI**: ImGui (EVE Photon UI styled)
**Audio**: OpenAL (spatial audio)
**Networking**: TCP sockets (asyncio-based server)
**Data**: JSON for all game content (100% moddable)
**Build System**: CMake 3.15+
**Libraries**: GLFW3, GLM, GLEW, nlohmann-json, STB

**Supported Platforms**:
- Windows (Visual Studio 2019+)
- Linux (GCC 9+)
- macOS (Clang 10+)

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
