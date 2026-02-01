# EVE OFFLINE - Developer Documentation

## Table of Contents
1. [Getting Started](#getting-started)
2. [Architecture Overview](#architecture-overview)
3. [Running the Game](#running-the-game)
4. [Modding Guide](#modding-guide)
5. [API Reference](#api-reference)

## Getting Started

### Prerequisites
- Python 3.11 or higher
- pip (Python package manager)

### Installation

1. Clone the repository:
```bash
git clone https://github.com/shifty81/EVEOFFLINE.git
cd EVEOFFLINE
```

2. Create a virtual environment (recommended):
```bash
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate
```

3. Install dependencies:
```bash
pip install -r requirements.txt
```

### Quick Start

**Start the server:**
```bash
python server/server.py
```

**Start a client (in a new terminal):**
```bash
python client/client.py "YourCharacterName"
```

## Architecture Overview

### Entity Component System (ECS)

The game uses an ECS architecture inspired by modern game engines:

- **Entities**: Game objects (ships, NPCs, projectiles)
- **Components**: Data containers (Position, Health, Weapon)
- **Systems**: Logic processors (MovementSystem, CombatSystem)

**Example: Creating a Ship**
```python
from engine.core.ecs import World
from engine.components.game_components import Position, Health, Ship

world = World()
entity = world.create_entity()
entity.add_component(Position(x=0, y=0, z=0))
entity.add_component(Health(hull_hp=350, armor_hp=400, shield_hp=450))
entity.add_component(Ship(ship_name="Rifter", ship_class="Frigate"))
```

### Data-Driven Design

Following Astrox Imperium's approach, all game content is defined in JSON files:

```
data/
├── ships/          # Ship definitions
├── modules/        # Module definitions
├── skills/         # Skill definitions
├── npcs/           # NPC definitions
├── missions/       # Mission templates
└── universe/       # Solar system data
```

**Example Ship Definition** (`data/ships/frigates.json`):
```json
{
  "rifter": {
    "name": "Rifter",
    "hull_hp": 350,
    "shield_hp": 450,
    "max_velocity": 325,
    "high_slots": 3,
    "bonuses": {
      "small_projectile_damage": 5
    }
  }
}
```

### Network Architecture

**Server-Authoritative Model**:
- Server simulates complete world state
- Clients send input, receive state updates
- Prevents cheating, ensures consistency

**Message Flow**:
```
Client → INPUT → Server
Server → STATE_UPDATE → All Clients
Server → SPAWN_ENTITY → All Clients
Client → CHAT → Server → All Clients
```

## Running the Game

### Server Configuration

Edit `server/config.json` (if it exists) or pass arguments:
```python
from server.server import run_server
run_server(host="0.0.0.0", port=8765)
```

### Client Configuration

The client can be customized:
```python
from client.client import run_client
import asyncio

asyncio.run(run_client(
    player_id="your_id",
    character_name="YourName",
    host="localhost",
    port=8765
))
```

### Testing Multiplayer

1. Start server in one terminal
2. Start multiple clients in separate terminals:
```bash
python client/client.py "Pilot1"
python client/client.py "Pilot2"
python client/client.py "Pilot3"
```

## Modding Guide

### Adding a New Ship

1. Create ship definition in `data/ships/your_ships.json`:
```json
{
  "custom_frigate": {
    "id": "custom_frigate",
    "name": "Custom Frigate",
    "class": "Frigate",
    "hull_hp": 500,
    "armor_hp": 450,
    "shield_hp": 600,
    "max_velocity": 350,
    "high_slots": 4,
    "mid_slots": 3,
    "low_slots": 2,
    "bonuses": {
      "small_hybrid_damage": 10
    }
  }
}
```

2. The DataLoader will automatically load it on server start

### Adding a New Module

1. Create module in `data/modules/custom_modules.json`:
```json
{
  "mega_laser": {
    "id": "mega_laser",
    "name": "Mega Laser",
    "type": "weapon",
    "slot": "high",
    "cpu": 25,
    "powergrid": 15,
    "damage": 100,
    "optimal_range": 5000
  }
}
```

### Creating Custom Missions

1. Add mission to `data/missions/custom_missions.json`:
```json
{
  "my_mission": {
    "id": "my_mission",
    "name": "Custom Mission",
    "level": 1,
    "type": "combat",
    "objectives": [
      {
        "type": "destroy",
        "target": "serpentis_scout",
        "count": 10
      }
    ],
    "rewards": {
      "isk": 100000,
      "loyalty_points": 500
    }
  }
}
```

### Modding Best Practices

1. **Never modify core engine files** - only edit data files
2. **Use unique IDs** for your content to avoid conflicts
3. **Test balance** - ensure your additions are fun but fair
4. **Share your mods** - contribute back to the community!

## API Reference

### Core Classes

#### Engine
```python
class Engine:
    def __init__(self, target_fps: int = 60)
    def initialize(self)
    def run(self)
    def update(self, delta_time: float)
    def stop(self)
```

#### World
```python
class World:
    def create_entity(self, entity_id: Optional[str] = None) -> Entity
    def destroy_entity(self, entity_id: str)
    def get_entity(self, entity_id: str) -> Optional[Entity]
    def add_system(self, system: System)
    def update(self, delta_time: float)
    def get_entities(self, *component_types) -> List[Entity]
```

#### Entity
```python
class Entity:
    def add_component(self, component: Component) -> 'Entity'
    def remove_component(self, component_type: Type[Component]) -> 'Entity'
    def get_component(self, component_type: Type[Component]) -> Optional[Component]
    def has_component(self, component_type: Type[Component]) -> bool
```

### Game Components

#### Position
```python
@dataclass
class Position(Component):
    x: float = 0.0
    y: float = 0.0
    z: float = 0.0
    rotation: float = 0.0
```

#### Health
```python
@dataclass
class Health(Component):
    hull_hp: float
    armor_hp: float
    shield_hp: float
    # ... resistances, etc.
```

#### Ship
```python
@dataclass
class Ship(Component):
    ship_type: str
    ship_name: str
    race: str
    cpu_max: float
    powergrid_max: float
    # ... targeting, etc.
```

### Game Systems

#### MovementSystem
Handles entity movement and velocity

#### WeaponSystem
```python
class WeaponSystem(System):
    def fire_weapon(self, shooter_entity, target_entity_id: str) -> bool
```

#### AISystem
Controls NPC behavior (idle, approaching, orbiting, attacking)

#### TargetingSystem
```python
class TargetingSystem(System):
    def start_lock(self, entity, target_entity_id: str) -> bool
```

### Network Protocol

#### Message Types
- `CONNECT` / `DISCONNECT`: Connection management
- `STATE_UPDATE`: World state synchronization
- `INPUT_*`: Player input
- `SPAWN_ENTITY` / `DESTROY_ENTITY`: Entity lifecycle
- `CHAT`: Chat messages

#### Creating Messages
```python
from engine.network.protocol import create_message, MessageType

msg = create_message(MessageType.CHAT, {
    'sender': 'Player1',
    'message': 'Hello world!'
})
```

### Data Loader

```python
from engine.utils.data_loader import DataLoader

loader = DataLoader()
loader.load_all()

# Access data
ship_data = loader.get_ship('rifter')
module_data = loader.get_module('200mm_autocannon')
skill_data = loader.get_skill('gunnery')
```

## Performance Tips

1. **Server tick rate**: Default 30 Hz, adjust in `GameServer.__init__`
2. **State update frequency**: Every 3 ticks (10 Hz), adjust in `update_loop()`
3. **Interest management**: Future feature for large player counts
4. **Client-side prediction**: Implement for responsive local movement

## Troubleshooting

**Server won't start:**
- Check port 8765 is not in use
- Ensure data files are present in `data/` directory

**Client can't connect:**
- Verify server is running
- Check firewall settings
- Ensure host/port are correct

**Performance issues:**
- Reduce server tick rate
- Limit number of entities
- Use profiling to identify bottlenecks

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

[To be determined]

## Credits

- Inspired by EVE ONLINE (CCP Games)
- Modding approach inspired by Astrox Imperium
- Community contributors
