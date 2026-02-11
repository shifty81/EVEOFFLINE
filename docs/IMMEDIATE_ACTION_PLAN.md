# EVE OFFLINE - Immediate Action Plan

**Date**: February 11, 2026  
**Purpose**: Define concrete next steps following baseline system completion

---

## Context

All 27 baseline server systems are complete with 832/832 tests passing. The project is ready to move from "building systems" to "bringing the world to life."

---

## Priority Queue

### 🔥 Critical Path (Blocks other work)

#### 1. Complete Network Interpolation (1-2 weeks)
**Why First**: Poor network smoothness hurts all gameplay  
**Current State**: Server broadcasts snapshots, but client doesn't interpolate  
**Work Required**:
- [ ] Client-side entity position interpolation
- [ ] Velocity-based prediction
- [ ] Delta compression for bandwidth
- [ ] Jitter buffer for packet timing
- [ ] Lag compensation testing

**Success Criteria**:
- Smooth ship movement at 100ms latency
- <5% CPU overhead for interpolation
- No visual "jitter" or "teleporting"

**Technical Notes**:
- Use linear interpolation (LERP) for positions
- Use exponential smoothing for velocities
- Maintain 100ms interpolation buffer
- Server tick: 20 Hz, Client render: 60 Hz

---

#### 2. Ship HUD Control Ring (2-3 weeks)
**Why Second**: Core UI element for all gameplay  
**Current State**: ImGui panels for stats, needs custom HUD  
**Work Required**:
- [ ] Design retained-mode UI framework (replacing ImGui for game UI)
- [ ] Implement circular shield/armor/hull arcs
- [ ] Add capacitor vertical bar
- [ ] Display velocity arc with color states
- [ ] Alert stack for warnings
- [ ] Damage feedback (shield ripple, armor flash, hull shake)

**Success Criteria**:
- HUD renders at 60 FPS with 200 entities
- Readable from peripheral vision
- EVE-style visual theme
- No ImGui for game UI (keep for debug)

**Technical Notes**:
- Bottom-center anchored
- Shield (outer), Armor (middle), Hull (inner) rings
- Clockwise depletion from 12 o'clock
- Use theme colors from `data/ui/eve_dark_theme.json`

---

### 🎯 High Priority (Enables content)

#### 3. Basic AI Economic Actors - Miners (3-4 weeks)
**Why Third**: Foundation for dynamic economy  
**Current State**: NPCs exist but don't "live" in the world  
**Work Required**:
- [ ] Design AI actor state machine (Idle → Patrol → Mine → Haul → Sell → Repeat)
- [ ] Implement miner AI (select asteroid, activate lasers, fill cargo)
- [ ] Implement hauler AI (travel to station, dock, sell ore)
- [ ] Add AI wallet system (ISK tracking per NPC)
- [ ] Create AI goals (profit maximization)
- [ ] Add AI decision-making (choose best ore based on market prices)

**Success Criteria**:
- 10 AI miners autonomously harvest ore
- AI haulers transport ore to stations
- AI sells ore on market
- AI makes profit over time
- Player can disrupt by destroying haulers → prices spike

**Technical Notes**:
- AI entities are full ECS entities (not "fake")
- AI uses same systems as players (mining, inventory, market)
- AI state persists across server restarts
- AI can die permanently (drops loot)

---

#### 4. AI Pirates & Security (2-3 weeks)
**Why Fourth**: Creates risk/reward for economy  
**Prerequisites**: AI Miners complete  
**Work Required**:
- [ ] Design pirate AI (patrol trade routes, attack haulers)
- [ ] Implement security response AI (warp to attack, engage pirates)
- [ ] Add faction standings integration (pirates = -10, security = +10)
- [ ] Create dynamic spawn system (more pirates → more security)
- [ ] Add loot drops from destroyed haulers

**Success Criteria**:
- Pirates attack AI haulers 10% of the time
- Security responds within 30 seconds
- Destroyed haulers drop cargo
- Players can salvage or steal loot
- Economic impact visible (prices increase when haulers die)

**Technical Notes**:
- Pirates warp to trade lanes
- Security spawns near stations
- Use existing CombatSystem and TargetingSystem
- Faction standings affect AI behavior

---

### 📈 Medium Priority (Polish & Content)

#### 5. Custom UI Window System (4-6 weeks)
**Why Fifth**: Improves UX, but HUD is more critical  
**Prerequisites**: Ship HUD complete  
**Work Required**:
- [ ] Design retained-mode window framework
- [ ] Implement DockNode tree (split/leaf nodes)
- [ ] Add window docking/undocking
- [ ] Port existing ImGui panels (inventory, fitting, market)
- [ ] Add keyboard-first navigation
- [ ] Implement data binding (observer pattern)

**Success Criteria**:
- All game UI uses custom system (ImGui only for debug)
- Windows can dock/float
- Keyboard navigation works
- EVE-style theme applied
- <2ms render time per frame

**Technical Notes**:
- Keep ImGui available for dev tools
- Use theme from `data/ui/eve_dark_theme.json`
- Support window presets (combat, trading, mining)
- Persist window layout in player profile

---

#### 6. Advanced Mission Generation (3-4 weeks)
**Why Sixth**: Content variety, but static missions work for now  
**Prerequisites**: None (can work in parallel)  
**Work Required**:
- [ ] Design procedural mission templates
- [ ] Add dynamic objective generation
- [ ] Implement difficulty scaling (player skills, ship class)
- [ ] Add branching mission chains
- [ ] Create persistent mission consequences

**Success Criteria**:
- 100+ unique missions from 20 templates
- Difficulty scales with player progression
- Missions feel different each time
- Choices matter (branching paths)

**Technical Notes**:
- Use existing MissionSystem
- Add template system (JSON-defined parameters)
- Use player skills to scale rewards/difficulty
- Integrate with faction standings

---

#### 7. Performance Profiling & Optimization (2-3 weeks)
**Why Seventh**: Good to do before scaling up  
**Prerequisites**: AI actors and custom UI complete  
**Work Required**:
- [ ] Profile server tick performance
- [ ] Identify hot paths (likely entity queries)
- [ ] Add spatial partitioning (grid or quadtree)
- [ ] Optimize frequent queries (targeting, nearby entities)
- [ ] Test with 500+ entities

**Success Criteria**:
- Server maintains 20 Hz tick rate with 500 entities
- <10ms average tick time
- Entity queries <1ms
- No frame drops on client with 200 visible ships

**Technical Notes**:
- Use gprof or Valgrind for profiling
- Consider spatial hash grid (simple, fast)
- Cache frequently accessed components
- Add performance metrics dashboard

---

## Decision Framework

When choosing what to work on:

1. **Blockers First**: If it blocks other work, do it now
2. **Player Impact**: What improves the experience most?
3. **Risk**: Do high-risk items early (easier to change)
4. **Dependencies**: Respect technical dependencies

---

## What NOT to Do (Yet)

### ❌ Don't Waste Time On:

1. **PvP Systems** - Project is PvE-focused, PvP is optional future feature
2. **Advanced Graphics** - Current procedural ships work fine, focus on gameplay
3. **Steam Integration** - Optional, low priority (already stubbed)
4. **Database Migration** - In-memory works for now, do this when scaling
5. **Extensive Balance** - Content exists, balance after AI economy is working
6. **Over-Engineering** - Build the minimum viable feature first

---

## Success Metrics

After completing these tasks, the project should have:

1. **Playable Experience**: Smooth network, functional HUD, engaging combat
2. **Living World**: AI miners, haulers, pirates create dynamic economy
3. **Polish**: Custom UI feels like EVE Online
4. **Performance**: Handles 200 ships at 60 FPS
5. **Content**: Procedural missions provide variety

---

## Estimated Timeline

| Task | Duration | Cumulative |
|------|----------|------------|
| Network Interpolation | 1-2 weeks | 2 weeks |
| Ship HUD | 2-3 weeks | 5 weeks |
| AI Miners | 3-4 weeks | 9 weeks |
| AI Pirates | 2-3 weeks | 12 weeks |
| Custom UI | 4-6 weeks | 18 weeks |
| Mission Generation | 3-4 weeks | 22 weeks |
| Performance | 2-3 weeks | 25 weeks |

**Total**: ~6 months to "living world" state

With parallel work (missions while building UI), could compress to **4-5 months**.

---

## Next Session Goals

For the immediate next coding session:

1. **Start Network Interpolation**
   - Review current client network code
   - Design interpolation system
   - Implement basic LERP for positions
   - Test with 2 clients

2. **Plan Ship HUD**
   - Sketch HUD layout
   - Design data flow (ECS → UI)
   - Choose rendering approach (immediate mode, retained mode)

3. **Prototype AI Miner**
   - Create simple state machine
   - Test mining behavior
   - Verify integration with existing systems

---

## Resources Needed

1. **Reference Materials**
   - EVE Online UI screenshots for HUD design
   - Network interpolation algorithms (LERP, exponential smoothing)
   - AI state machine patterns

2. **Testing Environment**
   - Multiple client instances for network testing
   - Profiling tools (gprof, Valgrind, perf)
   - Fleet battle scenario (100+ ships)

3. **Community Feedback**
   - Early alpha testers for network feel
   - UI/UX feedback on HUD design
   - Balance feedback on AI behavior

---

## Risk Mitigation

### Technical Risks

1. **Network Interpolation Complexity**
   - **Risk**: Interpolation adds latency or jitter
   - **Mitigation**: Start simple (LERP), iterate based on testing
   - **Fallback**: Keep current snapshot system as option

2. **Custom UI Performance**
   - **Risk**: Custom UI slower than ImGui
   - **Mitigation**: Profile early, optimize hot paths
   - **Fallback**: Keep ImGui if performance suffers

3. **AI Actor Complexity**
   - **Risk**: AI behavior creates server lag
   - **Mitigation**: Limit AI count, optimize state machine
   - **Fallback**: Reduce AI tick rate (5 Hz instead of 20 Hz)

### Schedule Risks

1. **Underestimated Complexity**
   - **Risk**: Tasks take 2x longer than estimated
   - **Mitigation**: Build MVPs first, add polish later
   - **Fallback**: Cut features, focus on core experience

2. **Scope Creep**
   - **Risk**: Adding features not on roadmap
   - **Mitigation**: Strict adherence to priority queue
   - **Fallback**: Move new ideas to "Future Work" list

---

## Conclusion

The project is in excellent shape with all baseline systems complete. The next phase focuses on **bringing the world to life** through smooth networking, intuitive UI, and AI economic actors.

**First Step**: Complete network interpolation (1-2 weeks)

**Ultimate Goal**: Living, breathing EVE-style universe with player-driven economy

---

*Last Updated: February 11, 2026*
