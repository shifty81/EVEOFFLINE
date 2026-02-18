# Atlas — Project Roadmap

**Status**: In active R&D and development — actively testing until further notice

---

## Table of Contents
1. [Project Vision](#project-vision)
2. [Master Implementation Plan](#master-implementation-plan)
3. [Current Status](#current-status)
4. [Completed Work](#completed-work)
5. [In Progress](#in-progress)
6. [Planned Features](#planned-features)
7. [Future Considerations](#future-considerations)
8. [Vertical Slice: One Full Star System](#-vertical-slice-one-full-star-system-next-major-milestone)
9. [Development Milestones](#development-milestones)

---

## Project Vision

EVE OFFLINE is a PVE-focused space MMO inspired by EVE Online, designed for small groups of players (2-20). The project aims to recreate the EVE Online experience with:
- C++ and OpenGL game engine with ECS architecture
- Server-authoritative multiplayer for cooperative gameplay
- EVE-like mechanics: ships, skills, fitting, combat, missions, exploration
- Fully moddable via editable JSON files
- Focus on PVE content without PVP stress
- AI-driven economy where NPCs are real economic actors
- Custom retained-mode UI system (replacing ImGui for game UI)
- Deterministic simulation for networking and replays

**What we are NOT building:**
- PvP combat
- Twitch shooter mechanics
- Asset-copied EVE clone
- Client-authoritative logic

---

## Master Implementation Plan

> Based on comprehensive baseline design document (February 2026) and `scopeforprojectsofar.rtf` project planning document

### Architecture Pillars

| Layer | Responsibility |
|-------|---------------|
| **Server (Authoritative)** | Tick-based simulation, ECS, AI, economy, combat, persistence |
| **Client (Display)** | Rendering, interpolation, UI, input, audio — no game logic authority |
| **Networking** | Server sends snapshots → Client caches → Interpolation → Render |

### Simulation Model
- **Tick-based** (10-20 Hz), not frame-based
- **Deterministic** logic for networking and replays
- All systems process on server ticks: Movement, Combat, Industry, AI, Market

### AI as Economic Actors (Critical for PvE)
AI NPCs are real participants in the economy, not fake spawn-and-die mobs:
- **Miners** — Choose asteroids, mine ore, haul to stations
- **Haulers** — Buy/sell on market, transport between stations
- **Industrialists** — Manufacture goods, respond to orders
- **Traders** — Market speculation, arbitrage
- **Pirates** — Disrupt trade routes, attack miners
- **Authorities** — Respond to security threats
- **Mercenaries** — Accept player contracts

Each AI owns ships, has a wallet, responds to market prices, and can die permanently. Player actions distort the economy: blow up haulers → prices spike; mine too much → prices crash.

### Economy Engine
- No fake NPC market orders — everything is produced, transported, consumed, destroyed
- Regional markets with delayed information
- Dynamic taxation and broker fees
- Shipping risk and supply/demand curves

### Custom UI Strategy (Replacing ImGui for Game UI)
- **Retained-mode** windowed UI system
- **Window docking** (DockNode tree with split/leaf nodes)
- **EVE-style dark theme** (defined in `data/ui/eve_dark_theme.json`)
- **Ship HUD**: Control ring (shield/armor/hull), capacitor bar, module rack, target brackets
- **Keyboard-first** interaction with mouse support
- **Data binding** via observer pattern (no polling)
- ImGui retained only for debug/dev tools

### Modular Procedural Ship Generation
- Ships generated from modular parts assembled along a hull spine
- **ShipSeed** struct: seed, faction, hull class, tech tier, role
- Same seed = same ship forever (networking + saves)
- Part families: Hull Spine, Bow/Nose, Stern/Engines, Mid Modules, Hardpoints
- Faction shape language drives silhouettes and materials
- Procedural materials (base color, edge wear, panel lines, grime, emissive)

### Ship HUD Architecture ("Cold Control Ring")
The Ship HUD is anchored bottom-center, designed for peripheral readability:
- **Control Ring**: Outer ring (shield), middle ring (armor), inner core (hull) — arc-based with clockwise depletion
- **Velocity Arc**: Speed indicator around control ring, color changes for boost/align/webbed states
- **Capacitor Bar**: Vertical bar with gradient from green (full) to red (critical)
- **Module Rack**: Horizontal row grouped by slot type (High/Mid/Low), 32×32 icons with cooldown sweeps
- **Target Brackets**: Four-corner brackets in space, color-coded by relation, lock progress arcs
- **Alert Stack**: Priority-based warnings above ring ("CAP LOW", "STRUCTURE CRITICAL", "SCRAMBLED")
- **Damage Feedback**: Shield (blue ripple), armor (yellow flash), hull (red pulse + shake)

### Combat Damage Layers
Shield → Armor → Hull damage progression:
- **Shield**: Regenerates over time, takes full damage, visual blue ripple effects
- **Armor**: No regen, reduced damage via resists, visual sparks and plating cracks
- **Hull**: No regen, critical effects below 25%, visual fires, smoke, flicker

### AI Combat Behavior Tiers
AI behavior scales by difficulty tier:
| Tier | Behavior |
|------|----------|
| Civilian | Flee when engaged |
| Militia | Attack blindly |
| Navy | Maintain optimal range |
| Elite | Focus fire, coordinated |
| Boss | Spawns reinforcements |

AI state machine: Idle → Patrol → Engage → Orbit → Retreat

### Economy-Driven Ship Generation
NPC ships reflect economic state:
- Resource-rich systems → More mining ships with bulky hulls
- War-torn systems → Damaged ships, more combat patrols
- High-tech systems → Sleeker, advanced hulls
- Player actions (mining, piracy, trade protection) visibly change traffic patterns

### LOD & Impostor System
Performance scaling for large fleet battles:
| Distance | LOD Level | Update Rate |
|----------|-----------|-------------|
| <5km | LOD0 (full detail) | 30 Hz |
| 5-20km | LOD1 (reduced segments) | 15 Hz |
| 20-80km | LOD2 (merged mesh) | 5 Hz |
| >80km | Impostor (billboard) | 1 Hz |

### Salvage & Wrecks System
Post-combat gameplay loop:
- Ship destruction → Wreck spawn with original ShipSeed
- Salvage value affected by: ship class, tech tier, damage type, overkill amount
- Wreck visuals: damaged ship impostor, broken modules floating, fire slowly extinguishing
- Salvage gameplay: target wreck, activate salvager, progress bar, RNG roll influenced by skills

### Overview & Targeting Integration
EVE-style targeting system:
- Overview is a filtered ECS query of Targetable entities, sorted by distance/threat
- Click overview row or in-space bracket → synced EntityID selection
- Lock time = function of target signature and player sensor strength
- Weapon fire auto-aims to locked target

### Mod Support Architecture
Data-driven modding without code injection:
- `ai_profiles.json`: Define AI behavior (orbitRange, retreatThreshold, focusFire)
- `economy_rules.json`: Define system economy curves (pirateSpawnRate, miningYield)
- `ship_templates.json`: Define ship stats and visual parameters
- All mods maintain deterministic simulation

### Implementation Priority Order
1. ✅ Server tick loop solid
2. ✅ Core ECS systems (combat, movement, targeting, AI)
3. ✅ Ship/module/skill data pipeline
4. ✅ Drone, Insurance, Bounty, Market systems
5. 🔄 Snapshot replication & client interpolation
6. 🔄 Custom UI windows (floating, then docking) — Chat, Drone, Notification panels added
7. 🔄 Ship HUD (control ring, module rack, brackets)
8. 🔄 AI economic actors (miners, haulers, traders) — AI mining behavior complete
9. 🔄 Full economy simulation — Refining system complete, market pricing validated
10. ⬜ Advanced mission generation
11. ⬜ Universe map & travel
12. ⬜ Polish & modding tools

### C++ Server Systems Status

| System | Status | Tests |
|--------|--------|-------|
| CapacitorSystem | ✅ Complete | 10 assertions |
| ShieldRechargeSystem | ✅ Complete | 5 assertions |
| WeaponSystem | ✅ Complete | 16 assertions |
| CombatSystem | ✅ Complete | Damage application |
| TargetingSystem | ✅ Complete | 8 assertions |
| MovementSystem | ✅ Complete | 14 assertions |
| AISystem | ✅ Complete | Via weapon tests |
| FleetSystem | ✅ Complete | 40+ assertions |
| MissionSystem | ✅ Complete | 10 assertions |
| SkillSystem | ✅ Complete | 10 assertions |
| ModuleSystem | ✅ Complete | 12 assertions |
| InventorySystem | ✅ Complete | 19 assertions |
| LootSystem | ✅ Complete | 11 assertions |
| WormholeSystem | ✅ Complete | 16 assertions |
| DroneSystem | ✅ Complete | 33 assertions |
| InsuranceSystem | ✅ Complete | 21 assertions |
| BountySystem | ✅ Complete | 14 assertions |
| MarketSystem | ✅ Complete | 11 assertions |
| CorporationSystem | ✅ Complete | 37 assertions |
| ContractSystem | ✅ Complete | 36 assertions |
| PISystem | ✅ Complete | 14 assertions |
| ManufacturingSystem | ✅ Complete | 21 assertions |
| ResearchSystem | ✅ Complete | 18 assertions |
| ChatSystem | ✅ Complete | 28 assertions |
| CharacterCreationSystem | ✅ Complete | 23 assertions |
| TournamentSystem | ✅ Complete | 24 assertions |
| LeaderboardSystem | ✅ Complete | 23 assertions |
| StationSystem | ✅ Complete | 29 assertions |
| WreckSalvageSystem | ✅ Complete | 22 assertions |
| MiningSystem | ✅ Complete | 7 assertions |
| RefiningSystem | ✅ Complete | 9 assertions |
| MissionTemplateSystem | ✅ Complete | 6 assertions |
| MissionGeneratorSystem | ✅ Complete | 4 assertions |
| ReputationSystem | ✅ Complete | 6 assertions |
| **Total** | **34 systems** | **966 assertions** |

### Data Loaders Status

| Loader | Status | Records |
|--------|--------|---------|
| ShipDatabase | ✅ Complete | 102 ship templates |
| NpcDatabase | ✅ Complete | 32 NPC templates |
| WormholeDatabase | ✅ Complete | 6 classes, 6 effects |
| WorldPersistence | ✅ Complete | Full save/load |

### Systems Still Needed (from Baseline)

| System | Priority | Description |
|--------|----------|-------------|
| ~~ChatSystem~~ | ~~Medium~~ | ✅ Implemented (28 assertions) |
| ~~CharacterCreationSystem~~ | ~~Low~~ | ✅ Implemented (23 assertions) |
| ~~TournamentSystem~~ | ~~Low~~ | ✅ Implemented (24 assertions) |
| ~~LeaderboardSystem~~ | ~~Low~~ | ✅ Implemented (23 assertions) |

All baseline systems are now implemented.

---

## Current Status

### 🎯 Active R&D and Development

**Overall Progress**: Core features implemented, actively testing and expanding  
**Status**: All ship model integration complete (58+ ships). Tech II ships, capitals, mining barges, and exhumers implemented with 3D models. Mission system expansion complete with missions across 5 levels. Game systems being structured around the EVE Online manual.

---

## Completed Work

### ✅ Phase 1: Core Engine (Complete)
**Completed**: Q4 2025

- [x] **Entity Component System (ECS)** - Modern game architecture
  - World management with entity lifecycle
  - Component-based architecture
  - System-based game logic
- [x] **Core Game Components** (11 components)
  - Position, Velocity, Health, Ship, Weapon
  - AI, Target, Capacitor, Shield, Faction
- [x] **Core Game Systems** (8 systems)
  - Movement, Combat, AI, Targeting
  - Capacitor, Shield Recharge, Weapon, Faction
- [x] **Data-Driven Architecture**
  - JSON-based data files for all content
  - DataLoader for dynamic content loading
  - Modding-friendly design
- [x] **Network Infrastructure**
  - asyncio-based TCP server
  - JSON message protocol
  - Client-server state synchronization
  - 30 Hz server tick rate, 10 Hz client updates
- [x] **Testing Framework**
  - Comprehensive test suite
  - Test runner utility
  - All tests passing

### ✅ Phase 2: Extended Content & Features (Complete)
**Completed**: Q4 2025 - Q1 2026

#### Ships & Content
- [x] **14 Ships** across 3 classes
  - 4 Tech I Frigates (Rifter, Merlin, Tristan, Punisher)
  - 4 Tech I Destroyers (Thrasher, Cormorant, Catalyst, Coercer)
  - 6 Tech I Cruisers (Stabber, Caracal, Vexor, Maller, Rupture, Moa)
- [x] **70 Modules** for ship fitting
  - 10 weapon systems (small/medium, all damage types)
  - 18 defensive modules (shields, armor, hardeners)
  - 18 utility modules (EWAR, propulsion, tracking)
  - 14 drones (light/medium/heavy combat + utility)
- [x] **47 Skills** with prerequisites
  - Weapon skills for all turret types
  - Ship piloting (Frigate/Destroyer/Cruiser for all 4 races)
  - Drone operation (6 skill tree)
  - Engineering and electronic warfare
- [x] **13 NPCs** across multiple factions
  - Serpentis, Guristas, Blood Raiders, Sansha's Nation
  - Angel Cartel, Rogue Drones
  - Frigate, Destroyer, and Cruiser class NPCs
- [x] **15 Missions** (Level 1-4)
  - Combat, exploration, courier, mining missions
  - Varied objectives and rewards

#### Core Gameplay Systems
- [x] **Module Fitting System**
  - CPU/PowerGrid management
  - Slot validation (high/mid/low/rig)
  - Module activation/deactivation
- [x] **Drone System**
  - Launch, recall, engage mechanics
  - Bandwidth management
  - Drone AI and control
  - Multiple drone types
- [x] **Skill Training System**
  - Skill queue management
  - SP (Skill Points) accumulation
  - Skill bonuses application
  - Prerequisite validation
- [x] **Mission System**
  - Accept, track, complete missions
  - Mission rewards (ISK, LP, items)
  - Multiple mission types
- [x] **Navigation & Warp**
  - FTL travel mechanics
  - Alignment calculations
  - Warp speed based on ship stats
- [x] **Docking System**
  - Station docking/undocking
  - Station services access
  - Inventory management while docked
- [x] **Stargate System**
  - System-to-system travel
  - Jump mechanics
  - Universe connectivity
- [x] **Advanced Movement**
  - Approach command
  - Orbit command
  - Speed and distance management
- [x] **Combat Enhancements**
  - Full resistance system (EM/Thermal/Kinetic/Explosive)
  - Damage type calculations
  - Optimal/falloff range mechanics
  - Shield/Armor/Hull damage distribution

#### Visual & UI Features
- [x] **Pygame-based GUI** (2D)
  - 2D space visualization with star field
  - Visual ship representations
  - Health bar overlays (Shield/Armor/Hull)
  - Weapon effects and visual feedback
- [x] **Interactive Camera**
  - Pan, zoom controls
  - Follow mode
  - Smooth camera movement
- [x] **HUD/UI Overlay**
  - Status information display
  - Combat log
  - Target information
  - Ship status readouts
- [x] **Standalone GUI Demo** - No server required
- [x] **GUI Multiplayer Client** - Connect to server with graphics

### ✅ Phase 3: Advanced Gameplay Systems (Complete)
**Completed**: Q1 2026

#### Industry & Economy
- [x] **Manufacturing System**
  - Blueprint management (BPO/BPC)
  - Material Efficiency (ME) research (0-10 levels)
  - Time Efficiency (TE) research (0-20 levels)
  - Manufacturing queue system
  - Blueprint copying mechanics
  - Material requirements with efficiency bonuses
- [x] **Market System**
  - Buy/sell order placement
  - Market order book with price sorting
  - Instant buy/sell transactions
  - ISK wallet management
  - Broker fees (3%) and sales tax (2%)
  - Transaction history tracking
  - NPC base prices
  - Trade hubs (Jita, Amarr, Dodixie, Rens)
- [x] **Inventory System**
  - Item storage with capacity management
  - Cargo hold mechanics
  - Item stacking
  - Inventory operations (add, remove, transfer)

#### Exploration & Rewards
- [x] **Exploration System**
  - Probe scanning mechanics
  - 5 signature types (Combat, Relic, Data, Gas, Wormholes)
  - Scanner probe formation and positioning
  - Scan strength calculations
  - Progressive scan completion (0-100%)
  - Site completion with rewards
  - Directional scanner (D-Scan) with cone angles
- [x] **Loot System**
  - NPC loot drops on death
  - Loot containers (wrecks, cargo cans)
  - Weighted loot tables with rarities
  - Guaranteed + random loot
  - ISK drops
  - Wreck salvaging mechanics
  - Container despawning timers
  - Loot quality tiers (Common to Officer)

#### Group Content
- [x] **Fleet System**
  - Fleet creation and management
  - Fleet roles (FC, Wing/Squad Commanders)
  - Squad and wing organization (up to 256 members)
  - Fleet bonuses from boosters
    - Armor: +10% HP, +5% resists
    - Shield: +10% HP, +5% resists
    - Skirmish: +15% speed, +10% agility
    - Information: +20% range, +15% scan res
  - Target broadcasting
  - Fleet warping
  - Fleet chat and coordination

#### Testing & Quality
- [x] **Comprehensive Test Coverage**
  - 52+ test functions across 5 test suites
  - 56+ individual test cases
  - 100% pass rate
  - < 1 second execution time
  - Tests for all Phase 3 systems
- [x] **Test Infrastructure**
  - Unified test runner (`run_tests.py`)
  - Individual test files for each system
  - Automated test discovery
- [x] **Security & Code Quality**
  - CodeQL security scanning
  - 0 security vulnerabilities
  - Code review process
  - Clean, maintainable code

### ✅ Phase 4: Corporation & Social (Complete)
**Completed**: Q1 2026

#### Corporation Management
- [x] **Corporation System**
  - Corporation creation and management (1M ISK creation cost)
  - Member roles: CEO, Director, Member
  - Corporation tickers (3-5 characters)
  - Member management (invite, remove, role changes)
  - Max 50 members per player corporation
  - NPC corporations for all 4 races initialized
- [x] **Corporation Hangars**
  - 3 hangar divisions with role-based access
  - Division 1: All members
  - Division 2: Directors and CEO
  - Division 3: CEO only
  - Item storage and retrieval
- [x] **Corporation Wallet**
  - Shared ISK pool
  - Deposit/withdrawal with permission system
  - Directors and CEO have wallet access
  - Automatic tax collection from member earnings
- [x] **Corporation Taxes**
  - Configurable tax rate (default 10%)
  - NPC corps charge 11% tax
  - Automatic tax deduction on member earnings
  - Tax funds go to corporation wallet

#### Social Features
- [x] **Contact & Standings System**
  - Add/remove contacts
  - Personal standings (-10 to +10 scale)
  - Standing management and updates
  - Contact blocking/unblocking
- [x] **Mail System**
  - Send/receive mail messages
  - Multiple recipients support
  - Inbox and sent folders
  - Unread message counter
  - Mark as read functionality
  - Delete messages
  - Message labels
  - Blocked sender filtering
- [x] **Chat System**
  - Create private chat channels
  - Password-protected channels
  - Channel operators and moderation
  - Join/leave channels
  - Channel membership tracking
  - Chat message history (last 100 messages)
  - Character muting per channel
  - System channels (Local, Help, Rookie Help)
  - Corporation chat channels

#### Additional Economy
- [x] **Contract System**
  - Item Exchange Contracts
    - Buy/sell items for ISK
    - Item + ISK exchange
    - Collateral support
    - Public/private availability
  - Courier Contracts
    - Transport items between locations
    - Reward and collateral system
    - Time limits for completion
    - Volume tracking
    - Success/failure mechanics
  - Auction Contracts (framework)
    - Starting bid and buyout price
    - Bid history tracking
    - Highest bidder system
  - Contract Management
    - Broker fees (1% of contract value)
    - Contract expiration
    - Cancel outstanding contracts
    - Contract search and filtering

#### Testing & Quality
- [x] **Comprehensive Test Coverage**
  - 39 new test functions across 2 test suites
  - Corporation System: 15 tests (all passing)
  - Social System: 24 tests (all passing)
  - 100% pass rate
  - Total: 91+ test functions across 7 test suites
- [x] **Code Quality**
  - Clean, maintainable code
  - Consistent with existing architecture
  - Full integration with ECS framework

### ✅ Phase 5: 3D Graphics Core Features (Complete)
**Completed**: Q1-Q2 2026

Phase 5 core development completed with procedural ship models, performance optimization, and advanced particle systems. Remaining polish items (PBR materials, audio, asset pipeline) moved to future work.

#### 3D Client Foundation
- [x] **Panda3D Integration** - 3D engine setup and configuration
- [x] **Network Client** - TCP/JSON protocol for server communication
- [x] **Entity Management** - State synchronization with interpolation
- [x] **EVE-Style Camera System** - Orbit, zoom, pan controls
- [x] **Star Field Renderer** - Beautiful space background with 1500+ stars
- [x] **HUD System** - Multi-panel interface
  - Ship status (shields, armor, hull)
  - Target information (distance, health)
  - Speed and position display
  - Combat log (scrolling messages)

#### Visual Effects & Rendering
- [x] **Enhanced Lighting System** - Multi-light setup with fill lights
- [x] **3D Health Bars** - Billboard-rendered health bars above ships
- [x] **Visual Effects** - Weapon beams, projectiles, explosions
- [x] **Shield Hit Effects** - Blue ripple effects on impact
- [x] **Automatic Shader Generation** - Better material appearance

#### Procedural Ship Models (NEW!)
- [x] **84 Unique Ship Models** - 12 ships × 7 factions
  - 4 Frigates: Rifter, Merlin, Tristan, Punisher
  - 4 Destroyers: Thrasher, Cormorant, Catalyst, Coercer
  - 6 Cruisers: Stabber, Caracal, Vexor, Maller, Rupture, Moa (includes variants)
- [x] **Class-Specific Geometry**
  - Frigates: Compact wedge shape with dual engines
  - Destroyers: Long angular design with 3 turrets and dual engines
  - Cruisers: Large ellipsoid with wing structures and quad engines
- [x] **Faction Color Schemes** - 7 distinct visual identities
  - Minmatar (rust brown), Caldari (steel blue), Gallente (dark green)
  - Amarr (gold-brass), Serpentis (purple), Guristas (dark red), Blood Raiders (blood red)
- [x] **Model Caching System** - Efficient model reuse and memory management

#### Performance Optimization (NEW!)
- [x] **4-Level LOD System** - Distance-based detail levels
  - High Detail (< 100 units): Full geometry, 30 Hz updates
  - Medium Detail (100-300 units): 15 Hz updates
  - Low Detail (300-600 units): 5 Hz updates
  - Culled (> 1000 units): Hidden, no updates
- [x] **Distance-Based Culling** - Entities beyond 1000 units automatically hidden
- [x] **Update Rate Throttling** - Reduced CPU usage for distant objects
- [x] **Performance Statistics** - Real-time tracking and monitoring
- [x] **71% FPS Improvement** - From 35 FPS to 60 FPS with 200 entities

#### Advanced Particle System (NEW!)
- [x] **5 Particle Effect Types**
  - Engine trails (blue glowing particles with velocity motion)
  - Shield impacts (cyan/blue radial bursts)
  - Explosions (orange/yellow particle bursts with expansion)
  - Debris (gray metallic tumbling pieces)
  - Warp effects (blue/white streaking tunnel effects)
- [x] **Lifecycle Management**
  - Automatic particle aging and cleanup
  - Smooth animations (position, scale, color transitions)
  - 1000 particle capacity with automatic culling
- [x] **Billboard Rendering** - Particles always face camera
- [x] **Alpha Blending** - Transparent particle effects

#### Testing & Quality
- [x] **Comprehensive Test Coverage**
  - 84/84 ship model tests passing
  - Performance optimization tests (12 test cases)
  - Particle system tests
  - All existing tests still passing (100% compatibility)
- [x] **Code Quality**
  - ~1,850 lines of production code
  - ~370 lines of test code
  - Full documentation in PHASE5_ENHANCEMENTS.md
  - 0 security vulnerabilities (CodeQL verified)

### ✅ Phase 6: Advanced Content & Tech II Ships (COMPLETE)
**Completed**: Q2 2026

#### 3D Client Ship Model Expansion
- [x] **Ship Classification System Updated**
  - Tech II Assault Frigate recognition (6 ships)
  - Tech II Cruiser recognition (20 ships) - **NEW**
  - Battlecruiser recognition (4 ships)
  - Battleship recognition (4 ships)
  - Updated classification methods for all ship types

- [x] **Procedural Model Generation**
  - Tech II Cruiser models (enhanced cruiser design, 6 engines) - **NEW**
  - Battlecruiser models created (medium-large ships, 10-unit length)
  - Battleship models created (massive capital-class, 15-unit length)
  - 6 weapon hardpoints for Battlecruisers
  - 8 weapon hardpoints for Battleships
  - 4-6 engine arrays with glowing exhausts

- [x] **Tech II Cruiser Content** - **NEW**
  - 8 Heavy Assault Cruisers (Vagabond, Cerberus, Ishtar, Zealot, Muninn, Eagle, Deimos, Sacrilege)
  - 4 Heavy Interdiction Cruisers (Broadsword, Onyx, Phobos, Devoter)
  - 8 Recon Ships - Force & Combat (Huginn, Rapier, Falcon, Rook, Arazu, Lachesis, Pilgrim, Curse)
  - 4 Logistics Cruisers (Scimitar, Basilisk, Oneiros, Guardian)
  - Complete stats, bonuses, and resistances for all ships
  - 168 new 3D models (24 ships × 7 factions)

- [x] **Mission System Expansion**
  - 12 new NPCs (Battlecruisers and Battleships)
  - 28 total missions (Level 1-4)
  - Balanced rewards and difficulty progression

- [x] **Comprehensive Testing**
  - 322 total ship models (46 ships × 7 factions) - **UPDATED**
  - All models generated successfully
  - 100% test pass rate
  - Zero performance impact due to efficient caching

- [x] **Documentation**
  - Complete technical documentation (PHASE6_SHIP_MODELS.md)
  - Tech II Cruiser documentation (PHASE6_TECH2_CRUISERS.md) - **NEW**
  - Mission content documentation (PHASE6_CONTENT_EXPANSION.md)
  - Usage examples and integration guides
  - Performance metrics and statistics

---

## In Progress

### 🚀 Phase 6: Additional Content (Optional)
**Status**: Core content complete, optional enhancements available  
**Timeline**: Q2-Q3 2026

Optional enhancements:
- ✅ More Tech II variants: 4 Command Ships (Claymore, Vulture, Astarte, Absolution)
- ✅ More modules: 8 Tech II EWAR modules, 4 Tech II Logistics modules
- ✅ Epic arc missions: 4 arcs (12 storyline missions) across all empire factions
- ✅ Faction modules: 8 faction modules (4 weapon upgrades, 4 defensive)
- ✅ Officer modules: 4 rare officer-grade modules
- ✅ More Tech II variants (second HAC per race: Muninn, Eagle, Deimos, Sacrilege)
- ✅ Additional mission content (Level 5 missions — 8 high-end PVE missions)

---

## Planned Features

### ✅ Phase 5 Polish: 3D Graphics Enhancements (COMPLETE)
**Timeline**: Q2 2026  
**Status**: ✅ COMPLETE

Phase 5 core features (Panda3D client, ship models, performance optimization, particles) and polish features (Asset Pipeline, PBR Materials, Audio System) are now complete!

#### Asset Pipeline (✅ COMPLETE)
- [x] Import external 3D model formats
  - [x] .obj file support
  - [x] .gltf/.glb support
  - [x] .fbx support (via conversion with fbx2bam)
- [x] Model validation and optimization
- [x] Texture loading and management
- [x] Asset caching system

#### PBR Materials & Lighting (✅ COMPLETE)
- [x] Physically-based rendering (PBR) shader system
  - [x] Metallic/roughness workflow
  - [x] Normal mapping
  - [x] Emission maps
- [x] Enhanced lighting
  - [x] Multi-light setup with fill lights
  - [ ] Dynamic shadows (future enhancement)
  - [ ] Ambient occlusion (future enhancement)
  - [ ] Bloom and HDR (future enhancement)
- [x] Realistic material properties for ships

#### Audio System (✅ COMPLETE)
- [x] Sound effects engine integration
  - [x] Weapon fire sounds
  - [x] Explosion sounds
  - [x] Ship engine sounds
  - [x] UI interaction sounds
- [x] Music system
  - [x] Background music tracks
  - [x] Dynamic music based on game state
  - [x] Volume controls
- [x] 3D audio positioning

#### Additional UI/UX (✅ COMPLETE)
- [x] More interactive UI panels
  - [x] Inventory management UI
  - [x] Fitting window
  - [x] Market interface
  - [x] Station services
- [x] Minimap/radar display
- [x] Enhanced targeting interface
- [ ] Visual feedback improvements (future enhancement)

**Estimated Effort**: 4-6 weeks (completed in 1 session!)  
**Blockers**: None (Phase 5 core complete, can be added incrementally)

---

### 📋 Phase 6: Advanced Content (Planned)
**Timeline**: Q3-Q4 2026  
**Priority**: Medium

#### More Ships
- [x] Tech II ships (all classes)
- [x] Battleships (4 races)
- [x] Tech II Battleships (Marauders: Vargur, Golem, Kronos, Paladin)
- [x] Battlecruisers (4 races)
- [x] Tech II Frigates (Assault, Interceptor, Covert Ops, Stealth Bomber)
- [x] Tech II Destroyers (Interdictors: Sabre, Flycatcher, Eris, Heretic)
- [x] Tech II Cruisers (HAC, HIC, Recon, Logistics)
- [x] Industrial ships
- [x] Mining barges
- [x] Capital ships (Carriers, Dreadnoughts)

#### More Modules
- [x] Tech II modules (all types)
- [x] Officer modules (rare drops)
- [x] Faction modules
- [x] Capital-sized modules (shield booster, armor repairer, logistics, weapons, siege, triage, doomsday)
- [x] Advanced EWAR modules
- [x] Cloaking devices (Prototype, Improved, Covert Ops)
- [x] Jump drives (Economy, Standard)
- [x] Medium Tech II weapons (autocannon, blasters, railgun, pulse/beam lasers, heavy/rapid light missiles)
- [x] Large Tech II weapons (800mm autocannon, 1400mm artillery, mega pulse/tachyon beam lasers, 425mm railgun, neutron blaster, cruise/torpedo launchers)

#### More Skills
- [x] Capital ship skills (carriers, dreadnoughts, titans for all 4 races)
- [x] Advanced weapon skills (large turret specializations, capital turrets)
- [x] Leadership skills (Wing Command, Fleet Command, Warfare Link Specialist, 4 warfare specializations, Mining Director)
- [x] Jump skills (Jump Drive Operation, Calibration, Fuel Conservation)
- [x] Cloaking skills
- [x] Advanced industrial skills (Advanced Industry, Supply Chain Management)

#### More Missions & Content
- [x] Level 5 missions (6 extreme difficulty fleet missions)
- [x] Epic mission arcs
- [x] Incursions (group PVE events) — 8 encounters, 4 tiers, 6 NPC types
- [x] More NPC factions (Mordu's Legion, Sisters of EVE)
- [x] Rare NPC spawns (4 faction commanders, 4 officer NPCs)
- [x] More exploration sites (12 new templates: combat, relic, data, wormhole anomalies)
- [x] Special anomalies (wormhole anomalies for C1-C3 and C4-C6 space)

**Estimated Effort**: 6-8 weeks  
**Blockers**: None

---

### 📋 Phase 7: Advanced Systems (COMPLETE) ✅
**Timeline**: Q4 2026+  
**Priority**: High
**Status**: Mining, Ice Mining, Planetary Interaction, Research & Invention, C++ Server–Client Integration, Wormhole Space, and Advanced Fleet Mechanics complete.

#### ✅ Mining & Resource Gathering (COMPLETE)
- [x] **Mining Laser Operations** - Cycle-based ore extraction
- [x] **15 Ore Types** - From common Veldspar to legendary Mercoxit
- [x] **Mining Components** - MiningLaser, MiningYield, OreHold
- [x] **Mining Modules** - 4 laser types, 3 upgrades, survey scanner
- [x] **Mining Skills** - Mining, Astrogeology, Mining Upgrades, etc.
- [x] **Ore Reprocessing** - Refine ore into minerals at stations
- [x] **Refining Skills** - Reprocessing and Reprocessing Efficiency
- [x] **Skill-Based Yields** - Up to +50% from skills
- [x] **Module Bonuses** - Mining Laser Upgrades with stacking penalties
- [x] **Mining Barges** - Procurer, Retriever, Covetor (3 ships)
- [x] **3D Mining Barge Models** - Industrial design with 21 models (3 ships × 7 factions)
- [x] **Comprehensive Testing** - 29 tests (25 mining + 4 barge + 25 ice), 100% pass rate
- [x] **Complete Documentation** - PHASE7_MINING.md + PHASE7_ICE_MINING.md created

**See [docs/development/PHASE7_MINING.md](docs/development/PHASE7_MINING.md) for complete mining documentation!**
**See [docs/development/PHASE7_ICE_MINING.md](docs/development/PHASE7_ICE_MINING.md) for ice mining documentation!**

#### Ice Mining (✅ COMPLETE - NEW!)
- [x] **Ice Harvesting** - Cycle-based ice extraction (5 min cycles)
- [x] **12 Ice Types** - From Clear Icicle to Enriched Clear Icicle
- [x] **Ice Harvester Modules** - Ice Harvester I/II
- [x] **Ice Skills** - Ice Harvesting (-5% cycle time), Ice Processing (+2% yield)
- [x] **Ice Reprocessing** - Refine into isotopes and fuel materials
- [x] **Isotope Production** - Helium, Nitrogen, Oxygen isotopes for capital ships
- [x] **Ice Fields** - Persistent ice belts with depletion mechanics
- [x] **Full Testing** - 25 ice mining tests, 100% pass rate

#### Mining & Resource Gathering (Optional Enhancements)
- [x] Exhumer ships (Skiff, Mackinaw, Hulk)
- [x] Gas harvesting
- [x] Moon mining (group content)
- [x] Ore compression
- [x] Mining missions (10 missions, levels 1-4)

#### ✅ C++ Server–Client Integration (COMPLETE)
- [x] **Game Session Manager** - Bridges TCP networking to ECS world
- [x] **Client Connect Handling** - Player entity spawning on connect
- [x] **Connect Ack Protocol** - Returns player_entity_id to client
- [x] **State Broadcast** - Sends entity positions/velocities/health each tick
- [x] **Spawn Notifications** - Sends spawn_entity to newly connected clients
- [x] **Input Processing** - Handles input_move to update player velocity
- [x] **Disconnect Cleanup** - Removes player entity, notifies other clients
- [x] **NPC Demo Entities** - 3 hostile NPCs spawned on server startup
- [x] **Chat Broadcasting** - Relays chat messages to all connected clients
- [x] **Server-side target locking protocol** - TARGET_LOCK/TARGET_UNLOCK messages with TargetingSystem integration
- [x] **Server-side module activation** - MODULE_ACTIVATE/MODULE_DEACTIVATE messages with weapon/capacitor validation
- [x] **Data-driven ship stats from JSON files** - ShipDatabase loads 66 ship templates from data/ships/*.json

#### ✅ Resource Gathering Systems (COMPLETE)
- [x] **Asteroid mining** - Mining lasers, 15 ore types, mining barges
- [x] **Ore processing/refining** - Reprocessing system with skills
- [x] **Ice mining** - Ice harvesters, 12 ice types, isotope production
- [x] **Moon mining** - Group content for T2 materials
- [x] **Gas harvesting** - Cloud scooping for reactions
- [x] **Ore compression** - Reduce ore volume for transport

#### Future Mining Enhancements (Optional)
- [x] Mining crystals (improved yield) — 30 crystals (Tech I/II for 15 ore types)
- [x] Mining missions (10 missions, levels 1-4)

#### ✅ Planetary Interaction (COMPLETE)
- [x] **Planet scanning** - 8 planet types with resource deposits
- [x] **Colony management** - Command center with CPU/PG management
- [x] **Resource extraction** - ECU with 5 extraction heads
- [x] **Manufacturing chains** - Basic & Advanced Industrial Facilities
- [x] **Customs offices** - Launchpad for import/export
- [x] **Production Chains** - 16 T0 → 15 T1 → 21 T2 → 9 T3 → 6 T4 materials
- [x] **13 Tests** - 100% pass rate
- [x] **Complete Documentation** - PHASE7_PLANETARY_INTERACTION.md

**See [docs/development/PHASE7_PLANETARY_INTERACTION.md](docs/development/PHASE7_PLANETARY_INTERACTION.md) for complete PI documentation!**

#### ✅ Research & Invention (COMPLETE)
- [x] **Tech II Blueprint Invention** - Invent T2 BPCs from T1 BPCs
- [x] **Datacores** - 10 types for different science skills
- [x] **R&D Agents** - Passive datacore generation (50 RP/day per skill level)
- [x] **Invention Success Rates** - 18-34% base, modified by skills and decryptors
- [x] **10 Decryptors** - Modify success chance, runs, ME, and TE
- [x] **15 Science Skills** - 10 core sciences + 4 encryption methods + base Science
- [x] **Skill Bonuses** - +1% success per level per skill
- [x] **8 Tests** - 100% pass rate
- [x] **Complete Documentation** - PHASE7_RESEARCH_INVENTION.md

**See [docs/development/PHASE7_RESEARCH_INVENTION.md](docs/development/PHASE7_RESEARCH_INVENTION.md) for complete invention documentation!**

#### ✅ Wormhole Space (COMPLETE)
- [x] **Wormhole Generation** - WormholeConnection component with dynamic spawning support
- [x] **Wormhole Effects** - 6 system-wide effect types (Magnetar, Pulsar, Black Hole, Wolf-Rayet, Red Giant, Cataclysmic Variable)
- [x] **Mass & Stability** - Wormhole mass tracking, single-ship mass limits, lifetime decay, and collapse mechanics
- [x] **Sleeper NPCs** - 5 Sleeper NPC templates across frigate/cruiser/battleship classes with 70% omni-resists
- [x] **Enhanced Exploration Sites** - SolarSystem component for wormhole-class tracking and effect application
- [x] **Wormhole Classes** - C1-C6 difficulty tiers with scaling rewards, sleeper spawns, and ship restrictions
- [x] **WormholeDatabase** - Data-driven JSON loader for wormhole classes and effects
- [x] **WormholeSystem** - ECS system for lifetime decay, mass depletion, and jump validation
- [x] **Protocol Extensions** - WORMHOLE_SCAN and WORMHOLE_JUMP message types
- [x] **Comprehensive Testing** - 40 new test assertions across 9 test functions, 102/102 total pass rate

**See data/wormholes/ for wormhole class and effect definitions!**

#### ✅ Advanced Fleet Mechanics (COMPLETE)
- [x] **Fleet Creation & Disbanding** - Create/disband fleets with FC permissions
- [x] **Fleet Membership** - Invite, join, leave with auto-promotion on FC departure
- [x] **Fleet Roles** - FleetCommander, WingCommander, SquadCommander, Member roles with promotion/demotion
- [x] **Fleet Formations** - Wing and squad organization with assignment mechanics
- [x] **Fleet Bonuses** - 4 booster types (Armor +10% HP/+5% resist, Shield +10% HP/+5% resist, Skirmish +15% speed/+10% agility, Information +20% range/+15% scan res)
- [x] **Target Broadcasting** - Broadcast targets to all fleet members for synchronized locking
- [x] **Fleet Warp** - FC/Wing Commander initiated fleet-wide warp commands
- [x] **FleetMembership Component** - ECS component tracking fleet ID, role, squad, wing, and active bonuses
- [x] **Comprehensive Testing** - 77 new test assertions, 178/178 total pass rate

**Estimated Effort**: 12-16 weeks  
**Blockers**: Phase 4-6 completion

---

### 📋 Phase 8: Cinematic Warp & Transitional States (In Progress)
**Timeline**: 2027  
**Priority**: High  
**Goal**: Transform warp travel from empty transit into an immersive, meditative gameplay state

#### Warp Tunnel Visual System
- [x] **Layered shader stack** — 5-layer warp tunnel (radial distortion, starfield velocity bloom, procedural noise skin, ship silhouette anchor, vignette + peripheral falloff) — WarpCinematicSystem computes 4 visual layers via computeLayers()
- [x] **Ship-mass-driven intensity** — Heavier ships bend space harder, deeper bass, slower tunnel oscillation — computeCompositeIntensity() uses mass_norm
- [x] **Warp entry/exit choreography** — Alignment phase → FOV tighten → tunnel cruise → deceleration bloom → arrival — WarpState::WarpPhase enum (None, Align, Entry, Cruise, Event, Exit)
- [x] **Meditative breathing effect** — Slow pulsing during cruise phase (~12.5s cycle for frigates, ~20s for capitals), subtle color temperature shift, syncs with audio system via getBreathingIntensity()
- [ ] **Performance budget** — ≤1.2ms total GPU cost (single fullscreen pass, no dynamic branching)

#### Warp Audio System
- [x] **Engine core sub-bass** (30–60 Hz sine, controller rumble sync) — WarpAudioProfile engine_core_volume/pitch
- [x] **Warp field harmonics** (phase-shifted stereo, slow LFO evolution) — WarpAudioProfile harmonics_volume/pitch
- [x] **Environmental Doppler shimmer** (distance-triggered one-shots, long tails) — WarpAudioProfile shimmer_volume
- [x] **Audio generation** — AudioGenerator::generate_warp_drone (meditative drone with mass-based pitch), generate_warp_entry (rising acceleration), generate_warp_exit (falling deceleration)
- [x] **Audio event system** — WarpEffectRenderer fires WarpAudioEvent (ENTRY_START, CRUISE_START/STOP, EXIT_START/COMPLETE) on phase transitions
- [x] **Breathing sync** — getBreathingIntensity() provides visual-to-audio modulation sync
- [ ] **Optional meditation layer** (sustained pads, no melody, fade in after 15–20s of warp)
- [ ] **Audio progression curve** — Tension → stabilize → bloom → full meditative state

#### Warp Anomalies & Mid-Warp Events
- [x] **Visual-only phenomena** (~1 per 3–5 warps) — tunnel eddies, color shifts, phantom star clusters — WarpAnomalySystem with 4 visual templates
- [x] **Sensory distortions** (~1 per 10–15 warps) — bass deepens, harmonics detune — WarpAnomalySystem with 3 sensory templates
- [x] **Spacetime shear events** (~1 per 50–100 warps) — fracturing tunnel, distant megastructures, lore log rewards — WarpAnomalySystem with 2 shear templates
- [x] **Legendary events** (ultra-rare, opt-in) — collapsed gate scars, ancient battlefield echoes — WarpAnomalySystem with 1 legendary template (1/200 chance)

#### HUD Travel Mode
- [ ] **Soft edge treatment** — Edges soften, bright colors desaturate, tactical warnings muted during warp
- [ ] **Safe-area padding** — 32–48px bottom margin, HUD scaled inward to 95%
- [ ] **Optional UI flair** (player toggle) — Animated brackets, UI glow synced to engine bass, subtle HUD parallax

#### Accessibility & Comfort
- [x] **Motion intensity slider** (0%–100% distortion control) — WarpProfile comfort_scale, accessibility scaling in WarpCinematicSystem
- [ ] **Bass intensity slider** (maps to engine core volume)
- [ ] **Peripheral blur scalar**
- [ ] **Tunnel geometry toggle** (off = star streaks only, on = full warp skin)
- [ ] **Auto-comfort rules** — Reduce oscillation on FPS drop, clamp distortion on ultrawide

#### Warp ECS Components
- [x] `WarpStateComponent` (phase, warpTime, distanceRemaining) — WarpState component with WarpPhase enum
- [x] `WarpProfileComponent` (warpSpeed, massNorm, intensity, comfortScale) — WarpProfile component with serialization
- [x] `WarpVisualComponent` (distortionStrength, tunnelNoiseScale, vignetteAmount) — WarpVisual component with serialization
- [x] `WarpAudioComponent` (bassLevel, harmonicLevel, shimmerChance, meditationLayerActive) — WarpAudioProfile component with serialization
- [x] `WarpEventComponent` (currentEvent, eventTimer, severity) — WarpEvent component with serialization

---

### 📋 Phase 9: Fleet AI Personality & Social Systems (In Progress)
**Timeline**: 2027  
**Priority**: High  
**Goal**: Make fleet members feel like living characters with memory, personality, and emotional depth

#### Captain Personality System
- [x] **Personality axes** — Aggression, sociability, optimism, professionalism, loyalty, humor, risk tolerance — CaptainPersonality component with 8 personality axes + CaptainPersonalitySystem
- [x] **Faction personality profiles** — Each of the 4 factions has distinct cultural modifiers affecting chatter tone and behavior — FactionCulture component with per-faction modifiers
- [x] **Personality-driven behavior** — Affects chatter frequency, formation tightness, morale response, willingness to stay — FleetChatterSystem + FleetFormationSystem integration

#### Fleet Radio Chatter
- [x] **Context-aware dialogue** — Chatter during warp, mining, combat, exploration, idle, and salvage operations — FleetChatterSystem with context-based line selection
- [x] **Interruptible chatter** — Higher-priority events (combat alerts, anomalies) naturally cut off lower-priority small talk — FleetChatterSystem::interruptChatter() with priority comparison
- [x] **Timing rules** — One line every 20–45s max, no overlap, cooldowns between speakers — isAnyoneSpeaking() overlap prevention + std::clamp(cooldown, 20, 45)
- [x] **Combat memory references** — Captains reference actual encounters (ships destroyed, close calls, who saved who) — CaptainMemory component with MemoryEntry records
- [ ] **Positional audio** — Voices originate from ship position in formation, warp tunnel reverb effects

#### Fleet Morale & Memory
- [x] **Persistent fleet memory** — Each captain tracks missions together, wins, losses, ships lost, saves — FleetMorale component + CaptainMemory component
- [x] **Morale model** — Soft scoring: wins × 1.0 − losses × 1.5 − shipsLost × 2.0 + saves × 1.2 — FleetMoraleSystem with configurable weights
- [x] **Morale states** (Inspired → Steady → Doubtful → Disengaged) — Affects chatter tone, formation, willingness — FleetMorale::morale_state with 4 states
- [x] **Emotional arcs** — Captains change over time (optimist → disillusioned, survivor → mentor) — EmotionalArcSystem with EmotionalState component

#### Captain Social Graph
- [x] **Relationship tracking** — Bidirectional affinity scores (-100 grudge → +100 bond) — CaptainRelationship component with affinity per pair
- [x] **Relationship modifiers** — Saved in combat (+10), abandoned (-20), shared long warp (+passive), kill credit stolen (-8) — CaptainRelationshipSystem::modifyRelationship()
- [x] **Friendship effects** — Fly closer, back up faster, reference each other — FleetFormationSystem::applyRelationshipSpacing() sets spacing_modifier 0.7 for friends
- [x] **Grudge effects** — Fly wider, delay responses, sarcastic comms — spacing_modifier 1.5 for grudges via CaptainRelationship affinity

#### Fleet Dreams & Rumors
- [x] **Rumor system** — Non-authoritative truths from warp anomalies, rare visuals, near-miss events — RumorLog component with belief_strength tracking
- [x] **Rumor propagation** — Spread through chatter, reinforced if seen again, fade if unconfirmed — FleetChatterSystem::propagateRumor() with belief halving for second-hand, reinforcement on re-hearing
- [ ] **Rumor-to-questline graduation** — Repeated rumors surface as optional investigations or encounter chains

#### Fleet Departure & Transfers
- [x] **Disagreement model** — Based on risk × (1-aggression) + losses × (1-optimism) + task mismatch — FleetChatterSystem::computeDisagreement()
- [ ] **Organic departure** — Comes up in chatter first → formal request → peaceful departure or splinter fleet
- [ ] **Transfer requests** — High morale captains request bigger ships; low morale request escort-only roles

#### Player Silence Awareness
- [x] **Player presence tracking** — Time since last command, time since last speech — PlayerPresence component with serialization
- [x] **Silence interpretation** — Fleet reacts to player silence ("Quiet today, boss", "You alright up there?") — FleetChatterSystem::getSilenceAwareLine() triggers after 120s of PlayerPresence silence

#### Fleet Chatter ECS Components
- [x] `CaptainPersonalityComponent` (aggression, sociability, optimism, professionalism) — CaptainPersonality with 8 axes + serialization
- [x] `CaptainRelationshipComponent` (otherCaptain, affinity) — CaptainRelationship with relationships vector + serialization
- [x] `EmotionalStateComponent` (confidence, trustInPlayer, fatigue, hope) — EmotionalState component + serialization
- [x] `RumorComponent` (type, beliefStrength, personallyWitnessed) — RumorLog component + serialization
- [x] `PlayerPresenceComponent` (timeSinceLastCommand, timeSinceLastSpeech) — PlayerPresence component + serialization
- [x] `FactionCultureComponent` (faction behavior modifiers) — FactionCulture component + serialization

---

### 📋 Living Universe: Background Simulation & NPC Intents (In Progress)
**Timeline**: 2026  
**Priority**: High  
**Goal**: A universe that reacts over time and remembers player impact. NPCs as real actors.

#### Core Background Simulation
- [x] **SimStarSystemState component** — Per-system state vector (traffic, economy, security, threat, faction influence, pirate activity, resource availability, price modifier)
- [x] **BackgroundSimulationSystem** — Continuous background simulation tick updating state vectors
- [x] **Threat decay** — Threat naturally decays toward 0 over time
- [x] **Economic recovery** — Economy recovers toward 0.5 baseline
- [x] **Resource regeneration** — Resources slowly regenerate toward 1.0
- [x] **Traffic drift** — Traffic level drifts toward baseline
- [x] **Pirate activity growth** — Pirate activity increases in low-security systems
- [x] **Dynamic price modifier** — Prices respond to supply/demand (0.5–2.0 range)
- [x] **Threshold-based events** — Pirate surge, resource shortage, lockdown triggered by conditions
- [x] **Event timer system** — Events have configurable duration, cleared when conditions subside
- [x] **Query API** — getSystemState(), isEventActive(), getSystemsWithEvent()

#### Intent-Driven NPC AI
- [x] **SimNPCIntent component** — Intent enum (Idle, Trade, Patrol, Hunt, Explore, Flee, Escort, Salvage, Mine, Haul, Dock)
- [x] **NPC archetypes** — Trader, Pirate, Patrol, Miner, Hauler, Industrialist with personality-driven weights
- [x] **NPCIntentSystem** — Evaluates intents based on archetype weights, system state, and personal state
- [x] **Intent scoring** — Combines personality weight, star system state, health, and cargo level
- [x] **Flee override** — NPCs automatically flee when hull below 25%
- [x] **Archetype weight profiles** — applyArchetypeWeights() for each archetype
- [x] **Intent cooldown** — Prevents rapid intent switching (configurable interval)
- [x] **Force intent API** — forceIntent() for scripted/event-driven behavior
- [x] **Query API** — getIntent(), getNPCsWithIntent(), getNPCsByArchetype(), scoreIntents()

#### Remaining Work
- [ ] NPC behavior trees (per archetype)
- [ ] NPC rerouting based on system danger
- [ ] Ambient traffic spawns driven by system state
- [ ] AI as real economic actors (wallets, ship ownership, permanent death)
- [ ] Local reputation per system
- [ ] Wreck persistence and salvage NPCs
- [ ] Security response delay
- [ ] System threat adjustment from combat
- [ ] Station news feed
- [ ] Rumors about player actions (`AtlasInformationPropagationSystem`)
- [ ] Visual cues (lockdowns, traffic density)
- [ ] Economy engine: supply/demand curves driven by NPC activity

---

### 📋 Phase 10: Tactical Overlay & Strategy View (In Progress)
**Timeline**: 2027  
**Priority**: Medium  
**Goal**: Passive, data-truth spatial overlay for precise distance judgment and positioning

#### Core Overlay System
- [x] **Distance rings** — Concentric circles at fixed world-unit radii (5, 10, 20, 30, 50, 100 units/km) — TacticalOverlayState::ring_distances with configurable distances
- [x] **Tool range ring** — Dynamic ring for active tool (mining laser, weapon), color-coded by type — TacticalOverlaySystem::setToolRange()
- [x] **Entity projection** — Flattened tactical plane with vertical encoding (ticks for above/below) — TacticalProjection component with projected_x/y + vertical_offset
- [x] **Shared filters** — Overlay shares filter state with Overview and world brackets — TacticalOverlayState::filter_categories + setFilterCategories()/getFilterCategories()

#### Interaction Rules
- [x] **Passive display only** — No clickable elements, no dragging, no entity selection — TacticalOverlayState::passive_display_only flag (true by default)
- [x] **Toggle hotkey** (V, configurable) — TacticalOverlaySystem::toggleOverlay()
- [x] **Scales to large entity counts** — Muted asteroids, high-contrast hostiles, heavier structure silhouettes — TacticalOverlayState::entity_display_priority + setEntityDisplayPriority()

#### Staged Implementation
- [x] **Stage 1**: Toggle overlay + distance rings (no entities) — TacticalOverlaySystem with toggle + configurable ring distances
- [x] **Stage 2**: Entity projection with vertical ticks and shared filters — TacticalProjection component with serialization
- [x] **Stage 3**: Tool awareness (active tool range, color coding) — TacticalOverlaySystem::setToolRange() with tool_type
- [ ] **Stage 4**: Fleet extensions (anchor rings, wing bands)

#### Tactical Overlay ECS Components
- [x] `TacticalOverlayState` (enabled, ringDistances[]) — implemented with serialization
- [x] `TacticalProjection` (projectedPosition, verticalOffset) — implemented with serialization

---

### 📋 Phase 11: Fleet-as-Civilization & Endgame (Planned)
**Timeline**: 2027–2028  
**Priority**: Medium  
**Goal**: Transform late-game fleets into traveling civilizations with distributed economy and station building

#### Fleet Progression Stages
- [ ] **Phase 1** (Early): Max 5 ships — Player + 4 captains, basic personalities + chatter
- [ ] **Phase 2** (Mid): Up to 15 ships — 3 wings × 5, wing commanders, role specialization (mining/combat/logistics)
- [ ] **Phase 3** (End): 25 ships — 5 wings × 5, full doctrine (mining, salvage, logistics, escort, construction)

#### Fleet Cargo Pool
- [ ] **Distributed inventory** — Fleet cargo pool backed by real ship inventories (not magic storage)
- [ ] **Ship loss = cargo loss** — Fleet pool recalculates immediately on ship destruction
- [ ] **Capacity scaling** — Σ(shipCargo × logisticsEfficiency × captainSkill × moraleModifier)
- [ ] **Fleet inventory UI** — Scrollable categories (salvage, fleet supplies, artifacts, rumors)

#### Station Deployment
- [ ] **Station-deployment ship class** — Ships that deploy into permanent stations
- [ ] **Attachable station modules** — Upgrade and add features to deployed stations
- [ ] **Solar system upgrading** — Station presence upgrades stats of the settled system

#### Fleet Warp Formation
- [ ] **Fleet warp-in-formation** — All fleet members warp visibly alongside player (no teleport pop-ins)
- [ ] **Formation types** — Fighters (loose diamond), frigates (tight echelon), capitals (wide, slow drift)
- [ ] **Formation breathing** — Subtle 0.02–0.05 Hz oscillation for organic feel
- [ ] **Visual interaction** — Warp distortion bends around large ships, smaller ships' wakes ripple

#### Civilization-Scale Systems
- [ ] **Titan as civilizational threshold** — Requires stable logistics, loyal captains, fleet history, fleet-scale industry
- [ ] **Fleet as moving polity** — Distributed economy, not magic storage
- [ ] **Save-file persistent fleet history** — Captain personalities, relationships, major events, rumors persist across sessions

---

### 📋 Phase 12: Procedural Ship Generation Overhaul (Planned)
**Timeline**: 2027  
**Priority**: High  
**Goal**: Ships that read in silhouette — spine-based hull grammar replacing blob-assembly

#### Hull Grammar System
- [ ] **Spine selection** — Needle, Wedge, Hammer, Slab, Ring — defines ship purpose
- [ ] **Functional zone ordering** — Command/bridge → mid-hull → engine block (always in order, parameterized)
- [ ] **Greebles last** — Antennas, armor plates, cargo pods added only after silhouette is clean
- [ ] **Bilateral symmetry enforcement**
- [ ] **Aspect ratio clamping** and hull elongation (1.5×)
- [ ] **Clear engine cluster** generation for every hull type

#### Faction Shape Language
- [ ] **Solari** — Golden, elegant spires, flowing curves
- [ ] **Veyren** — Angular, utilitarian blocks, sharp edges
- [ ] **Aurelian** — Sleek, organic forms, swept shapes
- [ ] **Keldari** — Rugged, industrial bulk, visible internals

#### Damage & Visual States
- [ ] **Damage decals** — Missing modules, hull breaches visible on damaged ships
- [ ] **Economy-driven generation** — NPC ships reflect economic state (resource-rich = mining hulls, war-torn = damaged patrols)

---

## Future Considerations

### Long-term Goals (2027+)

#### Performance & Scalability
- [ ] Database persistence (SQLite → PostgreSQL)
- [ ] Performance profiling and optimization
- [ ] Interest management for large player counts
- [ ] Client-side prediction for responsive movement
- [x] Spatial partitioning for efficient entity queries — SpatialHashSystem with O(1) queryNear() and queryNeighbours()
- [ ] Multi-threaded server processing
- [x] Large-scale fleet battle stress testing (150-300 ships) — 200-ship multi-system tick stress test (LOD + spatial hash + shield + movement)
- [x] LOD system with impostor billboards for distant ships — LODSystem with 4 distance tiers and force_visible override
- [ ] Group AI abstraction (FleetController with squad leaders)

#### DevOps & Deployment
- [x] CI/CD pipeline (GitHub Actions) — Server and Client workflows
- [x] Automated testing on PR — Server tests (521 assertions) run on push/PR
- [x] Docker containerization — Multi-stage Dockerfile for dedicated server
- [x] Crash reporting and logging — Structured Logger with levels, file output, and crash-safe exception handling
- [x] Server monitoring and analytics — ServerMetrics tracking tick timing, entity/player counts, and uptime
- [ ] Cloud deployment guides (AWS, GCP, Azure)

#### Community & Modding
- [ ] Mod manager utility
- [ ] Content creation tools
- [ ] Mission editor
- [ ] Ship designer
- [ ] Modding documentation and tutorials
- [ ] Community content repository
- [ ] AI profile mods (`ai_profiles.json` for behavior customization)
- [ ] Economy rule mods (`economy_rules.json` for system economy curves)
- [ ] Ship template mods (`ship_templates.json` for custom ships)

#### Additional Features
- [ ] PvP toggle option (optional for those who want it)
- [x] Tournament system
- [x] Leaderboards and achievements
- [ ] In-game web browser (dotlan-style maps)
- [ ] Voice chat integration
- [ ] Mobile companion app

### 📋 Vertical Slice: One Full Star System (Next Major Milestone)
**Timeline**: 3-6 months  
**Priority**: Critical  
**Goal**: Prove all gameplay loops work together in one complete star system

#### System Contents (Scope Lock)
- 1 Trade Hub Station
- 2 Mining Belts
- 1 Pirate Zone
- 3-5 Procedural Anomalies
- AI Traffic (miners, patrols, haulers)
- Player Spawn Point

#### Phase 1 (Weeks 1-3): Foundational Gameplay Loop
- [x] Procedural ship hull + weapons generation — Modular hull assembly with per-faction parts and weapon turrets (Phase 1.1 complete)
- [x] Shield/armor/hull damage with visual feedback — DamageEvent component with DamageEffectHelper for shield/armor/hull visuals
- [x] Basic AI combat (engage, orbit, retreat) — Dynamic orbit by ship class, engagement range, target selection strategies
- [x] Station docking and repair service — StationSystem with dock/undock/repair (29 tests)
- **Success Criteria**: Player can undock, fight NPCs, take damage, dock and repair

#### Phase 2 (Weeks 4-6): Wrecks, Salvage & Economy
- [x] Ship destruction → wreck spawning — WreckSalvageSystem with lifetime decay (22 tests)
- [x] Salvage gameplay mechanics — salvageWreck with range check and item transfer
- [x] Basic mineral economy — MiningSystem + RefiningSystem + MarketSystem with NPC market seeding
- [x] Mining AI ships active — AISystem mining behavior with deposit discovery (6 tests)
- [x] Resource tracking per system — SystemResources component with per-mineral tracking
- **Success Criteria**: NPCs mine, pirates attack, wrecks remain, salvage yields resources, economy changes

#### Phase 3 (Weeks 7-9): Exploration & Anomalies
- [x] Scanner system implementation — ScannerSystem with probe-based scanning, signal accumulation (8 tests)
- [x] Anomaly generation from system seed — AnomalySystem with deterministic seed-based generation (9 tests)
- [x] Combat & mining anomalies — 6 anomaly types (Combat, Mining, Data, Relic, Gas, Wormhole)
- [x] Difficulty scaling by location — DifficultyScalingSystem with security-based multipliers (8 tests)
- [x] Visual distortion cues for anomalies — AnomalyVisualCue component with per-type distortion profiles
- [x] Scanner/Anomaly protocol messages — SCAN_START, SCAN_STOP, SCAN_RESULT, ANOMALY_LIST (19 tests)
- [x] Scanner UI panel — scanner.rml with probe status, scan progress bar, start/stop controls, results table
- [x] Anomaly rendering support — ANOMALY type in Celestial enum, VisualCue enum, SolarSystemScene management methods
- [x] End-to-end scan → discover → warp integration tests (22 tests)
- **Success Criteria**: Player scans, finds new content, it feels special ✅

#### Phase 4 (Weeks 10-12): Procedural Missions & Reputation
- [x] Mission templates implementation — MissionTemplateSystem with 16 templates across 5 types and 5 levels (6 tests)
- [x] Mission generation from world state — MissionGeneratorSystem generates context-appropriate missions from security, resources, anomalies (4 tests)
- [x] Faction reputation system — ReputationSystem with derived standings, agent access gating, 9 faction relationships (6 tests)
- [x] Hostile/friendly AI behavior based on rep — AISystem::selectTarget skips positive-standing entities (2 tests)
- [x] Mission-driven economy shifts — MissionSystem reduces spawn rate (combat) and ore reserves (mining) on completion (3 tests)
- **Success Criteria**: Missions change AI behavior, reputation alters encounters

#### Phase 5 (Weeks 13-16): Persistence & Stress Testing
- [x] Save/load system state — WorldPersistence serializes 32 component types with file I/O (122 new test assertions)
- [x] Fleet state persistence — CaptainPersonality, FleetMorale, CaptainRelationship, EmotionalState, CaptainMemory, FleetFormation, FleetCargoPool, RumorLog all serialized
- [x] Economy persistence — Station, Docked, Wreck, MineralDeposit, SystemResources, MarketHub all serialized
- [x] LOD & impostors for large battles — LODPriority server component with impostor distance, client LODManager integration, LODSystem server-side priority computation with 4 distance tiers
- [x] 100+ ship fleet stress test — 150-ship stress test with LOD priority verification (5 assertions), 200-ship multi-system tick stress test (8 assertions)
- [x] Spatial partitioning — SpatialHashSystem for O(1) queryNear() and queryNeighbours() proximity queries
- [x] Compressed save format — WorldPersistence gzip compression (5-10× smaller files)
- **Success Criteria**: Quit, reload, world remembers everything, stable performance

#### Complete Loop Validation
Player undocks → Scans anomaly → Fights pirates → Ship explodes → Wreck created → Salvage collected → Economy shifts → AI fleets adapt → Missions change → Reputation updates → Save game → Reload → World remembers

---

## Development Milestones

### 2025
- **Q4 2025**: Phase 1 & 2 completed
  - Core engine with ECS
  - Extended content (70 modules, 47 skills, 14 ships)
  - Basic gameplay systems
  - 2D pygame graphics
  - Multiplayer networking

### 2026
- **Q1 2026**: Phase 3 completed ✅
  - Manufacturing system
  - Market economy
  - Exploration system
  - Loot system
  - Fleet system
  - Comprehensive testing

- **Q1 2026**: Phase 4 completed ✅
  - Corporation system (creation, management, roles)
  - Corporation hangars and wallets
  - Social features (contacts, standings, mail)
  - Chat system (channels, moderation)
  - Contract system (item exchange, courier, auction)
  - 39 new tests (91+ total test functions)
  
- **Q1-Q2 2026**: Phase 5 Core completed ✅
  - Panda3D 3D client implementation
  - 84 procedural ship models (12 ships × 7 factions)
  - Performance optimization (60+ FPS, LOD system, culling)
  - Advanced particle system (5 effect types)
  - EVE-style camera and HUD
  - Visual effects (weapon beams, explosions, shield hits)
  - Comprehensive testing (84+ new tests)
  
- **Q2 2026**: Phase 5 Polish completed ✅
  - Asset pipeline for external 3D models (.obj, .gltf, .fbx)
  - PBR materials and realistic lighting
  - Audio system integration with 3D positioning
  - Comprehensive testing and documentation
  
- **Q2-Q4 2026**: Phase 6 (Next - Planned)
  - More ships, modules, skills
  - More missions and content
  - Advanced NPC behaviors
  
- **Q4 2026+**: Phase 7 (Future)
  - Advanced gameplay systems
  - Mining, PI, invention
  - Wormhole space

### 2027
- **Q1–Q2 2027**: Phase 8 — Cinematic Warp & Transitional States
  - Warp tunnel shader stack (5-layer visual system)
  - Adaptive warp audio (sub-bass, harmonics, meditation layer)
  - Warp anomalies & rare mid-warp events
  - HUD travel mode & accessibility settings

- **Q2–Q3 2027**: Phase 9 — Fleet AI Personality & Social Systems
  - Captain personality axes & faction culture modifiers
  - Context-aware fleet radio chatter (warp, mining, combat, idle)
  - Fleet morale, memory, social graph (friendships & grudges)
  - Emotional arcs, rumor system, fleet departure mechanics

- **Q3 2027**: Phase 10 — Tactical Overlay & Strategy View
  - Passive 2.5D spatial overlay with distance rings
  - Entity projection, tool range rings, shared filters
  - Staged rollout (skeleton → entities → tools → fleet extensions)

- **Q3 2027**: Phase 12 — Procedural Ship Generation Overhaul
  - Spine-based hull grammar (Needle, Wedge, Hammer, Slab, Ring)
  - Functional zone ordering, greebles last
  - Faction shape language enforcement
  - Silhouette-first design replacing blob-assembly

- **Q4 2027–2028**: Phase 11 — Fleet-as-Civilization & Endgame
  - 25-ship fleet progression (5 wings × 5)
  - Fleet cargo pool & distributed economy
  - Station deployment & solar system upgrading
  - Fleet warp formation with visual interaction
  - Titan as civilizational threshold

---

## Success Metrics

### Current Achievement
- ✅ **95+ test functions** - All passing (521 assertions across test suites)
- ✅ **49 ships** - Tech I, Tech II, and Mining Barges across all classes
- ✅ **70 modules** - Full fitting options
- ✅ **47 skills** - Complete skill tree
- ✅ **8 major gameplay systems** - Manufacturing, Market, Exploration, Loot, Fleet, Corporation, Social, Contracts
- ✅ **Zero security vulnerabilities** - CodeQL verified
- ✅ **Multiplayer functional** - Server-client architecture working
- ✅ **Corporation system functional** - Full corp management
- ✅ **Social features working** - Corp chat, mail, contacts, contracts
- ✅ **3D client functional** - Panda3D-based 3D client with full networking
- ✅ **343 procedural ship models** - Faction-specific designs (49 ships × 7 factions)
- ✅ **60+ FPS performance** - Achieved with LOD and culling
- ✅ **Advanced particle effects** - 5 effect types, 1000+ particles
- ✅ **Asset Pipeline** - Import external 3D models (.obj, .gltf, .fbx)
- ✅ **PBR Materials** - Physically-based rendering with metallic/roughness
- ✅ **Audio System** - Sound effects and music with 3D positioning
- ✅ **Tech II Cruisers** - HAC, HIC, Recon, Logistics (20 ships)
- ✅ **Phase 7 Mining System** - Complete mining & resource gathering with barges
- ✅ **C++ Server Game Session** - Client connect/disconnect, entity spawning, state broadcast
- ✅ **C++ Server Dedicated Systems** - CapacitorSystem, ShieldRechargeSystem, WeaponSystem
- ✅ **C++ Server Target Locking** - Server-side TARGET_LOCK/TARGET_UNLOCK protocol
- ✅ **C++ Server Module Activation** - Server-side MODULE_ACTIVATE/MODULE_DEACTIVATE protocol
- ✅ **C++ Server ShipDatabase** - Data-driven ship stats from 66 JSON ship templates
- ✅ **C++ Server FleetSystem** - Fleet creation, membership, roles, bonuses, target broadcast, fleet warp
- ✅ **C++ Server InventorySystem** - Item add/remove/transfer with capacity limits (m3)
- ✅ **C++ Server LootSystem** - Wreck creation from NPC LootTable, loot collection with ISK payout
- ✅ **C++ Server NpcDatabase** - Data-driven NPC stats from 32 JSON templates

### Phase 7 Goals (COMPLETE)
- [x] **Mining & Resource Gathering** - Core system complete ✅
- [x] **15 ore types** with complete mineral data ✅
- [x] **Mining skills** (8 new skills) ✅
- [x] **Ore reprocessing** with efficiency system ✅
- [x] **29 mining tests** (100% pass rate) ✅
- [x] **Mining barge ships** - Procurer, Retriever, Covetor ✅
- [x] **Exhumer ships** - Skiff, Mackinaw, Hulk with Strip Miner II ✅
- [x] **Gas harvesting system** - 9 gas types, harvester modules, skill bonuses ✅
- [x] **Ore compression** - 15 ore + 12 ice types, batch compression, skill bonuses ✅
- [x] **Moon mining** - 10 moon ore types, refinery extraction, belt fracturing ✅
- [x] **C++ Server Game Session** - Client connect, state broadcast, NPC spawning ✅
- [x] **C++ Server CapacitorSystem** - Capacitor recharge and consumption ✅
- [x] **C++ Server ShieldRechargeSystem** - Passive shield regeneration ✅
- [x] **C++ Server WeaponSystem** - Weapon cooldowns, auto-fire, capacitor cost, damage cascade ✅
- [x] **C++ Server Target Locking Protocol** - Server-side target lock/unlock messages ✅
- [x] **C++ Server Module Activation Protocol** - Server-side module activate/deactivate messages ✅
- [x] **C++ Server ShipDatabase** - Data-driven ship stats from 66 JSON templates ✅
- [x] **C++ Server FleetSystem** - Fleet creation, membership, roles, bonuses, warp ✅

### Phase 7 Fleet System Goals (COMPLETE)
- [x] **Fleet System** - Complete fleet management in C++ server ✅
- [x] **Fleet Creation/Disbanding** - With FC permissions ✅
- [x] **Fleet Membership** - Invite, join, leave, auto-promote ✅
- [x] **Fleet Roles** - FC, Wing Commander, Squad Commander, Member ✅
- [x] **Wing/Squad Organization** - Hierarchical fleet structure ✅
- [x] **Fleet Bonuses** - 4 booster types with stat bonuses ✅
- [x] **Target Broadcasting** - Synchronized fleet target locking ✅
- [x] **Fleet Warp** - Fleet-wide warp initiation ✅
- [x] **FleetMembership Component** - ECS component for fleet data ✅
- [x] **77 fleet tests** (178/178 total pass rate) ✅

---

## Contributing

Want to contribute? Check out our priorities:

**High Priority (Help Wanted):**
- Phase 6: Additional ship designs and stats (second HACs per race, more Tech II ships)
- ✅ Phase 6: Command Ships added (Claymore, Vulture, Astarte, Absolution)
- ✅ Phase 6: Tech II EWAR and Logistics modules added
- Phase 6: More mission content (Level 5 missions, epic arcs)
- Testing and bug reports
- 3D asset creation (ship models, station models)

**Medium Priority:**
- UI/UX improvements (additional 3D client panels for inventory, fitting, market)
- Documentation improvements
- Content creation (NPCs, missions, exploration sites)
- Performance profiling and optimization

**Low Priority:**
- Advanced features (Phase 7: mining, PI, wormholes)
- Additional gameplay systems
- Community tools and mod support

See [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.

---

## Questions & Feedback

Have questions about the roadmap? Want to suggest features?

- Open an issue on GitHub
- Join our community discussions
- Check out the documentation

---

## Changelog

### R&D - Manual-Aligned Systems (Current)
- Character creation system with 4 races, bloodlines, and attributes
- Clone system with medical clones and jump clones
- Implant system for attribute enhancement (5 slots, 4 grades)
- CONCORD security enforcement and criminal flag mechanics
- Insurance system with 6 coverage levels
- Contract and escrow system (item exchange, courier, auction)
- Corporation system with NPC and player corp mechanics
- Deadspace complexes with 5 difficulty tiers and escalation
- Added Learning, Social, Trade, Leadership, Corporation Management, Mechanic skill categories
- Added Cybernetics and Infomorph Psychology skills for implants/clones
- Added trade, scenario, and storyline mission types per EVE manual
- Design document restructured around EVE Online manual chapters

### R&D - C++ Server Systems
- C++ Server: Added dedicated CapacitorSystem (recharge + consumption API)
- C++ Server: Added dedicated ShieldRechargeSystem (passive regeneration)
- C++ Server: Added WeaponSystem (cooldowns, auto-fire for AI, capacitor cost, damage cascade)
- C++ Server: Added `capacitor_cost` field to Weapon component
- C++ Server: Refactored CombatSystem — shield/capacitor recharge and weapon cooldowns moved to dedicated systems
- C++ Server: 7 systems now registered (Capacitor, ShieldRecharge, AI, Targeting, Movement, Weapon, Combat)
- 31 new C++ tests (all passing)

### R&D - C++ Server Game Session
- C++ Server Game Session Manager implemented
- Server now processes client connect/disconnect messages
- Player entity spawning with Rifter-class frigate stats
- State broadcast: entity positions, velocities, health sent each tick
- NPC demo entities spawned on server startup (Serpentis, Guristas, Blood Raiders)
- Chat message relay to all connected clients
- Protocol compatibility with C++ OpenGL client (connect_ack, spawn_entity, state_update, destroy_entity)
- Server fully builds and runs on Linux (CMake)

### R&D - Phase 5 Polish
- Phase 5 Polish completed: Asset Pipeline, PBR Materials, Audio System
- Added Asset Loader for external 3D models (.obj, .gltf, .fbx)
- Implemented PBR Materials system with metallic/roughness workflow
- Added Audio System with 3D spatial positioning
- Support for weapon sounds, explosions, engine sounds, music
- Comprehensive test coverage for all new systems

### R&D - Phase 5 Core
- Phase 5 Core completed: 3D Graphics and Performance
- Added 84 procedural ship models (12 ships × 7 factions)
- Implemented performance optimization system (60+ FPS, LOD, culling)
- Added advanced particle system (5 effect types)
- Completed 3D client foundation with Panda3D
- 84+ new test functions for Phase 5 features

### R&D - Phase 4 Social
- Phase 4 completed: Corporation & Social features
- Added Corporation System with full management
- Added Social System with contacts, mail, and chat
- Added Contract System for player-to-player trading
- 39 new test functions (91+ total)

### R&D - Initial
- Initial roadmap document
- Complete Phase 1-3 status
- Planned Phase 4-7 features
- 3D graphics options analysis

---

**Status**: In active R&D and development — actively testing until further notice

*This roadmap is a living document and will be updated as the project evolves.*
