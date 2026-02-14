# Feature Specification

Structured feature list and design rules for the BlenderSpaceshipGenerator
and the EVEOFFLINE game engine that consumes its output.

> For engine-specific data structures, JSON schemas, and C++ mappings see
> **[ENGINE_INTEGRATION.md](ENGINE_INTEGRATION.md)**.

---

## Table of Contents

1. [Design Pillars](#1-design-pillars)
2. [Brick Taxonomy & LEGO Logic](#2-brick-taxonomy--lego-logic)
3. [Hardpoint Grid System](#3-hardpoint-grid-system)
4. [Spine-First Assembly Pipeline](#4-spine-first-assembly-pipeline)
5. [Hull Shaping](#5-hull-shaping)
6. [Hull Skinning (Control Volumes → SDF Mesh)](#6-hull-skinning-control-volumes--sdf-mesh)
7. [Engine Archetypes](#7-engine-archetypes)
8. [Turret System](#8-turret-system)
9. [Ship DNA](#9-ship-dna)
10. [Interior Generation](#10-interior-generation)
11. [Module System](#11-module-system)
12. [Design Styles & Factions](#12-design-styles--factions)
13. [Texture & Material System](#13-texture--material-system)
14. [Station Generation](#14-station-generation)
15. [Asteroid Belt Generation](#15-asteroid-belt-generation)
16. [EVEOFFLINE / Atlas Integration](#16-eveoffline--atlas-integration)
17. [Damage Propagation & Salvage](#17-damage-propagation--salvage)
18. [Player Build Mode UX](#18-player-build-mode-ux)
19. [Implementation Status](#19-implementation-status)

---

## 1. Design Pillars

These principles guide every decision in the generator and the game engine:

1. **Ships are assemblies, not sculptures** — every part is a typed brick
   on a grid.
2. **Every block has a purpose** — hull plates protect, reactors power,
   engines thrust.
3. **Negative space is allowed** — hangars, exhausts, and weapon recesses
   are carved, not filled.
4. **Orientation matters** — engines must face backward, weapons need
   clearance arcs.
5. **Symmetry is a choice, not a default** — players and the generator can
   build asymmetric ships.
6. **Bricks are authoritative; meshes are derived** — the hull visual is
   computed from bricks, never hand-authored.

---

## 2. Brick Taxonomy & LEGO Logic

18 brick types across 5 categories.  Players can only place bricks — no
freeform meshes.

### Categories

| Category     | Bricks |
|-------------|--------|
| **CORE**     | Reactor Core, Power Bus, Structural Spine |
| **HULL**     | Hull Plate, Hull Wedge, Hull Corner, Armor Block |
| **FUNCTION** | Engine Block, Thruster, Capacitor, Shield Emitter |
| **UTILITY**  | Hardpoint Mount, Docking Clamp, Sensor Mast, Antenna Dish |
| **DETAIL**   | Panel, Vent, Pipe |

### Brick Properties

Each brick declares:

- **Size** — grid cells occupied (e.g. 2×3×2 for an engine block)
- **Shape** — proxy geometry (box, cylinder, sphere, cone, wedge)
- **Scale band** — primary (1.0×), structural (0.7×), or detail (0.22×)
- **Hardpoints** — attachment points with role and direction

### Scale Hierarchy (Rule of Three)

| Band        | Factor | Usage |
|-------------|--------|-------|
| Primary     | 1.0    | Core hull, reactor, main engines |
| Structural  | 0.7    | Hull plates, armor, capacitors |
| Detail      | 0.22   | Vents, panels, antennas |

Detail geometry must never compete visually with hull mass.

> **Full brick table →** [ENGINE_INTEGRATION.md §4](ENGINE_INTEGRATION.md#4-brick-taxonomy)

---

## 3. Hardpoint Grid System

### Grid Snapping

Bricks snap to a 3D grid whose cell size depends on ship class:

| Ship Class   | Grid (m) | Ship Class   | Grid (m) |
|-------------|----------|-------------|----------|
| Shuttle      | 0.5      | Carrier      | 4.0      |
| Fighter      | 1.0      | Dreadnought  | 4.0      |
| Frigate      | 1.0      | Capital      | 6.0      |
| Destroyer    | 1.5      | Titan        | 8.0      |
| Cruiser      | 2.0      | Industrial   | 1.5      |
| Battleship   | 3.0      | Mining Barge | 1.0      |

### Hardpoint Rules

Each brick declares hardpoints with a role and direction vector:

```python
brick.hardpoints = [
    {"role": "engine",  "direction": (0, -1, 0)},
    {"role": "weapon",  "direction": (1,  0, 0)},
]
```

- No hardpoint → cannot attach that system.
- Two bricks connect when adjacent hardpoint roles are compatible and
  direction vectors are opposing.

---

## 4. Spine-First Assembly Pipeline

Every ship starts with a spine, not a hull.

```
[Engine] ── [Reactor] ── [Capacitor] ── [Bridge]
```

Everything else hangs off this core.

### 9-Stage Generation Order

| Stage | Action | Notes |
|-------|--------|-------|
| 1 | Core hull (spine) | Structural spine placed first |
| 2 | Cockpit / bridge | Positioned at ship front |
| 3 | Major structures (wings) | Smaller vessels only |
| 4 | Engines (archetype-varied) | Main thrust, maneuvering, utility |
| 5 | Weapons & turrets | Hardpoint-based placement |
| 6 | Detail modules | Cargo, shields, sensors, etc. |
| 7 | Interior (optional) | FPV-ready rooms and corridors |
| 8 | Hull taper / deform pass | Silhouette shaping |
| 9 | Bevel + auto-smooth cleanup | Manufactured look |

### Why Spine-First Matters

- Prevents "blob ships" — forces intentional structure.
- Guarantees power connectivity from reactor to all systems.
- Enables structural integrity simulation for damage.

---

## 5. Hull Shaping

### Taper Pass

After assembly, the hull is tapered along the Y axis to break the box look:

```python
taper_hull(obj, axis='Y', factor=0.85)
```

- `factor=1.0` → no taper (identity)
- `factor=0.85` → subtle spaceship feel (default)
- `factor=0.5` → aggressive taper

### Cleanup Pass

```python
apply_cleanup_pass(obj)
```

Adds:
- **Bevel modifier** — width proportional to hull size, 2 segments, 30° angle limit
- **Edge split modifier** — 30° split angle for hard edges

This alone makes ships feel manufactured instead of primitive.

---

## 6. Hull Skinning (Control Volumes → SDF Mesh)

### Concept

Players and the generator place functional bricks.  The hull mesh is
**derived** from those bricks, never hand-authored.

```
[Control Bricks]
     ↓
[SDF / Volume Field]
     ↓
[Volume to Mesh]
     ↓
[Hull Skin + Bevel + Normals]
```

### How It Works

1. Each brick creates a **control volume** (simple proxy mesh).
2. Control volumes are converted to a volume field (Mesh to Volume).
3. ADD volumes are unioned; SUBTRACT volumes are differenced.
4. Volume is converted back to mesh.
5. Result is smoothed and shade-smoothed.

### Hull Influence Weights

| Brick Type    | Hull Weight | Notes |
|--------------|-------------|-------|
| Reactor       | 3.0         | Thick core volume |
| Armor Block   | 2.0         | Local hull bulge |
| Engine Block  | 1.5         | Exhaust shaping |
| Capacitor     | 1.2         | Internal volume |
| Hull Plate    | 1.0         | Standard skin |
| Weapon Mount  | 0.8         | Socket area |
| Utility       | 0.5         | Minimal influence |
| Antenna       | 0.2         | Almost no hull effect |

### Negative Space (Subtractive Volumes)

SUBTRACT volumes carve cavities for:
- Engine exhausts
- Hangar bays
- Weapon recesses

This is how EVE-style gaps and engineered voids are achieved.

### Why This System Works

- Moving a control shape → hull flows and re-wraps automatically.
- Damage removes control shapes → hull visibly collapses.
- Same system works for generator, player build mode, and runtime damage.

> **Geometry Nodes graph details →**
> [ENGINE_INTEGRATION.md §9](ENGINE_INTEGRATION.md#9-hull-skinning-pipeline)

---

## 7. Engine Archetypes

Engines vary by role instead of being identical cylinders:

| Archetype       | Size   | Nozzle Flare | Glow | Role |
|----------------|--------|-------------|------|------|
| MAIN_THRUST     | Large  | Yes          | 5.0  | Primary propulsion |
| MANEUVERING     | Small  | No           | 2.0  | Attitude control |
| UTILITY_EXHAUST | Medium | No           | 1.0  | Auxiliary systems |

### Selection Rule

The first ~60 % of a ship's engines are main thrust, ~25 % maneuvering,
and the rest utility exhaust.

### Engine Placement Rules

- Engines must face backward (−Y direction).
- Main thrust engines get a cone-shaped nozzle flare child object.
- Each engine stores its archetype as a custom property for game engine
  mapping.

---

## 8. Turret System

### Turret Geometry

Each turret is a 3-part assembly:
1. **Base cylinder** — mounting point
2. **Rotation ring** (torus) — visual turret traverse
3. **Barrel** (cylinder) — weapon barrel

### Custom Properties (Engine-Facing)

| Property          | Type   | Example              |
|------------------|--------|---------------------|
| `turret_index`    | int    | 1 (1-based)          |
| `turret_type`     | string | `"projectile"`       |
| `tracking_speed`  | float  | 30.0 (deg/s)         |
| `rotation_limits` | string | `"yaw:360,pitch:90"` |
| `hardpoint_size`  | float  | 1.44                 |

Maximum turrets per ship: **10**.

### Weapon Rules

- Weapon mounts block hull placement at the same grid cell.
- Turrets require clearance arc (no obstructing bricks in fire cone).
- Power connection required for turret operation.

---

## 9. Ship DNA

Ship DNA is the canonical, reproducible serialisation of a ship.

### Format

```json
{
  "seed": 917221,
  "class": "CRUISER",
  "style": "SOLARI",
  "naming_prefix": "EVEOFFLINE",
  "grid_size": 2.0,
  "bricks": [
    {"type": "STRUCTURAL_SPINE", "pos": [0, 0, 0]},
    {"type": "ENGINE_BLOCK", "pos": [0, -4, 0], "archetype": "MAIN_THRUST"}
  ]
}
```

### What Ship DNA Enables

- **Reproducibility** — same seed + brick list → identical ship.
- **Save/load** — export DNA to file, reimport later.
- **NPC generation** — NPCs use the same brick system as players.
- **Damage** — destroying bricks removes entries from the DNA.
- **Salvage** — dropped bricks reference real brick types.
- **Sharing** — exchange ship designs between players.

### Storage

Ship DNA is stored as a JSON custom property `ship_dna` on the hull object
in Blender.  It can also be exported to standalone `.json` files.

---

## 10. Interior Generation

### Standards (Human Scale)

| Measurement     | Value |
|----------------|-------|
| Human height    | 1.8 m |
| Door height     | 2.0 m |
| Door width      | 1.0 m |
| Corridor width  | 1.5 m |
| Corridor height | 2.5 m |
| Room height     | 3.0 m |

### Room Types

| Room           | Description |
|---------------|-------------|
| Cockpit        | Pilot seat + control panel (small ships) |
| Bridge         | Command chair + nav console + walls (large ships) |
| Corridor       | Floor + ceiling + side walls |
| Corridor Network | Main corridor + side branches (crew > 20) |
| Crew Quarters  | Bunks scaled to crew capacity |
| Cargo Bay      | Open area with cargo containers |
| Engine Room    | Reactor core with orange glow material |

### Progressive Complexity

| Ship Size      | Interior Layout |
|---------------|----------------|
| Shuttle/Fighter | Cockpit only |
| Corvette/Frigate | Cockpit + corridor + crew quarters |
| Destroyer+     | Bridge + corridor network + quarters + cargo + engine room |

---

## 11. Module System

6 module types with progressive availability:

| Module  | Shape    | Scale | Available For |
|---------|----------|-------|--------------|
| CARGO   | Box      | 1.0×  | Medium+ ships |
| WEAPON  | Cylinder | 0.8×  | All ships |
| SHIELD  | Sphere   | 0.7×  | All ships |
| HANGAR  | Box      | 1.5×  | Large ships only |
| SENSOR  | Cone     | 0.5×  | All ships |
| POWER   | Cylinder | 0.9×  | Medium+ ships |

### Availability by Class

- **Small** (Shuttle, Fighter): Weapon, Shield, Sensor
- **Medium** (Corvette, Frigate): + Cargo, Power
- **Large** (Cruiser+): All module types including Hangar

---

## 12. Design Styles & Factions

### Game-Inspired Styles

| Style          | Character | Hull Treatment |
|---------------|-----------|---------------|
| X4 Foundations  | Angular, geometric | Sharp bevels |
| Elite Dangerous | Sleek, aerodynamic | Tapered shapes |
| Eve Online      | Organic, flowing | Spherical cast modifier |
| No Man's Sky    | Colorful, varied | Rounded + bevel hybrid |
| Mixed           | Combination | Light bevel |

### EVEOFFLINE Factions

| Faction   | Visual Style | Tank Focus | Hull Modifier |
|----------|-------------|------------|--------------|
| Solari    | Golden, elegant | Armor | Smooth bevel + light cast |
| Veyren    | Angular, utilitarian | Shield | Sharp bevel |
| Aurelian  | Sleek, organic | Drones | Subdivision + spherical cast |
| Keldari   | Rugged, industrial | Missiles | Heavy bevel |

---

## 13. Texture & Material System

### Material Roles

| Role           | Assignment Rule | PBR Properties |
|---------------|----------------|---------------|
| Hull material  | Default for hull/structure | Metallic 0.7, Roughness 0.4 |
| Accent material| Weapons, cockpit, wings, turrets | Metallic 0.9, Roughness 0.2, Emission 0.5 |
| Engine material| Engine objects | Emission 5.0 (blue glow) |

### Procedural Texturing

- **Noise texture** — hull panel variation (Scale 15, Detail 8)
- **Voronoi texture** — hull plating pattern (Scale 8)
- **Weathering** — noise-based dirt mask (configurable 0.0–1.0)

### NMS Color System

NMS style picks a random vibrant color set per seed from 6 pre-defined
palettes, giving each ship a unique but consistent scheme.

---

## 14. Station Generation

### Station Types

| Type        | Scale | Sections | Docking Bays | Hangars |
|------------|-------|----------|-------------|---------|
| Industrial  | 80    | 4        | 2           | Yes |
| Military    | 100   | 5        | 3           | Yes |
| Commercial  | 90    | 4        | 4           | No |
| Research    | 60    | 3        | 1           | No |
| Mining      | 70    | 3        | 2           | Yes |
| Astrahus    | 50    | 3        | 2           | No |
| Fortizar    | 100   | 5        | 4           | Yes |
| Keepstar    | 200   | 8        | 8           | Yes |

### Faction Architecture

| Faction  | Symmetry   | Features |
|---------|-----------|----------|
| Solari   | Radial     | Spires + domes, golden cathedral |
| Veyren   | Cubic      | Angular blocks, minimal |
| Aurelian | Organic    | Spherical domes, organic curves |
| Keldari  | Asymmetric | Scaffolding, rusted patchwork |

---

## 15. Asteroid Belt Generation

### Ore Types (16)

All ore types from EVEOFFLINE, ranging from common highsec (Dustite) to
rare nullsec (Nexorite).  Each ore has distinct color, roughness, and
metallic PBR values.

### Belt Layouts

| Layout      | Shape | Default Count |
|------------|-------|--------------|
| Semicircle  | Standard arc | 30 |
| Sphere      | Spherical distribution | 40 |
| Cluster     | Dense anomaly cluster | 50 |
| Ring        | Sparse outer ring | 25 |

Each asteroid gets procedural deformation (icosphere + displacement),
random rotation/size, and PBR material matching ore visual data.

---

## 16. EVEOFFLINE / Atlas Integration

### Import Pipeline

1. Load ship JSON from `EVEOFFLINE/data/ships/*.json`
2. Map `race` → faction style, `class` → ship class, `generation_seed` → seed
3. Calculate module slots from high/mid/low slot totals
4. Generate matching geometry

### Export Pipeline

1. Select hull object in Blender
2. Set export path to `EVEOFFLINE/data/ships/obj_models/`
3. Export OBJ (forward: −Z, up: Y)
4. Rename file to match ship `id` from JSON

### Batch Generation

A batch script can generate all ships headlessly:

```bash
blender --background --python batch_generate.py
```

> **Full integration guide →** [EVEOFFLINE_GUIDE.md](EVEOFFLINE_GUIDE.md)
>
> **Engine data formats →** [ENGINE_INTEGRATION.md](ENGINE_INTEGRATION.md)

---

## 17. Damage Propagation & Salvage

### Damage Rules

1. Brick HP reaches 0 → brick entity destroyed.
2. Control volume removed → hull auto-reskins (visible collapse).
3. Neighbour bricks re-evaluated for structural integrity.
4. If disconnected from spine → power loss → eventual break-off.

### Structural Integrity

Each brick tracks:
- **Parent connection** — which spine segment it connects through
- **Load count** — how many bricks depend on it

When a structural brick is destroyed, all children lose connection, power
cascades, hull thins, and the section eventually detaches as debris.

### Visual Feedback

| State             | Visual |
|------------------|--------|
| Power loss        | Hull darkens |
| Structural stress | Crack/glow lines |
| Hull thinning     | Visible dents |
| Detachment        | Chunk separation + debris |

### Salvage

When a ship dies:
- Bricks pop off as individual entities
- Functional bricks (reactor, engine, weapons) can be recovered
- Recovered bricks retain type and partial HP
- Hull plates fly free with physics

---

## 18. Player Build Mode UX

### Controls

- Click + drag → move control volume
- Snap to grid → automatic
- Ghost preview → shows placement before confirming
- Color-coded blocks → role identification

### Visual Overlays

| Colour | Meaning |
|--------|---------|
| 🔴 Red   | No power connection |
| 🔵 Blue  | Hull mass influence |
| 🟢 Green | Valid snap position |

### Player Mental Model

> "I'm placing ship systems — the hull forms itself."

The player never sculpts hull geometry.  They place functional bricks and
the SDF hull skin wraps around them automatically.

### Required UI Elements

- Power flow overlay (shows generation vs consumption)
- Thrust vector arrows (shows engine direction)
- Structural integrity warnings (disconnected sections highlighted)
- Hardpoint compatibility indicators

---

## 19. Implementation Status

### ✅ Implemented

- [x] 18 ship classes (Shuttle → Titan + utility + NMS variants)
- [x] 9-stage spine-first assembly pipeline
- [x] Brick taxonomy (18 types, 5 categories)
- [x] Scale hierarchy (primary / structural / detail)
- [x] Grid snapping per ship class
- [x] Engine archetypes (main thrust / maneuvering / utility)
- [x] Turret system with custom properties
- [x] Ship DNA JSON export/import
- [x] Hull taper pass
- [x] Bevel + auto-smooth cleanup pass
- [x] Interior generation (cockpit, bridge, corridors, quarters, cargo, engine room)
- [x] Module system (6 types, progressive availability)
- [x] 4 game-inspired styles + 4 EVEOFFLINE factions + NMS
- [x] Procedural PBR textures with weathering
- [x] Station generation (8 types, 4 faction architectures)
- [x] Asteroid belt generation (16 ore types, 4 layouts)
- [x] EVEOFFLINE JSON import
- [x] OBJ export for Atlas engine
- [x] Naming prefix system
- [x] Batch generation support

### 🔲 Planned (Engine-Side)

- [ ] ECS component system (BrickComponent, PowerComponent, etc.)
- [ ] Runtime SDF hull meshing
- [ ] Damage propagation system
- [ ] Salvage and brick recovery
- [ ] Player build mode with snap validation
- [ ] Power flow simulation
- [ ] Structural integrity checks
- [ ] LOD generation pipeline
- [ ] Collision mesh generation
- [ ] Animation system (bay doors, landing gear, turret rotation)

> **ECS structs and system pseudocode →**
> [ENGINE_INTEGRATION.md §10–§12](ENGINE_INTEGRATION.md#10-ecs-component-mapping)
