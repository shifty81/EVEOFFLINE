# Asteroid Field System for EVE OFFLINE

**Version**: 1.0  
**Date**: February 2, 2026  
**Status**: Design & Implementation

---

## Overview

This document describes the asteroid field system based on EVE Online mechanics. Asteroid fields are designated, warp-toable locations containing mineable asteroids.

---

## Asteroid Types

### High-Sec Ores (0.5-1.0 Security)
- **Veldspar**: Most common, low value, high volume
- **Scordite**: Common, moderate minerals
- **Pyroxeres**: Common, good minerals
- **Plagioclase**: Uncommon, valuable minerals

### Low-Sec Ores (0.1-0.4 Security)
- **Omber**: Uncommon, good value
- **Kernite**: Rare, valuable
- **Jaspet**: Rare, very valuable

### Null-Sec Ores (0.0 Security)
- **Hemorphite**: Very rare, extremely valuable
- **Hedbergite**: Very rare, high-end minerals
- **Arkonor**: Ultra-rare, best minerals
- **Bistot**: Ultra-rare, premium value

### Size Variants
Each ore type has variants:
- **Standard**: Base ore
- **Concentrated**: 5% more yield
- **Dense**: 10% more yield
- **Crystalline**: 15% more yield (rare)

---

## Field Layouts

### Semicircle Layout
- **Shape**: Dense arching semicircle
- **Radius**: ~50 km
- **Asteroid Count**: 50-100
- **Distribution**: Clustered in small groups
- **Warp-in**: Center of arc

### Spherical Layout  
- **Shape**: Distributed sphere
- **Radius**: ~70 km
- **Asteroid Count**: 100-200
- **Distribution**: Scattered with clusters
- **Warp-in**: Center of sphere

---

## Mechanics

### Mining Process
1. **Warp to Belt**: Ship warps to center point
2. **Approach Asteroid**: Fly 10-15 km to target rock
3. **Activate Mining Laser**: Extract ore over time
4. **Collect Ore**: Ore goes to cargo hold
5. **Move to Next Rock**: When depleted

### Respawn System
- **Daily Downtime**: Asteroids respawn during server maintenance
- **Over-mining**: Heavily mined belts respawn smaller
- **Growth**: Partially mined belts can stay large or grow
- **Finite**: Asteroids are completely depletable

### Special Sites
- **Cosmic Anomalies**: Scanner-found, denser, better ore
- **Moon Mining**: Player-created temporary fields
- **Phased Fields**: Time-limited high-value sites

---

## Implementation Notes

### Data Structure
```json
{
  "asteroid_belts": [
    {
      "id": "belt_jita_1",
      "system": "Jita",
      "name": "Asteroid Belt I",
      "layout": "semicircle",
      "security": 1.0,
      "warp_point": {"x": 0, "y": 0, "z": 0},
      "asteroids": [
        {
          "id": "ast_001",
          "type": "Veldspar",
          "variant": "Dense",
          "position": {"x": 25000, "y": 5000, "z": 0},
          "size": "large",
          "ore_remaining": 50000,
          "ore_total": 50000
        }
      ],
      "npc_spawns": true,
      "last_respawn": "2026-02-02T00:00:00Z"
    }
  ]
}
```

### Mining System Components
- **MiningLaser** component (range, yield, cycle time)
- **CargoHold** component (capacity, current load)
- **MiningTarget** component (current asteroid)
- **MiningSystem** (handles extraction, depletion)

### Visual Representation
- 3D asteroid models (various sizes)
- Particle effects for mining laser
- Ore chunks floating (visual feedback)
- Belt overview markers

---

## Game Balance

### Mining Yields (per cycle)
- **Frigate** (Venture): 500 m³/min
- **Barge** (Procurer): 1,500 m³/min
- **Exhumer** (Hulk): 3,000 m³/min

### Asteroid Sizes
- **Small**: 5,000-10,000 m³
- **Medium**: 20,000-50,000 m³
- **Large**: 100,000-200,000 m³
- **Huge**: 500,000-1,000,000 m³

### Ore Values (per m³)
- **Veldspar**: 10 ISK
- **Scordite**: 25 ISK
- **Plagioclase**: 50 ISK
- **Kernite**: 150 ISK
- **Arkonor**: 500 ISK

---

**Status**: Ready for implementation in Phase 5C or Phase 6
