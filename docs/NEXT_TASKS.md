# EVE OFFLINE - Next Tasks Recommendations

> **Update (February 10, 2026)**: Second HAC variants added (Muninn, Eagle, Deimos, Sacrilege). Tech II EWAR and Logistics modules verified complete. The game now has 102 ships, 159+ modules, 137 skills, and comprehensive content across all tiers.

## Current Status (February 2026)

### Completed Phases
- ✅ **Phase 1-2**: Core Engine & Extended Content (Q4 2025)
- ✅ **Phase 3**: Manufacturing, Market, Exploration, Loot, Fleet (Q1 2026)
- ✅ **Phase 4**: Corporation & Social Systems (Q1 2026)
- ✅ **Phase 5**: 3D Graphics Core & Polish (Q1-Q2 2026)
- ✅ **Phase 6**: Advanced Content & Tech II Ships (Q2 2026)
- ✅ **Phase 7**: Mining, PI, Research, C++ Server Integration, Wormholes, Fleet (Q4 2026)
- ✅ **Quick Wins**: Tutorial, Modding Guide, Code Cleanup (February 2026)
- ✅ **Medium-Term**: External Model Loading, Standings System, Tech II Content, Epic Arcs, Faction/Officer Modules (February 2026)

### Project Highlights
- **102 ships** across all classes (Frigates to Titans, including Command Ships, Interceptors, Covert Ops, Industrials, Marauders, Interdictors, Stealth Bombers)
- **714 procedural ship models** (102 ships × 7 factions)
- **Custom model support**: .obj, .gltf, .glb formats
- **159+ modules** for ship fitting (Tech I, Tech II, Faction, Officer, Capital, Mining Crystals)
- **137 skills** with complete skill tree (including 14 Leadership skills)
- **4 epic arc mission chains** with 12 storyline missions
- **8 rare NPC spawns** (4 faction commanders, 4 officer NPCs)
- **10 mining missions** across levels 1-4
- **18 exploration site templates** (combat, relic, data, gas, wormhole)
- **32 NPC types** across 8 factions (including Mordu's Legion, Sisters of EVE)
- **9 major gameplay systems** fully implemented (including Incursions)
- **95+ test functions** all passing (415 assertions)
- **Zero security vulnerabilities** (CodeQL verified)
- **C++ OpenGL client** with full 3D rendering
- **C++ dedicated server** with ECS architecture
- **Multiplayer functional** with server-client integration

---

## Recommended Next Tasks

### Priority 1: Code Quality & Maintenance

#### 1.1 Address Optional TODOs
Most TODOs in the codebase are for optional/future features:

**Steam Integration** (`cpp_server/src/auth/steam_auth.cpp`)
- Status: Optional feature, requires Steam SDK
- Priority: Low (nice-to-have)
- Note: System already has stubs in place

**Model Loading** (`cpp_client/src/rendering/model.cpp`)
- Status: Procedural generation works well
- Priority: Low (only needed for custom models)
- Note: Could add support for .obj, .gltf, .fbx formats

**Visual Enhancements**
- Tech II ship visual details
- Proper beam and particle shaders  
- Health bar borders
- Priority: Low (current visuals are functional)

**Standings System** (`cpp_client/src/ui/overview_panel.cpp`)
- Status: ✅ **COMPLETED** (February 2026)
- Implementation:
  - ✅ Standings component with personal, corp, and faction standings
  - ✅ getStandingWith() method with priority hierarchy
  - ✅ modifyStanding() with automatic clamping
  - ✅ Full serialization/deserialization support
  - ✅ Integration with player and NPC spawning
  - ✅ Client-side faction-based standing calculation
  - ✅ Comprehensive test coverage
  - ✅ Full documentation in docs/STANDINGS_SYSTEM.md
- Note: System is functional and ready for gameplay integration

#### 1.2 Documentation Updates
- ✅ All phases well-documented
- ✅ ROADMAP.md is comprehensive and up-to-date
- ✅ Build guides for all platforms
- Potential additions:
  - Tutorial for new contributors
  - Modding guide for content creators
  - Performance tuning guide

### Priority 2: Phase 6 Optional Enhancements

From ROADMAP.md "In Progress" section:

#### 2.1 Additional Tech II Variants
- ✅ Second HAC per race (8 total: Vagabond, Cerberus, Ishtar, Zealot, Muninn, Eagle, Deimos, Sacrilege)
- ✅ Tech II Battlecruisers (Command Ships) — 4 ships
- ✅ Tech II Interceptors — 8 ships (4 combat, 4 fleet)
- ✅ Covert Ops Frigates — 4 ships
- ✅ Industrial Ships — 4 haulers
- Estimated effort: 2-3 weeks

#### 2.2 Additional Mission Content
- ✅ Level 5 missions (high-end PVE) — 8 missions
- ✅ Epic mission arcs (story-driven content) — 4 arcs, 12 missions
- ✅ Incursions (group PVE events) — 8 encounters across 4 tiers
- Estimated effort: 2-3 weeks

#### 2.3 More Modules
- ✅ Tech II EWAR modules (Multispectral ECM II, Tracking Disruptor II, Remote Sensor Dampener II, Target Painter II, Stasis Webifier II, Warp Disruptor II, Warp Scrambler II, Sensor Booster II)
- ✅ Tech II logistics modules (Large/Medium Remote Armor Repairer II, Large/Medium Remote Shield Booster II)
- ✅ Officer modules (rare drops) — 4 officer modules
- ✅ Faction modules — 8 faction modules
- ✅ Capital-sized modules: 15 modules (shield, armor, logistics, weapons, siege, triage, doomsday)
- ✅ Cloaking devices: 3 types (Prototype, Improved, Covert Ops)
- ✅ Jump drives: 2 types (Economy, Standard)
- Estimated effort: 1-2 weeks

### Priority 3: Advanced Features (Phase 8+)

#### 3.1 Performance & Scalability
- Database persistence (SQLite → PostgreSQL)
- Performance profiling and optimization
- Interest management for large player counts
- Client-side prediction
- Multi-threaded server processing

#### 3.2 DevOps & Deployment
- ✅ CI/CD pipeline — GitHub Actions for server (build + 362 tests) and client
- ✅ Automated testing on PR — Server tests run on push and pull requests
- ✅ Docker containerization — Multi-stage Dockerfile for dedicated server
- ✅ Crash reporting and logging — Structured Logger with levels (DEBUG–FATAL), file output, and exception handling in main()
- ✅ Server monitoring and analytics — ServerMetrics tracking tick timing, entity/player counts, uptime, periodic stats logging
- Cloud deployment guides

#### 3.3 Additional Game Systems
- PvP toggle option (optional for those who want it)
- Tournament system
- Leaderboards and achievements
- In-game web browser (dotlan-style maps)

#### 3.4 Community & Modding
- Mod manager utility
- Content creation tools
- Mission editor
- Ship designer
- Modding documentation

---

## Immediate Action Items

### Quick Wins (Can be done immediately)

1. ✅ **Add Tutorial Documentation** (COMPLETE)
   - ✅ Created comprehensive "Getting Started" tutorial (docs/TUTORIAL.md)
   - ✅ Step-by-step gameplay guide with all major systems covered
   - ✅ Common tasks reference and FAQ section

2. ✅ **Add Modding Guide** (COMPLETE)
   - ✅ Created comprehensive modding guide (docs/MODDING_GUIDE.md)
   - ✅ How to create custom ships in JSON with examples
   - ✅ How to add new modules with templates
   - ✅ How to create missions with detailed instructions
   - ✅ How to adjust game balance with guidelines

3. **Improve README.md** (OPTIONAL)
   - Add screenshots/GIFs when available
   - Add "Quick Start" video link (when exists)
   - Add community links (Discord, forums, etc.) when established
   - README already comprehensive and well-structured

4. ✅ **Code Cleanup** (COMPLETE)
   - ✅ Code is clean with minimal TODOs (only for optional features)
   - ✅ Consistent code style throughout
   - ✅ Proper documentation and comments where needed

### Medium-Term Tasks (1-2 weeks each)

1. ✅ **Implement External Model Loading** (COMPLETED - February 2026)
   - ✅ Added .obj file support via tinyobjloader
   - ✅ Added .gltf/.glb support via tinygltf
   - ✅ Updated asset pipeline and CMake configuration
   - ✅ Added model validation and error handling
   - ✅ Comprehensive documentation in MODDING_GUIDE.md and MODEL_LOADING.md
   - ✅ Supports vertex positions, normals, UVs, and material colors
   - ✅ Auto-detection of format by file extension
   - Note: Fully implemented and documented, ready for game integration

2. ✅ **Expand Standings System** (COMPLETED - February 2026)
   - ✅ Created Standings component with full hierarchy
   - ✅ Added standings calculation with priority system
   - ✅ Added UI integration with faction-based calculation
   - ✅ Integrated with player/NPC spawning
   - ✅ Full test coverage and documentation

3. ✅ **Add More Tech II Content** (COMPLETED - February 2026)
   - ✅ 4 Command Ships (Claymore, Vulture, Astarte, Absolution)
   - ✅ 8 Tech II EWAR modules
   - ✅ 4 Tech II Logistics modules
   - ✅ Command Ship class recognition in 3D client

4. ✅ **Add Epic Arc Missions** (COMPLETED - February 2026)
   - ✅ 4 epic arcs (Amarr, Caldari, Gallente, Minmatar)
   - ✅ 12 storyline missions with chapter progression
   - ✅ Escalating difficulty and rewards across chapters
   - ✅ Faction module rewards for completing arcs

5. ✅ **Add Faction & Officer Modules** (COMPLETED - February 2026)
   - ✅ 8 faction modules (weapons and defensive, one per faction)
   - ✅ 4 officer modules (rare drops from pirate commanders)
   - ✅ Proper meta_level hierarchy (faction: 8, officer: 12)

6. ✅ **Add Capital Modules, Cloaking & Jump Drives** (COMPLETED - February 2026)
   - ✅ 15 capital-sized modules (shield, armor, logistics, weapons, siege, triage)
   - ✅ 3 cloaking devices (Prototype, Improved, Covert Ops)
   - ✅ 2 jump drives (Economy, Standard)
   - ✅ Doomsday device for Titans

7. ✅ **Add Advanced Skills** (COMPLETED - February 2026)
   - ✅ 13 capital ship skills (carriers, dreadnoughts, titans for all 4 races)
   - ✅ 3 jump skills (operation, calibration, fuel conservation)
   - ✅ 1 cloaking skill
   - ✅ 6 advanced weapon skills (specializations and capital turrets)
   - ✅ 2 advanced industrial skills

8. ✅ **Add Rare NPC Spawns** (COMPLETED - February 2026)
   - ✅ 4 faction fleet commanders (Serpentis, Guristas, Blood Raiders, Angel Cartel)
   - ✅ 4 officer NPCs (Tobias, Draclira, Cormack, Hakim)
   - ✅ Faction-specific loot drops

9. ✅ **Add Mining Missions** (COMPLETED - February 2026)
   - ✅ 10 mining missions across levels 1-4
   - ✅ Progressive difficulty from highsec Veldspar to nullsec Mercoxit
   - ✅ NPC defense spawns in higher level missions

10. ✅ **Add Leadership Skills** (COMPLETED - February 2026)
    - ✅ Wing Command skill (advanced fleet command)
    - ✅ Fleet Command skill (elite fleet command)
    - ✅ Warfare Link Specialist skill (command burst modules)
    - ✅ 4 warfare specializations (Armored, Information, Siege, Skirmish)
    - ✅ Mining Director skill (advanced mining fleet command)

11. ✅ **Add More Exploration Sites** (COMPLETED - February 2026)
    - ✅ 12 new exploration site templates added
    - ✅ Combat sites across all difficulty levels (Blood Raider, Angel, Sansha, Serpentis, Guristas)
    - ✅ Relic sites (Sleeper Enclave, Forgotten Perimeter)
    - ✅ Data sites (Unsecured Frontier, Central Guristas Survey, Serpentis Lookout)
    - ✅ Wormhole anomaly entries (C1-C3 and C4-C6 destinations)
    - ✅ 5 new loot item types (sleeper components, decryptors, faction modules, etc.)

12. ✅ **Add New NPC Factions** (COMPLETED - February 2026)
    - ✅ Mordu's Legion: 4 NPCs (frigate, cruiser, battlecruiser, battleship) with kiting behavior and warp disruptors
    - ✅ Sisters of EVE: 3 NPCs (frigate, cruiser, battleship) with remote repair and scanning capabilities

13. ✅ **Add Incursions System** (COMPLETED - February 2026)
    - ✅ 8 incursion encounters across 4 tiers (vanguard, assault, headquarters, mothership)
    - ✅ 6 incursion-specific NPC types (overseer, logistics cruiser, commander, carrier, Kundalini, mothership)
    - ✅ Influence mechanic with system-wide effects (cyno jamming, security penalty, bounty reduction)
    - ✅ Scaled fleet size requirements (10-pilot vanguard to 80-pilot mothership)
    - ✅ Multi-wave encounters with progressive difficulty
    - ✅ ISK, loyalty points, CONCORD LP, and standing rewards

14. ✅ **Add Mining Crystals** (COMPLETED - February 2026)
    - ✅ 30 mining crystal types (Tech I and Tech II for all 15 ore types)
    - ✅ Tech I crystals: +62.5% mining yield, low volatility
    - ✅ Tech II crystals: +75% mining yield, higher volatility, skill requirements
    - ✅ Compatible with Modulated Strip Miners and Deep Core Strip Miners
    - ✅ Mercoxit crystals restricted to Deep Core Strip Miners only

15. ✅ **Add Interceptor Frigates** (COMPLETED - February 2026)
    - ✅ 4 Combat Interceptors (Claw, Crow, Taranis, Crusader)
    - ✅ 4 Fleet Interceptors (Stiletto, Raptor, Ares, Malediction)
    - ✅ High speed (400-470 m/s) and agility for fast tackle
    - ✅ Fleet Interceptors with interdiction nullifier (bubble immunity)
    - ✅ MWD signature bloom reduction bonuses
    - ✅ Race-specific weapon and tank bonuses

16. ✅ **Add Covert Ops Frigates** (COMPLETED - February 2026)
    - ✅ 4 Covert Ops frigates (Cheetah, Buzzard, Helios, Anathema)
    - ✅ Scan probe strength and deviation reduction bonuses
    - ✅ Covert Ops cloaking device compatibility
    - ✅ Hacking and virus coherence bonuses
    - ✅ High CPU for fitting scanning modules

17. ✅ **Add Industrial Ships** (COMPLETED - February 2026)
    - ✅ 4 Industrial haulers (Hoarder, Badger, Iteron Mark V, Bestower)
    - ✅ Large cargo capacity (3800-5800 m3 base)
    - ✅ Cargo capacity bonuses per skill level
    - ✅ Race-specific tank bonuses (shield/armor)

18. ✅ **Add Marauder Battleships** (COMPLETED - February 2026)
    - ✅ 4 Marauders (Vargur, Golem, Kronos, Paladin)
    - ✅ Bastion Module bonuses for sustained PVE combat
    - ✅ Tractor beam range/velocity bonuses
    - ✅ Micro Jump Drive cooldown reduction
    - ✅ Race-specific weapon bonuses (projectile, missile, hybrid, energy)

19. ✅ **Add Medium Tech II Weapons** (COMPLETED - February 2026)
    - ✅ 425mm AutoCannon II (medium projectile)
    - ✅ Heavy Neutron Blaster II (medium hybrid blaster)
    - ✅ 250mm Railgun II (medium hybrid railgun)
    - ✅ Focused Medium Pulse Laser II (medium energy)
    - ✅ Focused Medium Beam Laser II (medium energy)
    - ✅ Heavy Missile Launcher II (medium missile)
    - ✅ Rapid Light Missile Launcher II (medium missile)
    - ✅ Improved stats over Tech I variants (damage, range, tracking)

20. ✅ **Fix ShipDatabase Loader** (COMPLETED - February 2026)
    - ✅ Added industrials.json to ShipDatabase file list
    - ✅ Added tech2_battleships.json to ShipDatabase file list
    - ✅ Total loaded ships increased from 82 to 90

21. ✅ **Add Interdictor Destroyers** (COMPLETED - February 2026)
    - ✅ 4 Interdictors (Sabre, Flycatcher, Eris, Heretic)
    - ✅ Warp disruption probe (bubble) launcher capability
    - ✅ Race-specific weapon bonuses (projectile, missile, hybrid, energy)
    - ✅ Destroyer-class hull with Tech II stats
    - ✅ Added tech2_destroyers.json to ShipDatabase file list

22. ✅ **Add Stealth Bomber Frigates** (COMPLETED - February 2026)
    - ✅ 4 Stealth Bombers (Hound, Manticore, Nemesis, Purifier)
    - ✅ Torpedo damage bonuses with race-specific damage types
    - ✅ Bomb launcher capability
    - ✅ Covert Ops cloaking device compatibility
    - ✅ Long targeting range for standoff engagement

23. ✅ **Add Large Tech II Weapons** (COMPLETED - February 2026)
    - ✅ 800mm Repeating Cannon II (large projectile autocannon)
    - ✅ 1400mm Howitzer Artillery II (large projectile artillery)
    - ✅ Mega Pulse Laser II (large energy pulse)
    - ✅ Tachyon Beam Laser II (large energy beam)
    - ✅ 425mm Railgun II (large hybrid railgun)
    - ✅ Neutron Blaster Cannon II (large hybrid blaster)
    - ✅ Cruise Missile Launcher II (large missile)
    - ✅ Torpedo Launcher II (large missile)

### Long-Term Goals (1-3 months each)

1. **Performance Optimization**
   - Profile server performance
   - Optimize entity queries
   - Add spatial partitioning
   - Implement interest management

2. **Persistent Universe**
   - Add PostgreSQL support
   - Implement world persistence
   - Add player data backup
   - Add server migration tools

3. **Advanced Content**
   - Level 5 missions
   - ✅ Epic arcs
   - ✅ Incursions (group PVE)
   - Tournament system

---

## Decision Framework

When choosing what to work on next, consider:

1. **Impact**: How much does it improve the game?
2. **Effort**: How long will it take?
3. **Risk**: How likely is it to break existing features?
4. **Dependencies**: What needs to be done first?

### High Impact, Low Effort (Do First)
- Documentation improvements
- Simple content additions (ships, modules)
- Bug fixes

### High Impact, High Effort (Plan Carefully)
- New game systems
- Major performance improvements
- Large content expansions

### Low Impact, Low Effort (Nice-to-Have)
- Visual polish
- Minor UI improvements
- Code cleanup

### Low Impact, High Effort (Avoid Unless Necessary)
- Speculative features
- Over-engineering
- Premature optimization

---

## Conclusion

The EVE OFFLINE project is in excellent shape with all major systems implemented and working. The next steps should focus on:

1. **Short term**: Content expansion (more ships, modules, missions)
2. **Medium term**: Performance optimization and polish
3. **Long term**: Advanced features and community tools

The project has a solid foundation and can grow in multiple directions based on user feedback and priorities.

---

*Last Updated: February 10, 2026*
*Status: Quick Wins, Medium-Term Tasks complete. Logging, monitoring, and crash reporting added. 102 ships, 159+ modules, 415 test assertions. Ready for Phase 8 planning.*
