# Enhanced Procedural Ship Generation - Scale and Detail System

## Overview

This document describes the enhanced procedural ship generation system in EVE OFFLINE, including the proper scale relationships between entities and the new detail generation system that creates EVE-like ship designs.

## Scale System

### Game Units to Real-World Conversion

The game uses a consistent scale system where **1 game unit ≈ 50 meters**.

This maintains proper visual relationships between all entities in space while keeping numbers manageable for rendering and gameplay.

### Entity Scales

#### Ships (Game Units → Real Meters)

| Ship Class | Length (units) | Width (units) | Height (units) | Real Length (m) | Real Width (m) |
|------------|----------------|---------------|----------------|-----------------|----------------|
| **Frigates** | 3.5 | 0.9 | 0.7 | 175m | 45m |
| **Destroyers** | 5.0 | 0.7 | 0.6 | 250m | 35m |
| **Cruisers** | 6.0 | 1.8 | 1.5 | 300m | 90m |
| **Battlecruisers** | 8.5 | 2.5 | 2.0 | 425m | 125m |
| **Battleships** | 12.0 | 3.5 | 3.0 | 600m | 175m |
| **Mining Barges** | 6.0 | 3.0 | 2.0 | 300m | 150m |
| **Carriers** | 15.0 | 6.0 | 4.0 | 750m | 300m |
| **Dreadnoughts** | 12.0 | 4.5 | 5.0 | 600m | 225m |
| **Titans** | 25.0 | 8.0 | 7.0 | 1,250m | 400m |

#### Other Entities

| Entity | Size (units) | Real Size (m) | Notes |
|--------|--------------|---------------|-------|
| **Stations** | 20.0 (diameter) | 1,000m | Massive structures with docking bays |
| **Asteroids** | 2.5 (radius) | 125m | Varies by type; small-medium sized |
| **Large Asteroids** | 5.0 (radius) | 250m | Larger ore deposits |

### EVE Online Comparison

Our scales closely match EVE Online proportions:

| Ship Class | EVE Online | Our Scale | Match % |
|------------|------------|-----------|---------|
| Frigates | 50-100m | 175m | ~150% (more visible) |
| Destroyers | 100-150m | 250m | ~170% (more visible) |
| Cruisers | 200-400m | 300m | Perfect ✓ |
| Battlecruisers | 400-600m | 425m | Perfect ✓ |
| Battleships | 400-500m | 600m | ~120% (slightly larger) |
| Carriers | 1,000-2,500m | 750m | Scaled down for gameplay |
| Dreadnoughts | 1,000-1,500m | 600m | Scaled down for gameplay |
| Titans | 3,000-18,000m | 1,250m | Scaled down for gameplay |

**Note:** Small ships are slightly larger than EVE Online to ensure visibility and gameplay clarity. Capital ships are scaled down to prevent overwhelming the viewport while maintaining impressive visual presence.

## Enhanced Procedural Generation

### Ship Design Traits

Each ship is generated with faction-specific design characteristics:

#### ShipDesignTraits Structure

```cpp
struct ShipDesignTraits {
    DesignStyle style;        // Faction design language
    
    // Visual characteristics
    bool hasSpires;           // Amarr vertical spires
    bool isAsymmetric;        // Minmatar irregular design
    bool hasExposedFramework; // Minmatar visible structure
    bool isBlocky;            // Caldari angular forms
    bool isOrganic;           // Gallente smooth curves
    
    // Weapon configuration
    int turretHardpoints;     // Visible turret mounts
    int missileHardpoints;    // Missile launcher bays
    int droneHardpoints;      // Drone bay indicators
    
    // Engine configuration
    int engineCount;          // Number of exhausts
    bool hasLargeEngines;     // Massive engine banks
    
    // Detail scale
    float detailScale;        // Scale factor for details
    float asymmetryFactor;    // 0=symmetric, 1=highly asymmetric
};
```

### Faction Design Languages

#### Caldari - Industrial Blocky
**Visual Style:** Angular, city-block architecture, utilitarian
**Colors:** Steel blue, dark blue, light blue
**Characteristics:**
- Rectangular weapon platforms
- Flat, wide profiles
- Hard edges and corners
- Functional, no-nonsense aesthetics

**Design Traits:**
```cpp
- isBlocky = true
- turretHardpoints = varies by class
- engineCount = standard for class
- asymmetryFactor = 0.0 (perfectly symmetric)
```

#### Amarr - Ornate Golden
**Visual Style:** Cathedral-like, vertical emphasis, religious aesthetics
**Colors:** Gold-brass, dark gold, bright gold
**Characteristics:**
- Vertical spires extending upward
- Smooth flowing curves with sharp points
- Ornate decorative elements
- High vertical profile

**Design Traits:**
```cpp
- hasSpires = true
- turretHardpoints = laser turrets
- engineCount = standard for class
- asymmetryFactor = 0.0 (perfectly symmetric)
```

#### Gallente - Organic Smooth
**Visual Style:** Flowing curves, biomechanical, elegant forms
**Colors:** Dark green-gray, darker green, light green
**Characteristics:**
- Smooth organic transitions
- Rounded weapon pods
- Drone bay indicators
- Balanced, elegant proportions

**Design Traits:**
```cpp
- isOrganic = true
- droneHardpoints = present
- turretHardpoints = hybrid turrets
- asymmetryFactor = 0.0 (perfectly symmetric)
```

#### Minmatar - Asymmetric Rustic
**Visual Style:** Welded-together, exposed framework, irregular
**Colors:** Rust brown, dark brown, light rust
**Characteristics:**
- Asymmetric hull sections
- Visible structural framework
- Irregular engine placement
- Rugged, utilitarian appearance

**Design Traits:**
```cpp
- isAsymmetric = true
- hasExposedFramework = true
- turretHardpoints = projectile turrets
- asymmetryFactor = 0.3 (moderately asymmetric)
```

### Weapon Hardpoint Configuration

Weapon hardpoints scale by ship class:

| Ship Class | Turret Hardpoints | Missile Hardpoints | Drone Hardpoints |
|------------|-------------------|-------------------|------------------|
| Frigate | 2 | 0 | 0 |
| Destroyer | 4 | 0 | 0 |
| Cruiser | 4 | 2 | 1 |
| Battlecruiser | 6 | 2 | 2 |
| Battleship | 8 | 4 | 2 |
| Carrier | 0 | 0 | Fighter bays |
| Dreadnought | 2 (siege) | 0 | 0 |
| Titan | 1 (doomsday) | 0 | 0 |

### Detail Generation Functions

#### addWeaponHardpoints()
Generates small turret mount geometry on the hull.

**Parameters:**
- `posZ`: Position along ship length
- `offsetX`: Horizontal offset from centerline
- `offsetY`: Vertical offset
- `count`: Number of hardpoints
- `color`: Hardpoint color (typically accent color)

**Visual Result:**
- Small protruding turret bases
- Positioned symmetrically on hull
- Sized appropriately for ship class

#### addEngineDetail()
Generates engine exhaust cone geometry with glow effect.

**Parameters:**
- `posZ`: Position at rear of ship
- `width`: Engine cluster width
- `height`: Engine cluster height
- `count`: Number of engine exhausts
- `color`: Glow color (typically bright accent)

**Visual Result:**
- Conical exhaust geometry
- Arranged in circular pattern
- Brighter colors suggest engine thrust

#### addHullPanelLines()
Generates panel line detail across hull sections.

**Parameters:**
- `startZ`: Start position
- `endZ`: End position
- `width`: Hull width
- `color`: Panel color (typically darker primary)

**Visual Result:**
- Subtle panel separation lines
- Distributed evenly along hull
- Adds surface detail without complexity

#### addSpireDetail()
Generates Amarr-style vertical spire structures.

**Parameters:**
- `posZ`: Position along ship length
- `height`: Spire height
- `color`: Spire color (typically gold)

**Visual Result:**
- Pointed vertical extension
- Tapered from base to point
- Distinctive Amarr aesthetic

#### addAsymmetricDetail()
Generates Minmatar-style irregular protruding structures.

**Parameters:**
- `posZ`: Position along ship length
- `offset`: Offset from centerline
- `color`: Structure color (typically rust)

**Visual Result:**
- Asymmetric protruding section
- Irregular angles and shapes
- Welded-together appearance

## Usage Examples

### Creating an Enhanced Frigate

```cpp
// Frigate with enhanced details
auto model = Model::createFrigateModel(colors);

// Details are automatically added:
// - 2 weapon hardpoints on forward hull
// - 2 engine exhausts at rear with glow
// - Hull panel lines for surface detail
```

### Faction-Specific Appearance

**Caldari Frigate:**
- Blocky rectangular sections
- Sharp angular transitions
- Blue-gray coloring
- 2 turret hardpoints on top

**Amarr Frigate:**
- Vertical spire on command section
- Smooth flowing curves
- Gold-brass coloring
- 2 laser hardpoints on sides

**Gallente Frigate:**
- Organic rounded forms
- Smooth surface transitions
- Green-gray coloring
- 2 hybrid hardpoints, 1 drone bay

**Minmatar Frigate:**
- Asymmetric hull sections
- Irregular protruding structures
- Rust-brown coloring
- 2 projectile hardpoints, exposed framework

## Performance Considerations

### Vertex Counts

Enhanced details add minimal geometry:

| Detail Type | Vertices Added | Triangles Added |
|-------------|----------------|-----------------|
| Weapon Hardpoint | ~3 per hardpoint | ~1 per hardpoint |
| Engine Detail | ~3 per exhaust | ~1 per exhaust |
| Hull Panel Lines | ~3 per panel | ~1 per panel |
| Spire Detail | ~4 per spire | ~2 per spire |
| Asymmetric Detail | ~3 per structure | ~1 per structure |

**Total Impact:**
- Frigate: +40-50 vertices (~20-25 triangles)
- Battleship: +100-120 vertices (~50-60 triangles)

This is negligible compared to base geometry (frigates: 40-60 base vertices).

### Rendering

All detail geometry uses the same material system as base ship geometry, so there's no additional shader cost. The LOD (Level of Detail) system can optionally exclude detail geometry at long distances.

## Future Enhancements

### Planned Additions

1. **Texture Coordinates**
   - Generate UV coordinates for all geometry
   - Enable texture mapping for hull patterns
   - Support faction decals and markings

2. **More Detail Types**
   - Antenna arrays for Caldari
   - Drone bay doors for Gallente
   - Welding seams for Minmatar
   - Decorative trim for Amarr

3. **Dynamic Detail**
   - Engine glow intensity based on velocity
   - Weapon hardpoint activation animation
   - Damage effects on hull panels

4. **Ship-Specific Customization**
   - Named ships (Rifter, Merlin, etc.) get unique details
   - Tech II variants more pronounced differences
   - Pirate faction hybrid designs

## Technical Implementation Notes

### Coordinate System

Ships are modeled with:
- **+Z axis**: Forward (nose direction)
- **+X axis**: Right (starboard)
- **+Y axis**: Up (dorsal)

This matches the game's world coordinate system for consistent orientation.

### Scale Guidelines for Modders

When creating custom details:
1. Keep detail size proportional to ship (use `detailScale` trait)
2. Weapon hardpoints should be ~5-10% of ship width
3. Engine exhausts should be ~10-15% of ship width
4. Panel lines should be ~1-2% of ship length spacing

### Code Structure

All detail generation functions follow the pattern:
```cpp
void addDetail(vertices, indices, position, scale, count, color) {
    int startIdx = vertices.size();
    
    // Add new vertices
    vertices.push_back(...);
    
    // Add indices referencing new vertices
    indices.push_back(startIdx + ...);
}
```

This ensures details are properly integrated into the existing mesh without affecting base geometry.

---

*Last Updated: February 7, 2026*
*Version: 1.0 - Enhanced Procedural Generation*
