# New Gameplay Systems - Phase 3

This document describes all the new gameplay systems added in Phase 3 to bring EVE OFFLINE closer to the full EVE Online experience.

## 🏭 Manufacturing System

**File:** `engine/systems/industry_system.py`

### Features
- Blueprint management (BPO/BPC)
- Material Efficiency (ME) research (0-10 levels)
- Time Efficiency (TE) research (0-20 levels)
- Manufacturing jobs with material requirements
- Blueprint copying
- Manufacturing queue system

### Components
- `Blueprint` - Blueprint data (item, materials, efficiency)
- `BlueprintInventory` - Collection of blueprints owned
- `Inventory` - Item storage with capacity management
- `ManufacturingQueue` - Active manufacturing jobs

### Key Methods
```python
industry = IndustrySystem(world)

# Start manufacturing
job_id = industry.start_manufacturing(entity, "rifter_blueprint", quantity=2)

# Research blueprints
industry.research_material_efficiency(entity, "rifter_blueprint")
industry.research_time_efficiency(entity, "rifter_blueprint")

# Copy blueprints
industry.copy_blueprint(entity, "rifter_blueprint", runs=10)

# Check jobs
active_jobs = industry.get_active_jobs(entity)
```

### Data Files
- `data/industry/blueprints.json` - Blueprint definitions and materials

---

## 💰 Market System

**File:** `engine/systems/market_system.py`

### Features
- Buy and sell market orders
- Instant buy/sell transactions
- ISK wallet management
- Market order book per location
- Transaction history
- Broker fees and sales tax (EVE-like)
- NPC base prices

### Components
- `Wallet` - ISK currency storage
- `MarketAccess` - Market location and active orders
- `MarketOrder` - Individual buy/sell order

### Key Methods
```python
market = MarketSystem(world)

# Place orders
order_id = market.place_order(
    entity, "tritanium", OrderType.SELL, price=6.0, quantity=10000
)

# Instant trading
market.instant_buy(buyer, "tritanium", quantity=5000, max_price=7.0)
market.instant_sell(seller, "rifter", quantity=1, min_price=300000.0)

# Cancel orders
market.cancel_order(entity, order_id)

# Check prices
prices = market.get_market_price("tritanium", "jita_4_4")
# Returns: {'buy': 5.5, 'sell': 6.0, 'average': 5.75}
```

### Economic Features
- **Broker Fee**: 3% on order placement
- **Sales Tax**: 2% on completed transactions
- **Order Book**: Sorted by price (sell low→high, buy high→low)
- **Trade Hubs**: Jita, Amarr, Dodixie, Rens

### Data Files
- `data/market/prices.json` - NPC base prices and trade hub info

---

## 🔍 Exploration System

**File:** `engine/systems/exploration_system.py`

### Features
- Probe scanning mechanics
- Cosmic signatures (Combat, Relic, Data, Gas, Wormholes)
- Scanner probe formation and positioning
- Scan strength calculation based on probe coverage
- Site completion and rewards
- Directional scanner (D-Scan)

### Components
- `ProbeScanner` - Scanner probes and scan capability
- `ShipScanner` - Directional scanner
- `ExplorationData` - Tracking scanned/completed sites
- `CosmicSignature` - Scannable signature in space

### Signature Types
1. **Combat Sites** - NPCs to fight, bounties
2. **Relic Sites** - Ancient artifacts, archaeology
3. **Data Sites** - Datacores, hacking
4. **Gas Sites** - Harvestable gas clouds
5. **Wormholes** - Connections to other systems

### Key Methods
```python
exploration = ExplorationSystem(world)

# Generate signatures in a system
exploration.generate_signatures("jita", count=5)

# Launch and position probes
exploration.launch_probes(explorer, count=8)
exploration.move_probe(explorer, probe_id, (2.0, 2.0, 2.0))

# Scan for signatures
detected = exploration.scan(explorer, "jita")

# Warp to and complete sites
exploration.warp_to_signature(explorer, "jita", signature_id)
rewards = exploration.complete_site(explorer, "jita", signature_id)

# Directional scan
nearby_entities = exploration.directional_scan(explorer, current_time)
```

### Scan Mechanics
- Probes have scan range and strength
- Better probe formation = better scan results
- Multiple scans needed to reach 100%
- Scan progress: 0-20% per scan depending on formation

### Data Files
- `data/exploration/sites.json` - Signature templates and loot

---

## 📦 Loot System

**File:** `engine/systems/loot_system.py`

### Features
- Loot drops from NPCs
- Loot containers (wrecks, cargo cans)
- Weighted loot tables
- Guaranteed + random loot
- ISK drops
- Wreck salvaging
- Container despawning

### Components
- `LootContainer` - Container holding loot
- `LootContainers` - Component tracking containers
- `LootTable` - Weighted drop table
- `LootItem` - Individual loot item with rarity

### Loot Qualities
- Common
- Uncommon
- Rare
- Very Rare
- Officer (highest quality)

### Key Methods
```python
loot_system = LootSystem(world)

# Spawn loot from destroyed NPC
container_id = loot_system.spawn_loot_from_npc(
    npc_entity, "serpentis_cruiser", current_time
)

# Loot a container
looted_items = loot_system.loot_container(looter_entity, container_id)

# Salvage a wreck
salvage_materials = loot_system.salvage_wreck(salvager_entity, container_id)

# Find nearby containers
nearby = loot_system.get_nearby_containers((x, y, z), max_range=100.0)
```

### Default Loot Tables
- **Frigate Loot**: Salvage, ammo, small modules, 5-15K ISK
- **Cruiser Loot**: Medium modules, datacores, rare salvage, 50-150K ISK
- **Salvage Materials**: Common and rare salvage from wrecks

---

## 👥 Fleet System

**File:** `engine/systems/fleet_system.py`

### Features
- Fleet creation and management
- Fleet roles (Member, Squad Commander, Wing Commander, Fleet Commander)
- Fleet bonuses from boosters
- Squad and wing organization
- Target broadcasting
- Fleet warping
- Fleet member tracking

### Components
- `Fleet` - Fleet data structure
- `FleetMember` - Individual member info
- `FleetMembership` - Component for entities in a fleet

### Fleet Roles
1. **Member** - Basic fleet member
2. **Squad Commander** - Leads a squad (up to 10 members)
3. **Wing Commander** - Leads a wing (up to 5 squads)
4. **Fleet Commander** - Leads entire fleet (up to 256 members)

### Fleet Boosters
Active boosters provide bonuses to all fleet members:

1. **Armor Booster**
   - +10% armor HP
   - +5% armor resistances

2. **Shield Booster**
   - +10% shield HP
   - +5% shield resistances

3. **Skirmish Booster**
   - +15% max speed
   - +10% agility

4. **Information Booster**
   - +20% targeting range
   - +15% scan resolution

### Key Methods
```python
fleet_system = FleetSystem(world)

# Create fleet
fleet_id = fleet_system.create_fleet(commander, "Alpha Fleet")

# Invite members
fleet_system.invite_to_fleet(fleet_id, inviter, invitee)

# Leave fleet
fleet_system.leave_fleet(member)

# Activate boosters
fleet_system.set_fleet_booster(fleet_id, booster_entity, FleetBoosterType.SHIELD)

# Broadcast target
fleet_system.broadcast_target(fleet_id, broadcaster, target)

# Warp fleet
fleet_system.warp_fleet_to(fleet_id, commander, "Jita IV - Moon 4")

# Organization
fleet_system.assign_squad(fleet_id, member, squad_id=1)
fleet_system.promote_to_squad_commander(fleet_id, fc, member, squad_id=1)
```

### Fleet Capacity
- Maximum 256 members per fleet
- Up to 5 wings per fleet
- Up to 5 squads per wing
- Up to 10 members per squad

---

## 🎮 Integration Examples

### Complete Gameplay Loop Example

```python
from engine.core.ecs import World
from engine.systems.industry_system import IndustrySystem, Inventory
from engine.systems.market_system import MarketSystem, Wallet
from engine.systems.exploration_system import ExplorationSystem
from engine.systems.loot_system import LootSystem
from engine.systems.fleet_system import FleetSystem

# Initialize world and systems
world = World()
industry = IndustrySystem(world)
market = MarketSystem(world)
exploration = ExplorationSystem(world)
loot = LootSystem(world)
fleet = FleetSystem(world)

# Create player
player = world.create_entity()
player.add_component(Wallet(isk=10000000.0))
player.add_component(Inventory(capacity=1000.0))

# 1. BUY MATERIALS
market.instant_buy(player, "tritanium", 50000, max_price=6.0)
market.instant_buy(player, "pyerite", 15000, max_price=12.0)

# 2. MANUFACTURE SHIP
job_id = industry.start_manufacturing(player, "rifter_blueprint", quantity=1)

# Wait for manufacturing...
industry.update(delta_time, current_time)

# 3. SELL SHIP
market.place_order(player, "rifter", OrderType.SELL, price=380000.0, quantity=1)

# 4. EXPLORE
exploration.launch_probes(player, count=8)
exploration.scan(player, "jita")
exploration.complete_site(player, "jita", "sig_001")

# 5. LOOT
loot.loot_container(player, container_id)
loot.salvage_wreck(player, wreck_id)

# 6. FLEET UP
fleet_id = fleet.create_fleet(player, "Mining Op")
fleet.invite_to_fleet(fleet_id, player, friend)
fleet.set_fleet_booster(fleet_id, player, FleetBoosterType.SHIELD)
```

---

## 📊 System Integration Matrix

| System | Requires | Integrates With | Outputs |
|--------|----------|-----------------|---------|
| **Manufacturing** | Inventory, Blueprints | Market (buy materials) | Ships, Modules, Ammo |
| **Market** | Wallet, Inventory | Manufacturing, Exploration | ISK, Items |
| **Exploration** | Scanner Probes | Loot (site rewards) | Signatures, Loot |
| **Loot** | Inventory | Combat (NPC drops) | Items, ISK, Salvage |
| **Fleet** | - | All combat systems | Bonuses, Coordination |

---

## 🎯 Gameplay Loops Enabled

### 1. Industrial Loop
```
Mine Ore → Refine Materials → Manufacture Items → Sell on Market → Profit
```

### 2. Exploration Loop
```
Scan Signatures → Warp to Site → Complete/Hack → Loot Container → Sell Loot
```

### 3. Combat-Loot Loop
```
Accept Mission → Kill NPCs → Loot Wrecks → Salvage → Sell Salvage/Loot
```

### 4. Trading Loop
```
Buy Low → Transport → Sell High → Profit → Invest in More Cargo
```

### 5. Fleet PvE Loop
```
Form Fleet → Share Fleet Bonuses → Run Group Content → Split Loot
```

---

## 📈 Statistics

### Code Added
- **5 New Systems**: ~65,000 characters of code
- **15 New Components**: Blueprint, Wallet, LootContainer, Fleet, etc.
- **100+ New Methods**: Complete APIs for all systems
- **3 Data Files**: Blueprints, prices, exploration sites

### EVE Online Features Implemented
- ✅ Manufacturing and industry
- ✅ Market trading (buy/sell orders)
- ✅ Exploration and scanning
- ✅ Loot and salvaging
- ✅ Fleet mechanics and bonuses
- ✅ ISK economy
- ✅ Material requirements
- ✅ Blueprint research

### What's Next (Phase 4)
- [ ] 3D graphics implementation (see LANGUAGE_AND_3D_OPTIONS.md)
- [ ] Advanced mining mechanics
- [ ] Corporation management
- [ ] Sovereignty mechanics (for larger groups)
- [ ] PvP mechanics (if desired)
- [ ] Faction warfare
- [ ] Incursions and dynamic events

---

## 🚀 Performance Notes

All systems are designed to be efficient:
- **O(1) lookups** for most operations using dictionaries
- **Spatial partitioning** for proximity checks
- **Lazy updates** - systems only process active entities
- **Memory efficient** - components only added when needed

Recommended update rates:
- Manufacturing: 1 Hz (every second)
- Market: 0.1 Hz (every 10 seconds)
- Exploration: 1 Hz
- Loot: 0.5 Hz (every 2 seconds)
- Fleet: 1 Hz

---

## 📚 See Also

- [LANGUAGE_AND_3D_OPTIONS.md](LANGUAGE_AND_3D_OPTIONS.md) - Language choice and 3D graphics options
- [EVE_MECHANICS.md](EVE_MECHANICS.md) - EVE Online mechanics reference
- [DESIGN.md](DESIGN.md) - Overall game design
- [demo_new_features.py](demo_new_features.py) - Runnable demo of new systems

---

*EVE OFFLINE - Bringing the EVE Online experience to small groups*
