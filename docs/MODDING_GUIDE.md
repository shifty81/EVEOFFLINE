# EVE OFFLINE - Modding Guide

Welcome to the EVE OFFLINE modding guide! This document will teach you how to create and modify game content using JSON files.

## Table of Contents
1. [Introduction](#introduction)
2. [Getting Started](#getting-started)
3. [Ship Modding](#ship-modding)
4. [Module Modding](#module-modding)
5. [Mission Creation](#mission-creation)
6. [Skill Customization](#skill-customization)
7. [Balance Guidelines](#balance-guidelines)
8. [Testing Your Mods](#testing-your-mods)
9. [Troubleshooting](#troubleshooting)

---

## Introduction

EVE OFFLINE is fully data-driven, meaning all game content is defined in JSON files located in the `data/` directory. You can modify these files to:
- Create new ships
- Add or modify modules
- Design custom missions
- Adjust skills and progression
- Tune game balance

**Important**: Always backup files before modifying them!

---

## Getting Started

### File Structure

```
data/
├── ships/              # Ship definitions
│   ├── frigates.json
│   ├── cruisers.json
│   ├── battleships.json
│   └── ...
├── modules/            # Module definitions
│   ├── weapons.json
│   ├── shields.json
│   └── ...
├── skills/             # Skill definitions
│   └── skills.json
├── missions/           # Mission templates
│   └── missions.json
├── npcs/               # NPC definitions
│   └── npcs.json
└── universe/           # Solar system data
    └── systems.json
```

### JSON Basics

JSON files use key-value pairs:
```json
{
  "ship_name": {
    "property": "value",
    "number_property": 123,
    "nested_object": {
      "nested_property": "nested_value"
    }
  }
}
```

**Rules**:
- Use double quotes `"` for strings
- No trailing commas
- Numbers don't need quotes
- Boolean values: `true` or `false`
- **No comments allowed** - JSON doesn't support `//` or `/* */` comments

**Note**: The examples in this guide show field descriptions separately for clarity. When creating your own JSON files, do not include inline comments - they will cause parse errors.

---

## Ship Modding

### Ship File Location
Ships are organized by class in `data/ships/`:
- `frigates.json` - Frigates and Tech II frigates
- `cruisers.json` - Cruisers  
- `tech2_cruisers.json` - HACs, HICs, Recons, Logistics
- `battlecruisers.json` - Battlecruisers
- `battleships.json` - Battleships
- `capitals.json` - Carriers, Dreadnoughts, Titans
- `mining_barges.json` - Mining barges
- `exhumers.json` - Exhumers

### Ship Template

Here's a complete ship definition with explanations:

```json
{
  "my_custom_ship": {
    "id": "my_custom_ship",
    "name": "My Custom Ship",
    "class": "Frigate",
    "race": "Minmatar",
    "description": "A custom ship with unique characteristics.",
    "hull_hp": 350,
    "armor_hp": 400,
    "shield_hp": 450,
    "capacitor": 280,
    "cpu": 140,
    "powergrid": 38,
    "high_slots": 3,
    "mid_slots": 3,
    "low_slots": 3,
    "rig_slots": 3,
    "cargo_capacity": 120,
    "max_velocity": 325,
    "inertia_modifier": 2.8,
    "signature_radius": 37,
    "scan_resolution": 620,
    "max_locked_targets": 4,
    "max_targeting_range": 18000,
    "shield_recharge_time": 625,
    "capacitor_recharge_time": 157,
    "bonuses": {
      "small_projectile_damage": 5,
      "small_projectile_falloff": 10,
      "tracking_speed": 7.5
    },
    "resistances": {
      "shield": {"em": 0, "thermal": 20, "kinetic": 40, "explosive": 50},
      "armor": {"em": 60, "thermal": 35, "kinetic": 25, "explosive": 10},
      "hull": {"em": 33, "thermal": 33, "kinetic": 33, "explosive": 33}
    }
  }
}
```

**Field Descriptions:**
- `hull_hp` - Structure HP
- `armor_hp` - Armor HP  
- `shield_hp` - Shield HP
- `capacitor` - Capacitor capacity (GJ)
- `cpu` - CPU (tf)
- `powergrid` - PowerGrid (MW)
- `high_slots` - Weapon/utility slots
- `mid_slots` - Shield/EWAR slots
- `low_slots` - Armor/damage mods
- `rig_slots` - Rig slots
- `cargo_capacity` - Cargo hold size (m³)
- `max_velocity` - Maximum speed (m/s)
- `inertia_modifier` - Agility (lower = more agile)
- `signature_radius` - Target size (m, smaller = harder to hit)
- `scan_resolution` - Lock speed (mm, higher = faster lock)
- `max_locked_targets` - Maximum simultaneous targets
- `max_targeting_range` - Maximum target distance (m)
- `shield_recharge_time` - Shield recharge time (seconds)
- `capacitor_recharge_time` - Capacitor recharge time (seconds)
- `bonuses` - Ship bonuses per skill level (% increase)
- `resistances` - Damage resistances by layer and type (%)

### Ship Classes

Available ship classes:
- `Frigate` - Small, fast, low HP
- `Destroyer` - Anti-frigate, high DPS
- `Cruiser` - Medium size, balanced
- `Battlecruiser` - Heavy cruiser
- `Battleship` - Large, high HP, slow
- `Carrier` - Capital ship, fighter support
- `Dreadnought` - Capital ship, siege mode
- `Titan` - Supercapital, massive
- `Mining Barge` - Mining focused
- `Exhumer` - Advanced mining

### Available Races
- `Minmatar` - Projectile weapons, fast, armor tank
- `Caldari` - Hybrid weapons, shield tank
- `Gallente` - Hybrid/drones, armor tank
- `Amarr` - Energy weapons, armor tank

### Ship Bonus Types

Common bonus types you can use:
- Weapon bonuses: `small_projectile_damage`, `medium_hybrid_damage`, `large_energy_damage`
- Range bonuses: `small_projectile_falloff`, `medium_hybrid_optimal`
- Defense bonuses: `shield_hp`, `armor_hp`, `shield_recharge_rate`
- Speed bonuses: `max_velocity`, `agility`
- Targeting bonuses: `targeting_range`, `scan_resolution`
- Capacitor bonuses: `capacitor_capacity`, `capacitor_recharge`

### Adding a New Ship

1. Open the appropriate JSON file (e.g., `frigates.json`)
2. Add your ship entry after the last existing ship
3. Ensure proper JSON syntax (commas between entries)
4. Test the ship in-game

**Example - Adding a custom frigate:**

```json
{
  "existing_ship": {
    ...existing ship data...
  },
  "lightning_strike": {
    "id": "lightning_strike",
    "name": "Lightning Strike",
    "class": "Frigate",
    "race": "Caldari",
    "description": "An experimental frigate designed for hit-and-run tactics.",
    "hull_hp": 320,
    "armor_hp": 350,
    "shield_hp": 800,
    "capacitor": 300,
    "cpu": 150,
    "powergrid": 35,
    "high_slots": 4,
    "mid_slots": 4,
    "low_slots": 2,
    "rig_slots": 3,
    "cargo_capacity": 100,
    "max_velocity": 380,
    "inertia_modifier": 2.5,
    "signature_radius": 32,
    "scan_resolution": 650,
    "max_locked_targets": 5,
    "max_targeting_range": 30000,
    "shield_recharge_time": 500,
    "capacitor_recharge_time": 140,
    "bonuses": {
      "small_hybrid_damage": 7,
      "small_hybrid_optimal": 10,
      "shield_recharge_rate": 5
    },
    "resistances": {
      "shield": {"em": 20, "thermal": 40, "kinetic": 60, "explosive": 50},
      "armor": {"em": 50, "thermal": 45, "kinetic": 25, "explosive": 10},
      "hull": {"em": 33, "thermal": 33, "kinetic": 33, "explosive": 33}
    }
  }
}
```

---

## Module Modding

### Module File Location
Modules are in `data/modules/`:
- `weapons.json` - Turrets, launchers
- `shields.json` - Shield modules
- `armor.json` - Armor modules
- `propulsion.json` - Afterburners, MWDs
- `ewar.json` - Electronic warfare
- `drones.json` - Drone definitions

### Module Template

```json
{
  "custom_weapon": {
    "id": "custom_weapon",
    "name": "Custom Weapon I",
    "type": "Weapon",
    "slot": "high",
    "size": "small",
    "cpu": 12,
    "powergrid": 4,
    "capacitor_use": 3.5,
    "activation_time": 5.0,
    "damage": 25,
    "damage_type": "kinetic",
    "optimal_range": 5000,
    "falloff_range": 8000,
    "tracking": 0.5,
    "rate_of_fire": 5.0,
    "description": "A custom weapon with unique characteristics."
  }
}
```

### Module Properties

**Common Properties:**
- `id` - Unique identifier
- `name` - Display name
- `type` - Module type (Weapon, Shield, Armor, etc.)
- `slot` - Fitting slot: `high`, `mid`, `low`, `rig`
- `size` - Size class: `small`, `medium`, `large`, `capital`, `extra_large`
- `cpu` - CPU requirement (tf)
- `powergrid` - PowerGrid requirement (MW)
- `description` - Description text

**Weapon-Specific:**
- `capacitor_use` - Cap per activation (GJ)
- `activation_time` - Cycle time (seconds)
- `damage` - Base damage
- `damage_type` - `em`, `thermal`, `kinetic`, or `explosive`
- `optimal_range` - Optimal range (m)
- `falloff_range` - Falloff range (m)
- `tracking` - Tracking speed
- `rate_of_fire` - Seconds between shots

**Defense-Specific:**
- `shield_boost` - Shield HP restored
- `armor_repair` - Armor HP restored
- `resistance_bonus` - Resistance % increase
- `shield_hp_bonus` - Shield HP % increase

---

## Mission Creation

### Mission File Location
Missions are in `data/missions/missions.json`

### Mission Template

```json
{
  "custom_mission_1": {
    "id": "custom_mission_1",
    "name": "Custom Mission",
    "level": 1,
    "type": "combat",
    "agent": "Agent Name",
    "corporation": "Corporation Name",
    "description": "Mission briefing text.",
    "objectives": [
      "Destroy hostile ships",
      "Return to station"
    ],
    "location": "Asteroid Belt",
    "enemies": [
      {"type": "npc_frigate", "count": 3},
      {"type": "npc_destroyer", "count": 1}
    ],
    "rewards": {
      "isk": 50000,
      "loyalty_points": 250,
      "items": [
        {"id": "item_name", "quantity": 1}
      ]
    },
    "time_limit": 3600
  }
}
```

### Mission Types
- `combat` - Kill all enemies
- `mining` - Mine specific ore
- `courier` - Transport items
- `trade` - Buy/sell items
- `exploration` - Scan signatures
- `scenario` - Scripted events
- `storyline` - Story missions

### Mission Levels
- Level 1: Easy (frigates)
- Level 2: Moderate (destroyers/cruisers)
- Level 3: Hard (cruisers/battlecruisers)
- Level 4: Very Hard (battleships)
- Level 5: Extreme (capital ships)

---

## Skill Customization

### Skill File Location
Skills are in `data/skills/skills.json`

### Skill Template

```json
{
  "custom_skill": {
    "id": "custom_skill",
    "name": "Custom Skill",
    "category": "Combat",
    "description": "A custom skill for testing.",
    "rank": 1,
    "primary_attribute": "perception",
    "secondary_attribute": "willpower",
    "prerequisites": {
      "gunnery": 3
    },
    "bonuses": {
      "small_turret_damage": 5
    }
  }
}
```

### Skill Properties

- `rank` - Training time multiplier (1-16, most skills are 1-5)
- `primary_attribute` - Main attribute (perception, willpower, intelligence, charisma, memory)
- `secondary_attribute` - Secondary attribute
- `prerequisites` - Required skills: `{"skill_id": level}`
- `bonuses` - What the skill improves

### Skill Categories
- `Combat` - Weapon skills
- `Defense` - Shield/armor skills
- `Navigation` - Speed/agility
- `Engineering` - Cap/CPU/PG
- `Electronics` - Targeting/EWAR
- `Drones` - Drone operation
- `Leadership` - Fleet bonuses
- `Industry` - Manufacturing/mining
- `Trade` - Market skills
- `Science` - Research/invention

---

## Balance Guidelines

### General Principles

1. **More powerful = More expensive**
   - Higher CPU/PG requirements
   - Longer skill training
   - Higher ISK cost

2. **Trade-offs are good**
   - High damage → Low tank
   - High speed → Low HP
   - Long range → Low DPS

3. **Stay within class bounds**
   - Frigates: 300-500 HP, 250-400 m/s
   - Cruisers: 1000-2000 HP, 200-300 m/s
   - Battleships: 5000-10000 HP, 100-150 m/s

### Ship Balance Checklist

- [ ] HP pools are appropriate for class
- [ ] Speed is reasonable for size
- [ ] CPU/PG can fit expected modules
- [ ] Bonuses match intended role
- [ ] Resistances are balanced (no immunity)
- [ ] Slot layout supports role

### Module Balance Checklist

- [ ] CPU/PG requirements scale with power
- [ ] Damage is comparable to similar modules
- [ ] Range isn't excessive for class
- [ ] Capacitor use is sustainable
- [ ] Activation time is reasonable

---

## Testing Your Mods

### Step 1: Validate JSON
Use a JSON validator to check syntax:
- [jsonlint.com](https://jsonlint.com)
- VS Code with JSON extension
- `jq` command line tool

### Step 2: Start the Game
```bash
# Build and run
./build.sh
cd build/bin
./eve_client "YourName"
```

### Step 3: Test In-Game
1. Create a character
2. Check if your ship/module appears
3. Try fitting and using it
4. Check for crashes or errors
5. Verify balance feels right

### Step 4: Check Logs
Look for errors in:
- Console output
- Game logs
- Crash reports

---

## Troubleshooting

### Common Issues

**Problem**: Ship doesn't appear in-game
- **Solution**: Check JSON syntax, ensure proper commas
- **Solution**: Verify the ship ID is unique
- **Solution**: Check class/race spelling

**Problem**: Game crashes when selecting ship
- **Solution**: Check all required fields are present
- **Solution**: Verify numeric values are valid
- **Solution**: Check bonus types are recognized

**Problem**: Module can't be fitted
- **Solution**: Check slot type matches ship slots
- **Solution**: Verify CPU/PG requirements are reasonable
- **Solution**: Check size matches ship class

**Problem**: JSON parse error
- **Solution**: Run through JSON validator
- **Solution**: Check for trailing commas
- **Solution**: Ensure all strings use double quotes

### Getting Help

1. Check existing JSON files for examples
2. Read error messages carefully
3. Test changes incrementally
4. Backup before making changes
5. Ask in community forums/Discord

---

## Advanced Modding

### Creating a Total Conversion Mod

1. Create a new data folder structure
2. Replace all JSON files with your content
3. Update references and IDs
4. Test thoroughly
5. Share with the community!

### Mod Organization

Organize your mods:
```
my_mod/
├── README.md          # Mod description
├── ships/             # Modified ships
├── modules/           # Modified modules
├── missions/          # New missions
└── installation.md    # Installation instructions
```

### Sharing Your Mods

1. Document what you changed
2. Include installation instructions
3. Test with a fresh install
4. Share on GitHub or forums
5. Accept feedback and iterate!

---

## Resources

### Official Documentation
- [Project README](../README.md)
- [Design Document](design/DESIGN.md)
- [EVE Manual Reference](EVE_MANUAL_REFERENCE.md)

### JSON Tools
- [JSONLint](https://jsonlint.com) - Validate JSON
- [VS Code](https://code.visualstudio.com) - Code editor with JSON support
- [jq](https://stedolan.github.io/jq/) - Command-line JSON processor

### Community
- GitHub Issues - Report bugs
- GitHub Discussions - Ask questions
- Discord - Chat with modders

---

## Example Mods

### Example 1: Fast Frigate

A high-speed frigate with lower tank:

```json
{
  "interceptor_custom": {
    "id": "interceptor_custom",
    "name": "Speed Demon",
    "class": "Frigate",
    "race": "Minmatar",
    "description": "The fastest ship in its class.",
    "hull_hp": 250,
    "armor_hp": 300,
    "shield_hp": 350,
    "capacitor": 250,
    "cpu": 130,
    "powergrid": 32,
    "high_slots": 3,
    "mid_slots": 3,
    "low_slots": 3,
    "rig_slots": 3,
    "cargo_capacity": 80,
    "max_velocity": 450,
    "inertia_modifier": 2.0,
    "signature_radius": 28,
    "scan_resolution": 700,
    "max_locked_targets": 5,
    "max_targeting_range": 35000,
    "shield_recharge_time": 700,
    "capacitor_recharge_time": 150,
    "bonuses": {
      "max_velocity": 10,
      "agility": 15,
      "targeting_range": 20
    },
    "resistances": {
      "shield": {"em": 0, "thermal": 15, "kinetic": 35, "explosive": 45},
      "armor": {"em": 55, "thermal": 30, "kinetic": 20, "explosive": 5},
      "hull": {"em": 33, "thermal": 33, "kinetic": 33, "explosive": 33}
    }
  }
}
```

### Example 2: Tank Cruiser

A durable cruiser with lower speed:

```json
{
  "fortress_cruiser": {
    "id": "fortress_cruiser",
    "name": "Fortress",
    "class": "Cruiser",
    "race": "Amarr",
    "description": "Built to withstand punishment.",
    "hull_hp": 1800,
    "armor_hp": 2400,
    "shield_hp": 1600,
    "capacitor": 1800,
    "cpu": 400,
    "powergrid": 130,
    "high_slots": 4,
    "mid_slots": 3,
    "low_slots": 6,
    "rig_slots": 3,
    "cargo_capacity": 300,
    "max_velocity": 180,
    "inertia_modifier": 0.4,
    "signature_radius": 135,
    "scan_resolution": 250,
    "max_locked_targets": 5,
    "max_targeting_range": 60000,
    "shield_recharge_time": 1200,
    "capacitor_recharge_time": 380,
    "bonuses": {
      "armor_hp": 5,
      "armor_resistances": 4,
      "capacitor_capacity": 5
    },
    "resistances": {
      "shield": {"em": 0, "thermal": 20, "kinetic": 40, "explosive": 50},
      "armor": {"em": 70, "thermal": 45, "kinetic": 35, "explosive": 20},
      "hull": {"em": 33, "thermal": 33, "kinetic": 33, "explosive": 33}
    }
  }
}
```

---

## Conclusion

Modding EVE OFFLINE is designed to be accessible and powerful. Start small, test frequently, and don't be afraid to experiment!

**Remember**:
- Always backup files before modifying
- Test changes incrementally  
- Share your creations with the community
- Have fun!

Happy modding! 🚀

---

*Last Updated: February 7, 2026*
*For questions or help, open a GitHub issue or join the Discord.*
