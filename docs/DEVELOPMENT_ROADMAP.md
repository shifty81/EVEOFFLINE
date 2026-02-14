# Atlas Engine — Locked-In Development Roadmap

> **Source of truth**: Derived from `features1.md` (canonical project bible).
> This document defines the locked-in development path for EVE OFFLINE built on the Atlas Engine.
> All new work must align with this roadmap. No feature ships unless it passes the design enforcement rules below.

---

## Design Enforcement Rules (Non-Negotiable)

Every new system or feature **must** pass all three checks:

1. **Atlas-fit** — Does it belong naturally inside Atlas Engine?
2. **Simulation-first** — Does it work without UI, audio, or player input?
3. **Lore-consistent** — Would captains in-universe believe this exists?

If it fails any one → it doesn't ship.

---

## Architecture Pillars

| Layer | Responsibility |
|-------|---------------|
| **Server (Authoritative)** | Tick-based simulation, ECS, AI, economy, combat, persistence |
| **Client (Display)** | Rendering, interpolation, UI, input, audio — no game logic authority |
| **Networking** | Server sends snapshots → Client caches → Interpolation → Render |

- **Tick-based simulation** (10–20 Hz), deterministic for networking and replays
- **Atlas Engine & Atlas UI exclusively** — all systems are Atlas-native
- **No ImGui for game UI** — retained only for debug/dev tools

---

## Naming Conventions

| Scope | Prefix | Example |
|-------|--------|---------|
| Engine-wide | `Atlas` | `AtlasDeterministicScheduler` |
| Simulation (headless) | `Sim` | `SimFleetMoraleComponent` |
| Rendering | `AtlasRender` | `AtlasRenderTacticalOverlay` |
| UI | `UI_` | `UI_TacticalOverlayRing` |
| Game-specific | `EVO_` | `EVO_PirateCoalitionDoctrine` |

**Forbidden:** Generic names (`Manager`, `Controller`, `Handler`), engine-agnostic names (`InventorySystem`, `AIController`).

- **Components** = pure data, no logic, serializable
- **Systems** = stateless logic processors, deterministic, scheduled by Atlas ECS

---

## Development Phases

### Phase 0: Foundation (Current — v0.1)
> Get the engine running, title screen working, basic flight.

- [x] Atlas Engine C++ client with OpenGL rendering
- [x] Atlas UI immediate-mode system (Photon Dark theme)
- [x] Title screen with Undock / Settings / Quit
- [x] Ship HUD (control ring, capacitor, module rack)
- [x] Atlas Console (debug)
- [x] Pause menu
- [x] Entity Component System (ECS) basics
- [x] TCP networking client
- [x] Embedded server for single-player
- [x] Procedural ship generation (seed-based)
- [x] Camera system and ship physics
- [x] Solar system scene with celestial bodies
- [x] Context menu and radial menu
- [x] Audio system (OpenAL)
- [x] Deferred rendering pipeline
- [x] Shadow mapping
- [x] Post-processing effects
- [ ] **Fix: Title screen buttons clickable** (consumeMouse ordering)
- [ ] Warp visual effects (tunnel shader stack)
- [ ] Basic combat loop (lock, fire, damage)

### Phase 1: Core Fleet (v0.1–v0.2)
> Player + small fleet, captain personalities, basic flight & combat.

- [ ] Fleet system: player + up to 4 captains (max 5 ships)
- [ ] `SimCaptainPsychologyComponent` (loyalty, aggression, paranoia, ambition)
- [ ] `AtlasCaptainChatterSystem` — contextual fleet dialogue
- [ ] Captain morale & memory persistence
- [ ] Basic formation in warp
- [ ] Ship fitting system (high/mid/low slots)
- [ ] Skill system fundamentals
- [ ] Mining mechanics (asteroid fields, ore extraction)
- [ ] Station docking and services
- [ ] Warp system with cinematic tunnel
  - Shader layers: radial distortion, starfield velocity bloom, tunnel skin, vignette
  - Adaptive audio: engine core (sub-bass), warp field harmonics, environmental shimmer
  - Dynamic intensity based on ship mass
  - Accessibility controls (motion, bass, blur intensity)

### Phase 2: Living Universe (v0.2)
> The universe moves without the player. NPCs are real actors.

**Goal:** A universe that reacts over time and remembers player impact.

#### Core Systems
- [ ] `SimStarSystemStateComponent` — per-system state vector (traffic, economy, security, threat, faction influence)
- [ ] `AtlasBackgroundSimulationSystem` — continuous background simulation tick
- [ ] Threshold-based system events (pirate surge, shortages, lockdowns)
- [ ] Local reputation per system

#### NPC Life
- [ ] Intent-driven NPC AI (`SimNPCIntentComponent`)
  - Intents: Trade, Patrol, Hunt, Explore, Flee, Escort, Salvage
  - Intent scoring: `NPCIntent ChooseIntent(NPC& npc, StarSystemState& sys)`
- [ ] NPC archetypes: Trader, Pirate, Patrol, Miner, Hauler, Industrialist
- [ ] NPC behavior trees (per archetype)
- [ ] NPC rerouting based on system danger
- [ ] Ambient traffic spawns driven by system state
- [ ] AI as real economic actors (wallets, ship ownership, permanent death)

#### Combat Aftermath
- [ ] Wreck persistence
- [ ] Salvage NPCs
- [ ] Security response delay
- [ ] System threat adjustment from combat

#### World Feedback
- [ ] Station news feed
- [ ] Rumors about player actions (`AtlasInformationPropagationSystem`)
- [ ] Visual cues (lockdowns, traffic density)

#### Economy Engine
- [ ] No fake NPC market orders — everything produced, transported, consumed, destroyed
- [ ] Regional markets with delayed information
- [ ] Dynamic taxation and broker fees
- [ ] Supply/demand curves driven by NPC activity
- [ ] `AtlasEconomicFlowSystem`

### Phase 3: Wing System & Midgame (v0.3)
> Expand fleet to 15 ships, wing roles, deeper fleet psychology.

- [ ] Wing system: 3 wings × 5 ships
- [ ] Wing commanders with decision authority
- [ ] Wing roles: Mining, Combat, Logistics
- [ ] Wing-level morale
- [ ] Wing chatter overrides individual chatter
- [ ] Commander disagreement system
- [ ] `AtlasFleetDoctrineSystem`
- [ ] Tactical Overlay (`AtlasSpatialProjectionSystem`)
  - Passive 2.5D spatial overlay
  - Truthful distances and positioning
  - Shared filtering with Overview
  - Non-interactive (read-only)
- [ ] Overview panel with entity filtering
- [ ] Imperfect information system (scan confidence ranges, intel decay)
- [ ] Captain backgrounds (former miner, ex-military, etc.)

### Phase 4: Full Fleet Doctrine & Endgame (v0.4+)
> 25 ships, 5 wings, philosophical fractures, civilization-scale gameplay.

- [ ] Full fleet: 5 wings × 5 ships (25 total)
- [ ] Doctrine slots: Mining/Salvage, Logistics/Repair, Escort, Construction
- [ ] Fleet ideology system — captains with philosophical alignment
- [ ] Fleet fracture mechanics (disagreement → potential departure)
- [ ] `AtlasFleetMoraleResolutionSystem`
- [ ] Mobile industry and logistics
- [ ] Fleet cargo pooling (civilization-scale inventory)
- [ ] `AtlasPersistenceDeltaSystem` — long-term consequence tracking

### Phase 5: Titan Systems & Meta-Threat (v0.5+)
> Galactic-scale emergent threats. Titans are processes, not spawns.

- [ ] `SimTitanAssemblyProgressComponent` — progress, secrecy, resource pressure
- [ ] `AtlasTitanAssemblyPressureSystem` — pirate resources → assembly progress
- [ ] `AtlasOuterRimLogisticsDistortionSystem` — trade route distortion
- [ ] `AtlasRumorPropagationSystem` — incomplete intel leaks
- [ ] `AtlasGalacticResponseCurveSystem` — AI faction response escalation
- [ ] Warp anomalies & rare mid-warp events
  - Visual-only phenomena (common)
  - Sensory distortions (uncommon)
  - Spacetime shear events (rare, memorable)
  - Legendary warp events (ultra-rare, screenshot-worthy)
- [ ] Pirate coalition doctrine
- [ ] Evidence-based player discovery (no UI reveals)
- [ ] Galactic response curves (empire reinforcement, trade rerouting)

---

## Explicitly Out of Scope

These are **not** part of this project:

- ❌ PvP combat
- ❌ Player empires
- ❌ Full MMO economy simulation
- ❌ Scripted storylines or quest systems
- ❌ Twitch shooter mechanics
- ❌ Clickable tactical overlays
- ❌ Fake distances or rubber-band difficulty
- ❌ Asset-copied EVE clone
- ❌ Client-authoritative logic
- ❌ Theme-park content

---

## Atlas UI Widget Taxonomy

### Primitive Widgets
| Widget | Purpose |
|--------|---------|
| `UI_Panel` | Layout container |
| `UI_Text` | Text rendering |
| `UI_Icon` | Symbolic glyph |
| `UI_Bar` | Capacitor / progress |
| `UI_Ring` | Range indicators |

### Composite Widgets
| Widget | Purpose |
|--------|---------|
| `UI_ModuleRack` | Ship modules |
| `UI_TargetPanel` | Locked targets |
| `UI_OverviewTable` | Entity list |
| `UI_TacticalOverlay` | Spatial truth |

### Interaction Widgets
| Widget | Purpose |
|--------|---------|
| `UI_ContextMenu` | Right-click actions |
| `UI_FilterEditor` | Overview filters |
| `UI_FittingSlot` | Ship fitting |

---

## Feature → Atlas Subsystem Mapping

| Feature | Atlas Subsystem |
|---------|----------------|
| Fleet morale | `AtlasFleetMoraleResolutionSystem` |
| Captain chatter | `AtlasCaptainChatterSystem` |
| Warp tunnels | `AtlasWarpTransitSimulationSystem` |
| Tactical overlay | `AtlasSpatialProjectionSystem` |
| Economy | `AtlasEconomicFlowSystem` |
| Pirate coalitions | `AtlasFactionDoctrineSystem` |
| Rumors | `AtlasInformationPropagationSystem` |
| Long-term consequences | `AtlasPersistenceDeltaSystem` |
| Titan assembly | `AtlasTitanAssemblyPressureSystem` |
| Background simulation | `AtlasBackgroundSimulationSystem` |
| NPC intent | `SimNPCIntentComponent` + behavior trees |
| Galactic response | `AtlasGalacticResponseCurveSystem` |

---

## Performance Budgets (Warp System)

### Visual Budget (1080p @ 60fps target)
| Layer | Budget |
|-------|--------|
| Radial distortion | ≤ 0.3 ms |
| Star stretch | ≤ 0.2 ms |
| Tunnel noise | ≤ 0.4 ms |
| Vignette / blur | ≤ 0.2 ms |
| **Total warp visuals** | **≤ 1.2 ms** |

### Audio Budget
| System | Cost |
|--------|------|
| Engine core | Always on (cheap) |
| Harmonics | 2–4 voices max |
| Shimmer events | 1-shot, pooled |
| Ambient pad | Single stereo loop |

---

## Ship HUD Architecture ("Cold Control Ring")

Anchored bottom-center, designed for peripheral readability:

- **Control Ring**: Shield (outer) → Armor (middle) → Hull (inner core), arc-based with clockwise depletion
- **Velocity Arc**: Speed indicator around control ring, color changes for boost/align/webbed states
- **Capacitor Bar**: Vertical bar with gradient (green → red)
- **Module Rack**: Horizontal row grouped by slot type (High/Mid/Low), 32×32 icons with cooldown sweeps
- **Target Brackets**: Four-corner brackets in space, color-coded by relation, lock progress arcs
- **Alert Stack**: Priority-based warnings above ring
- **Damage Feedback**: Shield (blue ripple), armor (yellow flash), hull (red pulse + shake)
