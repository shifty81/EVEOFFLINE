# Ship Modeling System

This document describes the procedural ship modeling system used in EVE OFFLINE to generate 3D models for ships, stations, and asteroids that are inspired by EVE Online's iconic designs.

> **See Also:** [SCALE_AND_DETAIL_SYSTEM.md](SCALE_AND_DETAIL_SYSTEM.md) for comprehensive scale information and the enhanced detail generation system.

## Overview

The modeling system procedurally generates ship geometry based on ship class and faction, creating distinctive silhouettes that evoke EVE Online's design language without using actual EVE assets.

### Enhanced Procedural Generation (New!)

As of February 2026, the system now includes:
- **Enhanced detail generation** with weapon hardpoints, engine glow, and hull panels
- **Faction-specific traits** (Caldari blocky, Amarr spires, Gallente organic, Minmatar asymmetric)
- **Dynamic detail scaling** based on ship class
- **Proper scale relationships** between all entities (ships, stations, asteroids)

See [SCALE_AND_DETAIL_SYSTEM.md](SCALE_AND_DETAIL_SYSTEM.md) for complete details.

## Ship Classes

### Frigates
**Size**: 3.5 units length x 0.9 units width
**Characteristics**:
- Sleek, aggressive nose profile
- Expanding front-mid section showing weapon hardpoints
- Compact main hull
- Dual engine pods at rear
- Fast and nimble appearance

**EVE Online Comparison**: Frigates capture the small, agile nature of EVE frigates like the Rifter, Merlin, and Punisher. The elongated profile and visible engine clusters match the design aesthetic.

### Destroyers
**Size**: 5.0 units length x 0.7 units width
**Characteristics**:
- Sharp, aggressive nose
- Narrow forward weapon section
- Distinctive dual-hull spine design
- Split rear engines
- Long, thin profile emphasizing speed

**EVE Online Comparison**: Inspired by EVE destroyers like the Thrasher and Coercer. The dual-hull design and elongated shape reflect the destroyer's role as a long-range attack vessel.

### Cruisers
**Size**: 6.0 units length x 1.8 units width
**Characteristics**:
- Prominent command bridge section
- Substantial forward section
- Wide, powerful main hull
- Multiple sections showing complexity
- Heavy propulsion systems

**EVE Online Comparison**: Modeled after EVE cruisers like the Stabber, Moa, and Thorax. The increased mass and presence reflect the cruiser's role as a versatile combat platform.

### Battlecruisers
**Size**: 8.5 units length x 2.5 units width
**Characteristics**:
- Forward command tower
- Massive weapon platform sections
- Broad, imposing main hull
- Multiple powerful engines
- Intimidating profile

**EVE Online Comparison**: Based on EVE battlecruisers like the Cyclone, Ferox, and Brutix. The threatening silhouette and heavy weapon mountings capture their combat-focused design.

### Battleships
**Size**: 12.0 units length x 3.5 units width
**Characteristics**:
- Command citadel structure
- Forward battle section with weapon batteries
- Extremely wide superstructure
- Multiple massive engine banks
- Maximum firepower appearance

**EVE Online Comparison**: Inspired by EVE battleships like the Tempest, Raven, Dominix, and Apocalypse. The sheer size and presence mirror the battleship's role as the largest subcapital combat vessel.

### Carriers (Capital Ships)
**Size**: 15.0 units length x 6.0 units width
**Characteristics**:
- Small front command section
- Flat, wide carrier deck
- Distinctive capital ship silhouette
- Massive rear engine section
- Fighter bay appearance

**EVE Online Comparison**: Modeled after carriers like the Archon, Thanatos, Chimera, and Nidhoggur. The flat deck design suggests fighter operations similar to EVE's carrier mechanics.

### Dreadnoughts (Capital Ships)
**Size**: 12.0 units length x 4.5 units width
**Characteristics**:
- Compact but heavily armored appearance
- Massive forward weapon platform
- Thick, bulky hull sections
- Heavy armor plating visual suggestion
- Rear engine array

**EVE Online Comparison**: Based on dreadnoughts like the Revelation, Moros, Phoenix, and Naglfar. The compact, weapon-heavy design reflects the dreadnought's siege role.

### Titans (Supercapitals)
**Size**: 25.0 units length x 8.0 units width
**Characteristics**:
- Absolutely massive scale
- Distinctive command tower
- Wide forward sections
- Multiple hull sections showing complexity
- Massive engine clusters
- Imposing and intimidating

**EVE Online Comparison**: Inspired by titans like the Avatar, Erebus, Leviathan, and Ragnarok. The enormous size and complex geometry reflect the titan's status as the most powerful ship class.

## Faction Colors

The system uses distinctive color schemes for each faction to match EVE Online's racial aesthetics:

### Minmatar
- **Primary**: Rust brown (0.5, 0.35, 0.25)
- **Secondary**: Dark brown (0.3, 0.2, 0.15)
- **Accent**: Light rust (0.8, 0.6, 0.3)
- **Style**: Rust-belt aesthetic, asymmetric, functional

### Caldari
- **Primary**: Steel blue (0.35, 0.45, 0.55)
- **Secondary**: Dark blue (0.2, 0.25, 0.35)
- **Accent**: Light blue (0.5, 0.7, 0.9)
- **Style**: Industrial, blocky, utilitarian

### Gallente
- **Primary**: Dark green-gray (0.3, 0.4, 0.35)
- **Secondary**: Darker green (0.2, 0.3, 0.25)
- **Accent**: Light green (0.4, 0.7, 0.5)
- **Style**: Organic curves, smooth surfaces

### Amarr
- **Primary**: Gold-brass (0.6, 0.55, 0.45)
- **Secondary**: Dark gold (0.4, 0.35, 0.25)
- **Accent**: Bright gold (0.9, 0.8, 0.5)
- **Style**: Ornate, cathedral-like, spires

## Pirate Factions

### Serpentis
- **Colors**: Purple tones (0.4, 0.25, 0.45) with dark accents
- **Style**: Sleek, modified Gallente designs

### Guristas
- **Colors**: Dark red (0.5, 0.2, 0.2) with crimson accents
- **Style**: Modified Caldari designs

### Blood Raiders
- **Colors**: Blood red (0.4, 0.15, 0.15) with almost-black sections
- **Style**: Dark, menacing appearance

## Stations

**Size**: 20.0 units
**Characteristics**:
- Central hub structure
- Radial symmetry with 8 segments
- Docking spokes extending outward
- Massive, stationary appearance
- Faction-specific color schemes

**Types**: Industrial, Military, Commercial, Research
**EVE Online Comparison**: Based on NPC station designs with faction-specific architectural elements from the station_visual_data.json.

## Asteroids

**Size**: 2.5 units (variable)
**Characteristics**:
- Irregular, rocky shapes
- Ore-type specific coloring
- Procedural surface variation
- Natural, unrefined appearance

**Ore Types**:
- **Veldspar**: Brown-orange (0.6, 0.4, 0.2)
- **Scordite**: Gray metallic (0.5, 0.5, 0.55)
- **Pyroxeres**: Red-brown (0.7, 0.3, 0.2)
- **Plagioclase**: Green-gray (0.3, 0.5, 0.4)
- **Omber**: Golden-brown (0.8, 0.6, 0.3)
- **Kernite**: Blue-cyan (0.3, 0.6, 0.7)
- **Jaspet**: Dark red (0.6, 0.2, 0.3)
- **Hemorphite**: Bright red-orange (0.9, 0.3, 0.2)

**EVE Online Comparison**: Colors match the visual appearance of ore asteroids in EVE Online based on the asteroid_visual_data.json specifications.

## Technical Implementation

### Model Class
Located in `cpp_client/src/rendering/model.cpp` and `cpp_client/include/rendering/model.h`

Key methods:
- `createShipModel()`: Main entry point for ship model generation
- `createFrigateModel()` through `createTitanModel()`: Class-specific generators
- `createStationModel()`: Station structure generation
- `createAsteroidModel()`: Asteroid generation with ore-type variation
- `getFactionColors()`: Color scheme lookup

### Mesh Generation
Models are generated as procedural mesh geometry using:
- Vertex positions with normals
- Color per vertex (faction colors)
- Triangle indices for rendering
- Multiple sections with varied scales for detail

### Caching
The system includes model caching to avoid regenerating identical models:
```cpp
static std::map<std::string, std::shared_ptr<Model>> s_modelCache;
```

## Future Enhancements

### Planned Features
1. **Racial Design Language**: Add faction-specific geometric characteristics:
   - Amarr: Spires and cathedral elements
   - Caldari: Blocky, city-block shapes  
   - Gallente: Organic curves and flowing lines
   - Minmatar: Asymmetric, bolted-together appearance

2. **Model File Loading**: Support for loading external 3D model files:
   - OBJ format support
   - GLTF/GLB format support
   - Asset directory structure
   - Model library for custom ship designs

3. **Enhanced Detail**:
   - Surface detail and paneling
   - Weapon hardpoint visualization
   - Engine glow effects
   - Shield bubble effects

4. **Animation Support**:
   - Engine exhaust particles
   - Rotating sections (e.g., mining barge strip miners)
   - Warp animation effects

## Comparing to EVE Online

This modeling system creates procedural approximations of EVE Online's ship designs. While not using actual EVE assets, the models capture the essence of each ship class:

- **Scale Relationships**: Size ratios between classes match EVE's relative scales
- **Visual Language**: Faction color schemes and general aesthetics reflect EVE's design
- **Class Characteristics**: Each ship class has distinctive features matching their role
- **Capital Ships**: The massive scale and presence of capitals is preserved
- **Stations**: Station designs reflect the architectural variety in EVE
- **Asteroids**: Ore types have distinct visual appearances

The procedural approach allows for:
- Real-time generation without asset loading
- Infinite variation within design constraints
- Small memory footprint
- Easy modification and extension

## Usage Example

```cpp
// Create a Minmatar frigate (Rifter-style)
auto model = Model::createShipModel("Rifter", "Minmatar");
model->draw();

// Create an Amarr carrier (Archon-style)
auto carrier = Model::createShipModel("Archon", "Amarr");
carrier->draw();

// Create a Veldspar asteroid
auto asteroid = Model::createShipModel("Veldspar", "");
asteroid->draw();
```

## Credits

Ship modeling system designed to evoke EVE Online's iconic ship designs without using CCP Games assets. All models are procedurally generated approximations inspired by EVE Online's design language.

**Note**: EVE OFFLINE is an independent fan project and is not affiliated with or endorsed by CCP Games.
