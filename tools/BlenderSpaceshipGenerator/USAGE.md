# Installation and Usage Guide

## Quick Start

### Installation Steps

1. **Download the Addon**
   - Clone or download this repository
   - If downloaded as ZIP, extract it to a known location

2. **Install in Blender**
   - Open Blender (version 2.80 or higher)
   - Go to `Edit` → `Preferences`
   - Select the `Add-ons` tab
   - Click the `Install...` button
   - Navigate to the BlenderSpaceshipGenerator folder
   - Select the folder and click `Install Add-on`

3. **Enable the Addon**
   - Search for "Spaceship Generator" in the add-ons list
   - Check the box next to the addon name to enable it
   - The addon is now ready to use!

### First Ship Generation

1. **Open the Panel**
   - In the 3D Viewport, press `N` to open the sidebar
   - Click on the `Spaceship` tab
   - You'll see the Spaceship Generator panel

2. **Basic Configuration**
   - Select a ship class (try "Fighter" for your first ship)
   - Leave other settings at default
   - Click `Generate Spaceship`

3. **View Your Ship**
   - The ship will appear at the 3D cursor location
   - Use middle mouse button to rotate view
   - Scroll to zoom in/out
   - Your ship is organized in a collection with all parts

## Detailed Usage

### Understanding Ship Classes

Each ship class has predefined characteristics:

**Shuttle** - Perfect for beginners
- Small, simple design
- 2 crew capacity
- Basic cockpit interior
- Ideal for learning the addon

**Fighter** - Classic combat ship
- Single-seat design
- Wings for agility
- Weapon hardpoints
- Compact but detailed

**Corvette** - Small crew ship
- 4 crew members
- Basic interior with quarters
- Multiple engines
- Good balance of features

**Frigate** - Medium vessel
- 10 crew capacity
- Full corridor system
- Multiple rooms
- Weapon systems

**Destroyer** - Heavy combat
- 25 crew
- Extensive interior
- Heavy weapons
- Larger scale

**Cruiser** - Large multi-role
- 50 crew
- Complex interior layout
- Bridge and multiple decks
- Module capacity

**Battleship** - Heavy capital
- 100 crew
- Maximum weapons
- Large scale
- Command capabilities

**Carrier** - Fleet support
- 200 crew
- Hangar facilities
- Support systems
- Very large

**Capital** - Largest class
- 500 crew capacity
- Maximum everything
- City-sized vessel
- Ultimate ship

### Configuration Options

**Ship Class**
- Choose from 18 ship sizes (Shuttle through Titan plus utility/NMS variants)
- Affects scale, complexity, and features

**Style**
- Mixed: Balanced design from all games
- X4: Angular, industrial, geometric
- Elite Dangerous: Sleek, aerodynamic
- Eve Online: Organic, flowing curves
- EVEOFFLINE Factions: Solari, Veyren, Aurelian, Keldari
- NMS: Colorful, varied, organic

**Random Seed**
- Number from 1 to infinity
- Same seed = same ship design
- Change for variations

**Generate Interior**
- Enabled: Full interior with rooms
- Disabled: Exterior only (faster generation)

**Module Slots**
- 0-10 additional modules
- Larger ships support more modules
- Adds cargo, weapons, shields, etc.

**Hull Complexity**
- 0.1 to 3.0 range
- Lower: Simple, clean geometry
- Higher: More detailed, complex shapes
- Affects generation time

**Symmetry**
- Enabled: Mirrored left/right design
- Disabled: Asymmetric, unique look
- Most fighters use symmetry

**Hull Taper**
- 0.5 to 1.0 range
- Lower: Stronger tapered silhouette (nose/tail narrowing)
- 1.0: No taper (original boxy look)
- Default 0.85 gives a subtle spaceship feel

### Ship DNA

Every generated ship stores its Ship DNA as a JSON custom property on the
hull object.  This lets you:
- **Reproduce** the exact same ship from a seed + brick list
- **Export** the DNA to a `.json` file (Ship DNA Export in the panel)
- **Share** ship designs or feed them into game engines
- **Destroy & salvage** individual bricks in gameplay

Ship DNA records every placed brick type and position so ships become
assemblies rather than monolithic meshes.

### Engine Archetypes

Engines now vary by role instead of being identical cylinders:
- **Main Thrust** — large, deep, with a cone-shaped nozzle flare and strong
  blue glow. Sells scale.
- **Maneuvering** — small thrusters for attitude control, subtle glow.
- **Utility Exhaust** — flat vents for auxiliary systems.

## Advanced Tips

### Creating Ship Variations

Generate multiple ships with different seeds:
1. Set ship class to "Fighter"
2. Generate with seed 1
3. Move the ship aside
4. Change seed to 2
5. Generate again
6. Compare designs

### Customizing After Generation

The generated ship is fully editable:
- Select individual parts in the collection
- Move, rotate, scale parts
- Add your own details
- Apply materials and textures
- Parent new objects to the hull

### Performance Tips

For faster generation:
- Disable interior for exterior-only work
- Use lower hull complexity (0.5-1.0)
- Fewer module slots
- Smaller ship classes

### Interior Exploration

To explore ship interiors in FPV:
1. Generate ship with interior enabled
2. Select any interior object
3. Press `/` to enter local view
4. Press `Shift + F` to enter walk mode
5. Use WASD to move, mouse to look
6. Press `Esc` to exit walk mode

All interiors are human-scaled (1.8m tall) for realistic FPV.

## Examples

### Example 1: Small Fighter Squadron

```
Ship Class: Fighter
Style: Elite Dangerous
Seeds: 1, 2, 3, 4, 5
Interior: Disabled
Modules: 0
Symmetry: Enabled
```

Creates a squadron of similar but varied fighters.

### Example 2: Capital Ship with Full Interior

```
Ship Class: Capital
Style: Mixed
Seed: 42
Interior: Enabled
Modules: 10
Hull Complexity: 2.0
Symmetry: Enabled
```

Creates a massive capital ship with full interior for exploration.

### Example 3: Asymmetric Cargo Hauler

```
Ship Class: Frigate
Style: X4
Seed: 100
Interior: Enabled
Modules: 8 (will be mostly cargo)
Hull Complexity: 1.0
Symmetry: Disabled
```

Creates a unique industrial cargo vessel.

## Troubleshooting

**Addon doesn't appear after installation**
- Make sure you enabled it in preferences
- Check for Python errors in System Console (Window → Toggle System Console)
- Verify Blender version is 2.80+

**Generation is slow**
- Reduce hull complexity
- Disable interior generation
- Use smaller ship classes
- Fewer module slots

**Ship appears at wrong location**
- Ship generates at 3D cursor position
- Press Shift+S → Cursor to World Origin
- Then generate ship

**Parts seem disconnected**
- All parts are parented to hull
- Select hull and press Alt+G to reset position
- Check that parenting is intact

**Interior is too small/large**
- Interior uses standard human scale (1.8m)
- Check your Blender unit settings
- Scene → Unit Scale should be 1.0

## Next Steps

- Experiment with different ship classes
- Try all design styles
- Explore generated interiors in walk mode
- Add your own custom materials
- Create a fleet of ships
- Export for use in games or renders

For more help, check the GitHub repository issues page.
