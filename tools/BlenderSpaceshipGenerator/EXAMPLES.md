# Examples and Design Reference

## Ship Design Philosophy

The addon generates ships inspired by three major space games, each with distinct design philosophies:

### X4 Foundations Style
- **Characteristics**: Angular, geometric, industrial
- **Features**: Hard edges, paneled surfaces, modular construction
- **Best for**: Military vessels, cargo haulers, utilitarian ships
- **Example configs**:
  ```
  Ship Class: Destroyer
  Style: X4
  Hull Complexity: 1.0
  Symmetry: Enabled
  ```

### Elite Dangerous Style
- **Characteristics**: Sleek, aerodynamic, futuristic
- **Features**: Smooth curves, streamlined shapes, elegant design
- **Best for**: Fighters, explorers, luxury vessels
- **Example configs**:
  ```
  Ship Class: Fighter
  Style: Elite Dangerous
  Hull Complexity: 1.5
  Symmetry: Enabled
  ```

### Eve Online Style
- **Characteristics**: Organic, flowing, unique
- **Features**: Asymmetric designs, alien aesthetics, sculptural forms
- **Best for**: Exotic ships, alien designs, artistic vessels
- **Example configs**:
  ```
  Ship Class: Cruiser
  Style: Eve Online
  Hull Complexity: 2.0
  Symmetry: Disabled
  ```

## Example Ship Configurations

### 1. Scout Fighter
```
Ship Class: Fighter
Style: Elite Dangerous
Seed: 42
Interior: Disabled
Module Slots: 0
Hull Complexity: 1.0
Symmetry: Enabled
```
**Purpose**: Fast, agile single-seat combat ship  
**Features**: Sleek design, wing-mounted weapons, minimal profile

### 2. Multi-Role Frigate
```
Ship Class: Frigate
Style: Mixed
Seed: 123
Interior: Enabled
Module Slots: 4
Hull Complexity: 1.5
Symmetry: Enabled
```
**Purpose**: Versatile medium ship for combat and exploration  
**Features**: Full interior, cargo capacity, weapon hardpoints, crew quarters

### 3. Industrial Hauler
```
Ship Class: Corvette
Style: X4
Seed: 999
Interior: Enabled
Module Slots: 6
Hull Complexity: 0.8
Symmetry: Disabled
```
**Purpose**: Cargo transport with minimal aesthetics  
**Features**: Maximum cargo modules, basic interior, industrial look

### 4. Fleet Carrier
```
Ship Class: Carrier
Style: Mixed
Seed: 777
Interior: Enabled
Module Slots: 8
Hull Complexity: 2.0
Symmetry: Enabled
```
**Purpose**: Large support ship for fleet operations  
**Features**: Hangar modules, extensive interior, command bridge, support systems

### 5. Capital Command Ship
```
Ship Class: Capital
Style: X4
Seed: 1000
Interior: Enabled
Module Slots: 10
Hull Complexity: 2.5
Symmetry: Enabled
```
**Purpose**: Flagship vessel for commanding fleets  
**Features**: Maximum size, full interior with multiple decks, heavy weapons

### 6. Exploration Vessel
```
Ship Class: Cruiser
Style: Elite Dangerous
Seed: 555
Interior: Enabled
Module Slots: 5
Hull Complexity: 1.8
Symmetry: Enabled
```
**Purpose**: Long-range exploration and research  
**Features**: Sensor modules, crew accommodations, scientific equipment

### 7. Asymmetric Raider
```
Ship Class: Destroyer
Style: Eve Online
Seed: 333
Interior: Enabled
Module Slots: 6
Hull Complexity: 2.2
Symmetry: Disabled
```
**Purpose**: Unique combat vessel with irregular design  
**Features**: Asymmetric weapon placement, organic shapes, intimidating appearance

## Module Combinations

### Combat Loadout
- Multiple Weapon modules
- Shield module
- Power module for weapon systems
- Minimal cargo

### Trading Loadout
- Maximum Cargo modules
- Shield module for protection
- Minimal weapons
- Focus on capacity

### Exploration Loadout
- Sensor modules
- Power module for long-range systems
- Cargo for supplies
- Shield module

### Support Loadout
- Hangar module for fighters/shuttles
- Cargo for supplies
- Shield module
- Power module

## Interior Design Tips

### Small Ships (Shuttle, Fighter)
- Cockpit only or minimal crew space
- Direct access from exterior
- Compact, efficient layout

### Medium Ships (Corvette, Frigate, Destroyer)
- Central corridor connecting areas
- Bridge/cockpit forward
- Crew quarters mid-ship
- Engineering aft
- 1-2 access points

### Large Ships (Cruiser, Battleship, Carrier, Capital)
- Multiple corridor networks
- Dedicated bridge
- Multiple crew decks
- Cargo bay areas
- Engine room complex
- Hangar decks (carriers)
- Multiple access points

## FPV Exploration

All interiors are designed for first-person exploration:

1. **Standard Dimensions**:
   - Human height: 1.8m
   - Door height: 2.0m
   - Corridor width: 1.5m
   - Room height: 3.0m

2. **Navigation**:
   - Corridors connect all major areas
   - Doorways provide access between sections
   - All spaces are navigable on foot

3. **Scale Reference**:
   - All measurements are in Blender units (≈ meters)
   - Character models should be ~1.8 units tall
   - Standard FPS camera height: 1.6-1.7 units

## Creating a Fleet

To create a cohesive fleet:

1. **Choose a style**: Use the same style for all ships
2. **Vary the seeds**: Use different seeds for variation
3. **Mix ship classes**: Include various sizes
4. **Consistent modules**: Use similar module types across fleet
5. **Color scheme**: Apply consistent materials/colors

### Example Fleet Configuration
```
Flagship: Capital, Style: X4, Seed: 1000
Escorts: 2x Destroyer, Style: X4, Seeds: 1001, 1002
Support: Carrier, Style: X4, Seed: 1003
Fighters: 6x Fighter, Style: X4, Seeds: 2001-2006
Scouts: 4x Corvette, Style: X4, Seeds: 3001-3004
```

## Performance Considerations

Generation time varies by configuration:

- **Fast** (< 1 second):
  - Small ships (Shuttle, Fighter)
  - No interior
  - Low hull complexity (0.5)
  - Few modules (0-2)

- **Medium** (1-5 seconds):
  - Medium ships (Corvette, Frigate)
  - With interior
  - Normal complexity (1.0-1.5)
  - Moderate modules (3-5)

- **Slow** (5-15 seconds):
  - Large ships (Capital)
  - Full interior
  - High complexity (2.0-3.0)
  - Many modules (8-10)

## Customization After Generation

Generated ships are fully editable:

1. **Add Details**: Model additional greebles, panels, antennas
2. **Materials**: Apply custom materials and textures
3. **Lighting**: Add internal and external lights
4. **Animation**: Animate engine glows, moving parts, bay doors
5. **Physics**: Add physics properties for game engines
6. **Optimization**: Reduce poly count for real-time use

## Export Tips

For use in game engines:

1. **Select entire collection**
2. **File → Export → FBX** or your format
3. **Options**:
   - Apply modifiers
   - Include animations (if added)
   - Correct orientation for your engine
4. **Scale**: Verify unit scale matches target engine

## Common Use Cases

### Game Development
- Generate varied enemy ships
- Create fleet assets quickly
- Populate space environments
- Background ships and stations

### Rendering/Art
- Science fiction illustrations
- Space battle scenes
- Concept art generation
- Portfolio pieces

### 3D Printing
- Physical models
- Miniatures for tabletop games
- Display pieces
- Prototypes

### Learning
- Study spaceship design principles
- Practice Blender skills
- Understand modular construction
- Learn procedural generation
