# EVE OFFLINE — Architecture Comparison & Viability Assessment

This document compares the current EVEOFFLINE project architecture against a comprehensive
EVE-like PVE game development blueprint, organized by core development pillars.
Each section scores coverage (✅ Implemented, ⚠️ Partial, ❌ Missing) and provides
recommendations for long-term viability.

---

## 1. Engine Infrastructure & Core Systems

| Requirement | Recommended Tech | Current Implementation | Status |
|---|---|---|---|
| Windowing/Context | GLFW or SDL2 | **GLFW 3.3+** — window management, input handling, fullscreen | ✅ |
| Graphics API | OpenGL or DirectX 12 | **OpenGL 4.5** (core profile) via GLEW/GLAD, GLM for math | ✅ |
| Rendering Pipeline | Forward or deferred | **Deferred rendering** (G-Buffer), PBR materials, shadow mapping, post-processing (bloom, tone mapping), instanced rendering | ✅ |
| Physics Engine | PhysX or Bullet | **Custom arcade physics** — velocity/acceleration vectors, no rigid-body collisions or broadphase | ⚠️ |
| ECS | Data-oriented ECS | **Custom ECS** in both C++ (`cpp_server/include/ecs/`) and Python (`engine/core/ecs.py`) with ~30 component types | ✅ |
| UI/Debug | Dear ImGui | **ImGui** — fitting panel, inventory, overview, market, missions, module browser, HUD | ✅ |
| Audio | OpenAL or FMOD | **OpenAL** (optional) — 3D spatial audio, procedural sound generation | ✅ |
| Asset Pipeline | Custom or library | **STB** for textures, LOD manager, frustum culler, asteroid field renderer | ✅ |

### Assessment
The engine foundation is **strong**. GLFW + OpenGL + ImGui is the correct stack for this
scope. The biggest gap is **physics** — the current custom arcade physics works for EVE's
flight model (which is not physics-based flight anyway), but integrating Bullet Physics
would add proper collision detection for asteroids, structures, and projectiles without
requiring full rigid-body simulation.

**Recommendation**: The current physics approach is actually **appropriate for EVE's
mechanics**. EVE Online itself does not use realistic physics — ships have artificial
movement commands (orbit, approach, keep at range). A full PhysX/Bullet integration
would be over-engineering unless you want physical asteroid collisions or missile
splash damage. Consider adding a **spatial hash / broadphase** for efficient
collision queries at scale (1000+ entities on grid) instead.

---

## 2. EVE-Specific Mechanics (C++ Logic)

| Requirement | Current Implementation | Status |
|---|---|---|
| Fitting System (High/Mid/Low/Rig/Drone slots) | `FittingPanel` UI, `ModuleInfo` structs, `Fitting` component with slot-based management | ✅ |
| CPU/Powergrid constraints | Fitting data tracks `cpu_used/max`, `powergrid_used/max` | ✅ |
| HP Model (Armor/Shield/Hull) | `Health` component with hull/armor/shield + per-type resistances (EM/Thermal/Kinetic/Explosive) | ✅ |
| Capacitor system | `Capacitor` component with energy pool management | ✅ |
| Movement: Orbit | AI state machine has orbit state | ✅ |
| Movement: Approach | AI state machine has approach state | ✅ |
| Movement: Keep at Range | Not explicitly implemented | ⚠️ |
| Movement: Align/Warp | Warp mechanics exist in navigation system | ✅ |
| Command/Wing Structure | Fleet system with FLEET_COMMANDER, WING_COMMANDER, SQUAD_COMMANDER roles; max 256 members | ✅ |
| Fleet Bonuses | Armor, shield, skirmish, information warfare bonuses | ✅ |
| Drone Bay/Control | `launch_drone` / `recall_drone` in game_systems; basic drone AI | ⚠️ |

### Assessment
EVE-specific mechanics are **extensively implemented**. The fitting system, damage model,
capacitor, and fleet hierarchy closely mirror EVE Online. The drone system needs expansion
(autonomous combat behaviors, bandwidth limits, drone HP/damage).

**Recommendation**: Add "Keep at Range" as a movement command — it's a simple distance-hold
algorithm. Expand drone AI with combat modes (aggressive/passive/focus fire) and bandwidth
limits per ship class.

---

## 3. Advanced AI & "Aggressive PVE"

| Requirement | Current Implementation | Status |
|---|---|---|
| NPC State Machine | `AISystem` with states: idle, approaching, orbiting, attacking, fleeing | ✅ |
| Gate Camping AI | Not implemented — no gate-camp behavior patterns | ❌ |
| Warp Disruption/Scrambling (tackle) | Not in AI logic — exists as module type but NPC doesn't use it | ⚠️ |
| Fleet Tactics (coordinated) | AI fleet coordination exists but limited | ⚠️ |
| Electronic Warfare (EWar) | Not implemented for NPCs | ❌ |
| Target Priority (drones first) | Basic target acquisition, no priority logic | ⚠️ |
| AI Pilot Agency (hireable) | Not implemented | ❌ |
| Autonomous Drone AI | Basic launch/recall, no autonomous combat decisions | ⚠️ |
| NPC Shield/Armor Repping | Not implemented for NPCs | ❌ |

### Assessment
AI is the **biggest gap** in the project. The basic state machine works for simple rats,
but "aggressive PVE" (lowsec-style combat) requires significantly more sophisticated
behavior.

**Recommendation — Priority Implementation Order**:
1. **Target Priority System** — NPCs should evaluate threats (drones → logistics → DPS)
2. **EWar Module Usage** — NPCs activate warp scramblers, target painters, sensor dampeners
3. **Gate Camping Behavior** — NPC fleet holds position, tackles on landing, focus fire
4. **Logistics AI** — NPC logi ships prioritize repping fleet members below threshold
5. **Scripting Layer (Lua)** — Consider Lua scripting for rapid iteration on AI behaviors
   without recompiling C++. This is a recommended addition from the blueprint.

---

## 4. Economy & Content Loop

| Requirement | Current Implementation | Status |
|---|---|---|
| Mining System | `MiningSystem` with laser cycles, ore extraction, skill modifiers | ✅ |
| Ice Mining | `IceMiningSystem` implemented | ✅ |
| Ore Reprocessing | Reprocessing system exists | ✅ |
| Industry/Manufacturing | `IndustrySystem` with blueprints, material requirements | ✅ |
| Market System | `MarketSystem` with buy/sell orders, order matching | ✅ |
| Wallet/Currency | `Wallet` component with ISK | ✅ |
| Planetary Interaction | `PlanetaryInteractionSystem` implemented | ✅ |
| Missions | `MissionSystem` with objectives, rewards | ✅ |
| Exploration Sites | `ExplorationSystem` implemented | ✅ |
| Dynamic Respawning | Asteroid respawn mechanics | ⚠️ |
| Procedural Site Generation | Asteroid fields generated, but sites are mostly static | ⚠️ |
| NPC Cargo/Hauler Interception | Not implemented | ❌ |
| Insurance | `InsuranceSystem` exists | ✅ |
| Contracts | `ContractSystem` exists | ✅ |
| Bounty System | `BountySystem` exists | ✅ |
| Corporation/Social | `CorporationSystem`, `SocialSystem` | ✅ |

### Assessment
The economy is **the strongest pillar** of the project. Almost all EVE economic systems
have at least a basic implementation. The content loop (mine → build → fit → fight) is
viable.

**Recommendation**: Add **procedural difficulty scaling** — sites that go uncleared should
upgrade in difficulty and reward quality over time, encouraging exploration. Add NPC
hauler convoys that transit between stations for interception gameplay.

---

## 5. Networking Architecture

| Requirement | Recommended | Current Implementation | Status |
|---|---|---|---|
| Architecture | P2P or Client-Server | **Client-Server** (authoritative server) | ✅ |
| Protocol | Custom binary or JSON | **JSON over TCP** (nlohmann/json) | ✅ |
| Message Types | State sync, commands | CONNECT, DISCONNECT, INPUT_MOVE, STATE_UPDATE, CHAT, COMMAND, SPAWN/REMOVE_ENTITY | ✅ |
| Session Management | Auth + sessions | `SessionManager` with Steam auth option | ✅ |
| Scale Target | 32 entities on grid | Current design supports small groups | ✅ |
| C++ Server | Optional | **`cpp_server`** with ECS, fully implemented | ✅ |
| Python Server | Fallback | **`server/server.py`** as alternative | ✅ |

### Assessment
Networking is **well-designed for the scope**. Client-server is actually preferable
to P2P for authoritative game state (prevents cheating, simpler entity synchronization).
JSON over TCP is fine for a small-group PVE game (< 32 players) — the overhead is
negligible at this scale.

**Recommendation**: For long-term scalability, consider adding **UDP for position updates**
(high-frequency, loss-tolerant) while keeping TCP for commands/chat (reliable). This is
a common hybrid approach. Not urgent — the current architecture works for the target
player count.

---

## 6. Overall Viability Score

| Pillar | Coverage | Score |
|---|---|---|
| Engine Infrastructure | All core systems present; deferred rendering, audio, asset pipeline | **9/10** |
| EVE-Specific Mechanics | Fitting, damage model, fleet, movement all implemented | **8/10** |
| AI & Aggressive PVE | Basic state machine only; needs EWar, tactics, gate camping | **4/10** |
| Economy & Content | Nearly complete EVE economy; mining through market | **9/10** |
| Networking | Client-server with auth, dual C++/Python servers | **8/10** |
| **Overall** | | **7.6/10** |

---

## 7. Long-Term Viability Verdict

### Is this a viable long-term project?

**Yes — emphatically.** The project has already built the hardest parts:
- A working deferred rendering pipeline with PBR
- A complete ECS architecture (both C++ and Python)
- EVE's complex economic simulation (market, industry, mining, PI)
- Ship fitting with the full damage resistance model
- Fleet management with command hierarchy
- Client-server networking with authentication

### Will it perform well?

**Yes, for the target scope.** Key performance-positive decisions:
- C++17 for the rendering client (critical path)
- Instanced rendering + LOD + frustum culling for draw-call optimization
- Deferred rendering allows many lights without per-light passes
- Python server for gameplay logic is fine — EVE Online itself uses
  Stackless Python for its server (game logic is not the bottleneck)

### What would make things easier?

1. **Lua Scripting for AI** — Hardest remaining work is AI. Embedding Lua
   (via sol2 or LuaBridge) would allow rapid iteration on NPC behaviors
   without recompiling C++. This is the single highest-impact addition.

2. **Behavior Trees Library** — Replace the simple state machine AI with
   a behavior tree system (e.g., BehaviorTree.CPP) for complex NPC tactics.

3. **Not PhysX/Bullet** — EVE's movement model is command-based, not
   physics-based. A spatial hash for proximity queries is more useful
   than a full physics engine.

### Recommended Next Development Priorities

```
Priority 1 (Critical): AI Systems
  → NPC EWar usage, target priority, gate camping behavior
  → Consider Lua scripting layer for rapid AI iteration

Priority 2 (High): Drone System Expansion
  → Autonomous combat modes, bandwidth limits, drone HP
  → Guard/assist/attack commands

Priority 3 (Medium): Procedural Content
  → Dynamic difficulty scaling for PVE sites
  → NPC hauler convoys for interception gameplay

Priority 4 (Low): Movement Polish
  → "Keep at Range" command
  → Manual piloting improvements

Priority 5 (Optional): Network Optimization
  → UDP channel for position updates (only needed at scale)
```

---

## 8. Technology Comparison Table

| System | Blueprint Recommends | Project Uses | Match? | Notes |
|---|---|---|---|---|
| Language | C++17/20 | C++17 + Python 3.11 | ✅ | Python for server logic mirrors EVE Online's architecture |
| Graphics | OpenGL / DX12 | OpenGL 4.5 | ✅ | OpenGL is correct for cross-platform |
| Physics | PhysX / Bullet | Custom arcade | ⚠️ | Appropriate for EVE's non-physics movement model |
| GUI | Dear ImGui | Dear ImGui | ✅ | Exact match |
| Scripting | Lua | None yet | ❌ | Highest-impact addition for AI iteration |
| Networking | Nakama / custom sockets | Custom TCP + JSON | ✅ | Appropriate for scope |
| Audio | (not specified) | OpenAL | ✅ | Good choice |
| ECS | (implied) | Custom ECS (C++ & Python) | ✅ | Working well |
| Data Format | (not specified) | JSON (nlohmann/json) | ✅ | Data-driven design |

---

*Document generated: 2026-02-06*
*Based on analysis of the full EVEOFFLINE repository codebase*
