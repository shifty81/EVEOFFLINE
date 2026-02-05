# Player-Owned Structures System

## Overview

The player-owned structures system allows players and corporations to deploy, manage, and maintain structures in space. This system is based on EVE Online's 2026 Equinox expansion structure mechanics, featuring Citadels, Orbital Skyhooks, Sovereignty Hubs, and more.

## Components

### Structure Component

The main component for player-owned structures with the following features:

- **Structure Type**: astrahus, fortizar, keepstar, orbital_skyhook, sovereignty_hub, metenox_moon_drill
- **Ownership**: Player and/or corporation ownership
- **State Management**: anchoring, online, reinforced, unanchoring, destroyed
- **Health Pools**: Hull, armor, and shield with EVE-style HP values
- **Fitting System**: CPU, PowerGrid, and module slots (high/mid/low/rig)
- **Fuel Management**: Fuel bay with capacity and consumption rates
- **Services**: Activatable services (market, manufacturing, reprocessing, etc.)
- **Defense**: Reinforcement cycles and vulnerability windows

### StructureService Component

Component for individual services running on a structure:

- **Service Type**: market, manufacturing, reprocessing, research, cloning
- **Status**: Online/offline
- **Fuel Cost**: Per-hour fuel consumption
- **Access Control**: Player ID access lists
- **Bonuses**: Service-specific bonus percentages

## Structure Types

### Citadels

#### Astrahus (Medium Citadel)
- **Anchoring Time**: 24 hours
- **Hull HP**: 5,000,000
- **Shield HP**: 2,500,000
- **Armor HP**: 3,750,000
- **CPU**: 2,000
- **PowerGrid**: 1,000
- **Hangar Capacity**: 100,000 m³
- **Fuel Bay**: 20,000 m³
- **Fuel Usage**: 5 units/hour

### Nullsec Structures

#### Orbital Skyhook
- **Anchoring Time**: 24 hours
- **Purpose**: Passive planetary resource harvesting
- **Hull HP**: 3,000,000
- **Fuel Usage**: 3 units/hour
- **Special**: Can be "siphoned" by hostile players

#### Sovereignty Hub
- **Anchoring Time**: 48 hours
- **Purpose**: System control and upgrades
- **Hull HP**: 10,000,000
- **Fuel Usage**: 10 units/hour
- **Special**: Controls system anomalies and resource spawns

#### Metenox Moon Drill
- **Anchoring Time**: 12 hours
- **Purpose**: Automated moon mining
- **Hull HP**: 2,000,000
- **Fuel Usage**: 2 units/hour
- **Special**: Lower cost than Tatara, automated operation

## StructureSystem API

### Deployment

```python
structure_id = structure_system.deploy_structure(
    player_entity_id="player_123",
    structure_type="astrahus",
    structure_name="My Citadel",
    system_id="jita",
    x=1000.0, y=2000.0, z=3000.0
)
```

Deploys a new structure at the specified location. The structure starts in "anchoring" state.

### Fuel Management

```python
# Add fuel to structure
success = structure_system.add_fuel(
    structure_entity_id=structure_id,
    fuel_type="fuel_blocks",
    amount=1000
)
```

Fuel is consumed automatically based on the structure's fuel_usage_rate. Structures go offline if fuel runs out.

### Service Activation

```python
# Activate a service
success = structure_system.activate_service(
    structure_entity_id=structure_id,
    service_type="market"
)
```

Services can only be activated when the structure is online.

### Structure Info

```python
# Get structure information
info = structure_system.get_structure_info(structure_entity_id=structure_id)

# Returns:
# {
#     "entity_id": "structure_123",
#     "name": "My Citadel",
#     "type": "astrahus",
#     "state": "online",
#     "owner_corp": "corp_456",
#     "owner_player": "player_123",
#     "position": (1000.0, 2000.0, 3000.0),
#     "hull_hp": 5000000.0,
#     "hull_max": 5000000.0,
#     "shield_hp": 2500000.0,
#     "shield_max": 2500000.0,
#     "armor_hp": 3750000.0,
#     "armor_max": 3750000.0,
#     "fuel": 1000,
#     "fuel_capacity": 20000.0,
#     "services": {"market": True},
#     "anchoring_time_remaining": 0
# }
```

### Unanchoring

```python
# Begin unanchoring process
success = structure_system.unanchor_structure(
    structure_entity_id=structure_id,
    player_entity_id=player_id
)
```

Only owners or corporation directors can unanchor structures. Unanchoring takes 24 hours.

### Location Queries

```python
# Get all structures in a system
structures = structure_system.get_structures_in_system("jita")
```

### Structure Destruction

```python
# Destroy structure with loot dropping (combat scenario)
structure_system.destroy_structure(
    structure_entity_id=structure_id,
    drop_loot=True  # Creates loot container with hangar contents
)

# Destroy structure without loot (safe unanchoring completion)
structure_system.destroy_structure(
    structure_entity_id=structure_id,
    drop_loot=False  # Items are safely recovered, no loot dropped
)
```

When `drop_loot=True`:
- Creates a "secure" loot container at the structure's position (offset by 10 units)
- Transfers all items from structure's hangar (Inventory component) to the container
- Container is owned by the structure's owner corporation
- Container despawns after 1 hour if not looted
- ISK in structure wallet is not dropped (handled separately by corp wallet system)

## Lifecycle

### 1. Deployment
- Player deploys structure at a location
- Structure enters "anchoring" state
- Anchoring timer starts (12-48 hours depending on type)

### 2. Anchoring
- Timer counts down
- Structure is vulnerable but not yet functional
- Cannot activate services

### 3. Online
- Anchoring complete
- Structure becomes functional
- Services can be activated
- Fuel consumption begins
- Vulnerable to attack

### 4. Fueling
- Fuel is consumed hourly based on fuel_usage_rate
- If fuel runs out, all services go offline
- Structure remains online but non-functional

### 5. Unanchoring (Optional)
- Owner initiates unanchoring
- 24-hour timer begins
- After completion, structure can be scooped

### 6. Destruction
- Structure takes damage beyond hull HP
- Structure explodes
- **Loot Dropping**:
  - If structure has items in its hangar (Inventory component), a secure loot container is created
  - Container type: "secure" (only owner corporation can access initially)
  - Container position: Offset 10 units from structure location
  - Container despawn time: 1 hour (3600 seconds)
  - All hangar items are transferred to the container
  - ISK is stored separately in corporation wallet (not dropped)
  - Loot dropping can be disabled for safe unanchoring scenarios
- Entity removed from world

Example of destroying a structure with loot:
```python
# Destroy structure and drop loot (combat scenario)
structure_system.destroy_structure(structure_id, drop_loot=True)

# Destroy structure without dropping loot (safe unanchoring completion)
structure_system.destroy_structure(structure_id, drop_loot=False)
```

## Permissions

### Deployment
- Any player can deploy structures they own
- Corporation structures require appropriate role

### Management
- Owner can manage their own structures
- CEO and Directors can manage corp structures
- Members can use services based on access lists

### Unanchoring
- Structure owner
- Corporation CEO
- Corporation Directors

## Integration Points

### With Corporation System
- Structures track corporation ownership
- Corporation permissions apply
- Tax revenue can fund fuel costs

### With Market System
- Structures can host market services
- Broker fees customizable
- Trade hub functionality

### With Manufacturing System
- Structures can host manufacturing services
- Industry bonuses applicable
- Material efficiency upgrades

### With Combat System
- Structures have health pools
- Damage types and resistances
- Reinforcement mechanics (future)

## Configuration

Structure statistics are loaded from `engine/systems/structure_system.py::_load_structure_data()`. These can be customized or loaded from external JSON files.

## Testing

Run the comprehensive test suite:

```bash
python test_structure_system.py
```

### Test Coverage
- ✅ Structure deployment (all types)
- ✅ Anchoring mechanics
- ✅ Fuel management and consumption
- ✅ Service activation
- ✅ Permission checks
- ✅ Health pools and fitting slots
- ✅ Multi-fuel support
- ✅ Structure destruction
- ✅ Location tracking

**Result**: 18/18 tests passing (100%)

## Future Enhancements

### Phase 1 (Current)
- ✅ Basic structure deployment
- ✅ Anchoring/unanchoring
- ✅ Fuel management
- ✅ Service activation
- ✅ Owner permissions

### Phase 2 (Planned)
- [ ] Reinforcement mechanics with timers
- [ ] Combat integration (damage structures)
- [ ] Vulnerability windows
- [ ] Defensive modules
- [ ] Shield/armor repairing

### Phase 3 (Planned)
- [ ] Structure fitting system
- [ ] Rigs and upgrades
- [ ] Service modules
- [ ] Advanced permissions (roles)
- [ ] Structure transfer

### Phase 4 (Planned)
- [ ] 3D client integration
- [ ] Structure models
- [ ] Visual anchoring progress
- [ ] Combat effects
- [ ] Service UI panels

## Example Usage

### Deploy and Manage a Citadel

```python
from engine.core.ecs import World
from engine.systems.structure_system import StructureSystem

# Create world and system
world = World()
structure_system = StructureSystem(world)

# Deploy structure
structure_id = structure_system.deploy_structure(
    player_entity_id=player_id,
    structure_type="astrahus",
    structure_name="Trade Hub Alpha",
    system_id="jita",
    x=1000.0, y=2000.0, z=3000.0
)

# Wait for anchoring (or fast-forward time in testing)
structure_system.update(dt=86401.0)  # 24 hours + 1 second

# Add fuel
structure_system.add_fuel(structure_id, "fuel_blocks", 10000)

# Activate services
structure_system.activate_service(structure_id, "market")
structure_system.activate_service(structure_id, "manufacturing")

# Query info
info = structure_system.get_structure_info(structure_id)
print(f"Structure '{info['name']}' is {info['state']}")
print(f"Fuel: {info['fuel']}/{info['fuel_capacity']}")
print(f"Services: {list(info['services'].keys())}")
```

## See Also

- [Structure Data Definitions](../../data/universe/player_structures.json)
- [Structure Type Data](../../data/universe/station_types.json)
- [Corporation System](../engine/systems/corporation_system.py)
- [Test Suite](../../test_structure_system.py)
- [EVE 2026 Equinox Expansion](../../data/universe/player_structures.json)
