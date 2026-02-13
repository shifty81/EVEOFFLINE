# Reference OBJ Ship Models

This directory contains reference 3D models used as **seed meshes** by the
`ProceduralShipGenerator`. Each uploaded OBJ is loaded, normalized, and then
procedurally modified (extrusions, greebles, engine/weapon module attachment)
to produce unique ship variants.

## Setup

Large reference models (> 1 MB) are stored as compressed archives in `testing/`
and are **not tracked in git**. Run the extraction script after cloning:

```bash
cd cpp_client
./extract_reference_models.sh
```

## Reference Model Inventory

### Full Ship Seeds (extracted from archives)

| Model | File | Size | Use |
|-------|------|------|-----|
| Intergalactic Spaceship | `Intergalactic_Spaceship-(Wavefront).obj` | 5 MB, 27K verts, 55K faces | Seed for frigates, destroyers, cruisers |
| Vulcan Dkyr Class | `Vulcan Dkyr Class/VulcanDKyrClass.obj` | 12 MB, 86K verts, 80K faces | Seed for battleships, carriers, dreadnoughts, titans |

### Modular Ship Parts (tracked in git)

Small OBJ modules with embedded **hardpoint markers** (`hp_*` objects):

#### Small Ship Modules (`modules/`)
| Module | File | Hardpoints |
|--------|------|------------|
| Core Hull | `core_s.obj` | `hp_engine_0`, `hp_weapon_0`, `hp_wing_L`, `hp_wing_R` |
| Engine | `engine_s.obj` | `hp_root` |
| Weapon | `weapon_s.obj` | `hp_root` |
| Wing | `wing_s.obj` | `hp_root` |

#### Medium/Large Ship Modules (`modules/`)
| Module | File | Hardpoints |
|--------|------|------------|
| Core Hull | `core_m.obj` | `hp_engine_0/1/2`, `hp_weapon_0`, `hp_spine` |
| Spine | `spine_m.obj` | `hp_forward`, `hp_aft`, `hp_turret_L`, `hp_turret_R` |
| Engine Block | `engine_block_m.obj` | `hp_root` |
| Turret | `turret_m.obj` | `hp_root` |
| Hangar Bay | `hangar_m.obj` | `hp_root` |

## How It Works

1. `ProceduralShipGenerator::findSeedOBJ()` searches this directory for
   an appropriate seed based on ship class:
   - Frigates/Destroyers/Cruisers → Intergalactic Spaceship
   - Battlecruisers → Intergalactic Spaceship (stretched)
   - Battleships/Carriers/Dreadnoughts/Titans → Vulcan Dkyr Class

2. The seed OBJ is parsed via `parseOBJ()` (using tinyobjloader), centred
   at origin, and normalized to ~100 units.

3. Mount points are detected from geometry extremes (rear → engines,
   top → weapons, tips → antennae).

4. Procedural modifications are applied: hull scaling, face extrusions,
   noise displacement, and bilateral symmetry enforcement.

5. Engine/weapon/antenna modules are attached at detected mount points.

6. The result is converted to an `atlas::Model` for rendering.

## Modular OBJ Hardpoint Convention

Modular OBJ parts use empty objects (single-vertex `o` entries) prefixed
with `hp_` as hardpoint markers:

- `hp_root` — Where this part attaches to a parent
- `hp_engine_N` — Engine mount position
- `hp_weapon_N` — Weapon mount position
- `hp_wing_L`/`hp_wing_R` — Wing attachment points
- `hp_spine` — Central spine connection
- `hp_forward`/`hp_aft` — Fore/aft attachment points
- `hp_turret_L`/`hp_turret_R` — Turret positions

These hardpoints are parsed during OBJ loading and used to position
child modules during ship assembly.
