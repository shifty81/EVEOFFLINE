# Technical Design Document

## Architecture Overview

The Blender Spaceship Generator is designed as a modular addon with clear separation of concerns:

```
BlenderSpaceshipGenerator/
├── __init__.py              # Main addon entry, UI, registration
├── ship_generator.py        # Orchestrates ship generation
├── ship_parts.py            # Generates individual ship components
├── interior_generator.py    # Creates interior spaces
├── module_system.py         # Handles attachable modules
├── brick_system.py          # Brick taxonomy, grid, Ship DNA
├── atlas_exporter.py        # EVEOFFLINE JSON import + OBJ export
├── station_generator.py     # Procedural station generation
├── asteroid_generator.py    # Asteroid belt generation
├── texture_generator.py     # Procedural PBR materials
├── README.md                # User documentation
├── USAGE.md                 # Installation and usage guide
├── EXAMPLES.md              # Example configurations
├── TECHNICAL.md             # This file
├── ENGINE_INTEGRATION.md    # Engine-facing data reference (for Atlas/EVEOFFLINE)
├── features.md              # Feature specification and design rules
├── EVEOFFLINE_GUIDE.md      # Start-to-finish EVEOFFLINE integration guide
└── IMPLEMENTATION_SUMMARY.md # Implementation overview
```

## Module Descriptions

### `__init__.py` - Main Addon Interface

**Purpose**: Blender addon registration, UI, and property management

**Key Components**:
- `bl_info`: Addon metadata for Blender
- `SpaceshipGeneratorProperties`: Property group for UI settings
- `SPACESHIP_OT_generate`: Operator for ship generation
- `SPACESHIP_PT_main_panel`: UI panel in 3D viewport sidebar
- `register()/unregister()`: Addon lifecycle management

**Properties**:
- `ship_class`: Enum for ship size/type
- `generate_interior`: Boolean for interior generation
- `module_slots`: Integer for module count
- `seed`: Integer for randomization
- `hull_complexity`: Float for geometry detail
- `symmetry`: Boolean for symmetric design
- `style`: Enum for design inspiration
- `naming_prefix`: String project naming prefix applied to all elements
- `turret_hardpoints`: Integer number of turret hardpoints (0-10)

### `ship_generator.py` - Generation Orchestration

**Purpose**: Coordinates the generation of complete ships

**Key Components**:
- `SHIP_CONFIGS`: Dictionary mapping ship classes to parameters
- `generate_spaceship()`: Main generation function
- `_prefixed_name()`: Applies optional naming prefix to all elements

**Ship Configuration Structure**:
```python
{
    'scale': float,           # Overall size multiplier
    'hull_segments': int,     # Hull subdivision count
    'engines': int,           # Number of engine units
    'weapons': int,           # Number of weapon hardpoints
    'turret_hardpoints': int, # Number of turret hardpoints (max 10)
    'wings': bool,            # Whether to add wings
    'crew_capacity': int,     # Number of crew members
}
```

**Generation Flow**:
1. Set random seed
2. Create ship collection
3. Generate hull (main body)
4. Generate cockpit/bridge
5. Generate engines
6. Generate wings (if applicable)
7. Generate weapon hardpoints
8. Generate turret hardpoints (with visual turret geometry)
9. Generate modules
10. Generate interior (if enabled)
11. Parent all objects to hull

### `ship_parts.py` - Component Generation

**Purpose**: Creates individual ship components

**Key Functions**:

**`generate_hull()`**:
- Creates main ship body
- Applies style-specific modifications
- Adds subdivision surface for smoothing
- Handles complexity levels

**`generate_cockpit()`**:
- Creates bridge/cockpit area
- Scales based on ship size
- Positions at ship front

**`generate_engines()`**:
- Creates engine units
- Handles symmetric/asymmetric placement
- Adds emissive materials for glow
- Positions at ship rear

**`generate_wings()`**:
- Creates wing structures
- Symmetric placement
- Appropriate for smaller ships

**`generate_weapon_hardpoints()`**:
- Creates weapon mount points
- Distributed along ship hull
- Can be symmetric or not

**`generate_turret_hardpoints()`**:
- Creates visual turret fittings (base cylinder + rotation ring + barrel)
- Ships may have up to 10 turret hardpoints
- Each turret carries custom properties for engine mapping:
  - `turret_index`: 1-based position index
  - `turret_type`: weapon type (e.g. `"projectile"`)
  - `tracking_speed`: rotation speed in degrees/sec
  - `rotation_limits`: `"yaw:360,pitch:90"` string
  - `hardpoint_size`: visual size of the turret
- Symmetrically placed along the dorsal (top) surface

**Style Functions**:
- `apply_x4_style()`: Angular, beveled edges
- `apply_elite_style()`: Tapered, sleek shapes
- `apply_eve_style()`: Organic modifications
- `apply_mixed_style()`: Combination approach

### `interior_generator.py` - Interior Spaces

**Purpose**: Creates FPV-ready ship interiors

**Key Constants**:
```python
HUMAN_HEIGHT = 1.8      # Standard human height
DOOR_HEIGHT = 2.0       # Doorway height
DOOR_WIDTH = 1.0        # Doorway width
CORRIDOR_WIDTH = 1.5    # Corridor width
CORRIDOR_HEIGHT = 2.5   # Corridor height
ROOM_HEIGHT = 3.0       # Room height
```

**Key Functions**:

**`generate_interior()`**:
- Main entry point
- Determines layout based on ship class
- Small ships: Simple cockpit
- Medium ships: Cockpit + crew area
- Large ships: Full multi-room layout

**`generate_bridge()`**:
- Command center for large ships
- Includes command chair and consoles
- Scaled to room standards

**`generate_corridor()`**:
- Creates corridor segments
- Floor, ceiling, walls
- Standard human dimensions

**`generate_corridor_network()`**:
- Connects multiple ship areas
- Main corridor + side branches
- Scales with crew capacity

**`generate_crew_quarters()`**:
- Creates living spaces
- Bunks based on crew capacity
- Personal space allocation

**`generate_cargo_bay()`**:
- Large open area
- Includes cargo containers
- Scaled to ship size

**`generate_engine_room()`**:
- Engineering section
- Reactor core with glow effect
- Technical atmosphere

**`generate_doorway()`**:
- Access points between areas
- Standard dimensions
- Positioned as needed

### `module_system.py` - Attachable Modules

**Purpose**: Generates additional ship modules

**Module Types**:
```python
CARGO: Box-shaped storage
WEAPON: Cylinder with barrels
SHIELD: Sphere with emitter ring
HANGAR: Box with bay doors
SENSOR: Cone with dish
POWER: Cylinder generator
```

**Key Functions**:

**`generate_modules()`**:
- Creates specified number of modules
- Selects appropriate types for ship class
- Distributes around ship

**`get_available_modules()`**:
- Small ships: Weapon, Shield, Sensor
- Medium ships: + Cargo, Power
- Large ships: All types

**`create_module()`**:
- Creates module based on type
- Positions around ship
- Adds type-specific details

**Detail Functions**:
- `add_weapon_barrels()`: Weapon emplacements
- `add_sensor_dish()`: Sensor array
- `add_shield_emitter()`: Shield generator ring
- `add_hangar_doors()`: Bay door panels

## Design Patterns

### 1. Factory Pattern
Each generation function acts as a factory, creating objects with specific configurations.

### 2. Strategy Pattern
Style-specific functions (`apply_x4_style`, etc.) implement different strategies for the same goal.

### 3. Composite Pattern
Ships are composed of multiple parts, all parented to the main hull.

### 4. Configuration Pattern
`SHIP_CONFIGS` and `MODULE_TYPES` provide data-driven configuration.

## Data Flow

```
User Input (UI)
    ↓
Properties (PropertyGroup)
    ↓
generate_spaceship()
    ↓
├─ ship_parts functions
├─ interior_generator functions
└─ module_system functions
    ↓
Blender Objects
    ↓
Scene Collection
```

## Object Hierarchy

When a naming prefix is set (e.g. `EVEOFFLINE`), every generated element
name is prepended with `EVEOFFLINE_`.  Without a prefix the names remain
unchanged.

```
[PREFIX_]Spaceship_CLASS_SEED (Collection)
├─ [PREFIX_]Hull (Parent Object)
│   ├─ [PREFIX_]Cockpit
│   ├─ [PREFIX_]Engine_L1
│   ├─ [PREFIX_]Engine_R1
│   ├─ [PREFIX_]Wing_Left
│   ├─ [PREFIX_]Wing_Right
│   ├─ [PREFIX_]Weapon_Hardpoint_1
│   ├─ [PREFIX_]Weapon_Hardpoint_2
│   ├─ [PREFIX_]Turret_Hardpoint_1  (custom props: turret_index, turret_type, tracking_speed, rotation_limits)
│   │   ├─ [PREFIX_]Turret_Ring_1
│   │   └─ [PREFIX_]Turret_Barrel_1
│   ├─ Module_1
│   │   ├─ Connector
│   │   └─ Type-specific parts
│   └─ Interior Objects
│       ├─ [PREFIX_]Bridge_Floor
│       ├─ [PREFIX_]Corridor_Floor
│       ├─ [PREFIX_]Quarters_Floor
│       └─ ...
```

## Material System

Materials are procedurally created using Blender's node system:

**Engine Glow**:
```
Emission Shader
├─ Color: Blue (0.2, 0.5, 1.0)
└─ Strength: 5.0
```

**Reactor Glow**:
```
Emission Shader
├─ Color: Orange (0.8, 0.3, 0.1)
└─ Strength: 3.0
```

**Shield Emitter**:
```
Emission Shader
├─ Color: Blue (0.3, 0.5, 1.0)
└─ Strength: 2.0
```

## Coordinate System

- **X-axis**: Left/Right (width)
- **Y-axis**: Front/Back (length)
- **Z-axis**: Up/Down (height)

Ship orientation:
- Front: +Y direction
- Rear: -Y direction
- Top: +Z direction
- Bottom: -Z direction

## Scaling System

All ships use a base scale multiplier:

```python
Small ships:  1.0 - 3.0 units
Medium ships: 5.0 - 8.0 units
Large ships:  12.0 - 35.0 units
```

Interior elements use absolute human-scale measurements regardless of ship size.

## Performance Considerations

### Generation Time Factors:
1. **Hull Complexity**: More subdivisions = more processing
2. **Module Count**: Each module creates multiple objects
3. **Interior Generation**: Significant object count increase
4. **Ship Size**: Larger ships have more components

### Optimization Strategies:
1. Use lower subdivision levels initially
2. Disable interior for exterior work
3. Limit module count during iteration
4. Use instancing for repeated elements (future enhancement)

## Extension Points

The addon is designed to be extended:

### Adding New Ship Classes:
1. Add entry to `SHIP_CONFIGS` in `ship_generator.py`
2. Add enum option in `__init__.py`
3. Consider interior layout in `interior_generator.py`

### Adding New Modules:
1. Add entry to `MODULE_TYPES` in `module_system.py`
2. Create shape function if needed
3. Create detail function for type-specific elements

### Adding New Styles:
1. Add enum option in `__init__.py`
2. Create `apply_STYLE_style()` function in `ship_parts.py`
3. Add style checks in generation functions

### Adding New Parts:
1. Create generation function in `ship_parts.py`
2. Call from `generate_spaceship()` in `ship_generator.py`
3. Add configuration options as needed

## Testing Strategy

The addon includes validation tests:

**`test_validation.py`**:
- Syntax validation
- Structure verification
- Metadata checking
- Documentation presence

Tests run without Blender installation, checking code structure and completeness.

## Future Enhancements

Potential improvements:

1. **Advanced Materials**: PBR textures, procedural wear
2. **Animation System**: Bay doors, landing gear, turrets
3. **Damage System**: Battle damage, debris
4. **Lighting System**: Interior and exterior lights
5. **Detail Pass**: Greebles, panels, antennas
6. **LOD System**: Multiple detail levels for performance
7. **Export Presets**: Game engine ready exports
8. **Preset Library**: Save/load ship configurations
9. **Advanced Interiors**: Furniture, equipment, details
10. **Physics Support**: Collision meshes, physics properties

## API Usage Example

```python
import bpy
from BlenderSpaceshipGenerator import ship_generator

# Generate a custom ship programmatically
hull = ship_generator.generate_spaceship(
    ship_class='CRUISER',
    seed=42,
    generate_interior=True,
    module_slots=5,
    hull_complexity=1.5,
    symmetry=True,
    style='X4'
)

# Access the generated objects
collection = bpy.data.collections['Spaceship_CRUISER_42']
for obj in collection.objects:
    print(f"Generated: {obj.name}")
```

## Blender API Dependencies

The addon uses these Blender Python API features:

- `bpy.ops.mesh`: Mesh primitive creation
- `bpy.ops.object`: Object operations
- `bpy.data`: Data access (meshes, materials, collections)
- `bpy.context`: Scene context access
- `bpy.types`: Type definitions (Operator, Panel, PropertyGroup)
- `bpy.props`: Property definitions (EnumProperty, BoolProperty, etc.)
- `bpy.utils`: Registration utilities

## License and Attribution

This addon is designed as an educational and creative tool. Users should be aware of the intellectual property associated with the inspirational games (X4, Elite Dangerous, Eve Online) and ensure their use complies with applicable licenses and terms.

## Related Documentation

- **[ENGINE_INTEGRATION.md](ENGINE_INTEGRATION.md)** — Ship DNA schemas, brick tables, ECS component mappings, turret custom properties, hull skinning pipeline, and all data formats needed to implement ship systems in the Atlas game engine.
- **[features.md](features.md)** — Complete feature specification with design pillars, implementation status, and planned engine-side systems.
- **[EVEOFFLINE_GUIDE.md](EVEOFFLINE_GUIDE.md)** — Step-by-step guide for importing EVEOFFLINE ship JSON and exporting OBJ models.
