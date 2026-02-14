# Atlas — Development Guidance (February 2026)

## Current Status

**Project Health**: ✅ Excellent
- **102 ships** across all classes (Frigates to Titans)
- **159+ modules** (Tech I, Tech II, Faction, Officer, Capital)
- **137 skills** with complete skill tree
- **29 C++ server systems** fully implemented
- **1139 test assertions** all passing
- **Zero security vulnerabilities** (CodeQL verified)
- **CI/CD pipelines** for both client and server

**Latest Milestone**: Phases 1-7 Complete (Q4 2025 - Q4 2026)

## Next Major Milestone: Vertical Slice - One Full Star System

**Timeline**: 3-6 months  
**Priority**: 🔥 **CRITICAL**  
**Goal**: Prove all gameplay loops work together in one complete star system

This is the most important next step for the project. All foundational systems are in place, but they need to be integrated into a cohesive, playable experience.

### Vertical Slice Scope

**System Contents**:
- 1 Trade Hub Station
- 2 Mining Belts
- 1 Pirate Zone
- 3-5 Procedural Anomalies
- AI Traffic (miners, patrols, haulers)
- Player Spawn Point

### Vertical Slice Phase 1 (Weeks 1-3): Foundational Gameplay Loop

**Status**: 🚧 IN PROGRESS

#### Task 1.1: Procedural Ship Hull + Weapons Generation ✅ COMPLETE

**Priority**: HIGHEST  
**Complexity**: Medium  
**Estimated Time**: 1-2 weeks  
**Completed**: February 12, 2026

**Objective**: Complete the integration of the modular ship generation system so that ships are rendered with visible hulls, weapons, and engines.

**Implementation Summary**:

**Completed Steps**:
1. ✅ Created `addPartToMesh()` helper function in Model class
   - Properly transforms vertices and normals using inverse transpose matrix
   - Handles index buffer offsets for combining multiple parts
   
2. ✅ Refactored `Model::createShipModelWithRacialDesign()` to use modular parts
   - Initializes ShipPartLibrary and ShipGenerationRules as static singletons
   - Retrieves forward, main, and rear hull parts from library
   - Assembles parts with proper positioning:
     - Forward hull at +0.4x scale
     - Main hull at origin
     - Rear hull at -0.4x scale
   
3. ✅ Added engine placement based on class rules
   - Engines positioned at -0.6x scale (rear of ship)
   - Distributed vertically based on engine count
   - Uses ShipPartType::ENGINE_MAIN from library
   
4. ✅ Added weapon hardpoint generation
   - Turrets along dorsal spine (0 to 0.6x range)
   - Missile launchers on ship sides
   - Counts based on class rules (min/max hardpoints)
   
5. ✅ Added faction-specific details
   - Solari: vertical spires above hull (ornate style)
   - Keldari: exposed framework on sides (industrial style)
   
6. ✅ Fallback to procedural generation if parts unavailable
   - Maintains compatibility with existing ship generation

**Files Modified**:
- `cpp_client/include/rendering/model.h` - Added `addPartToMesh()` declaration
- `cpp_client/src/rendering/model.cpp` - Implemented modular part assembly
- `docs/SHIP_GENERATION_NEXT_STEPS.md` - Updated status to reflect completion

**Next Steps for Validation**:
- Visual testing requires OpenGL/GLFW dependencies
- Performance testing (<100ms per ship)
- Verify faction distinctiveness across all 102 ships

---

#### Task 1.2: Shield/Armor/Hull Damage with Visual Feedback ✅ CLIENT INTEGRATION COMPLETE

**Priority**: High  
**Complexity**: Medium  
**Estimated Time**: 1 week — **COMPLETED**

**Objective**: Implement visible damage feedback when ships take damage

**Completed (February 13, 2026)**:
1. ✅ Added `DamageEvent` component tracking hit records (layer, type, flags)
2. ✅ CombatSystem emits DamageEvent on every damage application
3. ✅ Shield depleted / armor depleted / hull critical flags for visual escalation
4. ✅ Hit record includes damage amount, type, layer, and timestamp
5. ✅ `clearOldHits()` method for garbage collection of stale events
6. ✅ 5 new test functions verifying all damage event scenarios
7. ✅ Client-side `DamageEffectHelper` class:
   - Shield hits: blue ripple + SHIELD_HIT particles
   - Armor hits: orange sparks (DEBRIS emitter)
   - Hull hits: red debris + small explosion
   - Shield depleted: burst of 20 shield particles
   - Armor depleted: explosion (fire/smoke effect)
   - Hull critical: screen shake + 3-second alarm overlay
   - Proportional screen shake for high-damage hits (>100 damage)
8. ✅ `DAMAGE_EVENT` protocol message for server→client communication
9. ✅ `DamageEffectHelper::layerColor()` for damage overlay coloring

**Dependencies**: Task 1.1 ✅ COMPLETE

---

#### Task 1.3: Basic AI Combat (Engage, Orbit, Retreat) ✅ EXPANDED

**Priority**: High  
**Complexity**: Medium-High  
**Estimated Time**: 1-2 weeks — **CORE BEHAVIORS COMPLETE**

**Objective**: NPCs can engage players, orbit targets, and retreat when damaged

**Completed**:
- ✅ C++ server has WeaponSystem implemented
- ✅ C++ server has target locking protocol
- ✅ NPC database with 32 NPC templates
- ✅ AI health-based retreat logic (flee when total HP below configurable threshold)
- ✅ Configurable `flee_threshold` per-NPC (default 25%)
- ✅ Dynamic orbit distances by ship class:
  - Frigate/Destroyer: 5,000m (close brawlers)
  - Cruiser/Battlecruiser: 15,000m (medium range)
  - Battleship: 30,000m (long range)
  - Capital/Carrier/Dreadnought/Titan: 50,000m
  - `use_dynamic_orbit` flag on AI component
- ✅ Engagement range logic from weapon optimal + falloff
- ✅ Target selection strategies:
  - `Closest`: nearest player (default)
  - `LowestHP`: player with lowest HP fraction
  - `HighestThreat`: player dealing most damage to NPC
- ✅ 13 test functions verifying all AI behaviors (3 retreat + 10 new)

**Remaining**:
- AI vs AI combat testing
- AI vs player combat testing
- Coordinated fleet AI (focus fire, squad tactics)

---

#### Task 1.4: Station Docking and Repair Service ✅ COMPLETE

**Priority**: High  
**Complexity**: Medium  
**Estimated Time**: 1 week  
**Completed**: February 13, 2026

**Objective**: Players can dock at stations and repair their ships

**Implementation Summary**:

**Completed Steps**:

**Server-Side (February 13, 2026)**:
1. ✅ Added StationSystem to Server initialization
2. ✅ Created station_system_ pointer in Server class
3. ✅ Implemented DOCK_REQUEST handler in GameSession
   - Range validation (within docking_range)
   - Calls StationSystem::dockAtStation()
   - Sends DOCK_SUCCESS or DOCK_FAILED response
4. ✅ Implemented UNDOCK_REQUEST handler in GameSession
   - Calls StationSystem::undockFromStation()
   - Sends UNDOCK_SUCCESS response
5. ✅ Implemented REPAIR_REQUEST handler in GameSession
   - Calculates repair cost based on damage
   - Calls StationSystem::repairShip()
   - Sends REPAIR_RESULT with cost and new HP values
6. ✅ All 1021 server test assertions passing

**Client-Side UI (February 13, 2026)**:
1. ✅ Created station.rml with EVE-style UI layout
   - Station info panel (name, distance, docking range, repair rate)
   - Docking status display
   - Shield/Armor/Hull HP bars with visual indicators
   - Repair cost calculator
   - Action buttons (Dock, Undock, Repair)
2. ✅ Implemented UpdateStationServices() in RmlUiManager
   - Updates all station UI elements dynamically
   - Calculates repair costs from damage
   - Manages button enabled/disabled states
3. ✅ Implemented InstallStationEvents() for button handlers
   - Dock button triggers onDockRequest_ callback
   - Undock button triggers onUndockRequest_ callback
   - Repair button triggers onRepairRequest_ callback

**Client-Side Network (February 13, 2026)**:
1. ✅ Added station protocol messages to ProtocolHandler:
   - createDockRequestMessage(stationId)
   - createUndockRequestMessage()
   - createRepairRequestMessage()
   - isStationResponse(type) helper
2. ✅ Added station operations to NetworkManager:
   - sendDockRequest(stationId)
   - sendUndockRequest()
   - sendRepairRequest()
   - handleStationResponse(type, dataJson)
   - StationResponse struct with all relevant fields
3. ✅ Wired station callbacks through NetworkManager
   - setStationCallback() for response handling

**Remaining Integration**:
- Final wiring in Application/SolarSystemScene to connect UI ↔ NetworkManager
- Visual testing and edge case validation
- Documentation updates

---

### Vertical Slice Phase 2 (Weeks 4-6): Wrecks, Salvage & Economy

**Status**: 🚧 IN PROGRESS

**Completed (February 13, 2026)**:
- ✅ Ship destruction → wreck spawning: CombatSystem death callback auto-creates wreck + loot when hull reaches zero
- ✅ Salvage gameplay mechanics: WreckSalvageSystem + salvage drones transfer items from wrecks to player inventory
- ✅ Mining system: MiningSystem with MiningLaser cycle-based ore extraction from MineralDeposit entities
- ✅ Mining drones: mining_drone type in DroneSystem extracts ore from deposits into owner inventory
- ✅ Salvage drones: salvage_drone type in DroneSystem recovers items from wrecks with configurable success chance
- ✅ Resource tracking per system: SystemResources component tracks mineral types and remaining quantities
- ✅ Protocol messages: SALVAGE_REQUEST/RESULT, LOOT_ALL/RESULT, MINING_START/STOP/RESULT
- ✅ AI Mining state added to AI component
- ✅ 80 new test assertions (1115/1115 total passing)

**Remaining**:
- Basic mineral economy (market pricing for ores/minerals)
- Mining AI NPC behavior implementation (using Mining state)
- Mineral refining system (ore → refined minerals)
- Full economy loop testing

**Dependencies**: Phase 1 ✅ complete

---

### Vertical Slice Phase 3 (Weeks 7-9): Exploration & Anomalies

**Status**: 🚧 IN PROGRESS

**Completed (February 14, 2026)**:
- ✅ Scanner component with scan_strength, scan_deviation, probe tracking
- ✅ AnomalySignature component with signal_strength, difficulty, loot/NPC data
- ✅ SolarSystemSignatures component for per-system anomaly tracking
- ✅ ScannerSystem: start/stop scans, signal resolution via scan cycles, difficulty-based gain
- ✅ AnomalySpawnerSystem: seed-based deterministic anomaly generation, difficulty scaling by security level
- ✅ Anomaly count scales with security: high-sec 30% → null-sec 100% of max
- ✅ Difficulty scales inversely with security level
- ✅ 13 test functions, 41 new assertions (1226 total passing)

**Remaining**:
- Scanner UI implementation (client-side)
- Load exploration site data from JSON into spawned anomalies
- Combat & mining anomaly NPC spawning on warp-in
- Deadspace complex room progression

**Dependencies**: Phase 2 ✅ complete

---

### Vertical Slice Phase 4 (Weeks 10-12): Procedural Missions & Reputation

**Status**: NOT STARTED

**Tasks**:
- Mission templates implementation (use existing mission data)
- Mission generation from world state
- Faction reputation system (partially implemented in StandingsSystem)
- Hostile/friendly AI behavior based on reputation

**Dependencies**: Phase 3 complete

---

### Vertical Slice Phase 5 (Weeks 13-16): Persistence & Stress Testing

**Status**: NOT STARTED

**Tasks**:
- Save/load system state (use existing WorldPersistence)
- Fleet state persistence
- Economy persistence
- LOD & impostors for large battles
- 100+ ship fleet stress test

**Dependencies**: Phase 4 complete

---

## Alternative Priorities (If Vertical Slice is Not the Goal)

If the vertical slice is not the immediate priority, here are other valuable tasks:

### Option A: Content Expansion (Low Effort, High Impact)

**Add more game content using existing systems**:
- Add more Level 5 missions (8 planned, 0 exist)
- Add more exploration sites (18 exist, could add 10 more)
- Add more NPC factions (32 NPCs exist, could add pirate variants)
- Add more skills (137 exist, could add specialized skills)

**Effort**: 1-2 weeks  
**Value**: Medium (enriches gameplay)

---

### Option B: Performance Optimization (Medium Effort, High Impact)

**Optimize server and client performance**:
1. Profile C++ server tick performance
2. Add spatial partitioning for entity queries
3. Implement interest management (only send nearby entities to clients)
4. Add client-side prediction for movement
5. Optimize rendering (instanced rendering for ships)

**Effort**: 2-4 weeks  
**Value**: High (enables larger battles, more players)

---

### Option C: Database Persistence (High Effort, High Impact)

**Add PostgreSQL support for persistent universe**:
1. Design database schema for entities, players, corporations
2. Implement PostgreSQL adapter
3. Add save/load for all game state
4. Add backup and recovery tools
5. Add migration tools

**Effort**: 3-6 weeks  
**Value**: Very High (enables persistent universe)

---

## Recommended Next Action

🎯 **Start with Task 1.1: Procedural Ship Hull + Weapons Generation**

**Rationale**:
1. It's the first task in the critical Vertical Slice milestone
2. It's well-documented with clear implementation steps
3. It unlocks the rest of Phase 1 (damage feedback, AI combat, docking)
4. It's achievable in 1-2 weeks
5. It will make ships look much better visually

**Expected Outcome**:
- Ships will have visible weapon turrets
- Ships will have visible engines
- Each faction will have distinct visual characteristics
- Foundation for Phase 1 complete

---

## Development Process

**When working on any task**:

1. ✅ **Create a branch**: Use descriptive names like `feature/procedural-ship-generation`
2. ✅ **Write tests first**: Add tests to validate your changes
3. ✅ **Make minimal changes**: Change only what's needed
4. ✅ **Run existing tests**: Ensure no regressions (`make test-server`)
5. ✅ **Run linters**: Ensure code quality
6. ✅ **Run CodeQL**: Ensure no security vulnerabilities
7. ✅ **Get code review**: Use the code_review tool
8. ✅ **Document changes**: Update relevant docs in `docs/`
9. ✅ **Commit frequently**: Use `report_progress` to track work
10. ✅ **Create PR**: With clear description and checklist

---

## Getting Help

**Documentation**:
- `docs/ROADMAP.md` - Full project roadmap and milestones
- `docs/NEXT_TASKS.md` - Detailed task recommendations
- `docs/SHIP_GENERATION_NEXT_STEPS.md` - Ship generation integration guide
- `docs/cpp_client/` - Client architecture and systems
- `docs/guides/` - Build and setup guides

**Testing**:
- Server tests: `cd cpp_server/build && ctest` (832 assertions)
- Client tests: Manual testing required (OpenGL dependency)

**CI/CD**:
- GitHub Actions run automatically on PR
- Server tests run on every push
- CodeQL security scanning on every PR

---

*Last Updated: February 11, 2026*  
*Next Review: After Task 1.1 completion*
