# EVEOFFLINE / Atlas Engine Integration Reference

> **Audience:** Engine developers implementing the ship, station, and brick
> systems in C++.  This document is the single source of truth for every data
> structure, constant, pipeline step, and convention that the
> BlenderSpaceshipGenerator exports and the Atlas runtime must consume.

---

## Table of Contents

1. [Overview & Purpose](#1-overview--purpose)
2. [Coordinate System & Units](#2-coordinate-system--units)
3. [Ship DNA Schema](#3-ship-dna-schema)
4. [Brick Taxonomy](#4-brick-taxonomy)
5. [Hardpoint & Grid System](#5-hardpoint--grid-system)
6. [Engine Archetypes](#6-engine-archetypes)
7. [Ship Class Reference](#7-ship-class-reference)
8. [Turret System](#8-turret-system)
9. [Hull Skinning Pipeline](#9-hull-skinning-pipeline)
10. [ECS Component Mapping](#10-ecs-component-mapping)
11. [ECS Systems](#11-ecs-systems)
12. [Damage Propagation & Salvage](#12-damage-propagation--salvage)
13. [Material & Color System](#13-material--color-system)
14. [Station System](#14-station-system)
15. [Interior Standards](#15-interior-standards)
16. [Export Pipeline](#16-export-pipeline)
17. [Hull Influence Weights](#17-hull-influence-weights)
18. [Data File Conventions](#18-data-file-conventions)

---

## 1. Overview & Purpose

The BlenderSpaceshipGenerator produces game-ready 3D assets for the
EVEOFFLINE project.  It is both a Blender add-on (procedural generation at
author time) and a specification for the Atlas game engine (runtime systems).
The key principle is:

> **Bricks are authoritative; meshes are derived.**

Ships are assemblies of typed bricks placed on a 3D grid.  The visual hull
mesh is computed from those bricks — never hand-authored.  This means the
same data drives:

- Procedural NPC ship generation
- Player ship construction (build mode)
- Combat damage and structural collapse
- Salvage and recovery

The Atlas engine must be able to:

1. Load a Ship DNA JSON file.
2. Reconstruct the brick layout on a grid.
3. Generate (or load a pre-baked) hull mesh from control volumes.
4. Run ECS systems for power, damage, and structural integrity.

---

## 2. Coordinate System & Units

### Blender Authoring Space

| Axis | Direction    | Semantic                   |
|------|-------------|----------------------------|
| X    | Left / Right | Port (+X) / Starboard (−X) |
| Y    | Front / Back | Fore (+Y) / Aft (−Y)       |
| Z    | Up / Down    | Dorsal (+Z) / Ventral (−Z) |

- **1 Blender unit ≈ 1 metre.**
- Ship front faces **+Y**.  Engines face **−Y**.
- Top of the ship is **+Z**.

### Atlas Engine Space (OBJ Export)

The OBJ exporter transforms to Atlas conventions:

| Export Setting   | Value         |
|-----------------|---------------|
| `forward_axis`  | `NEGATIVE_Z`  |
| `up_axis`       | `Y`           |

After export the ship nose points along **−Z** in engine space, up is **+Y**.
All engine code should assume this convention for camera, physics, and
rendering.

### Unit Scale Reference

All dimensions in this document are in **metres** unless otherwise noted.

---

## 3. Ship DNA Schema

Ship DNA is the canonical serialisation of a ship.  It is produced by
`brick_system.generate_ship_dna()` and stored as a JSON custom property on
the hull object.

### Full JSON Schema

```jsonc
{
  // Random seed used for procedural generation (integer).
  "seed": 917221,

  // Ship class identifier — must be one of the SHIP_CONFIGS keys.
  "class": "CRUISER",

  // Visual style applied during generation.
  // One of: MIXED, X4, ELITE, EVE, SOLARI, VEYREN, AURELIAN, KELDARI, NMS
  "style": "SOLARI",

  // Naming prefix applied to all Blender objects during generation.
  "naming_prefix": "EVEOFFLINE",

  // Grid size in metres used for brick snapping (derived from ship class).
  "grid_size": 2.0,

  // Ordered list of every placed brick.
  "bricks": [
    {
      // Brick type — must be a key in BRICK_TYPES (see §4).
      "type": "STRUCTURAL_SPINE",

      // Grid-aligned position [x, y, z] in metres.
      "pos": [0, 0, 0],

      // (optional) Engine archetype when type is ENGINE_BLOCK.
      "archetype": "MAIN_THRUST"
    }
  ]
}
```

### Round-trip Guarantee

```python
dna = brick_system.generate_ship_dna(ship_class, seed, bricks, style, prefix)
json_str = brick_system.ship_dna_to_json(dna)
loaded = brick_system.ship_dna_from_json(json_str)
assert loaded == dna
```

The engine must be able to parse this JSON and reconstruct the full brick
layout.

---

## 4. Brick Taxonomy

18 brick types across 5 categories.

### Categories

| Category   | Brick Types |
|-----------|-------------|
| **CORE**     | REACTOR_CORE, POWER_BUS, STRUCTURAL_SPINE |
| **HULL**     | HULL_PLATE, HULL_WEDGE, HULL_CORNER, ARMOR_BLOCK |
| **FUNCTION** | ENGINE_BLOCK, THRUSTER, CAPACITOR, SHIELD_EMITTER |
| **UTILITY**  | HARDPOINT_MOUNT, DOCKING_CLAMP, SENSOR_MAST, ANTENNA_DISH |
| **DETAIL**   | PANEL, VENT, PIPE |

### Brick Properties

Every brick type carries these attributes:

| Field        | Type        | Description |
|-------------|-------------|-------------|
| `category`   | string      | One of CORE, HULL, FUNCTION, UTILITY, DETAIL |
| `size`       | int[3]      | Grid cells occupied (x, y, z) |
| `shape`      | string      | Proxy geometry — box, cylinder, sphere, cone, wedge |
| `scale_band` | string      | primary (1.0×), structural (0.7×), detail (0.22×) |
| `hardpoints` | list        | Attachment points (see §5) |

### Complete Brick Table

| Brick              | Category | Size    | Shape    | Scale Band  | Hardpoint Roles |
|--------------------|----------|---------|----------|-------------|-----------------|
| REACTOR_CORE       | CORE     | 2×2×2   | box      | primary     | power ×4        |
| POWER_BUS          | CORE     | 1×3×1   | box      | structural  | power ×2        |
| STRUCTURAL_SPINE   | CORE     | 1×4×1   | box      | primary     | attach ×6       |
| HULL_PLATE         | HULL     | 2×2×1   | box      | structural  | attach ×1       |
| HULL_WEDGE         | HULL     | 2×2×1   | wedge    | structural  | attach ×1       |
| HULL_CORNER        | HULL     | 1×1×1   | box      | structural  | attach ×1       |
| ARMOR_BLOCK        | HULL     | 2×2×2   | box      | structural  | attach ×1       |
| ENGINE_BLOCK       | FUNCTION | 2×3×2   | cylinder | primary     | exhaust, power  |
| THRUSTER           | FUNCTION | 1×1×1   | cylinder | detail      | exhaust ×1      |
| CAPACITOR          | FUNCTION | 1×2×1   | cylinder | structural  | power ×2        |
| SHIELD_EMITTER     | FUNCTION | 1×1×1   | sphere   | detail      | power ×1        |
| HARDPOINT_MOUNT    | UTILITY  | 1×1×1   | cylinder | detail      | weapon, attach  |
| DOCKING_CLAMP      | UTILITY  | 2×1×1   | box      | structural  | dock, attach    |
| SENSOR_MAST        | UTILITY  | 1×2×1   | cone     | detail      | attach ×1       |
| ANTENNA_DISH       | UTILITY  | 1×1×1   | cone     | detail      | attach ×1       |
| PANEL              | DETAIL   | 1×1×1   | box      | detail      | attach ×1       |
| VENT               | DETAIL   | 1×1×1   | box      | detail      | attach ×1       |
| PIPE               | DETAIL   | 1×2×1   | cylinder | detail      | attach ×2       |

### Scale Band Multipliers

| Band        | Multiplier | Usage |
|-------------|-----------|-------|
| `primary`   | 1.0       | Core hull, reactor, main engines |
| `structural`| 0.7       | Hull plates, armor, capacitors |
| `detail`    | 0.22      | Vents, panels, antennas |

The engine should scale visual brick size by
`grid_size × brick.size × scale_band_factor`.

---

## 5. Hardpoint & Grid System

### Grid Sizes (per ship class)

| Ship Class     | Grid Size (m) |
|---------------|---------------|
| SHUTTLE        | 0.5           |
| FIGHTER        | 1.0           |
| CORVETTE       | 1.0           |
| FRIGATE        | 1.0           |
| DESTROYER      | 1.5           |
| CRUISER        | 2.0           |
| BATTLECRUISER  | 2.0           |
| BATTLESHIP     | 3.0           |
| CARRIER        | 4.0           |
| DREADNOUGHT    | 4.0           |
| CAPITAL        | 6.0           |
| TITAN          | 8.0           |
| INDUSTRIAL     | 1.5           |
| MINING_BARGE   | 1.0           |
| EXHUMER        | 1.5           |
| EXPLORER       | 1.0           |
| HAULER         | 1.5           |
| EXOTIC         | 1.0           |

### Snap Validation

```python
def snap_to_grid(position, grid_size):
    return tuple(round(v / grid_size) * grid_size for v in position)

def validate_snap(brick_type_name, position, grid_size):
    snapped = snap_to_grid(position, grid_size)
    return snapped == tuple(round(v, 6) for v in position)
```

The engine must enforce grid alignment when players place bricks.

### Hardpoint Matching Rules

Each brick declares hardpoints with a `role` and `direction` vector.  Two
bricks may connect when:

1. They are adjacent on the grid.
2. Their facing hardpoint roles are compatible (e.g. `power↔power`,
   `attach↔attach`, `weapon↔attach`).
3. Their direction vectors are opposing (dot product ≈ −1).

If a brick has no matching hardpoint on a neighbouring cell, the placement
is invalid.

---

## 6. Engine Archetypes

Engines are not uniform — they are assigned an archetype based on their
index within the ship.  The first ~60 % are main thrust, ~25 % maneuvering,
and the rest utility exhaust.

| Archetype        | Depth Range | Radius Factor | Nozzle Flare | Glow Strength | Description |
|-----------------|-------------|---------------|-------------|---------------|-------------|
| MAIN_THRUST      | 1.0 – 1.4   | 1.0           | Yes          | 5.0           | Big, recessed primary engines |
| MANEUVERING      | 0.3 – 0.6   | 0.4           | No           | 2.0           | Small angled thrusters |
| UTILITY_EXHAUST  | 0.2 – 0.4   | 0.6           | No           | 1.0           | Flat vents for aux systems |

### Selection Algorithm

```python
def select_engine_archetype(index, total_engines):
    ratio = index / max(total_engines, 1)
    if ratio < 0.6:
        return 'MAIN_THRUST'
    elif ratio < 0.85:
        return 'MANEUVERING'
    return 'UTILITY_EXHAUST'
```

The engine should use the `archetype` field from Ship DNA to set glow
colour, particle effects, and thrust contribution.

---

## 7. Ship Class Reference

| Class          | Scale | Hull Segments | Engines | Weapons | Turrets | Wings | Crew |
|---------------|-------|--------------|---------|---------|---------|-------|------|
| SHUTTLE        | 1.0   | 3            | 2       | 0       | 0       | No    | 2    |
| FIGHTER        | 1.5   | 4            | 2       | 2       | 1       | Yes   | 1    |
| CORVETTE       | 3.0   | 5            | 3       | 4       | 2       | Yes   | 4    |
| FRIGATE        | 5.0   | 6            | 4       | 6       | 3       | No    | 10   |
| DESTROYER      | 8.0   | 7            | 4       | 8       | 4       | No    | 25   |
| CRUISER        | 12.0  | 8            | 6       | 12      | 6       | No    | 50   |
| BATTLECRUISER  | 15.0  | 9            | 6       | 14      | 7       | No    | 75   |
| BATTLESHIP     | 18.0  | 10           | 8       | 16      | 8       | No    | 100  |
| CARRIER        | 25.0  | 12           | 10      | 10      | 6       | No    | 200  |
| DREADNOUGHT    | 30.0  | 14           | 5       | 18      | 10      | No    | 400  |
| CAPITAL        | 35.0  | 15           | 12      | 20      | 10      | No    | 500  |
| TITAN          | 50.0  | 18           | 10      | 24      | 10      | No    | 1000 |
| INDUSTRIAL     | 6.0   | 5            | 3       | 1       | 0       | No    | 5    |
| MINING_BARGE   | 4.0   | 4            | 2       | 0       | 0       | No    | 3    |
| EXHUMER        | 5.0   | 5            | 3       | 0       | 0       | No    | 4    |
| EXPLORER       | 2.0   | 5            | 2       | 1       | 1       | Yes   | 1    |
| HAULER         | 5.5   | 6            | 4       | 0       | 0       | No    | 2    |
| EXOTIC         | 2.5   | 7            | 2       | 2       | 1       | Yes   | 1    |

### Generation Pipeline (9 Stages)

```
1. Core hull (spine)
2. Cockpit / bridge
3. Major structures (wings)
4. Engines (archetype-varied)
5. Weapons & turrets
6. Detail modules
7. Interior (optional)
8. Hull taper / deform pass
9. Bevel + auto-smooth cleanup pass
```

---

## 8. Turret System

Each turret hardpoint is a Blender object with custom properties that the
engine must read.

### Custom Properties (per turret object)

| Property          | Type   | Example              | Description |
|------------------|--------|---------------------|-------------|
| `turret_index`    | int    | `1`                  | 1-based position index |
| `turret_type`     | string | `"projectile"`       | Weapon type |
| `tracking_speed`  | float  | `30.0`               | Rotation speed (deg/s) |
| `rotation_limits` | string | `"yaw:360,pitch:90"` | Rotation constraints |
| `hardpoint_size`  | float  | `1.44`               | Visual size of turret |

### Visual Geometry

Each turret consists of three child meshes:

1. **Base** — flat cylinder at the mounting point
2. **Rotation ring** — torus around the base
3. **Barrel** — cylinder extending forward from the ring

Maximum turrets per ship: **10**.

### Engine Parsing

```cpp
// Pseudocode for reading turret data from OBJ custom properties
for (auto& obj : ship.children) {
    if (obj.hasProperty("turret_index")) {
        Turret t;
        t.index        = obj.getInt("turret_index");
        t.type         = obj.getString("turret_type");
        t.trackingSpeed = obj.getFloat("tracking_speed");
        t.size         = obj.getFloat("hardpoint_size");
        parseRotationLimits(obj.getString("rotation_limits"), t);
        turrets.push_back(t);
    }
}
```

---

## 9. Hull Skinning Pipeline

The advanced hull system uses control volumes + signed distance fields to
produce organic hulls.  This replaces the manual mesh approach for
next-generation ships.

### Pipeline Overview

```
[Brick Placement]
       ↓
[Control Volumes]  (simple proxy meshes — boxes, capsules, spheres)
       ↓
[SDF / Volume Field]
       ↓
[Volume to Mesh]
       ↓
[Smooth + Set Shade Smooth]
       ↓
[Baked Game-Ready Mesh]
```

### Control Volume Attributes

Each control volume carries:

| Attribute     | Type   | Range       | Description |
|--------------|--------|-------------|-------------|
| `hull_weight` | float  | 0.2 – 5.0  | Hull thickness influence |
| `mode`        | string | ADD / SUBTRACT | Union or cavity |

### Geometry Nodes Graph (`GN_ShipHullSkinner`)

Group inputs:

| Input                | Type       | Notes |
|---------------------|------------|-------|
| Control Collection   | Collection | `Ship_ControlVolumes` |
| Voxel Size           | Float      | Ship-class dependent (0.3 – 1.0) |
| Hull Threshold       | Float      | Hull thickness (0.1 – 0.3) |
| Smooth Iterations    | Int        | Post-smoothing passes |

Node flow:

```
Object Info → Realize Instances
  → Separate Geometry (ADD path / SUBTRACT path)
  → Mesh to Volume (density = hull_weight)
  → Volume Combine (union ADD, difference SUBTRACT)
  → Volume to Mesh (threshold, adaptivity=0.3)
  → Smooth
  → Set Shade Smooth
  → Group Output
```

### Baking for Runtime

```python
def bake_hull(obj):
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.modifier_apply(modifier="GeometryNodes")
    bpy.ops.object.shade_smooth()
```

For LODs: duplicate and decimate at 0.5× / 0.25× ratios.

### Engine Implementation

At runtime the Atlas engine should:

1. Load pre-baked OBJ hull meshes for static ships.
2. For player-built ships, either re-run SDF meshing on the GPU or use a
   pre-computed lookup.
3. When a brick is destroyed, remove its control volume and re-mesh the
   affected region.

---

## 10. ECS Component Mapping

These C++ structs map 1:1 to the Blender brick/ship data.

### BrickComponent

```cpp
struct BrickComponent {
    BrickType type;       // enum from BRICK_TYPES
    Vec3      gridPos;    // grid-aligned position
    Quat      rotation;   // orientation on grid
    float     hullWeight; // influence on hull skin
    int       hp;         // hit points
    float     mass;       // mass in kg
};
```

### PowerComponent

```cpp
struct PowerComponent {
    float generation;   // MW produced (reactors)
    float consumption;  // MW consumed (engines, shields, weapons)
};
```

### HullInfluenceComponent

```cpp
struct HullInfluenceComponent {
    float weight;       // hull skin thickness contribution
    bool  subtractive;  // true = carves cavity (hangars, exhausts)
};
```

### ConnectionComponent

```cpp
struct ConnectionComponent {
    Entity              parent;
    std::vector<Entity> children;
    bool                connectedToSpine; // structural integrity flag
};
```

### TurretComponent

```cpp
struct TurretComponent {
    int         index;
    std::string weaponType;    // "projectile", "beam", "missile"
    float       trackingSpeed; // degrees per second
    float       yawLimit;      // degrees (360 = full rotation)
    float       pitchLimit;    // degrees
    float       size;
};
```

---

## 11. ECS Systems

### BrickPlacementSystem

Validates and places bricks on the grid.

```
for each placement request:
    snap position to grid
    check grid cell is empty
    validate hardpoint compatibility with neighbours
    if valid:
        create BrickComponent entity
        create HullInfluenceComponent
        create ConnectionComponent (link to parent)
        if brick.type has power:
            create PowerComponent
        trigger HullRebuildSystem
```

### PowerFlowSystem

EVE-style capacitor logic.

```
for each entity with PowerComponent:
    total_generation += entity.power.generation
    total_consumption += entity.power.consumption

ship.capacitor += (total_generation - total_consumption) * dt

if ship.capacitor <= 0:
    disable non-essential systems (shields, weapons)
```

### HullRebuildSystem

Triggered when bricks are added or removed.

```
collect all HullInfluenceComponent entities
rebuild control volume set
re-run SDF meshing (or mark region dirty for async rebuild)
update collision mesh
```

### DamagePropagationSystem

```
for each brick that took damage this frame:
    brick.hp -= incoming_damage
    if brick.hp <= 0:
        destroy brick entity
        remove control volume → hull re-skins
        check neighbours for structural integrity:
            for each neighbour:
                if not connected_to_spine(neighbour):
                    mark as unsupported
                    begin detachment cascade
```

### SalvageSystem

```
when brick entity is destroyed:
    roll loot table based on brick.type
    spawn salvage entity at brick world position
    salvage.type = brick.type
    salvage.condition = random(0.1, 0.8)
```

---

## 12. Damage Propagation & Salvage

### Damage Rules

1. Brick HP reaches 0 → brick entity destroyed.
2. Control volume removed → hull auto-reskins (visible hull collapse).
3. Neighbour bricks re-evaluated for structural integrity.
4. If a brick is disconnected from the spine → power loss → eventual
   break-off.

### Structural Integrity

Each brick tracks:

- **Parent connection** — which spine segment it connects through
- **Load count** — how many bricks depend on it structurally

When a structural brick is destroyed:

1. All children lose their connection.
2. Power loss cascades through disconnected section.
3. Hull thins and darkens in affected area.
4. After integrity timeout, section detaches as debris.

### Visual Feedback

| State              | Visual Effect          |
|-------------------|------------------------|
| Power loss         | Hull darkens            |
| Structural stress  | Cracks / glow lines    |
| Hull thinning      | Visible dents           |
| Detachment         | Chunk separation + debris |

### Salvage

When a ship is destroyed:

- Bricks pop off as individual entities
- Power loss cascades from spine outward
- Hull plates fly free with physics
- Functional bricks (reactor, engine, weapons) can be recovered
- Recovered bricks retain type and partial HP

---

## 13. Material & Color System

### Faction Color Palettes

Each style has primary, secondary, and accent RGBA colours:

| Style     | Primary            | Secondary          | Accent             |
|----------|--------------------|--------------------|---------------------|
| MIXED     | (0.4, 0.4, 0.45)  | (0.3, 0.3, 0.35)  | (0.2, 0.5, 1.0)    |
| X4        | (0.35, 0.38, 0.4) | (0.25, 0.28, 0.3) | (0.9, 0.6, 0.1)    |
| ELITE     | (0.5, 0.5, 0.52)  | (0.3, 0.3, 0.32)  | (0.1, 0.7, 0.9)    |
| EVE       | (0.45, 0.42, 0.4) | (0.35, 0.32, 0.3) | (0.6, 0.2, 0.2)    |
| SOLARI    | (0.8, 0.65, 0.2)  | (0.6, 0.45, 0.15) | (1.0, 0.9, 0.5)    |
| VEYREN    | (0.4, 0.45, 0.5)  | (0.3, 0.35, 0.4)  | (0.3, 0.5, 0.9)    |
| AURELIAN  | (0.2, 0.45, 0.35) | (0.15, 0.35, 0.25)| (0.0, 0.8, 0.7)    |
| KELDARI   | (0.45, 0.35, 0.25)| (0.35, 0.25, 0.15)| (0.9, 0.5, 0.1)    |
| NMS       | Per-seed random    | Per-seed random    | Per-seed random     |

### PBR Properties

| Part          | Metallic | Roughness | Emission Strength |
|--------------|----------|-----------|-------------------|
| Hull          | 0.7      | 0.4       | 0.0               |
| Accent        | 0.9      | 0.2       | 0.5               |
| Engine glow   | —        | —         | 5.0               |
| Reactor glow  | —        | —         | 3.0               |
| Shield emitter| —        | —         | 2.0               |
| Turret        | 0.9      | 0.3       | 0.0               |

### Weathering

The weathering parameter (0.0 – 1.0) adds noise-based dirt and wear:

- Darker roughness in weathered areas
- Colour ramp mask (0.4 – 0.6 threshold)
- Scales with `amount × 0.4` added to base roughness

---

## 14. Station System

### Station Types

| Type        | Base Scale | Sections | Docking Bays | Hangars |
|------------|-----------|----------|-------------|---------|
| INDUSTRIAL  | 80        | 4        | 2           | Yes     |
| MILITARY    | 100       | 5        | 3           | Yes     |
| COMMERCIAL  | 90        | 4        | 4           | No      |
| RESEARCH    | 60        | 3        | 1           | No      |
| MINING      | 70        | 3        | 2           | Yes     |
| ASTRAHUS    | 50        | 3        | 2           | No      |
| FORTIZAR    | 100       | 5        | 4           | Yes     |
| KEEPSTAR    | 200       | 8        | 8           | Yes     |

### Faction Architecture

| Faction   | Symmetry   | Spires | Domes | Visual Style |
|----------|-----------|--------|-------|-------------|
| SOLARI    | Radial     | Yes    | Yes   | Golden cathedral |
| VEYREN    | Cubic      | No     | No    | Industrial blocks |
| AURELIAN  | Organic    | No     | Yes   | Organic domes |
| KELDARI   | Asymmetric | No     | No    | Rusted patchwork |

### Station Components

Each station is composed of:

1. **Central hub** — shape determined by faction symmetry
2. **Radiating sections** — arms extending from the hub
3. **Docking bays** — with emissive guide strips
4. **Faction features** — spires (Solari), domes (Solari/Aurelian), hangars

---

## 15. Interior Standards

All ship interiors are built to human scale for first-person exploration.

### Dimensions

| Measurement      | Value (m) |
|-----------------|-----------|
| Human height     | 1.8       |
| Door height      | 2.0       |
| Door width       | 1.0       |
| Corridor width   | 1.5       |
| Corridor height  | 2.5       |
| Room height      | 3.0       |

### Room Types by Ship Class

| Ship Class           | Cockpit | Bridge | Corridors | Quarters | Cargo Bay | Engine Room |
|---------------------|---------|--------|-----------|----------|-----------|-------------|
| Shuttle, Fighter     | ✓       |        |           |          |           |             |
| Corvette, Frigate    | ✓       |        | ✓         | ✓        |           |             |
| Destroyer+           |         | ✓      | ✓ (network)| ✓       | ✓         | ✓           |

### Engine Implementation Notes

- Interior meshes are separate objects parented to the hull.
- Corridors connect major areas; side branches added when crew > 20.
- The engine should use interior meshes as navigation mesh sources for NPC
  pathfinding.
- Door positions can be used as AI waypoints and loading zone triggers.

---

## 16. Export Pipeline

### OBJ Export Settings

```python
bpy.ops.wm.obj_export(
    filepath=filepath,
    export_selected_objects=True,
    apply_modifiers=True,
    export_uv=True,
    export_normals=True,
    export_materials=True,
    forward_axis='NEGATIVE_Z',
    up_axis='Y',
)
```

### Output Files

Each export produces:

- `{name}.obj` — geometry
- `{name}.mtl` — materials

### Naming Convention

The OBJ file name must match the ship `id` from EVEOFFLINE JSON for the
Atlas engine to find it:

```
EVEOFFLINE/data/ships/obj_models/{ship_id}.obj
EVEOFFLINE/data/ships/obj_models/{ship_id}.mtl
```

### EVEOFFLINE JSON → Generator Mapping

| JSON Field                          | Generator Param   |
|------------------------------------|-------------------|
| `class` (Frigate, Titan, etc.)      | `ship_class`      |
| `race` (Solari, Veyren, etc.)       | `style`           |
| `model_data.generation_seed`        | `seed`            |
| `model_data.turret_hardpoints`      | turret count      |
| `model_data.engine_count`           | engine count      |
| `high_slots + mid_slots + low_slots`| `module_slots` (÷3, max 10) |

---

## 17. Hull Influence Weights

When using the SDF hull skinning pipeline, each brick type contributes a
hull weight that determines how much the hull skin bulges or recedes around
it.

| Brick Type     | Hull Weight | Mode     | Notes |
|---------------|-------------|----------|-------|
| Reactor        | 3.0         | ADD      | Thick core volume |
| Armor Block    | 2.0         | ADD      | Local hull bulge |
| Engine Block   | 1.5         | ADD      | Exhaust shaping |
| Hull Plate     | 1.0         | ADD      | Standard skin |
| Weapon Mount   | 0.8         | ADD      | Socket area |
| Capacitor      | 1.2         | ADD      | Internal volume |
| Utility        | 0.5         | ADD      | Minimal influence |
| Antenna/Sensor | 0.2         | ADD      | Almost no hull effect |
| Engine Exhaust | —           | SUBTRACT | Carves cavity |
| Hangar Void    | —           | SUBTRACT | Carves large opening |
| Weapon Recess  | —           | SUBTRACT | Carves socket |

### Engine Implementation

```cpp
float getHullWeight(BrickType type) {
    switch (type) {
        case REACTOR_CORE:    return 3.0f;
        case ARMOR_BLOCK:     return 2.0f;
        case ENGINE_BLOCK:    return 1.5f;
        case CAPACITOR:       return 1.2f;
        case HULL_PLATE:      return 1.0f;
        case HARDPOINT_MOUNT: return 0.8f;
        case DOCKING_CLAMP:   return 0.5f;
        case SENSOR_MAST:     return 0.3f;
        case ANTENNA_DISH:    return 0.2f;
        default:              return 0.5f;
    }
}
```

---

## 18. Data File Conventions

### EVEOFFLINE Directory Structure

```
EVEOFFLINE/
├── data/
│   └── ships/
│       ├── frigates.json
│       ├── destroyers.json
│       ├── cruisers.json
│       ├── battlecruisers.json
│       ├── battleships.json
│       ├── capitals.json
│       ├── industrials.json
│       ├── mining_barges.json
│       ├── exhumers.json
│       ├── tech2_frigates.json
│       ├── tech2_destroyers.json
│       ├── tech2_cruisers.json
│       ├── tech2_battlecruisers.json
│       ├── tech2_battleships.json
│       └── obj_models/
│           ├── {ship_id}.obj
│           └── {ship_id}.mtl
```

### Ship DNA Files

Ship DNA JSON files can be saved independently:

```
{project}/ship_dna/{ship_class}_{seed}.json
```

### Blender Object Hierarchy

```
[PREFIX_]Spaceship_CLASS_SEED (Collection)
├── [PREFIX_]Hull (Parent Object)
│   ├── [PREFIX_]Cockpit
│   ├── [PREFIX_]Engine_L1 / Engine_R1
│   ├── [PREFIX_]Wing_Left / Wing_Right
│   ├── [PREFIX_]Weapon_Hardpoint_N
│   ├── [PREFIX_]Turret_Hardpoint_N
│   │   ├── [PREFIX_]Turret_Ring_N
│   │   └── [PREFIX_]Turret_Barrel_N
│   ├── Module_N
│   │   ├── Connector
│   │   └── Type-specific parts
│   └── Interior Objects
│       ├── [PREFIX_]Bridge_Floor
│       ├── [PREFIX_]Corridor_Floor
│       ├── [PREFIX_]Quarters_Floor
│       └── ...
```

The `[PREFIX_]` is applied when `naming_prefix` is set (e.g., `EVEOFFLINE_Hull`).

---

*This document was generated from the BlenderSpaceshipGenerator codebase.
Keep it in sync when brick types, ship classes, or export conventions change.*
