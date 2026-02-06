# EVE OFFLINE - Project Roadmap

**Last Updated**: February 3, 2026  
**Version**: 1.2

---

## Table of Contents
1. [Project Vision](#project-vision)
2. [Current Status](#current-status)
3. [Completed Work](#completed-work)
4. [In Progress](#in-progress)
5. [Planned Features](#planned-features)
6. [Future Considerations](#future-considerations)
7. [Development Milestones](#development-milestones)

---

## Project Vision

EVE OFFLINE is a PVE-focused space MMO inspired by EVE Online, designed for small groups of players (2-20). The project aims to recreate the EVE Online experience with:
- Custom Python game engine with ECS architecture
- Server-authoritative multiplayer for cooperative gameplay
- EVE-like mechanics: ships, skills, fitting, combat, missions, exploration
- Fully moddable via editable JSON files
- Focus on PVE content without PVP stress

---

## Current Status

### 🎯 Phase 6 - Advanced Content & Tech II Ships ✅

**Current Version**: Phase 6 Tech II Cruisers Complete  
**Overall Progress**: ~90% of core features implemented  
**Status**: All ship model integration complete (46 ships, 322 models). Tech II Cruisers (HAC, HIC, Recon, Logistics) fully implemented with 3D models. Mission system expansion complete with 28 missions across 4 levels.

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
  - 4 Heavy Assault Cruisers (Vagabond, Cerberus, Ishtar, Zealot)
  - 4 Heavy Interdiction Cruisers (Broadsword, Onyx, Phobos, Devoter)
  - 8 Recon Ships - Force & Combat (Huginn, Rapier, Falcon, Rook, Arazu, Lachesis, Pilgrim, Curse)
  - 4 Logistics Cruisers (Scimitar, Basilisk, Oneiros, Guardian)
  - Complete stats, bonuses, and resistances for all ships
  - 140 new 3D models (20 ships × 7 factions)

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
- More Tech II variants (second HAC per race, Tech II Battlecruisers)
- Additional mission content (Level 5 missions, epic arcs)
- More modules (Tech II EWAR, logistics modules)

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
- [ ] Tech II ships (all classes)
- [ ] Battleships (4 races)
- [ ] Battlecruisers (4 races)
- [ ] Tech II Frigates (Assault, Interceptor, Covert Ops)
- [ ] Tech II Cruisers (HAC, HIC, Recon, Logistics)
- [ ] Industrial ships
- [ ] Mining barges
- [ ] Capital ships (Carriers, Dreadnoughts)

#### More Modules
- [ ] Tech II modules (all types)
- [ ] Officer modules (rare drops)
- [ ] Faction modules
- [ ] Capital-sized modules
- [ ] Advanced EWAR modules
- [ ] Cloaking devices
- [ ] Jump drives

#### More Skills
- [ ] Capital ship skills
- [ ] Advanced weapon skills
- [ ] Leadership skills
- [ ] Jump skills
- [ ] Cloaking skills
- [ ] Advanced industrial skills

#### More Missions & Content
- [ ] Level 5 missions
- [ ] Epic mission arcs
- [ ] Incursions (group PVE events)
- [ ] More NPC factions
- [ ] Rare NPC spawns (faction/officer)
- [ ] More exploration sites
- [ ] Special anomalies

**Estimated Effort**: 6-8 weeks  
**Blockers**: None

---

### 📋 Phase 7: Advanced Systems (IN PROGRESS) 🚀
**Timeline**: Q4 2026+  
**Priority**: High (Mining system complete)
**Status**: Mining & Resource Gathering complete, other systems planned

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
- [ ] Mining missions (framework exists)

#### Other Advanced Systems (Planned)
- [ ] Asteroid mining mechanics (basic framework exists)
- [ ] Ore processing/refining
- [x] Moon mining (group content)
- [x] Gas harvesting
- [ ] Mining crystals and upgrades

#### Planetary Interaction
- [ ] Planet scanning
- [ ] Colony management
- [ ] Resource extraction
- [ ] Manufacturing chains
- [ ] Customs offices

#### Research & Invention
- [ ] Tech II blueprint invention
- [ ] Datacores and research agents
- [ ] Ancient relics (reverse engineering)
- [ ] Tech III component invention

#### Wormhole Space
- [ ] Wormhole generation
- [ ] Wormhole effects
- [ ] Sleeper NPCs
- [ ] Enhanced exploration sites

#### Advanced Fleet Mechanics
- [ ] Fleet formations
- [ ] Advanced fleet commands
- [ ] Fleet bookmarks
- [ ] Coordinate warps
- [ ] Capital ship fleet doctrines

**Estimated Effort**: 12-16 weeks  
**Blockers**: Phase 4-6 completion

---

## Future Considerations

### Long-term Goals (2027+)

#### Performance & Scalability
- [ ] Database persistence (SQLite → PostgreSQL)
- [ ] Performance profiling and optimization
- [ ] Interest management for large player counts
- [ ] Client-side prediction for responsive movement
- [ ] Spatial partitioning for efficient entity queries
- [ ] Multi-threaded server processing

#### DevOps & Deployment
- [ ] CI/CD pipeline (GitHub Actions)
- [ ] Automated testing on PR
- [ ] Docker containerization
- [ ] Cloud deployment guides (AWS, GCP, Azure)
- [ ] Server monitoring and analytics
- [ ] Crash reporting and logging

#### Community & Modding
- [ ] Mod manager utility
- [ ] Content creation tools
- [ ] Mission editor
- [ ] Ship designer
- [ ] Modding documentation and tutorials
- [ ] Community content repository

#### Additional Features
- [ ] PvP toggle option (optional for those who want it)
- [ ] Tournament system
- [ ] Leaderboards and achievements
- [ ] In-game web browser (dotlan-style maps)
- [ ] Voice chat integration
- [ ] Mobile companion app

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

---

## Success Metrics

### Current Achievement
- ✅ **95+ test functions** - All passing across 8 test suites
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

### Phase 7 Goals (IN PROGRESS)
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
- [ ] Other Phase 7 systems (PI, R&D, WH space, etc.)

---

## Contributing

Want to contribute? Check out our priorities:

**High Priority (Help Wanted):**
- Phase 6: Additional ship designs and stats (Tech II ships, Battlecruisers, Battleships)
- Phase 6: More modules (Tech II variants, officer modules)
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

### Version 1.3 (February 2026)
- Phase 5 Polish completed: Asset Pipeline, PBR Materials, Audio System
- Added Asset Loader for external 3D models (.obj, .gltf, .fbx)
- Implemented PBR Materials system with metallic/roughness workflow
- Added Audio System with 3D spatial positioning
- Support for weapon sounds, explosions, engine sounds, music
- Comprehensive test coverage for all new systems
- Updated roadmap to reflect Phase 5 complete status
- Phase 5 is now 100% complete!

### Version 1.2 (February 2026)
- Phase 5 Core completed: 3D Graphics and Performance
- Added 84 procedural ship models (12 ships × 7 factions)
- Implemented performance optimization system (60+ FPS, LOD, culling)
- Added advanced particle system (5 effect types)
- Completed 3D client foundation with Panda3D
- 84+ new test functions for Phase 5 features
- Updated roadmap to reflect Phase 5 core completion
- Remaining Phase 5 polish items moved to future work

### Version 1.1 (February 2026)
- Phase 4 completed: Corporation & Social features
- Added Corporation System with full management
- Added Social System with contacts, mail, and chat
- Added Contract System for player-to-player trading
- 39 new test functions (91+ total)
- Updated milestones and success metrics

### Version 1.0 (February 2026)
- Initial roadmap document
- Complete Phase 1-3 status
- Planned Phase 4-7 features
- 3D graphics options analysis
- Development timeline

---

**Last Updated**: February 3, 2026  
**Next Review**: April 2026

*This roadmap is a living document and will be updated as the project evolves.*
