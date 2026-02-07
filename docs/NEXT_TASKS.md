# EVE OFFLINE - Next Tasks Recommendations

## Current Status (February 2026)

### Completed Phases
- ✅ **Phase 1-2**: Core Engine & Extended Content (Q4 2025)
- ✅ **Phase 3**: Manufacturing, Market, Exploration, Loot, Fleet (Q1 2026)
- ✅ **Phase 4**: Corporation & Social Systems (Q1 2026)
- ✅ **Phase 5**: 3D Graphics Core & Polish (Q1-Q2 2026)
- ✅ **Phase 6**: Advanced Content & Tech II Ships (Q2 2026)
- ✅ **Phase 7**: Mining, PI, Research, C++ Server Integration, Wormholes, Fleet (Q4 2026)

### Project Highlights
- **49 ships** across all classes (Frigates to Titans)
- **343 procedural ship models** (49 ships × 7 factions)
- **70+ modules** for ship fitting
- **47 skills** with complete skill tree
- **8 major gameplay systems** fully implemented
- **95+ test functions** all passing
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
- Status: Basic implementation exists (player=0, NPC=-5)
- Priority: Medium (would enhance gameplay)
- Scope: Would need:
  - Standings database component
  - Standings calculation system
  - UI for managing standings
  - Integration with combat and missions

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
- Second HAC per race (currently 4, could have 8)
- Tech II Battlecruisers (Command Ships, etc.)
- Estimated effort: 2-3 weeks

#### 2.2 Additional Mission Content
- Level 5 missions (high-end PVE)
- Epic mission arcs (story-driven content)
- Estimated effort: 2-3 weeks

#### 2.3 More Modules
- Tech II EWAR modules
- Tech II logistics modules
- Officer modules (rare drops)
- Estimated effort: 1-2 weeks

### Priority 3: Advanced Features (Phase 8+)

#### 3.1 Performance & Scalability
- Database persistence (SQLite → PostgreSQL)
- Performance profiling and optimization
- Interest management for large player counts
- Client-side prediction
- Multi-threaded server processing

#### 3.2 DevOps & Deployment
- CI/CD pipeline expansion
- Docker containerization
- Cloud deployment guides
- Server monitoring and analytics

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

1. **Add Tutorial Documentation** (1-2 hours)
   - Create "Getting Started" tutorial
   - Step-by-step gameplay guide
   - Common tasks reference

2. **Add Modding Guide** (2-3 hours)
   - How to create custom ships in JSON
   - How to add new modules
   - How to create missions
   - How to adjust game balance

3. **Improve README.md** (30 minutes)
   - Add screenshots/GIFs if available
   - Add "Quick Start" video link (if exists)
   - Add community links

4. **Code Cleanup** (1-2 hours)
   - Remove commented-out code
   - Ensure consistent code style
   - Add missing docstrings

### Medium-Term Tasks (1-2 weeks each)

1. **Implement External Model Loading**
   - Add .obj file support
   - Add .gltf/.glb support  
   - Update asset pipeline
   - Add model validation

2. **Expand Standings System**
   - Create standings component
   - Add standings calculation
   - Add UI for managing standings
   - Integrate with factions

3. **Add More Tech II Content**
   - 4 more HACs (one per race)
   - 4 Command Ships
   - 8 Tech II EWAR modules
   - 4 Tech II Logistics modules

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
   - Epic arcs
   - Incursions (group PVE)
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

*Last Updated: February 7, 2026*
*Status: Ready for Phase 8 planning*
