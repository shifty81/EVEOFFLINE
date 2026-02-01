# EVE OFFLINE - Gameplay Demos

This document describes the interactive gameplay demonstrations available in EVE OFFLINE.

## 🎮 Interactive Demo (`interactive_demo.py`)

An interactive, menu-driven interface that allows you to explore all game features hands-on.

### Features Demonstrated

1. **Ship Status** - View complete ship information including:
   - Position in space
   - Shield/Armor/Hull HP
   - Capacitor energy
   - CPU and PowerGrid usage
   - Active drones

2. **Ship Fitting** - Fit modules to your ship:
   - High slot weapons (railguns, autocannons)
   - Mid slot modules (shield boosters, afterburners)
   - Low slot modules (damage controls, stabilizers)
   - Resource management (CPU/PowerGrid)

3. **Drone Operations** - Full drone control:
   - Launch and recall drones
   - Bandwidth management
   - Target engagement
   - Drone AI

4. **Combat System** - Engage enemies:
   - Target locking
   - Weapon firing
   - Damage application
   - Distance and range management

5. **Skills** - Character progression:
   - View trained skills
   - Skill bonuses
   - Skill point (SP) tracking

6. **Navigation** - Space travel:
   - Warp to locations
   - Movement commands
   - Distance calculations

7. **Missions** - Mission running:
   - Accept missions
   - Mission objectives
   - Rewards

8. **Scanner** - Directional scanning:
   - Detect nearby objects
   - Enemy ships
   - Stations and celestials

### How to Use

```bash
python interactive_demo.py
```

Follow the on-screen menu:
- Type `1` to view ship status
- Type `2` to fit modules
- Type `3` for drone operations
- Type `4` for combat
- Type `5` to view skills
- Type `6` for navigation
- Type `7` for missions
- Type `8` to scan
- Type `9` for help
- Type `0` to exit

## 📺 Gameplay Showcase (`showcase_gameplay.py`)

An automated demonstration that showcases all features in a scripted format. Perfect for:
- Taking screenshots
- Recording videos
- Quick feature overview
- Testing all systems

### What it Shows

1. **Initial Ship Status** - Starting configuration
2. **Sector Scanning** - Nearby objects and enemies
3. **Character Skills** - Trained skills with bonuses
4. **Drone Operations** - Launching and engaging
5. **Combat Engagement** - Target locking and firing
6. **Ship Fitting** - Module slots and resources
7. **Navigation** - Warp capabilities and distances
8. **Feature Summary** - Complete list of implemented features

### How to Use

```bash
python showcase_gameplay.py
```

The script runs automatically and displays all features in sequence.

## 🎯 Gameplay Screenshot

![EVE OFFLINE Gameplay](https://github.com/user-attachments/assets/1eae502f-875d-4fd0-a7e1-2dd3b1b60244)

The screenshot above shows the gameplay interface with all major features demonstrated:
- Ship status display with Shield/Armor/Hull HP
- Directional scanner showing nearby enemies and stations
- Character skills with progression (Gunnery Level 4, Drones Level 5)
- Drone operations with bandwidth management
- Combat engagement with multiple Serpentis enemies
- Ship fitting interface with CPU/PowerGrid tracking
- Navigation system with warp capabilities
- Complete feature checklist

## 🎮 Game Features Available

### ✅ Fully Implemented

- **Character System**
  - Skills and skill training
  - Skill bonuses (damage, HP, etc.)
  - Skill point progression

- **Ship Systems**
  - Ship fitting with slots (high/mid/low/rig)
  - CPU and PowerGrid management
  - Multiple ship classes (Frigate, Cruiser)
  - Shield/Armor/Hull defense layers

- **Drone System**
  - Drone bay with capacity
  - Bandwidth management
  - Launch/recall operations
  - Drone AI and combat

- **Combat System**
  - Target locking
  - Weapon systems
  - Damage types and resistances
  - Range mechanics

- **Navigation**
  - Warp drive system
  - Celestial objects (stations, gates)
  - Distance calculations
  - Movement commands

- **Mission System**
  - Mission acceptance
  - Objective tracking
  - Reward distribution

- **Scanning**
  - Directional scanner
  - Object detection
  - Distance measurement

## 🚀 Next Steps

### Planned Enhancements
- 2D/3D graphical rendering
- Advanced UI with HUD
- More mission types
- Market system
- Inventory management
- Fleet mechanics

## 💡 Tips for Players

1. **Start with the Interactive Demo** - It's the best way to learn the game
2. **Check Your Ship Status** - Keep an eye on shield/armor/hull
3. **Manage Your Drones** - Don't exceed bandwidth limits
4. **Fit Your Ship Wisely** - Balance CPU and PowerGrid usage
5. **Use the Scanner** - Know your surroundings before engaging
6. **Train Skills** - They provide important bonuses
7. **Watch Your Capacitor** - Don't run out of energy

## 📝 Development Notes

The demos were created to showcase the full EVE OFFLINE experience:
- Built on custom ECS (Entity Component System) engine
- Data-driven architecture with JSON files
- Server-authoritative multiplayer support
- Full EVE Online-inspired mechanics
- 100% moddable content

## 🎯 For Developers

Both demos use the same underlying systems:
- `engine/core/ecs.py` - Entity Component System
- `engine/components/game_components.py` - All game components
- `engine/systems/game_systems.py` - Core game systems
- `engine/systems/mission_system.py` - Mission management
- `engine/systems/navigation_system.py` - Navigation and warp
- `engine/utils/data_loader.py` - JSON data loading

The interactive demo (`interactive_demo.py`) provides a command-line interface while the showcase (`showcase_gameplay.py`) runs a scripted demonstration. Both can be used as references for implementing new features or creating custom game modes.

---

**EVE OFFLINE** - A PVE-focused space MMO inspired by EVE ONLINE
Built with ❤️ using Python and custom ECS architecture
