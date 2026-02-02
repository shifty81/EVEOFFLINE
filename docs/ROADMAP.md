# EVE OFFLINE - Project Roadmap

**Last Updated**: February 2026  
**Version**: 1.0

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

### 🎯 Phase 4 - COMPLETE ✅

**Current Version**: Phase 4 Complete  
**Overall Progress**: ~80% of core features implemented  
**Status**: Production-ready for small group PVE gameplay with full corporation and social systems

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

---

## In Progress

### 🚧 Phase 5: 3D Graphics & Polish (Current)
**Status**: Active Development  
**Timeline**: Q2-Q3 2026

Significant progress made on 3D client visual features:

#### ✅ Completed
- [x] Panda3D integration and setup
- [x] Network client with TCP/JSON protocol
- [x] Entity management with interpolation
- [x] EVE-style camera system (orbit, zoom, pan)
- [x] Basic entity renderer with placeholder shapes
- [x] Star field background (1500+ stars)
- [x] Visual effects system (weapon beams, projectiles)
- [x] Explosion effects on ship destruction
- [x] Shield hit effects (blue ripple)
- [x] HUD system with multiple panels:
  - Ship status (shields, armor, hull)
  - Target information (distance, health)
  - Speed and position display
  - Combat log (scrolling messages)
- [x] 3D health bars above ships (billboard effect)
- [x] Enhanced lighting (multi-light setup)
- [x] Automatic shader generation for better materials

#### 🚧 In Progress
- [ ] Ship 3D models (currently using placeholders)
- [ ] Advanced particle systems
- [ ] Performance optimization (LOD, culling)

#### 📋 Remaining
- [ ] PBR materials and textures
- [ ] Audio system integration
- [ ] Additional UI panels

---

## Planned Features

### 📋 Phase 5: 3D Graphics (Planned)
**Timeline**: Q2-Q3 2026  
**Priority**: High

See [LANGUAGE_AND_3D_OPTIONS.md](features/LANGUAGE_AND_3D_OPTIONS.md) for detailed analysis.

#### Recommended Approach: Hybrid Architecture
- [ ] **Keep Python for game server** (current implementation)
  - Server-authoritative gameplay
  - Easy modding with JSON
  - Flexible content updates
- [ ] **Build C++20 3D Client**
  - Modern C++20 features
  - High-performance 3D rendering
  - EVE Online-quality graphics
  - Separate from game logic

#### 3D Client Options (Choose One)
**Option A: Unreal Engine 5** ⭐ Recommended
- [ ] UE5 project setup
- [ ] Network client implementation
- [ ] Connect to Python server
- [ ] EVE-style visual effects
- [ ] PBR ship rendering
- [ ] UI/HUD system
- Pros: Best graphics quality, professional tools, Blueprint + C++
- Cons: Larger project size, steeper learning curve

**Option B: Custom OpenGL/Vulkan**
- [ ] Rendering engine architecture
- [ ] 3D model loading
- [ ] Shader system (PBR)
- [ ] Camera system
- [ ] UI rendering
- Pros: Full control, lightweight, educational
- Cons: More development time, more expertise required

**Option C: Unity** (Used by Astrox Imperium)
- [ ] Unity project setup
- [ ] Network client implementation
- [ ] Connect to Python server
- [ ] 3D space rendering
- [ ] UI/HUD system
- Pros: Good balance, proven for space games, easier than UE5
- Cons: C# instead of C++, licensing considerations

**Option D: Python 3D (Panda3D)**
- [ ] Panda3D integration
- [ ] 3D scene setup
- [ ] Model loading and rendering
- [ ] Shader system
- [ ] UI overlay
- Pros: Stay in Python, simpler integration
- Cons: Less performance, less professional appearance

#### Visual Style Implementation
- [ ] EVE Online aesthetic
  - Dark space themes
  - Semi-transparent UI
  - Gold/blue accents
  - PBR materials
  - Glowing effects
- [ ] Astrox Imperium aesthetic
  - 3D cockpit view
  - Clean, readable UI
  - Good lighting
  - Simplified but beautiful
- [ ] Asset creation pipeline
  - Ship models
  - Station models
  - Effect particles
  - UI elements

**Estimated Effort**: 8-12 weeks  
**Blockers**: Technology choice decision

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

### 📋 Phase 7: Advanced Systems (Future)
**Timeline**: Q4 2026+  
**Priority**: Low

#### Mining & Resource Gathering
- [ ] Asteroid mining mechanics
- [ ] Ore processing/refining
- [ ] Moon mining (group content)
- [ ] Gas harvesting
- [ ] Ice mining
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
  
- **Q2-Q3 2026**: Phase 5 (Current Planning)
  - 3D graphics implementation
  - Visual polish
  - Enhanced UI/UX
  
- **Q3-Q4 2026**: Phase 6 (Planned)
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
- ✅ **91+ test functions** - All passing across 7 test suites
- ✅ **14 ships** - 3 classes implemented
- ✅ **70 modules** - Full fitting options
- ✅ **47 skills** - Complete skill tree
- ✅ **8 major gameplay systems** - Manufacturing, Market, Exploration, Loot, Fleet, Corporation, Social, Contracts
- ✅ **Zero security vulnerabilities** - CodeQL verified
- ✅ **Multiplayer functional** - Server-client architecture working
- ✅ **Corporation system functional** - Full corp management
- ✅ **Social features working** - Corp chat, mail, contacts, contracts

### Target for Phase 5
- [ ] **3D client functional**
- [ ] **EVE-quality graphics**
- [ ] **30+ FPS performance**
- [ ] **Enhanced UI/UX**

### Target for Phase 6
- [ ] **30+ ships** (add Tech II and Battlecruisers/Battleships)
- [ ] **100+ modules** (add Tech II variants)
- [ ] **More mission content**

---

## Contributing

Want to contribute? Check out our priorities:

**High Priority (Help Wanted):**
- 3D graphics implementation (if you have experience with UE5/Unity/OpenGL)
- Additional ship designs and stats
- More mission content
- Testing and bug reports

**Medium Priority:**
- Corporation system implementation
- UI/UX improvements
- Performance optimization
- Documentation improvements

**Low Priority:**
- Advanced features (Phase 7)
- Additional content
- Community tools

See [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.

---

## Questions & Feedback

Have questions about the roadmap? Want to suggest features?

- Open an issue on GitHub
- Join our community discussions
- Check out the documentation

---

## Changelog

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

**Last Updated**: February 2, 2026  
**Next Review**: April 2026

*This roadmap is a living document and will be updated as the project evolves.*
