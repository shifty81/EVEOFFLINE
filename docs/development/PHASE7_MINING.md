# Phase 7: Mining & Resource Gathering System

**Status**: Core Implementation Complete ✅  
**Date**: February 3, 2026  
**Version**: 1.0

---

## Overview

The Mining & Resource Gathering system implements EVE Online-style asteroid mining mechanics, allowing players to extract ore from asteroid belts and refine it into valuable minerals. This is the first major feature of Phase 7 Advanced Systems.

### Key Features

- **Mining Laser Operations**: Cycle-based ore extraction with capacitor consumption
- **15 Ore Types**: From common Veldspar to ultra-rare Mercoxit
- **Skill-Based Yields**: Mining and Astrogeology skills increase yield
- **Module Bonuses**: Mining Laser Upgrades boost efficiency
- **Ore Reprocessing**: Refine ore into minerals at stations
- **Refining Skills**: Reprocessing and Reprocessing Efficiency increase mineral yields
- **Ore Holds**: Specialized cargo for mining barges and exhumers
- **Asteroid Belt System**: Pre-existing system for asteroid generation and management

---

## Components

### MiningLaser Component

Represents an active mining laser module fitted to a ship.

```python
@dataclass
class MiningLaser(Component):
    laser_type: str = "Miner I"
    cycle_time: float = 60.0          # seconds per mining cycle
    yield_amount: float = 40.0        # m3 per cycle
    optimal_range: float = 10000.0    # meters
    capacitor_usage: float = 60.0     # GJ per cycle
    is_active: bool = False
    current_cycle: float = 0.0
    target_asteroid_id: Optional[str] = None
```

### MiningYield Component

Tracks mining statistics and active operations.

```python
@dataclass
class MiningYield(Component):
    total_ore_mined: Dict[str, float] = field(default_factory=dict)
    active_mining: bool = False
    target_asteroid_id: Optional[str] = None
    mining_start_time: float = 0.0
    yield_multiplier: float = 1.0  # From skills/modules
```

### OreHold Component

Specialized cargo hold for ore (mining barges/exhumers only).

```python
@dataclass
class OreHold(Component):
    ore: Dict[str, float] = field(default_factory=dict)
    ore_hold_capacity: float = 5000.0  # m3
    ore_hold_used: float = 0.0
```

---

## Mining Modules

### Mining Lasers

Four types of mining lasers with increasing power and yield:

| Module | Cycle Time | Yield (m3) | Range (km) | Cap Usage | Requirements |
|--------|------------|------------|------------|-----------|--------------|
| Miner I | 60s | 40 | 10 | 60 GJ | None |
| Miner II | 60s | 60 | 12 | 72 GJ | Mining 5 |
| Modulated Deep Core Miner II | 60s | 80 | 15 | 80 GJ | Mining 5 |
| Strip Miner I | 180s | 540 | 15 | 360 GJ | Mining 4, Astrogeology 3 |

**Strip Miner I** is designed for mining barges and requires massive CPU (60) and Powergrid (1100).

### Mining Upgrades

Low-slot modules that increase mining yield:

| Module | Bonus | Slot | Requirements |
|--------|-------|------|--------------|
| Mining Laser Upgrade I | +5% yield | Low | None |
| Mining Laser Upgrade II | +9% yield | Low | Mining 5, Science 5 |

**Stacking Penalty**: Multiple upgrades have diminishing returns using EVE's stacking penalty formula.

### Survey Scanner

High-slot module for scanning asteroids to determine ore type and remaining quantity.

---

## Ore Types

### 15 Ore Types by Security Level

#### High-Sec Ores (0.5+)
- **Veldspar**: Common. Yields Tritanium.
- **Scordite**: Common. Yields Tritanium and Pyerite.
- **Pyroxeres**: Uncommon. Yields Tritanium, Pyerite, Mexallon, Nocxium.
- **Plagioclase**: Uncommon. Yields Tritanium, Pyerite, Mexallon.

#### Mid-Sec Ores (0.2-0.7)
- **Omber**: Uncommon. Yields Tritanium, Pyerite, Isogen.
- **Kernite**: Rare. Yields Tritanium, Mexallon, Isogen.
- **Jaspet**: Rare. Yields Mexallon, Nocxium, Zydrine.

#### Low-Sec Ores (0.0-0.2)
- **Hemorphite**: Rare. Yields Tritanium, Isogen, Nocxium, Zydrine.
- **Hedbergite**: Rare. Yields Pyerite, Isogen, Nocxium, Zydrine.

#### Null-Sec Ores (0.0 and below)
- **Gneiss**: Very Rare. Yields Pyerite, Mexallon, Isogen.
- **Dark Ochre**: Very Rare. Yields large amounts of Tritanium, Nocxium, Zydrine.
- **Crokite**: Ultra Rare. Yields massive Tritanium, Nocxium, Zydrine.
- **Bistot**: Ultra Rare. Yields massive Pyerite, Zydrine, Megacyte.
- **Arkonor**: Ultra Rare. Yields massive Tritanium, Mexallon, Megacyte.
- **Mercoxit**: Legendary. Yields Morphite (requires deep core mining).

### Reprocessing

Each ore type reprocesses in batches:
- Base batch size (e.g., Veldspar: 400 units, Scordite: 333 units)
- Yields specific minerals based on ore type
- Affected by station efficiency and skills

---

## Skills

### Mining Skills

| Skill | Bonus | Category | Multiplier |
|-------|-------|----------|------------|
| Mining | +5% yield per level | Resource Processing | 1x |
| Astrogeology | +5% yield per level | Resource Processing | 3x |
| Mining Upgrades | -5% CPU per level | Resource Processing | 1x |
| Mining Barge | +5% ore hold per level | Spaceship Command | 4x |
| Exhumers | +3% yield per level | Spaceship Command | 5x |

**Max Mining Bonus**: Mining V + Astrogeology V = +50% yield

### Refining Skills

| Skill | Bonus | Category | Multiplier |
|-------|-------|----------|------------|
| Reprocessing | +3% refining yield per level | Resource Processing | 1x |
| Reprocessing Efficiency | +2% refining yield per level | Resource Processing | 2x |

**Max Refining Bonus**: Reprocessing V + Reprocessing Efficiency V = +25%

### Prerequisites

- **Science**: Required for advanced skills (Mining Barge, Reprocessing Efficiency)

---

## Mining System

### Starting Mining

```python
mining_system = MiningSystem(world, asteroid_manager)

# Start mining an asteroid
success = mining_system.start_mining(
    entity=miner_entity,
    target_asteroid_id="asteroid_123",
    belt_id="jita_belt_1"
)
```

**Requirements**:
1. Entity must have MiningLaser component
2. Entity must have Position component
3. Entity must have Capacitor with sufficient energy
4. Asteroid must be within optimal range
5. Asteroid must not be depleted

### Mining Cycle

1. **Activation**: Checks range, capacitor, and asteroid status
2. **Cycle Progress**: Updates over time (delta_time)
3. **Cycle Complete**: 
   - Consumes capacitor
   - Extracts ore from asteroid
   - Applies skill/module bonuses
   - Stores ore in cargo/ore hold
   - Starts next cycle automatically

### Yield Calculation

```
Final Yield = Base Yield × (1 + Mining Skill Bonus) × (1 + Astrogeology Bonus) × Module Bonuses
```

**Example**:
- Miner II: 60 m3 base
- Mining V: +25%
- Astrogeology III: +15%
- Mining Laser Upgrade I: +5%

Result: 60 × 1.25 × 1.15 × 1.05 = **90.56 m3 per cycle**

### Stopping Mining

```python
mining_system.stop_mining(miner_entity)
```

Immediately stops current cycle and clears target.

---

## Ore Reprocessing

### Refining Ore

```python
industry_system = IndustrySystem(world)

# Reprocess ore into minerals
minerals = industry_system.reprocess_ore(
    entity=miner_entity,
    ore_type="veldspar",
    quantity=400,  # Must match reprocessing_base for ore type
    ore_data=veldspar_data,
    station_efficiency=0.50,
    skills_bonus=0.15  # From skills
)

# Returns: {'tritanium': 207}  (415 base × 1 batch × 0.50 efficiency)
```

### Refining Efficiency

```python
# Calculate efficiency
efficiency = industry_system.calculate_reprocessing_efficiency(
    entity=miner_entity,
    station_base=0.50
)

# With Reprocessing V and Reprocessing Efficiency V:
# 0.50 + 0.15 + 0.10 = 0.75 (75% efficiency)
```

**Station Efficiency**:
- NPC Stations: 50%
- Player Outposts: 50-54%
- Maxed Outpost: 52% + skills

**Total Cap**: 100% (1.0) efficiency maximum

---

## Integration with Game Systems

### Asteroid System Integration

The mining system integrates with the existing `AsteroidFieldManager`:

```python
# Find nearby asteroids
nearby = mining_system.get_nearby_asteroids(
    entity=miner_entity,
    max_distance=15000.0
)

# Returns: [(belt_id, asteroid, distance), ...]
```

### Inventory Integration

Mined ore is stored in:
1. **Ore Hold** (if present) - Priority storage for mining barges
2. **Cargo Hold** (Inventory component) - Fallback storage

### Market Integration

Minerals from reprocessing can be sold on the market system:
- Tritanium, Pyerite, Mexallon, Isogen, Nocxium, Zydrine, Megacyte, Morphite
- Prices determined by market supply/demand

---

## Usage Examples

### Basic Mining Operation

```python
# Setup
world = World()
asteroid_manager = AsteroidFieldManager()
asteroid_manager.load_ore_types("data/asteroid_fields/ore_types.json")
asteroid_manager.load_belts("data/asteroid_fields/belts.json")
mining_system = MiningSystem(world, asteroid_manager)

# Create miner
miner = world.create_entity()
miner.add_component(Position(x=0, y=0, z=0))
miner.add_component(Capacitor(capacitor=1000.0, capacitor_max=1000.0))
miner.add_component(MiningLaser(
    laser_type="Miner I",
    cycle_time=60.0,
    yield_amount=40.0
))
miner.add_component(Inventory(cargo_capacity=500.0))
miner.add_component(Skills(skills={"mining": 5, "astrogeology": 3}))

# Start mining
mining_system.start_mining(miner, "asteroid_id", "belt_id")

# Update mining cycles
for _ in range(5):
    mining_system.update(60.0)  # 5 cycles = 5 minutes

# Check results
inventory = miner.get_component(Inventory)
print(f"Mined ore: {inventory.items}")
```

### Reprocessing Workflow

```python
# Setup
industry_system = IndustrySystem(world)

# Load ore data
import json
with open("data/asteroid_fields/ore_types.json") as f:
    ore_data = json.load(f)["ore_types"]["veldspar"]

# Reprocess ore
minerals = industry_system.reprocess_ore(
    entity=miner,
    ore_type="veldspar",
    quantity=1200,  # 3 batches
    ore_data=ore_data,
    station_efficiency=0.50,
    skills_bonus=0.25
)

print(f"Gained minerals: {minerals}")
# Output: {'tritanium': 933}  (415 × 3 × 0.75)
```

---

## Testing

### Test Coverage

**Mining System Tests** (15 tests):
- Start/stop mining operations
- Range and capacitor checks
- Cycle progression and completion
- Skill bonuses
- Module bonuses
- Ore hold vs cargo storage
- Multiple cycles
- Statistics tracking

**Reprocessing Tests** (10 tests):
- Basic reprocessing
- Skill bonuses
- Multiple minerals
- Insufficient ore
- Ore hold priority
- Partial batches
- Efficiency calculations
- Efficiency cap

**Total**: 25 tests, 100% pass rate

### Running Tests

```bash
# Run all mining tests
python -m unittest test_mining_system test_ore_reprocessing -v

# Run specific test
python test_mining_system.py TestMiningSystem.test_mining_cycle_completion -v
```

---

## Performance Considerations

### Mining System
- Efficient distance calculations (cached positions)
- Asteroid lookup optimized with dictionary access
- No unnecessary updates when not mining

### Reprocessing
- Batch-based calculation (O(1) per mineral type)
- Direct dictionary operations
- No iteration over large datasets

---

## Future Enhancements

### Phase 7 Expansion (Optional)

1. **Mining Ships**
   - Procurer, Retriever, Covetor (Barges)
   - Skiff, Mackinaw, Hulk (Exhumers)
   - Ship bonuses to yield and capacity

2. **Ice Mining**
   - Ice Harvester modules
   - Ice products (fuel blocks, etc.)
   - Ice belts

3. **Gas Harvesting**
   - Gas Cloud Harvester modules
   - Gas clouds in wormholes/null-sec
   - Booster production

4. **Moon Mining**
   - Moon drills (structure-based)
   - Rare moon ores
   - Group activity content

5. **Compression**
   - Ore compression modules
   - Reduce volume for transport
   - Industrial Core for capitals

6. **Mining Missions**
   - Mining-type missions already supported
   - Specific ore requirements
   - Timed delivery

---

## API Reference

### MiningSystem

#### Methods

**`start_mining(entity, target_asteroid_id, belt_id=None) -> bool`**
- Start mining an asteroid
- Returns True if successful

**`stop_mining(entity) -> bool`**
- Stop current mining operation
- Returns True if mining was active

**`update(delta_time: float)`**
- Update all active mining operations
- Call every frame with time delta

**`get_nearby_asteroids(entity, max_distance=None) -> List[Tuple]`**
- Get asteroids within range
- Returns list of (belt_id, asteroid, distance)

**`get_mining_info(entity) -> Dict`**
- Get mining status and statistics
- Returns dict with cycle info, yields, etc.

### IndustrySystem (Reprocessing)

#### Methods

**`reprocess_ore(entity, ore_type, quantity, ore_data, station_efficiency, skills_bonus) -> Optional[Dict]`**
- Reprocess ore into minerals
- Returns mineral dict or None if failed

**`calculate_reprocessing_efficiency(entity, station_base=0.50) -> float`**
- Calculate total refining efficiency
- Returns efficiency from 0.0 to 1.0

---

## Conclusion

The Phase 7 Mining & Resource Gathering system provides a complete, EVE-style mining experience with:

- ✅ **Full cycle-based mining** with capacitor management
- ✅ **15 ore types** with realistic mineral compositions
- ✅ **Skill and module bonuses** for progression
- ✅ **Ore refining system** with efficiency calculations
- ✅ **25 comprehensive tests** (100% pass rate)
- ✅ **Complete documentation** with examples

The system integrates seamlessly with existing game systems (asteroid belts, inventory, market, skills) and provides a solid foundation for future mining-related features.

**Next Steps**: 
1. Add mining missions (framework exists)
2. Create mining barge ships
3. Integrate with 3D client for visual mining
4. Add mining-related UI panels

---

**Last Updated**: February 3, 2026  
**Version**: 1.0  
**Status**: Core Implementation Complete ✅
