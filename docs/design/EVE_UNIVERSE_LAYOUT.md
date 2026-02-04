# EVE Online-Style Universe Layout and Travel Mechanics

## Overview

This document describes the implementation of realistic EVE Online-style universe layout, station types, asteroid field mechanics, and travel/warp systems in EVE OFFLINE.

## Table of Contents

1. [Station Types and Layouts](#station-types-and-layouts)
2. [Asteroid Fields](#asteroid-fields)
3. [Warp Mechanics and Travel Times](#warp-mechanics-and-travel-times)
4. [System Scale and Distances](#system-scale-and-distances)
5. [Daily Gameplay Examples](#daily-gameplay-examples)

---

## Station Types and Layouts

### Station Types

EVE OFFLINE implements 6 primary station types based on EVE Online:

#### 1. **Industrial Stations**
- **Services**: Market, Repair, Fitting, Manufacturing, Research
- **Common In**: All security levels
- **Examples**:
  - Gallente: "Duvolle Laboratories Manufacturing Plant"
  - Caldari: "Caldari Provisions Warehouse"
  - Minmatar: "Boundless Creation Factory"
  - Amarr: "Carthum Conglomerate Factory"

#### 2. **Military Stations**
- **Services**: Repair, Fitting, Missions, Cloning, Insurance
- **Common In**: All security levels
- **Examples**:
  - Gallente: "Federation Navy Assembly Plant"
  - Caldari: "Caldari Navy Assembly Plant" (e.g., Jita 4-4)
  - Minmatar: "Republic Fleet Assembly Plant"
  - Amarr: "Imperial Navy Assembly Plant"

#### 3. **Commercial Hubs**
- **Services**: Market, Contracts, Courier, Cloning
- **Common In**: High-sec trade hubs
- **Examples**: Major markets like Jita, Dodixie, Hek

#### 4. **Research Facilities**
- **Services**: Research, Invention, Manufacturing, Fitting
- **Common In**: High-sec systems
- **Purpose**: Blueprint research and invention

#### 5. **University Stations**
- **Services**: Missions, Market, Cloning, Repair
- **Common In**: Starter systems and high-sec
- **Purpose**: New player training and education

#### 6. **Mining Stations**
- **Services**: Refining, Reprocessing, Market, Repair
- **Common In**: Systems with asteroid belts
- **Purpose**: Ore processing and mining support

### Faction-Specific Station Designs

#### Gallente
- **Style**: Organic domes with flowing curves
- **Colors**: Green, white, silver
- **Features**: Eco-friendly design, luxurious interior, nature integration
- **Size Scale**: 1.2x (larger than average)
- **Aesthetic**: "Safe-but-busy" environment with opulent design

#### Caldari
- **Style**: Utilitarian blocks
- **Colors**: Steel blue, gray, white
- **Features**: Angular design, functional efficiency, minimalist
- **Size Scale**: 1.0x (baseline)
- **Aesthetic**: Corporate and efficient

#### Minmatar
- **Style**: Rusted patchwork
- **Colors**: Rust brown, orange, dark gray
- **Features**: Asymmetric, salvaged parts, industrial
- **Size Scale**: 0.9x (compact)
- **Aesthetic**: Rough and utilitarian

#### Amarr
- **Style**: Golden cathedral
- **Colors**: Gold, brass, bronze, white
- **Features**: Ornate design, religious symbolism, grand scale
- **Size Scale**: 1.4x (largest)
- **Aesthetic**: Majestic and elegant

---

## Asteroid Fields

### Belt Sizes

#### Small Asteroid Cluster
- **Asteroid Count**: 5-10
- **Radius**: 30 km
- **Typical Yield**: Low
- **Spawn Locations**: Inner system, near planets

#### Medium Asteroid Field
- **Asteroid Count**: 10-20
- **Radius**: 50 km
- **Typical Yield**: Moderate
- **Spawn Locations**: Mid-system, standard belts

#### Large Asteroid Belt
- **Asteroid Count**: 20-40
- **Radius**: 80 km
- **Typical Yield**: High
- **Spawn Locations**: Outer system, dedicated mining areas

#### Colossal Asteroid Belt
- **Asteroid Count**: 40-80
- **Radius**: 150 km
- **Typical Yield**: Very High
- **Spawn Locations**: Prime mining locations
- **Note**: Often contested by multiple players

### Layout Patterns

1. **Cluster**: 5-10 large asteroids tightly grouped (2 km spacing)
2. **Semicircle**: Asteroids in a curved arc (5 km spacing)
3. **Scattered**: Random distribution across field (10 km spacing)
4. **Ring**: Circular ring formation (8 km spacing)

### Ore Distribution by Security Level

#### High-Sec (1.0 - 0.5)
- **1.0**: Veldspar, Scordite (abundant, high traffic)
- **0.9**: Veldspar, Scordite, Pyroxeres (abundant)
- **0.8**: Veldspar, Scordite, Pyroxeres, Plagioclase (good)
- **0.7**: Add Omber (good yield, moderate traffic)
- **0.6-0.5**: Plagioclase, Omber, Kernite (moderate yield)

#### Low-Sec (0.4 - 0.1)
- **0.4**: Kernite, Jaspet, Hemorphite (dangerous, NPC presence)
- **0.1-0.3**: Jaspet, Hemorphite, Hedbergite (very dangerous)

#### Null-Sec (0.0 and below)
- **0.0**: Hemorphite, Hedbergite, Gneiss, Dark Ochre (player controlled)
- **Deep Null**: Crokite, Bistot, Arkonor (legendary, alliance controlled)
- **Special**: Mercoxit in deep null (-0.5 and lower, requires special equipment)

### Respawn Mechanics

#### Daily Downtime (11:00 UTC)
- **Full Respawn**: All asteroid belts completely respawn
- **Server Maintenance**: Brief downtime for universe reset

#### Depletion System
- **Rule**: If a belt is completely mined out, it respawns **10% smaller** the next day
- **Recovery**: Takes 7 days to fully recover
- **Minimum Size**: Belts won't drop below 50% of original size
- **Partial Mining**: Partially mined asteroids remain until downtime

#### NPC Miners
- **Presence**: High-sec only
- **Behavior**: Visual only, do not consume ore
- **Purpose**: Atmosphere and immersion
- **Types**: ORE Mining Barge, Independent Miner, Corporate vessels

---

## Warp Mechanics and Travel Times

### Distance Units

#### Astronomical Unit (AU)
- **Value**: 149,597,870,700 meters (~150 million km)
- **Usage**: Standard space distance measurement
- **Reference**: Approximately Earth-to-Sun distance

### Warp Speeds by Ship Class

| Ship Class | AU/s | Examples |
|------------|------|----------|
| Frigate | 5.0 | Rifter, Merlin, Tristan, Punisher |
| Destroyer | 4.0 | Thrasher, Cormorant, Catalyst |
| Cruiser | 3.0 | Stabber, Caracal, Vexor, Maller |
| Battlecruiser | 2.5 | Cyclone, Ferox, Brutix, Harbinger |
| Battleship | 2.0 | Tempest, Raven, Dominix, Apocalypse |
| **Venture** | 8.0 | Fast mining frigate (exceptional) |
| **Procurer** | 3.0 | Standard mining barge |
| **Hulk** | 1.35 | Slow, specialized mining barge |

### Alignment Times

Time required to align ship for warp entry:

| Ship Type | Alignment Time |
|-----------|----------------|
| Frigate | 2.5 seconds |
| Destroyer | 4.0 seconds |
| Cruiser | 6.0 seconds |
| Battlecruiser | 8.0 seconds |
| Battleship | 10.0 seconds |
| Hulk | 12.0 seconds |

### Warp Acceleration Curve

**Phases**:
1. **Acceleration** (33% of distance): Ship accelerates to max warp speed
2. **Max Speed** (34% of distance): Ship travels at peak velocity
3. **Deceleration** (33% of distance): Ship slows for exit

**Minimum Warp Distance**: 0.1 AU

### Travel Time Examples

#### 1 AU Warp
- Frigate: **0.2 seconds** in warp
- Cruiser: **0.33 seconds** in warp
- Battleship: **0.5 seconds** in warp
- Hulk: **0.74 seconds** in warp

#### 10 AU Warp
- Frigate: **2 seconds** in warp
- Cruiser: **3.3 seconds** in warp
- Battleship: **5 seconds** in warp
- Hulk: **7.4 seconds** in warp

#### 100 AU Warp (Long Distance)
- Frigate: **20 seconds** in warp
- Cruiser: **33 seconds** in warp
- Battleship: **50 seconds** in warp
- Hulk: **74 seconds** in warp

### Complete Travel Examples

#### Gate-to-Gate (15 AU average)
- **Frigate**: 5.5 seconds (2.5s align + 3s warp)
- **Cruiser**: 11 seconds (6s align + 5s warp)
- **Battleship**: 13 seconds (10s align + 3s warp)
- **With Gate Activation**: Add 5-10 seconds
- **Typical Range**: **30-60 seconds total**

#### Belt-to-Station (10 AU)

**Venture (Fast Mining Frigate)**:
- Alignment: 3 seconds
- Warp: 1.25 seconds
- Docking: ~10 seconds
- **Total: ~15 seconds**
- **Round Trip: ~1 minute**

**Hulk (Slow Mining Barge)**:
- Alignment: 12 seconds
- Warp: 7.4 seconds
- Docking: ~10 seconds
- **Total: ~30 seconds**
- **Round Trip: ~1.5-2 minutes**

---

## System Scale and Distances

### Typical System Layout

- **System Radius**: 100-150 AU
- **Gate Spacing**: 10-150 AU (average 30 AU)
- **Belt Distances**:
  - Inner belts: 5-20 AU from sun
  - Mid belts: 20-50 AU from sun
  - Outer belts: 50-80 AU from sun
- **Station Distances**:
  - Near gate: 5-10 AU
  - Mid-system: 30-50 AU
  - Far locations: 80-100 AU

### Example: Dodixie System (Gallente)

**System Details**:
- Security: 0.9 (High-sec)
- Faction: Gallente
- Region: Sinq Laison
- System Radius: 120 AU

**Stations**:
1. **Dodixie IX - Moon 20 - Federation Navy Assembly Plant**
   - Distance: 45 AU from sun
   - Type: Military (major trade hub)
   - Services: Full (market, missions, fitting, etc.)

2. **Dodixie V - University of Caille**
   - Distance: 25 AU from sun
   - Type: University
   - Services: Missions, market, cloning

**Asteroid Belts**:
1. **Belt I** (Colossal): 18 AU from sun, 60 asteroids
2. **Belt II** (Large): 35 AU from sun, 35 asteroids
3. **Belt III** (Medium): 52 AU from sun, 18 asteroids

**Travel Times** (Belt I to Main Station, 27 AU):
- Venture: 7 seconds
- Cruiser: 15 seconds
- Hulk: 32 seconds

---

## Daily Gameplay Examples

### Typical Day in Gallente Space (0.5 Security System)

#### 11:05 UTC - Post-Downtime
- Downtime ends, all belts fully respawned to 100% capacity
- Check market in Dodixie trade hub
- Plan mining route for the day

#### 11:30 UTC - Travel to Mining System
- Undock from Dodixie
- Take Mining Barge (e.g., Procurer) to nearby 0.5 security system
- Travel time: 45 seconds per gate

#### 11:45 UTC - Arrive at Belt
- First colossal belt is empty - someone got there first
- Jump to next system (45 seconds)
- Competition is high immediately after downtime

#### 12:00 UTC - Mining Operations
- Find a "Colossal" belt with ore remaining
- Start mining operations
- Align ship toward safe station in case of gankers
- Mine Veldspar, Scordite, Pyroxeres

#### 13:00 UTC - First Load Complete
- Ore hold full (~5,000-10,000 m³)
- Warp to station:
  - Distance: 10 AU
  - Warp time: 20 seconds
  - Dock and refine ore
- Put minerals up for sale on market
- Return to belt for second load

#### Rest of Day
- Continue mining cycles
- Belts become progressively more depleted
- By evening, may need to find less popular systems
- Total daily income: Variable based on ore type and efficiency

### Mining Trip Breakdown (Hulk Example)

**Route**: Asteroid Belt → Mining Station (10 AU)

1. **Mining Phase**: 30-45 minutes (fill ore hold)
2. **Return Trip**:
   - Align to station: 12 seconds
   - Warp travel: 7.4 seconds
   - Request dock: 2 seconds
   - Docking approach: 8 seconds
   - **Total: ~30 seconds**
3. **Station Activities**:
   - Reprocess ore: 30 seconds
   - Sell minerals: 1-2 minutes
   - **Total: ~2 minutes**
4. **Return to Belt**:
   - Undock: 5 seconds
   - Align: 12 seconds
   - Warp: 7.4 seconds
   - Arrive at belt: 5 seconds
   - **Total: ~30 seconds**

**Complete Cycle**: ~35-50 minutes (mostly mining time)
**Travel Time**: ~1 minute each way
**Station Time**: ~2 minutes

---

## Implementation in EVE OFFLINE

### Data Files

1. **`data/universe/station_types.json`**
   - Defines all 6 station types
   - Faction-specific designs and styles
   - Service availability per type

2. **`data/universe/warp_mechanics.json`**
   - Distance units (AU, km)
   - Warp speeds by ship class
   - Alignment times
   - Travel time calculations
   - Acceleration curve mechanics

3. **`data/asteroid_fields/belt_layouts.json`**
   - Belt size definitions
   - Layout patterns
   - Respawn mechanics
   - Ore distribution by security
   - NPC miner behavior

4. **`data/universe/enhanced_systems.json`**
   - Complete system definitions
   - Station placements with distances
   - Asteroid belt locations
   - Stargate connections
   - Travel time examples

### Game Mechanics

#### Warp System
```
Total Travel Time = Alignment Time + Warp Time + Docking/Gate Activation

Warp Time = Distance (AU) / Ship Warp Speed (AU/s)
```

#### Belt Respawn
```
Daily Downtime (11:00 UTC):
  - All belts respawn to (current_capacity * 100%)
  
If belt completely mined:
  - Next day capacity = previous_capacity * 0.9
  
Recovery:
  - Each day unmined: capacity += 10%
  - Until capacity = 100%
```

#### Distance Calculation
```
1 AU = 149,597,870,700 meters
Warp speed varies by ship (1.35 AU/s to 10 AU/s)
Typical system: 100-150 AU radius
Gate spacing: 10-150 AU (avg 30 AU)
```

---

## Visual References

### Scale Comparison

| Distance | Example | Fast Ship | Slow Ship |
|----------|---------|-----------|-----------|
| 1 AU | Inner orbit | 0.2s | 0.74s |
| 10 AU | Belt to station | 2s | 7.4s |
| 30 AU | Gate to gate | 6s | 22s |
| 100 AU | System crossing | 20s | 74s |

### System Layout Example

```
        Sun
         |
    [5 AU] Belt I (Inner, small)
         |
   [15 AU] Mining Station
         |
   [25 AU] University Station
         |
   [35 AU] Belt II (Mid, large)
         |
   [45 AU] Main Trading Station
         |
   [65 AU] Belt III (Outer, colossal)
         |
   [85 AU] Stargate A
         |
  [110 AU] Stargate B
```

---

## Future Enhancements

1. **Dynamic Economy**: Ore prices based on supply/demand
2. **Player Mining Impact**: Track individual player depletion
3. **Anomalies**: Random rich asteroid spawns
4. **Ice Fields**: Special belts with ice harvesting
5. **Gas Clouds**: Alternate resource gathering
6. **Wormholes**: Temporary connections between systems
7. **Bookmark System**: Save custom warp points
8. **Fleet Warps**: Coordinated multi-ship travel

---

## References

- Based on EVE Online by CCP Games
- Data sourced from EVE Online gameplay mechanics
- Time/distance calculations from actual in-game measurements
- Station types and designs from EVE lore

**Last Updated**: February 2026
**Version**: 1.0
