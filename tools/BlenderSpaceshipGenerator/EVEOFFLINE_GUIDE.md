# EVEOFFLINE / Atlas Engine Integration Guide

Complete start-to-finish guide for using the Blender Spaceship Generator with the
[EVEOFFLINE](https://github.com/shifty81/EVEOFFLINE) project and its custom Atlas engine.

---

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Install the Addon in Blender](#install-the-addon-in-blender)
3. [Generate a Ship Manually](#generate-a-ship-manually)
4. [Import Ships from EVEOFFLINE JSON Data](#import-ships-from-eveoffline-json-data)
5. [Export OBJ Files for the Atlas Engine](#export-obj-files-for-the-atlas-engine)
6. [Place the Models in EVEOFFLINE](#place-the-models-in-eveoffline)
7. [Generate Stations](#generate-stations)
8. [Generate Asteroid Belts](#generate-asteroid-belts)
9. [Test in Engine](#test-in-engine)
10. [Batch Generation Script](#batch-generation-script)
11. [Faction Style Reference](#faction-style-reference)
12. [Troubleshooting](#troubleshooting)

---

## Prerequisites

| Requirement | Version |
|------------|---------|
| **Blender** | 2.80 or higher (3.x / 4.x recommended) |
| **EVEOFFLINE** | Latest `main` branch from [GitHub](https://github.com/shifty81/EVEOFFLINE) |
| **This addon** | Clone or download this repository |

Make sure your EVEOFFLINE project is built and the `data/ships/` directory
contains the ship JSON files (frigates.json, destroyers.json, etc.).

---

## Install the Addon in Blender

### Step 1 — Download

```bash
git clone https://github.com/shifty81/BlenderSpaceshipGenerator.git
```

Or download as a ZIP from the GitHub releases page.

### Step 2 — Install

1. Open **Blender**.
2. Go to **Edit → Preferences → Add-ons**.
3. Click **Install…** in the top-right corner.
4. Navigate to the `BlenderSpaceshipGenerator` folder (or the downloaded ZIP).
5. Click **Install Add-on**.

### Step 3 — Enable

1. In the Add-ons list, search for **"Spaceship Generator"**.
2. Check the checkbox next to it to enable.
3. The **Spaceship** tab now appears in the 3D Viewport sidebar.

### Step 4 — Verify

1. In the 3D Viewport, press **N** to open the sidebar.
2. Click the **Spaceship** tab.
3. You should see the full panel with Ship Configuration, Generation Options,
   and the **EVEOFFLINE / Atlas Integration** section at the bottom.

---

## Generate a Ship Manually

This is the simplest path — generate a ship from the UI and export it.

### Step 1 — Configure

In the **Spaceship** sidebar panel:

| Setting | Recommended for EVEOFFLINE |
|---------|---------------------------|
| **Ship Class** | Pick any (Frigate, Cruiser, Titan, etc.) |
| **Style** | Pick an EVEOFFLINE faction: **Solari**, **Veyren**, **Aurelian**, or **Keldari** |
| **Random Seed** | Use the `generation_seed` from your ship JSON, or any number |
| **Generate Interior** | Enable for FPV testing, disable for faster export |
| **Module Slots** | 2–5 for subcapitals, 5–10 for capitals |
| **Hull Complexity** | 1.0 (default) — increase for more detail |
| **Symmetry** | Enabled (recommended) |

### Step 2 — Generate

Click **Generate Spaceship**. The ship appears at the 3D cursor.

### Step 3 — Inspect

- **Orbit**: Middle-mouse-button drag
- **Zoom**: Scroll wheel
- **Walk inside** (if interior enabled): Select an interior object → press
  `Shift + F` → use WASD to move, mouse to look, `Esc` to exit

---

## Import Ships from EVEOFFLINE JSON Data

Instead of configuring each ship manually, you can import directly from
EVEOFFLINE's ship data files. The addon reads the JSON, extracts the faction,
class, seed, slot counts, and hardpoint data, and generates matching ships.

### Step 1 — Locate Your Ship JSON

Your EVEOFFLINE project structure:

```
EVEOFFLINE/
└── data/
    └── ships/
        ├── frigates.json
        ├── destroyers.json
        ├── cruisers.json
        ├── battlecruisers.json
        ├── battleships.json
        ├── capitals.json
        ├── industrials.json
        ├── mining_barges.json
        ├── exhumers.json
        ├── tech2_frigates.json
        ├── tech2_destroyers.json
        ├── tech2_cruisers.json
        ├── tech2_battlecruisers.json
        └── tech2_battleships.json
```

### Step 2 — Set the JSON Path

In the **EVEOFFLINE / Atlas Integration** section of the sidebar panel:

1. Click the folder icon next to **Ship JSON**.
2. Browse to your EVEOFFLINE project's `data/ships/` directory.
3. Select one JSON file (e.g., `frigates.json`).

### Step 3 — Import

Click **Import from EVEOFFLINE JSON**.

This will:
- Parse every ship definition in that file.
- Map the ship's `race` field to the matching faction style
  (Solari → elegant, Veyren → angular, Aurelian → organic, Keldari → rugged).
- Use the `generation_seed` from each ship's `model_data` for consistent generation.
- Use the ship's `class` field to pick the right ship class.
- Calculate module slots from high/mid/low slot totals.

Each ship is generated and placed in its own Blender collection named
`Spaceship_CLASS_SEED`.

### What Gets Mapped

| EVEOFFLINE JSON field | Generator parameter |
|-----------------------|--------------------|
| `class` (Frigate, Titan, etc.) | `ship_class` |
| `race` (Solari, Veyren, etc.) | `style` |
| `model_data.generation_seed` | `seed` |
| `model_data.turret_hardpoints` + `launcher_hardpoints` | weapon count |
| `model_data.engine_count` | engine count |
| `high_slots` + `mid_slots` + `low_slots` | `module_slots` (÷3, max 10) |

---

## Export OBJ Files for the Atlas Engine

### Step 1 — Select the Ship

In the 3D viewport or the Outliner, **click the Hull object** of the ship you
want to export. The hull is the parent object in the collection.

### Step 2 — Set Export Path

In the **EVEOFFLINE / Atlas Integration** section:

1. Click the folder icon next to **Export Path**.
2. Browse to your EVEOFFLINE project's model output directory:
   ```
   EVEOFFLINE/data/ships/obj_models/
   ```
   Or any other directory you prefer.

### Step 3 — Export

Click **Export OBJ for Atlas**.

This will:
- Select the hull and all child objects (cockpit, engines, weapons, modules, interior).
- Apply all modifiers (subdivision, edge split, cast).
- Export as `.obj` with matching `.mtl` material file.
- Use the correct axis orientation for the Atlas engine
  (forward: -Z, up: Y).

The exported file is named after the hull object, e.g., `Hull.obj`.

### Step 4 — Verify the Export

Check that the files exist:

```bash
ls -la EVEOFFLINE/data/ships/obj_models/
# Should see:
#   Hull.obj
#   Hull.mtl
```

---

## Place the Models in EVEOFFLINE

### Directory Structure

EVEOFFLINE expects ship models in:

```
EVEOFFLINE/data/ships/obj_models/
```

The existing `obj_models.7z` archive contains the base models. Place your
newly generated `.obj` and `.mtl` files alongside or extract and replace.

### Naming Convention

For the Atlas engine to find your model, name the OBJ file to match the ship
ID from the JSON. Before exporting, you can rename the hull object in Blender:

1. Select the hull in the Outliner.
2. Double-click the name and type the ship ID (e.g., `fang`, `empyrean`).
3. Export — the file will be named `fang.obj`.

Or rename the exported file manually:

```bash
mv Hull.obj fang.obj
mv Hull.mtl fang.mtl
```

---

## Test in Engine

### Step 1 — Build EVEOFFLINE

```bash
cd EVEOFFLINE
./build.sh          # Linux/macOS
# or
build.bat           # Windows
```

### Step 2 — Run the Client

```bash
cd build/bin
./eve_client "YourName"
```

### Step 3 — Verify Ship Renders

The Atlas engine will load the OBJ model referenced by each ship definition.
When you undock or view your ship in the game client, you should see the
procedurally generated geometry.

### Checklist

- [ ] OBJ file is in `data/ships/obj_models/`
- [ ] File name matches the ship `id` from the JSON
- [ ] Build completed without errors
- [ ] Ship renders correctly in the client viewport
- [ ] Turret hardpoints are visible at the correct positions
- [ ] Engine glow effect appears at the rear

---

## Batch Generation Script

To generate and export every ship in your EVEOFFLINE data at once, run this
script inside Blender's **Scripting** workspace (or paste into Blender's
Python console):

```python
import bpy
import os
import sys

# Add the addon directory to the path if needed
# sys.path.append('/path/to/BlenderSpaceshipGenerator')

from BlenderSpaceshipGenerator import ship_generator, atlas_exporter

# === CONFIGURATION ===
EVEOFFLINE_DIR = '/path/to/EVEOFFLINE'
SHIPS_DIR = os.path.join(EVEOFFLINE_DIR, 'data', 'ships')
EXPORT_DIR = os.path.join(EVEOFFLINE_DIR, 'data', 'ships', 'obj_models')
# =====================

os.makedirs(EXPORT_DIR, exist_ok=True)

# Load all ship definitions
all_ships = atlas_exporter.load_all_ships(SHIPS_DIR)
print(f"Found {len(all_ships)} ship definitions")

for ship_id, ship_data in all_ships.items():
    print(f"Generating {ship_id} ({ship_data.get('name', '')})...")

    # Clear scene
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete()

    # Parse EVEOFFLINE config
    config = atlas_exporter.parse_ship_config(ship_data)

    # Generate the ship
    hull = ship_generator.generate_spaceship(
        ship_class=config['ship_class'],
        seed=config['seed'],
        generate_interior=False,       # disable interior for game models
        module_slots=config['module_slots'],
        hull_complexity=1.0,
        symmetry=config['symmetry'],
        style=config['style'],
    )

    # Rename hull to ship ID for consistent file naming
    hull.name = ship_id

    # Select hull and children
    bpy.ops.object.select_all(action='DESELECT')
    hull.select_set(True)
    for child in hull.children_recursive:
        child.select_set(True)

    # Export
    filepath = os.path.join(EXPORT_DIR, f"{ship_id}.obj")
    atlas_exporter.export_obj(filepath)
    print(f"  Exported → {filepath}")

print("Done! All ships exported.")
```

### Running the Script

**Option A — From Blender GUI**:

1. Open Blender.
2. Switch to the **Scripting** workspace (tab at the top).
3. Click **New** to create a new text block.
4. Paste the script above.
5. Edit `EVEOFFLINE_DIR` to point to your EVEOFFLINE clone.
6. Click **Run Script** (▶ button).

**Option B — From the command line** (headless):

```bash
blender --background --python batch_generate.py
```

Save the script as `batch_generate.py` and run it. Blender runs headless
(no window) and exports all models.

---

## Faction Style Reference

Each EVEOFFLINE faction has a distinct visual style applied to the hull
geometry:

| Faction | Style | Visual Character | Tank Focus |
|---------|-------|-----------------|------------|
| **Solari** | Golden, elegant | Smooth bevels, gentle curves | Armor |
| **Veyren** | Angular, utilitarian | Sharp bevels, hard edges | Shield |
| **Aurelian** | Sleek, organic | Organic subdivision, spherical cast | Armor/Drones |
| **Keldari** | Rugged, industrial | Heavy bevels, blocky construction | Balanced |

The style is automatically selected when importing from EVEOFFLINE JSON based
on the ship's `race` field.

You can also manually select these styles from the **Style** dropdown in the
Blender panel.

---

## Generate Stations

The addon can generate procedural space stations matching EVEOFFLINE's station
types and faction architectures.

### Station Types

| Type | Description | Scale |
|------|-------------|-------|
| **Industrial** | Manufacturing and production facilities | 80 units |
| **Military** | Naval and defense installations | 100 units |
| **Commercial** | Trade and commerce hubs | 90 units |
| **Research** | Scientific research facilities | 60 units |
| **Mining** | Ore refinement and mining support | 70 units |
| **Astrahus** | Medium Upwell citadel | 50 units |
| **Fortizar** | Large Upwell citadel | 100 units |
| **Keepstar** | Extra-large Upwell citadel | 200 units |

### Faction Architecture

Each faction produces a distinct station visual:

| Faction | Architecture | Features |
|---------|-------------|----------|
| **Solari** | Golden cathedral | Spires, domes, radial symmetry |
| **Veyren** | Industrial blocks | Angular, cubic symmetry, minimal |
| **Aurelian** | Organic domes | Spherical, organic curves |
| **Keldari** | Rusted patchwork | Asymmetric, scaffolding, rough |

### How to Generate

1. In the **Spaceship** sidebar panel, scroll to **Station Generation**.
2. Select the **Station Type** (Industrial, Military, Keepstar, etc.).
3. Select the **Station Faction** (Solari, Veyren, Aurelian, Keldari).
4. Set the **Random Seed** (same seed = same station).
5. Click **Generate Station**.

The station appears at the 3D cursor, organized in its own collection.

### Export for EVEOFFLINE

Select the station hub object, set the export path, and click
**Export OBJ for Atlas** — the same workflow as ships.

---

## Generate Asteroid Belts

The addon generates asteroid belts with all 16 EVEOFFLINE ore types and
multiple belt configurations.

### Ore Types

All ore types from `data/universe/asteroid_visual_data.json` are supported:

| Ore | Color | Security Level |
|-----|-------|---------------|
| Dustite | Brown-orange | Highsec |
| Ferrite | Gray metallic | Highsec |
| Ignaite | Red-brown | Highsec |
| Crystite | Green crystalline | Highsec |
| Shadite | Golden-brown | Highsec/Lowsec |
| Corite | Blue-cyan | Highsec/Lowsec |
| Lumine | Dark red | Lowsec |
| Sangite | Bright red | Lowsec |
| Glacite | Golden | Lowsec |
| Densite | Light gray | Lowsec/Nullsec |
| Voidite | Dark brown | Nullsec |
| Spodumain | Silvery | Nullsec |
| Pyranite | Purple | Nullsec |
| Stellite | Green luminescent | Nullsec |
| Cosmite | Orange-gold | Nullsec |
| Nexorite | Cyan radioactive | Nullsec |

### Belt Layouts

| Layout | Shape | Default Count |
|--------|-------|--------------|
| **Semicircle** | Standard semicircular arc | 30 |
| **Sphere** | Spherical distribution | 40 |
| **Cluster** | Dense anomaly cluster | 50 |
| **Ring** | Sparse outer ring | 25 |

### How to Generate

1. In the **Spaceship** sidebar panel, scroll to **Asteroid Belt Generation**.
2. Select the **Belt Layout** (Semicircle, Sphere, Cluster, Ring).
3. Select the **Primary Ore** type.
4. Set the **Asteroid Count** (5–200).
5. Set the **Random Seed**.
6. Click **Generate Asteroid Belt**.

Each asteroid gets:
- Procedural irregular shape (icosphere + displacement)
- Random rotation and size variation
- PBR material matching the ore's color, roughness, and metallic values

### Export for EVEOFFLINE

Select the belt root empty, set the export path, and click
**Export OBJ for Atlas**.

---

## Troubleshooting

### "Select a valid EVEOFFLINE ship JSON file"

The **Ship JSON** path must point to a `.json` file. Use the folder icon
to browse to the file. Make sure the path ends with `.json`.

### "File not found"

Check that the path is correct. Blender uses `//` for relative paths.
Use an absolute path to be safe.

### Ship looks wrong compared to EVEOFFLINE data

The generator creates procedural geometry — it won't exactly replicate a
hand-modeled ship. The purpose is to produce a **consistent, seed-based
starting point** that matches the ship class, faction style, and hardpoint
count.

### OBJ export fails

- Make sure you have an **active object** selected (click a ship hull).
- Make sure the **Export Path** is set and the directory exists (or is
  writable so the addon can create it).
- If using Blender < 3.x, the built-in OBJ exporter API may differ. Use
  Blender 3.x+ for best results.

### Models don't load in the Atlas engine

- Verify the `.obj` file name matches the ship `id` from the JSON exactly.
- Verify the file is in `data/ships/obj_models/`.
- Check the Atlas engine console output for model loading errors.
- Make sure the `.mtl` file is in the same directory as the `.obj`.

### Addon doesn't appear in Blender

- Check that all Python files are in the same directory
  (`__init__.py`, `ship_generator.py`, `ship_parts.py`,
  `interior_generator.py`, `module_system.py`, `atlas_exporter.py`).
- Check the Blender System Console for import errors
  (Window → Toggle System Console on Windows, or launch Blender from a
  terminal on Linux/macOS).
- Verify Blender version is 2.80+.
