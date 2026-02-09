# OBJ Model Analysis - Reference Ship Geometry Research

## Overview

This document presents the findings from analyzing **311 OBJ ship models** extracted from
`data/ships/obj_models/obj_models.7z`. The analysis measured geometric traits—bounding box
proportions, aspect ratios, vertex complexity, and bilateral symmetry—to inform and refine
the procedural ship generation system.

The goal is to use these measured characteristics as **reference constraints** so that
procedurally generated ships share realistic proportions and faction-distinctive silhouettes
with the reference models, while still allowing controlled variation.

## Factions Analyzed

15 factions represented across 311 models:
Amarr (58), Angel (13), Caldari (61), Concord (8), Gallente (56), Jove (3),
Minmatar (56), Mordu (3), ORE (13), Sansha (9), Sleeper (7), SOCT (4),
SOE (3), Triglavian (14), Upwell (3).

## Ship Classes Analyzed

16 classes: Barge, Battlecruiser, Battleship, Capital, Capsule, Carrier, Cruiser,
Destroyer, Dreadnought, Fighter, Force Auxiliary, Freighter, Frigate, Industrial,
Shuttle, Titan.

---

## Per-Faction Geometric Summary

| Faction      | Ships | Avg Aspect L:W | Aspect Range L:W    | Avg Aspect L:H | Avg Vertices | Key Trait                      |
|:-------------|------:|:--------------:|:-------------------:|:--------------:|:------------:|:-------------------------------|
| Amarr        |    58 | 2.199          | 1.020 – 4.573       | 3.954          |  8,955       | Tall vertical spires, ornate   |
| Angel        |    13 | 2.338          | 1.156 – 5.755       | 3.808          |  7,281       | Aggressive, modified hulls     |
| Caldari      |    61 | 1.834          | 1.003 – 5.094       | 3.515          | 10,760       | Compact blocky, angular        |
| Concord      |     8 | 1.622          | 1.104 – 2.750       | 2.853          |  8,905       | Balanced, authoritative        |
| Gallente     |    56 | 2.278          | 1.035 – 10.442      | 3.664          |  7,327       | Wide organic curves            |
| Jove         |     3 | 1.612          | 1.077 – 2.214       | 2.762          |  5,072       | Alien, compact                 |
| Minmatar     |    56 | 2.284          | 1.010 – 5.755       | 4.631          | 12,305       | High L:H, tall industrial      |
| Mordu        |     3 | 1.672          | 1.421 – 2.026       | 6.672          |  9,367       | Extremely flat, elongated      |
| ORE          |    13 | 2.305          | 1.701 – 3.579       | 4.318          | 19,656       | Complex utility geometry       |
| Sansha       |     9 | 2.069          | 1.037 – 3.885       | 4.272          |  6,664       | Insectoid, spiky               |
| Sleeper      |     7 | 2.452          | 1.355 – 3.536       | 3.524          |  7,020       | Ancient, geometric             |
| SOCT         |     4 | 1.560          | 1.007 – 2.571       | 3.366          |  8,313       | Hybrid Jove derivatives        |
| SOE          |     3 | 1.664          | 1.243 – 2.017       | 2.057          | 10,047       | Rounded, exploration           |
| Triglavian   |    14 | 2.243          | 1.085 – 4.212       | 3.186          | 12,227       | Triangular, crystalline        |
| Upwell       |     3 | 1.274          | 1.239 – 1.311       | 1.768          | 13,834       | Near-cubic, station-like       |

### Key Faction Observations

- **Caldari**: Lowest avg L:W ratio (1.834) — ships are relatively compact/boxy. Moderate
  vertex counts with angular surfaces.
- **Minmatar**: Highest avg L:H ratio (4.631) — very flat/tall proportions with high vertex
  counts (12,305 avg), reflecting complex exposed-framework surface detail.
- **Gallente**: Widest aspect ratio range (1.035 – 10.442), reflecting diverse hull shapes
  from spherical (Dominix) to elongated (Thorax). Lowest average vertex count among major
  factions, suggesting smoother surfaces.
- **Amarr**: Moderate aspect ratios with high L:H (3.954), reflecting tall vertical spires
  extending above the hull.
- **ORE**: Highest average vertex count (19,656) — heavy surface greeble/detail reflecting
  industrial mining equipment.
- **Triglavian**: High vertex counts (12,227) matching crystalline surface tessellation.

---

## Per-Class Geometric Summary

| Class            | Ships | Avg Max Dim | Dim Range            | Avg Vertices | Avg Faces | Avg L:W |
|:-----------------|------:|:-----------:|:--------------------:|:------------:|:---------:|:-------:|
| Capsule          |     1 |         3.8 | 3.8 – 3.8           |   3,488      |   5,674   | 1.545   |
| Fighter          |     9 |        25.3 | 18.1 – 38.7         |   6,081      |   6,636   | 1.734   |
| Frigate          |    99 |        98.2 | 42.1 – 260.0        |   6,214      |   5,812   | 1.625   |
| Shuttle          |     5 |       142.9 | 40.7 – 515.3        |   4,392      |   4,296   | 1.577   |
| Destroyer        |    24 |       277.1 | 144.9 – 448.4       |   8,832      |   7,929   | 2.470   |
| Cruiser          |    59 |       337.4 | 156.8 – 612.9       |   9,775      |   8,675   | 1.927   |
| Barge            |     6 |       352.4 | 291.2 – 401.3       |  21,233      |  15,087   | 2.278   |
| Battlecruiser    |    20 |       489.0 | 327.6 – 677.3       |   9,354      |   7,814   | 1.987   |
| Industrial       |    25 |       658.9 | 288.2 – 1,240.1     |   9,215      |   8,043   | 3.398   |
| Battleship       |    33 |     1,164.2 | 726.9 – 2,385.3     |  11,790      |  11,456   | 2.209   |
| Freighter        |     5 |     1,854.2 | 1,031.3 – 2,481.1   |  13,732      |   9,620   | 2.773   |
| Capital          |     2 |     2,686.2 | 2,398.6 – 2,973.9   |  30,388      |  18,890   | 2.672   |
| Dreadnought      |     5 |     3,727.8 | 2,794.6 – 5,130.0   |  17,227      |  12,859   | 2.576   |
| Force Auxiliary   |     4 |     4,272.3 | 3,822.2 – 4,623.2   |  22,667      |  18,424   | 1.412   |
| Carrier          |     9 |     5,840.1 | 2,153.2 – 9,325.6   |  22,179      |  16,161   | 3.391   |
| Titan            |     5 |    16,590.5 | 13,831.9 – 18,140.0 |  40,257      |  26,316   | 4.371   |

### Key Class Observations

- **Size Progression**: Clear exponential scaling from Frigate (98) to Titan (16,590) with
  each class approximately 2-4x the previous.
- **Destroyers**: Highest L:W among subcapitals (2.470) — long, narrow hulls.
- **Industrials**: Very high L:W (3.398) — long barge-like shapes.
- **Titans**: Highest L:W (4.371) — extremely elongated superstructures.
- **Force Auxiliary**: Lowest L:W among capitals (1.412) — wide, blocky support vessels.
- **Complexity scales with class**: Frigate ~6K verts → Cruiser ~10K → Battleship ~12K →
  Capital ~30K → Titan ~40K.

---

## Texture Reference (Leshak Example)

The `data/ships/textures_example/` directory contains PBR texture maps for the Triglavian
Leshak battleship, demonstrating the full material pipeline:

| Texture File         | Purpose                    | Usage in Procedural Generation   |
|:---------------------|:---------------------------|:---------------------------------|
| leshak albedo.dds    | Base color / diffuse       | Faction color palette reference  |
| leshak normal*.dds   | Normal map (X/Y channels)  | Surface detail frequency         |
| leshak roughness.dds | Roughness map              | Material roughness ranges        |
| leshak_ar.dds        | Ambient reflection         | Metallic surface properties      |
| leshak occlusion.dds | Ambient occlusion          | Cavity/crevice depth reference   |
| leshak glow.dds      | Emissive / glow            | Engine glow, window placement    |
| leshak dirt.dds      | Weathering overlay         | Surface wear variation           |
| leshak paint.dds     | Paint mask                 | Color region boundaries          |
| leshak_pmdg.dds      | Paint/Metal/Dirt/Glow pack | Combined material channels       |
| leshak_specular.dds  | Specular intensity         | Highlight intensity reference    |

### Key Texture Observations

- PBR material pipeline with separated channels allows procedural generation to target
  specific material properties independently.
- Paint masks define distinct color regions — useful for faction color application.
- Dirt/weathering overlays suggest adding procedural noise for worn/battle-damaged looks.
- Glow maps concentrate emissive areas at engine exhausts and window strips.

---

## Recommendations for Procedural Generation

### 1. Data-Driven Proportions (`ReferenceModelTraits`)

Store the measured aspect ratios and size ranges as reference constraints. When generating
a ship, sample within the measured ranges for that faction and class:

```
Frigate proportions:  L:W ≈ 1.6, L:H ≈ 2.5,  max dim ≈ 42-260
Destroyer proportions: L:W ≈ 2.5, L:H ≈ 4.0,  max dim ≈ 145-448
Cruiser proportions:  L:W ≈ 1.9, L:H ≈ 3.0,  max dim ≈ 157-613
Battlecruiser:        L:W ≈ 2.0, L:H ≈ 3.2,  max dim ≈ 328-677
Battleship:           L:W ≈ 2.2, L:H ≈ 3.8,  max dim ≈ 727-2385
```

### 2. Faction-Specific Shape Modifiers

Apply faction-based modifiers to hull cross-section profiles:

- **Caldari**: Flatten cross-sections (low L:W, angular edges)
- **Minmatar**: Stretch height axis (high L:H), add framework protrusions
- **Gallente**: Round cross-sections, reduce vertex count (smoother)
- **Amarr**: Add vertical spire extensions, moderate all ratios
- **Triglavian**: Triangular cross-sections, high tessellation

### 3. Controlled Variation Ranges

Use the measured min/max ranges per faction to define valid variation bounds:

```
Amarr L:W variation:    1.02 – 4.57  (factor ≈ 4.5x range)
Caldari L:W variation:  1.00 – 5.09  (factor ≈ 5.1x range)
Gallente L:W variation: 1.04 – 10.44 (factor ≈ 10x range — widest)
Minmatar L:W variation: 1.01 – 5.76  (factor ≈ 5.7x range)
```

### 4. Complexity Scaling

Scale vertex density with ship class to match reference models:

```
Frigate:     ~6,200 vertices  (detail density: 1.0x)
Destroyer:   ~8,800 vertices  (detail density: 1.4x)
Cruiser:     ~9,800 vertices  (detail density: 1.6x)
Battlecruiser: ~9,400 vertices (detail density: 1.5x)
Battleship: ~11,800 vertices  (detail density: 1.9x)
Capital:    ~30,400 vertices  (detail density: 4.9x)
Titan:      ~40,300 vertices  (detail density: 6.5x)
```
