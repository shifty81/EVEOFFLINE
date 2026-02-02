# Astrox Imperium Research & Implementation Strategy

## Research Summary: Astrox Imperium's Approach

### Key Technical Findings

**Engine Choice: Unity 3D**
- Astrox Imperium is built entirely on Unity engine
- Provides robust 3D graphics, physics, and cross-platform support
- Excellent for indie development with strong community and asset ecosystem

**Data-Driven Architecture**
- Heavy emphasis on **text-based, editable data files**
- Game configuration stored in plain text (items_database, missions, skills, etc.)
- All stored in `MOD/` directory for easy access and modification
- Players can edit game data without specialized tools - just text editors
- Separation of content from core engine code

**Modding Philosophy**
- Extensive modding support built from the ground up
- Ship editor tools included with the game
- Community can create custom missions, ships, and modify balance
- This approach extends the game's life and creates community engagement

**Procedural Generation**
- Universe is procedurally generated with seed-based systems
- Over 100 unique star systems/maps
- Parameterized world generation (players can adjust universe size, difficulty)
- Provides replayability without massive manual content creation

**Simplified Complexity**
- Streamlined ship fitting compared to EVE (less intimidating)
- Reduced module complexity while maintaining depth
- Direct skill progression (not as time-gated as EVE)
- Single-player focus removes networking complexity

### Game Features Comparison

| Feature | EVE Online | Astrox Imperium | EVE OFFLINE (Recommended) |
|---------|-----------|-----------------|---------------------------|
| **Player Mode** | MMO | Single-player | Small group multiplayer (2-20) |
| **Economy** | Player-driven | NPC/AI simulated | Hybrid (NPC + player trading) |
| **Ship Fitting** | Highly complex | Streamlined | Balanced complexity |
| **Skills** | Real-time training | Direct progression | Time-based but accelerated |
| **Fleet System** | Corp/Alliance | 10 mercenary ships | Player fleets (5-10 ships) |
| **PvP** | Core feature | None | None (PvE only) |
| **Modding** | Limited | Extensive | Extensive (like Astrox) |
| **Universe** | Single persistent | Procedural | Persistent per server instance |
| **Career Paths** | All EVE professions | Mining, Combat, Trade, Exploration | Same as Astrox + Missions |

## Recommended Implementation Path

Based on Astrox Imperium's success and EVE OFFLINE's requirements for **multiplayer**, here's the optimal approach:

### 1. Engine Selection: **Python + Pygame/Pyglet OR Unity**

**Option A: Python-based Custom Engine** (Current approach)
- ✅ Complete control over networking for multiplayer
- ✅ Easier to implement server-authoritative model
- ✅ Lightweight for small groups
- ✅ Cross-platform (Windows, Linux, Mac)
- ❌ More work for graphics and physics
- ❌ Lower performance ceiling

**Option B: Unity 3D** (Astrox approach, adapted for multiplayer)
- ✅ Professional 3D graphics out of the box
- ✅ Built-in physics engine
- ✅ Large asset ecosystem
- ✅ Unity Netcode for GameObjects or Mirror for networking
- ✅ Proven by Astrox for EVE-like gameplay
- ❌ Steeper learning curve
- ❌ Proprietary engine (less control)

**RECOMMENDATION: Hybrid Approach**
1. Start with **Python custom engine** for Phase 1 (MVP, core systems)
2. Prove the concept with 2D top-down gameplay
3. Optionally migrate to Unity for Phase 2+ if 3D graphics become priority
4. Use text-based data files regardless of engine (Astrox's best practice)

### 2. Data Architecture: **Text-Based Configuration** (Following Astrox)

**Key Principle**: Separate game content from engine code

**Directory Structure**:
```
data/
├── ships/
│   ├── frigates.json
│   ├── cruisers.json
│   └── battleships.json
├── modules/
│   ├── weapons.json
│   ├── shields.json
│   └── engineering.json
├── skills/
│   └── skills.json
├── npcs/
│   ├── serpentis.json
│   ├── guristas.json
│   └── blood_raiders.json
├── missions/
│   ├── level1_missions.json
│   └── level2_missions.json
├── universe/
│   ├── systems.json
│   └── generation_config.json
└── mods/
    └── [user custom content]
```

**Benefits**:
- Easy balance adjustments without code changes
- Community can create custom content
- Version control friendly (text diffs)
- No compilation needed for content updates
- Modders can extend the game

### 3. Core Systems Priority (Based on Both Games)

**Phase 1: Foundation (Weeks 1-2)**
- [x] ECS architecture
- [ ] Text-based data loading system
- [ ] Basic ship movement and controls
- [ ] Simple 2D rendering
- [ ] Server-client networking foundation

**Phase 2: Combat Core (Weeks 3-4)**
- [ ] Weapon systems (turrets, missiles)
- [ ] Damage calculation with resistances
- [ ] Shield/armor/hull system
- [ ] NPC spawning and basic AI
- [ ] Target locking system

**Phase 3: Progression (Weeks 5-6)**
- [ ] Skills system (simplified from EVE, more direct than Astrox)
- [ ] Multiple ship types (3-5 ships per class)
- [ ] Module fitting with CPU/PG constraints
- [ ] Inventory management
- [ ] Basic NPC market (buy/sell)

**Phase 4: Content (Weeks 7-8)**
- [ ] Mission system (combat, mining, courier)
- [ ] 5-10 star systems with stargates
- [ ] Mining and resource gathering
- [ ] Loot and rewards
- [ ] NPC faction system

**Phase 5: Multiplayer Features (Weeks 9-10)**
- [ ] Fleet mechanics (shared encounters)
- [ ] Player-to-player trading
- [ ] Chat system
- [ ] Corporation basics (shared wallet, members)
- [ ] Shared mission instances

**Phase 6: Polish & Modding (Week 11+)**
- [ ] Modding tools and documentation
- [ ] Mission editor
- [ ] Ship editor (like Astrox)
- [ ] Better UI/UX
- [ ] Performance optimization
- [ ] Sound and music

### 4. Simplified Mechanics (Learning from Astrox)

**Ship Fitting**:
- Reduce slot types: High (3-6), Mid (3-6), Low (3-6), Rigs (2-3)
- Clear module categories
- Visual feedback for fitting constraints
- Preset fittings for new players

**Skills**:
- ~50-75 skills instead of EVE's 400+
- Training time: Minutes to hours, not days/weeks
- Immediate benefits (not incremental like EVE)
- Skill queue system

**Economy**:
- Hybrid system: NPC base prices + player trading
- No complex manufacturing chains (at least initially)
- Direct buy/sell at stations
- Optional: Player market orders in Phase 5+

**Universe**:
- 10-20 systems for initial release
- Optional procedural generation for endless expansion
- Instance-based mission deadspaces (like both games)
- Stargates for system travel

### 5. Networking Strategy (EVE OFFLINE Specific)

Since Astrox is single-player, we need a different approach:

**Server Architecture**:
- Authoritative dedicated server (prevents cheating)
- Client-server model (not P2P)
- TCP for reliable state sync
- UDP optional for position updates (optimization)

**State Synchronization**:
- Server simulates full world state
- Clients receive state updates (10-20 Hz)
- Client-side prediction for local player only
- Snapshot interpolation for other entities

**Scalability**:
- Target: 10-50 concurrent players per server instance
- Spatial partitioning (only sync nearby entities)
- Interest management (don't send irrelevant updates)

### 6. Modding Support (Following Astrox's Success)

**Built-in Tools**:
1. **Data Editor**: JSON editor with validation
2. **Ship Designer**: Visual ship configuration
3. **Mission Creator**: Drag-and-drop mission builder
4. **Universe Editor**: System layout and connections

**Modding API**:
- Python scripts for custom behaviors
- Hooks for events (on_ship_destroyed, on_mission_complete, etc.)
- Custom modules and ship bonuses
- New faction creation

**Community**:
- Official mod repository (GitHub or similar)
- Mod manager in game
- Easy install/uninstall
- Conflict detection

## Implementation Recommendations

### Technology Stack (Final)

**Core Engine**: Python 3.11+
**Graphics**: Pygame (2D, Phase 1) → Optional Unity migration (3D, Phase 2+)
**Networking**: 
- asyncio for async I/O
- websockets or TCP sockets
- Optional: Python-integrated Unity Netcode
**Data**: JSON for all game data
**Database**: SQLite for player persistence
**Math**: NumPy for vector operations
**Testing**: pytest for unit tests

### Why This Approach Works

1. **Proven by Astrox**: Text-based data works for complex space games
2. **Multiplayer Ready**: Python gives full networking control
3. **Moddable**: Following Astrox's most praised feature
4. **Incremental**: Can start simple, add complexity gradually
5. **Community-Driven**: Mods extend content without developer effort
6. **Accessible**: Lower complexity than EVE, more social than Astrox

### Key Takeaways from Astrox

1. **Don't overcomplicate**: Streamline EVE mechanics, keep the feel
2. **Empower players**: Make everything moddable
3. **Focus on core loop**: Mining, combat, trading, exploration
4. **Procedural generation**: Extends replayability cheaply
5. **Solo-friendly pacing**: Don't require weeks of training
6. **Atmosphere matters**: EVE's UI style and sound design are iconic

## Next Steps

1. ✅ Complete core ECS implementation (done)
2. Create data loading system for JSON files
3. Implement basic ship movement and rendering
4. Build simple server-client networking
5. Add first playable ship with weapons
6. Create first NPC encounter
7. Test with 2-3 players

This hybrid approach takes the best of both worlds: Astrox's accessibility and modding philosophy, plus multiplayer capability for group PvE fun.
