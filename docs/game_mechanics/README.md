# EVE 2026 Structures Quick Reference

This document provides quick access to information about EVE Online's 2026 player-owned structures and docking mechanics.

## 📁 Data Files

### Player Structures Data
**Location**: `data/universe/player_structures.json`

Contains complete data definitions for:
- Nullsec strategic structures (Orbital Skyhook, Sovereignty Hub, Metenox Moon Drill)
- Mining and industrial structures (Athanor 2026, Tatara 2026, modules)
- Wormhole-specific structures (Communal Granary, Upwell adjustments)
- Other deployables (CRAB, Advertising Centers, Small Personal Structures)
- Structure summary table
- 2026 design philosophy

### Station Types Data
**Location**: `data/universe/station_types.json`

Contains data for:
- NPC station types (Industrial, Military, Commercial, Research, University, Mining)
- Station visual layouts by faction (Amarr, Caldari, Gallente, Minmatar)
- Upwell structure types (Citadels, Engineering Complexes, Refineries)
- Structure features (Asset Safety, Access Control, Tax Rates, Fitting System)

## 📚 Documentation

### Structures 2026
**Location**: `docs/game_mechanics/STRUCTURES_2026.md`

Comprehensive documentation covering:
- **Nullsec Strategic Structures**: Orbital Skyhook, Sovereignty Hub, Metenox Moon Drill
- **Mining & Industrial Structures**: Updated Athanor/Tatara, Mining Survey Chipset, Industry Fitting System
- **Wormhole Structures**: Communal Granary, Upwell wormhole adjustments
- **Other Deployables**: CRAB, Advertising Centers, Small Personal Structures
- **Summary Table**: Quick reference of all structure types
- **Design Philosophy**: 2026 design changes and player impact

### Docking Mechanics
**Location**: `docs/game_mechanics/DOCKING_MECHANICS.md`

Complete guide to docking including:
- **Docked Scene**: Hangar view, NPC vs player-owned visuals, supercapital docking
- **Docked GUI**: Neocom, Station Services, interface elements
- **Key Differences**: NPC vs Player-Owned comparison table
- **Special Situations**: Wormhole docking, old POS, access denial
- **User Experience**: Docking/undocking sequences, QoL features

## 🎯 Key Structures at a Glance

| Structure | Type | Primary Role | Space Type |
|-----------|------|--------------|------------|
| **Orbital Skyhook** | Harvesting | Passive PI (Risky, Siphonable) | Nullsec |
| **Sovereignty Hub** | Control | System upgrades/resources | Nullsec |
| **Metenox Moon Drill** | Mining | Small-scale moon mining | Nullsec |
| **Communal Granary** | Boost | Mining yield bonus (Indestructible) | Wormhole |
| **Athanor** | Refinery | Moon mining (Medium) | All |
| **Tatara** | Refinery | Moon mining (Large) | All |
| **Keepstar** | Citadel | Supercapital staging (XL) | All |
| **CRAB** | Deployable | NPC combat for loot | Nullsec |

## 🔑 Key Features (2026)

### Industry Fitting System
- Structures now have slots like ships (High, Mid, Low, Rig)
- CPU and PowerGrid resource management
- Specialized modules for mining, defense, utility
- Example fits available in documentation

### Wormhole Changes
- **C1-C3**: 30% lower fuel costs + partial asset safety (new!)
- **C4-C6**: No asset safety (unchanged)
- **Communal Granary**: Non-destructible, provides system-wide mining bonuses

### Asset Safety
- **NPC Stations**: Absolute safety
- **Player Structures (High/Low/Null)**: Asset safety to nearest low-sec (15% fee)
- **Wormholes C1-C3**: Partial asset safety (15% fee)
- **Wormholes C4-C6**: No asset safety (full loot drop)

## 🎮 Design Philosophy

2026 structures emphasize:
- ✅ **Active Defense**: No more passive setups
- ✅ **Strategic Choices**: Specialized structures for specific roles
- ✅ **Collaboration**: Especially in wormholes (Communal Granary)
- ✅ **High Stakes**: Meaningful risk and reward
- ❌ **One-Size-Fits-All**: Gone - must choose specialization

## 🔗 Related Resources

- **Main README**: [README.md](../README.md)
- **Documentation Index**: [docs/README.md](README.md)
- **Roadmap**: [docs/ROADMAP.md](ROADMAP.md)
- **Game Design**: [docs/design/DESIGN.md](design/DESIGN.md)

---

*Last Updated: February 2026*
