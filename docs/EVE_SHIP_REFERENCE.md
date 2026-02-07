# EVE Online Ship Design Reference

This document provides reference information about EVE Online ship designs that inspired the procedural modeling system in EVE OFFLINE.

## Design Philosophy

EVE Online ships have distinct design languages based on their faction and role. Our procedural models aim to capture these characteristics:

### Faction Design Languages

#### Amarr Empire
**Real EVE Design Elements**:
- Golden/brass coloring
- Cathedral-like spires and towers
- Symmetrical, radial designs
- Ornate, religious aesthetic
- Smooth, flowing curves with sharp points
- High vertical profile

**Example Ships**:
- **Punisher** (Frigate): Compact with golden armor, distinctive side pods
- **Harbinger** (Battlecruiser): Long spires extending forward and back
- **Apocalypse** (Battleship): Massive cathedral-like structure with golden hull
- **Avatar** (Titan): Enormous golden spires, most ornate titan design

**Our Implementation**:
- Gold-brass color palette (0.6, 0.55, 0.45)
- Vertical emphasis in geometry where possible
- Smooth transitions between sections
- Bright accent colors for energy effects

#### Caldari State
**Real EVE Design Elements**:
- Blue-gray industrial coloring
- Blocky, utilitarian shapes
- City-block architecture
- Sharp angles and rectangular forms
- Functional, no-nonsense aesthetic
- Horizontal emphasis

**Example Ships**:
- **Merlin** (Frigate): Rectangular, blocky design with shield emphasis
- **Ferox** (Battlecruiser): Wide, flat profile with weapon platforms
- **Raven** (Battleship): Massive rectangular sections, iconic EVE design
- **Leviathan** (Titan): Enormous city-block structure, most massive titan

**Our Implementation**:
- Steel blue color scheme (0.35, 0.45, 0.55)
- Angular geometry transitions
- Emphasis on width and bulk
- Industrial, functional appearance

#### Gallente Federation
**Real EVE Design Elements**:
- Green-gray coloring
- Organic, flowing curves
- Smooth surfaces
- Rounded, elegant forms
- Biomechanical aesthetic
- Balanced proportions

**Example Ships**:
- **Tristan** (Frigate): Smooth curves with organic weapon pods
- **Brutix** (Battlecruiser): Flowing lines with elegant weapon platforms
- **Dominix** (Battleship): Drone-carrying organic curves, iconic design
- **Erebus** (Titan): Smooth, flowing titan with organic weapon mounts

**Our Implementation**:
- Dark green-gray palette (0.3, 0.4, 0.35)
- Curved transitions between sections
- Rounded profiles
- Organic shape emphasis

#### Minmatar Republic
**Real EVE Design Elements**:
- Rust-brown coloring
- Asymmetric, chaotic designs
- Exposed structure and framework
- Bolted-together appearance
- Utilitarian, scrap-metal aesthetic
- Angular but irregular

**Example Ships**:
- **Rifter** (Frigate): Asymmetric with visible welding, cult classic
- **Cyclone** (Battlecruiser): Irregular engine placement, rugged look
- **Tempest** (Battleship): Asymmetric design with exposed structure
- **Ragnarok** (Titan): Most asymmetric titan, rust-belt aesthetic

**Our Implementation**:
- Rust brown color scheme (0.5, 0.35, 0.25)
- Varied scaling factors for asymmetry effect
- Functional, unpolished appearance
- Multiple engine pods showing variety

### Ship Class Characteristics in EVE Online

#### Frigates
**EVE Design**: Small, fast, agile. Often 50-100m long. Quick response vessels with minimal crew.

**Key Features**:
- Sleek, elongated profiles
- Small but visible weapon hardpoints
- Compact engine clusters
- Quick, nimble appearance
- Minimal superstructure

**Famous Examples**:
- **Rifter**: The iconic EVE frigate, asymmetric Minmatar design
- **Dramiel**: Pirate frigate with distinctive wing-like structures
- **Interceptors**: Ultra-fast versions with even more elongated profiles

#### Destroyers  
**EVE Design**: Longer than frigates, 100-150m. Optimized for small weapon systems.

**Key Features**:
- Very elongated, thin profiles
- Multiple small turret/launcher hardpoints
- Spine-like central structure
- Split or dual engine design
- Aggressive, predatory appearance

**Famous Examples**:
- **Thrasher**: Minmatar artillery destroyer, multiple turrets visible
- **Coercer**: Amarr laser destroyer with forward-pointing spires
- **Catalyst**: Gallente blaster destroyer with rounded weapon pods

#### Cruisers
**EVE Design**: Medium-sized, 200-400m. Versatile combat platforms.

**Key Features**:
- Balanced proportions
- Visible command bridge section
- Mix of weapons and defenses
- Substantial engine systems
- Multi-role appearance

**Famous Examples**:
- **Stabber**: Sleek Minmatar cruiser with visible engines
- **Moa**: Caldari shield cruiser, blocky and wide
- **Thorax**: Gallente hybrid cruiser with smooth curves
- **Omen**: Amarr laser cruiser with golden spires

#### Battlecruisers
**EVE Design**: Large, 400-600m. Heavy attack vessels bridging cruisers and battleships.

**Key Features**:
- Imposing, massive profiles
- Heavy weapon platform sections
- Command tower or citadel visible
- Multiple large engine banks
- Intimidating presence

**Famous Examples**:
- **Cyclone**: Minmatar missile battlecruiser, asymmetric design
- **Ferox**: Caldari rail battlecruiser, wide weapon platforms
- **Brutix**: Gallente blaster battlecruiser, smooth and deadly
- **Harbinger**: Amarr laser battlecruiser with forward spires

#### Battleships
**EVE Design**: Massive, 600-1200m. Most powerful subcapital warships.

**Key Features**:
- Enormous size and presence
- Large weapon battery sections
- Prominent command citadel
- Massive engine arrays
- Ultimate firepower appearance

**Famous Examples**:
- **Tempest**: Minmatar projectile battleship, asymmetric powerhouse
- **Raven**: Caldari missile battleship, most iconic EVE ship
- **Dominix**: Gallente drone battleship with massive drone bays
- **Apocalypse**: Amarr laser battleship, cathedral-like with spires

#### Carriers (Capitals)
**EVE Design**: 1500-3000m. Fighter-launching support capitals.

**Key Features**:
- Flat carrier deck appearance
- Massive size
- Fighter bay sections visible
- Capital ship presence
- Support role visual cues

**Famous Examples**:
- **Archon**: Amarr carrier with golden architecture
- **Thanatos**: Gallente carrier with organic curves
- **Chimera**: Caldari carrier, blocky industrial design
- **Nidhoggur**: Minmatar carrier with asymmetric structure

#### Dreadnoughts (Capitals)
**EVE Design**: 1500-3000m. Siege-mode attack capitals.

**Key Features**:
- Compact but massive
- Enormous weapon platforms
- Heavy armor appearance
- Siege weapon emphasis
- Intimidating frontal profile

**Famous Examples**:
- **Revelation**: Amarr dreadnought with massive laser array
- **Moros**: Gallente dreadnought with huge blasters
- **Phoenix**: Caldari dreadnought with missile batteries
- **Naglfar**: Minmatar dreadnought with projectile cannons

#### Titans (Supercapitals)
**EVE Design**: 10,000-18,000m. Largest ships in New Eden.

**Key Features**:
- Absolutely enormous scale
- Doomsday weapon visible
- Multiple distinct sections
- Jump portal generators
- Presence that dominates grids

**Famous Examples**:
- **Avatar**: Amarr titan, most ornate with golden spires
- **Erebus**: Gallente titan, smooth curves and massive size
- **Leviathan**: Caldari titan, largest and most blocky
- **Ragnarok**: Minmatar titan, asymmetric and distinctive

### Size Comparisons

**Real EVE Online Scale**:
```
Frigate:        50-100m
Destroyer:      100-150m  
Cruiser:        200-400m
Battlecruiser:  400-600m
Battleship:     600-1200m
Carrier:        1500-3000m
Dreadnought:    1500-3000m
Titan:          10,000-18,000m
```

**Our Model Scale** (relative units):
```
Frigate:        3.5 units
Destroyer:      5.0 units
Cruiser:        6.0 units
Battlecruiser:  8.5 units
Battleship:     12.0 units
Carrier:        15.0 units
Dreadnought:    12.0 units
Titan:          25.0 units
```

The ratios preserve EVE's dramatic scale differences.

## Visual Reference

### Where to Find EVE Ship Images

For visual reference when understanding our models:

1. **EVE Online Website**: https://www.eveonline.com/
   - Ships section has high-quality renders

2. **EVE University**: https://wiki.eveuniversity.org/
   - Comprehensive ship information and images

3. **EVE Models Website**: Various fan sites host 3D model viewers
   - See ships from all angles
   - Understand geometry and details

4. **In-Game**: Best reference is seeing ships in EVE Online
   - Show info window has ship renders
   - Zoom in on ships in space
   - Right-click "Look At" to orbit camera

### Key Visual Elements to Notice

When looking at EVE ships for reference:

1. **Faction Identity**: Color scheme and overall aesthetic
2. **Size Relationships**: How classes relate in scale
3. **Weapon Hardpoints**: Where turrets/launchers mount
4. **Engine Configuration**: Number and placement of engines
5. **Asymmetry**: Especially in Minmatar designs
6. **Surface Detail**: Paneling, lights, markings

## Procedural Approximation Strategy

Our procedural models aim to capture the *essence* rather than exact geometry:

### What We Match
✓ Size relationships between classes
✓ Faction color schemes  
✓ Overall silhouettes and profiles
✓ Ship class characteristics
✓ Scale and presence
✓ General aesthetic feel

### What We Don't Match
✗ Exact hull geometry (legal/practical reasons)
✗ Specific surface details and paneling
✗ Turret/launcher hardpoints (complex animation)
✗ Precise proportions of specific ships
✗ Textures and decals

### Our Approach
- **Geometric Primitives**: Build shapes from basic forms
- **Scaling Variation**: Use varied scales to create complexity
- **Color Differentiation**: Faction colors provide identity
- **Size Emphasis**: Dramatic scale differences create impact
- **Procedural Generation**: Infinite variation within constraints

## Legal and Ethical Considerations

**Important**: EVE OFFLINE does not use any CCP Games assets:
- No EVE Online 3D models
- No EVE Online textures  
- No EVE Online intellectual property
- All models are original procedural creations

**Inspiration vs. Copying**:
- We draw inspiration from EVE's design language
- Our models are geometric approximations
- Think "spirit of" rather than "copy of"
- Similar to how many space games have "frigate" designs

**Fair Use and Fan Projects**:
- This is a non-commercial fan project
- We reference EVE for educational/inspirational purposes
- Not trying to replace or compete with EVE Online
- Clear distinction maintained in all documentation

## Conclusion

The procedural modeling system in EVE OFFLINE creates ship models that evoke EVE Online's iconic designs through:

1. **Faction color schemes** matching EVE's racial aesthetics
2. **Scale relationships** preserving dramatic size differences
3. **Shape characteristics** capturing class-specific profiles
4. **Design language** reflecting each faction's style
5. **Procedural variation** allowing infinite ship variants

While not perfect replicas, these models capture the feel of commanding an EVE Online fleet in a standalone experience.

---

*Note: All references to EVE Online, ship names, and faction names are property of CCP Games. This document is for educational and reference purposes only.*
