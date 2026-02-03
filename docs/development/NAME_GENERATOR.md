# Random Name Generator Integration Guide

## Overview

The EVE OFFLINE name generator provides procedurally generated names for various game elements, similar to EVE Online's naming conventions but with unique variations.

## Usage in Python

### Import

```python
from engine.utils.name_generator import NameGenerator, random_character_name
```

### Character Names

```python
# Generate character name (random gender)
name = NameGenerator.generate_character_name()
# Example: "Drake Voidwalker"

# Specify gender
male_name = NameGenerator.generate_character_name('male')
female_name = NameGenerator.generate_character_name('female')
```

### Ship Names

```python
# Random style
ship_name = NameGenerator.generate_ship_name()
# Example: "INS Thunderbolt"

# Specific styles
heroic = NameGenerator.generate_ship_name('heroic')      # "USS Dauntless"
celestial = NameGenerator.generate_ship_name('celestial')  # "RSS Andromeda"
mythic = NameGenerator.generate_ship_name('mythic')       # "CSS Hyperion"
descriptive = NameGenerator.generate_ship_name('descriptive')  # "GSS Maelstrom"
```

### Corporation Names

```python
corp_name = NameGenerator.generate_corporation_name()
# Example: "Stellar Industries"
```

### System and Station Names

```python
# System name
system = NameGenerator.generate_system_name()
# Example: "Alpha Centauri Prime"

# Station name
station = NameGenerator.generate_station_name()
# Example: "Trade Hub"

# Station name based on system
station_in_system = NameGenerator.generate_station_name(system)
# Example: "Alpha Centauri Prime Gateway"
```

### Mission Names

```python
mission = NameGenerator.generate_mission_name()
# Example: "Deadly Convoy"
```

### Exploration Sites

```python
site = NameGenerator.generate_exploration_site_name()
# Example: "Ancient Hideout"
```

### NPC Names

```python
# Pirate name
pirate = NameGenerator.generate_pirate_name()
# Example: "Captain Blackheart"

# Pilot callsign
callsign = NameGenerator.generate_pilot_callsign()
# Example: "Viper-7"
```

### Resource Names

```python
asteroid = NameGenerator.generate_asteroid_designation()
# Example: "Veldspar-4472"
```

## Integration Examples

### Character Creation

```python
from engine.utils.name_generator import NameGenerator

def create_player_character(player_id, gender=None):
    """Create a new player character with generated name"""
    character_name = NameGenerator.generate_character_name(gender)
    
    # Create character entity
    character = world.create_entity()
    character.add_component(Name(character_name))
    character.add_component(Player(player_id))
    
    return character
```

### Ship Spawning

```python
def spawn_ship(ship_type, faction):
    """Spawn a ship with a generated name"""
    # Determine style based on faction
    style_map = {
        'Minmatar': 'descriptive',
        'Caldari': 'heroic',
        'Gallente': 'celestial',
        'Amarr': 'mythic'
    }
    
    ship_name = NameGenerator.generate_ship_name(style_map.get(faction))
    
    ship = world.create_entity()
    ship.add_component(Name(ship_name))
    ship.add_component(Ship(ship_type))
    
    return ship
```

### Mission Generation

```python
def generate_dynamic_mission(difficulty_level):
    """Generate a mission with random name"""
    mission_name = NameGenerator.generate_mission_name()
    
    mission = {
        'name': mission_name,
        'level': difficulty_level,
        'description': f'Complete the {mission_name} operation',
        'reward': difficulty_level * 100000  # ISK
    }
    
    return mission
```

### Exploration Site Discovery

```python
def spawn_exploration_site(system_id):
    """Spawn a new exploration site with generated name"""
    site_name = NameGenerator.generate_exploration_site_name()
    
    site = world.create_entity()
    site.add_component(Name(site_name))
    site.add_component(ExplorationSite(system_id))
    site.add_component(Position(
        x=random.uniform(-10000, 10000),
        y=random.uniform(-10000, 10000),
        z=random.uniform(-10000, 10000)
    ))
    
    return site
```

### NPC Fleet Generation

```python
def spawn_pirate_fleet(faction, count):
    """Spawn a pirate fleet with generated names"""
    leader_name = NameGenerator.generate_pirate_name()
    
    fleet = []
    
    # Create fleet leader
    leader = spawn_npc(faction, is_leader=True)
    leader.add_component(Name(leader_name))
    fleet.append(leader)
    
    # Create fleet members with callsigns
    for i in range(count - 1):
        callsign = NameGenerator.generate_pilot_callsign()
        member = spawn_npc(faction, is_leader=False)
        member.add_component(Name(f"{faction} {callsign}"))
        fleet.append(member)
    
    return fleet
```

### Universe Generation

```python
def generate_solar_system():
    """Generate a new solar system with stations"""
    system_name = NameGenerator.generate_system_name()
    
    # Create system
    system = {
        'name': system_name,
        'security': random.uniform(0.0, 1.0),
        'stations': []
    }
    
    # Add 2-5 stations
    num_stations = random.randint(2, 5)
    for _ in range(num_stations):
        station_name = NameGenerator.generate_station_name(system_name)
        station = {
            'name': station_name,
            'services': ['market', 'repair', 'reprocess']
        }
        system['stations'].append(station)
    
    return system
```

### Asteroid Belt Generation

```python
def generate_asteroid_belt(system_id, belt_index):
    """Generate an asteroid belt with named asteroids"""
    asteroids = []
    
    num_asteroids = random.randint(50, 200)
    for _ in range(num_asteroids):
        designation = NameGenerator.generate_asteroid_designation()
        
        asteroid = world.create_entity()
        asteroid.add_component(Name(designation))
        asteroid.add_component(Asteroid(ore_type='Veldspar'))
        asteroid.add_component(Position(
            x=random.uniform(-5000, 5000),
            y=random.uniform(-1000, 1000),
            z=random.uniform(-5000, 5000)
        ))
        
        asteroids.append(asteroid)
    
    return asteroids
```

## C++ Integration

### Basic Usage

```cpp
#include "utils/name_generator.h"

using namespace eve::utils;

// Create generator instance
NameGenerator gen;

// Generate names
std::string character = gen.generateCharacterName(true);  // Male
std::string ship = gen.generateShipName(NameGenerator::ShipStyle::Heroic);
std::string corp = gen.generateCorporationName();
std::string system = gen.generateSystemName();
std::string mission = gen.generateMissionName();
```

### Server Integration

```cpp
// In server initialization
class GameServer {
private:
    NameGenerator name_gen_;
    
public:
    void spawnNPC(const std::string& faction) {
        // Generate NPC name
        std::string npc_name = name_gen_.generatePirateName();
        
        // Create NPC entity
        auto npc = createEntity();
        npc->setName(npc_name);
        npc->setFaction(faction);
    }
    
    void createMission() {
        std::string mission_name = name_gen_.generateMissionName();
        // ... create mission with name
    }
};
```

## Testing

Run the test suite to verify name generation:

```bash
python test_name_generator.py
```

This will:
- Test all name generation functions
- Verify uniqueness
- Display example outputs

## Customization

To add your own name components:

1. Edit `engine/utils/name_generator.py`
2. Add new entries to the appropriate lists (e.g., `FIRST_NAMES_MALE`, `SHIP_NAMES_HEROIC`)
3. Reload the module

Example:

```python
# Add custom ship names
NameGenerator.SHIP_NAMES_HEROIC.extend([
    "Unstoppable", "Unbreakable", "Unyielding"
])

# Now these can be generated
ship = NameGenerator.generate_ship_name('heroic')
```

## Best Practices

1. **Cache Generated Names**: For important entities (player ships, corporations), store the generated name
2. **Consistency**: Use the same style for similar entity types
3. **Faction Theming**: Map factions to name styles for consistency
4. **Uniqueness Checking**: For player-facing names, check against existing names
5. **Localization**: Add support for multiple languages if needed

## Performance

The name generator is lightweight:
- **Python**: ~0.1ms per name
- **C++**: ~0.01ms per name
- **Memory**: Minimal (lists are pre-allocated)

Safe to use in hot paths like NPC spawning or mission generation.
