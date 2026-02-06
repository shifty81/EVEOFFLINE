# Phase 7: Research & Invention System

**Status**: ✅ COMPLETE  
**Date**: February 6, 2026  
**Test Coverage**: 8 tests, 100% pass rate

---

## Overview

The Research & Invention system allows players to invent Tech II blueprint copies from Tech I blueprint copies using datacores and optional decryptors. This system is based on EVE Online's invention mechanics with appropriate simplifications for EVE OFFLINE.

## Features

### Tech II Blueprint Invention

Players can attempt to invent Tech II BPCs from Tech I BPCs:

- **Input Materials**:
  - 1x Tech I Blueprint Copy (consumed on attempt, win or lose)
  - 2x Datacores (different types, consumed on attempt)
  - 1x Decryptor (optional, modifies success and results)

- **Success Chance**:
  - Base rate varies by item type (18-34%)
  - Increased by science skill levels (1% per level per skill)
  - Modified by decryptors (+/- 10% to 90%)
  - Formula: `base_rate * (1 + skill_bonus) * decryptor_modifier`

- **Results on Success**:
  - Tech II Blueprint Copy with limited runs
  - ME (Material Efficiency) level: 2 + decryptor modifier
  - TE (Time Efficiency) level: 4 + decryptor modifier
  - Runs: base runs (1-10 depending on item type) + decryptor modifier

### Base Success Rates

| Item Type      | Base Success Rate |
|----------------|-------------------|
| Modules        | 34%               |
| Frigates       | 30%               |
| Destroyers     | 30%               |
| Cruisers       | 25%               |
| Battlecruisers | 20%               |
| Battleships    | 18%               |

### Science Skills

#### Required Skills for Invention

Players must have trained the required skills for each invention job:

**Core Science Skills** (10 types):
- Mechanical Engineering - Armor, structure, projectile items
- Electronic Engineering - Electronics, shields, missiles
- Rocket Science - Missile launchers and ships
- Quantum Physics - Energy weapons and shields
- Molecular Engineering - Hybrid weapons and drones
- Graviton Physics - Propulsion and mining equipment
- Laser Physics - Energy weapons
- Plasma Physics - Hybrid weapons
- Nanite Engineering - Armor and repair equipment
- High Energy Physics - Shield and capacitor equipment

**Encryption Methods** (4 types):
- Amarr Encryption Methods
- Caldari Encryption Methods
- Gallente Encryption Methods
- Minmatar Encryption Methods

All science skills:
- Prerequisite: Science 3
- Training multiplier: 3x
- Max level: 5
- Effect: +1% invention success per level (per skill)

### Datacores

10 types of datacores, each corresponding to a science skill:

- Mechanical Engineering Datacore
- Electronic Engineering Datacore
- Rocket Science Datacore
- Quantum Physics Datacore
- Molecular Engineering Datacore
- Graviton Physics Datacore
- Laser Physics Datacore
- Plasma Physics Datacore
- Nanite Engineering Datacore
- High Energy Physics Datacore

**Acquisition Methods**:
1. **R&D Agents** - Passive generation via research projects
2. **Exploration** - Data sites (future)
3. **Market** - Purchase from other players (future)

### R&D Agents

Players can establish research projects with R&D agents:

**Starting a Research Project**:
- Requires trained science skill (level 1+)
- Generates Research Points (RP) passively
- Generation rate: 50 RP/day per skill level
- No limit on number of concurrent projects

**Collecting Datacores**:
- 100 RP = 1 Datacore
- Can collect at any time
- Datacores added directly to inventory

Example: With Mechanical Engineering 5, an agent generates 250 RP/day = 2.5 datacores/day

### Decryptors

10 types of decryptors with varying effects:

| Decryptor                    | Probability | Runs | ME  | TE  |
|------------------------------|-------------|------|-----|-----|
| Attainment                   | +80%        | +4   | +1  | +2  |
| Augmentation                 | -10%        | +9   | +2  | +0  |
| Optimized Attainment         | +90%        | +2   | +1  | -2  |
| Optimized Augmentation       | +0%         | +7   | +2  | +1  |
| Process Decryption           | +60%        | +1   | +3  | +6  |
| Symmetry                     | +10%        | +2   | +1  | +8  |
| Parity                       | +50%        | +3   | -2  | +1  |
| War Strategon                | +40%        | +0   | +2  | +4  |
| Occult Data                  | +90%        | +0   | +0  | +0  |
| Accelerant                   | +20%        | +1   | +0  | +10 |

**Decryptor Effects**:
- **Probability**: Modifies success chance (multiplicative)
- **Runs**: Added to base runs of T2 BPC
- **ME**: Added to base ME of T2 BPC (base 2)
- **TE**: Added to base TE of T2 BPC (base 4)

**Acquisition**: Exploration data sites (future implementation)

## Skill Bonus Calculation

The skill bonus is cumulative across all required skills:

**Example**: Inventing a Caldari Frigate
- Required skills: Mechanical Engineering, Electronic Engineering, Caldari Encryption Methods
- If all at level 5: 15 total skill levels
- Skill bonus: 15 × 1% = 15%
- Base success for frigate: 30%
- Final success: 30% × (1 + 0.15) = 34.5%

With Optimized Attainment decryptor (+90%):
- Final success: 30% × (1 + 0.15) × (1 + 0.90) = 65.6%

## T2 BPC Statistics

### Default Runs by Item Type

| Item Type      | Default Runs |
|----------------|--------------|
| Modules        | 10           |
| Frigates       | 1            |
| Destroyers     | 1            |
| Cruisers       | 1            |
| Battlecruisers | 1            |
| Battleships    | 1            |

### Default Efficiency Levels

All invented T2 BPCs start with:
- **ME**: 2 (before decryptor modifiers)
- **TE**: 4 (before decryptor modifiers)

These are lower than researched BPOs but sufficient for profitable manufacturing.

## Usage Examples

### Example 1: Basic Module Invention

**Goal**: Invent a Tech II Stasis Webifier

**Requirements**:
- Stasis Webifier I Blueprint Copy
- Molecular Engineering Datacore
- Quantum Physics Datacore
- Skills: Molecular Engineering 3, Quantum Physics 3, Gallente Encryption Methods 3

**Process**:
```python
# Player has materials in inventory
success, t2_bpc, msg = invention_system.attempt_invention(
    player,
    "stasis_web_t1_bpc",
    "datacore_molecular",
    "datacore_quantum"
)

# Base 34% for module * (1 + 9*0.01) = 37.06% success
# Result: T2 BPC with 10 runs, ME 2, TE 4
```

### Example 2: Frigate Invention with Decryptor

**Goal**: Invent a Hawk (Caldari Assault Frigate) with optimal stats

**Requirements**:
- Merlin Blueprint I Copy (base for Hawk)
- Mechanical Engineering Datacore
- Electronic Engineering Datacore
- Decryptor - Attainment (+80% success, +4 runs, +1 ME, +2 TE)
- Skills: Mechanical Engineering 5, Electronic Engineering 5, Caldari Encryption Methods 5

**Process**:
```python
success, t2_bpc, msg = invention_system.attempt_invention(
    player,
    "merlin_t1_bpc",
    "datacore_mech",
    "datacore_elec",
    "decryptor_attainment"
)

# Base 30% * (1 + 15*0.01) * (1 + 0.80) = 62.1% success
# Result: Hawk BPC with 5 runs (1+4), ME 3 (2+1), TE 6 (4+2)
```

### Example 3: R&D Agent Research

**Goal**: Generate datacores passively

**Setup**:
```python
# Start research project
invention_system.start_research_agent(
    player,
    "agent_jita_mecheng",
    "mechanical_engineering"  # Player has this at level 4
)

# After 10 days of game time
success, count, msg = invention_system.collect_datacores(
    player,
    "agent_jita_mecheng",
    days_elapsed=10
)

# Result: 20 Mechanical Engineering Datacores
# Calculation: 4 levels * 50 RP/day * 10 days = 2000 RP = 20 datacores
```

## API Reference

### InventionSystem Class

#### can_invent(entity, t1_bpc_id, datacore1_id, datacore2_id, decryptor_id=None)

Check if an entity can perform invention.

**Parameters**:
- `entity`: Entity object with Inventory and SkillTraining components
- `t1_bpc_id`: ID of T1 BPC in inventory
- `datacore1_id`: ID of first datacore in inventory
- `datacore2_id`: ID of second datacore in inventory
- `decryptor_id`: Optional ID of decryptor in inventory

**Returns**: `(bool, str)` - (can_invent, error_message)

#### calculate_success_chance(entity, t1_bpc_id, decryptor_id=None)

Calculate the probability of successful invention.

**Parameters**:
- `entity`: Entity object
- `t1_bpc_id`: ID of T1 BPC
- `decryptor_id`: Optional decryptor ID

**Returns**: `float` - Success chance (0.0 to 1.0)

#### attempt_invention(entity, t1_bpc_id, datacore1_id, datacore2_id, decryptor_id=None)

Attempt to invent a T2 BPC.

**Parameters**:
- `entity`: Entity object
- `t1_bpc_id`: ID of T1 BPC (will be consumed)
- `datacore1_id`: ID of first datacore (will be consumed)
- `datacore2_id`: ID of second datacore (will be consumed)
- `decryptor_id`: Optional decryptor ID (will be consumed if provided)

**Returns**: `(bool, Optional[Dict], str)` - (success, t2_bpc_data, message)

**Note**: Materials are consumed whether the invention succeeds or fails.

#### start_research_agent(entity, agent_id, science_skill)

Start a research project with an R&D agent.

**Parameters**:
- `entity`: Entity object
- `agent_id`: Unique agent identifier
- `science_skill`: Name of science skill to research

**Returns**: `(bool, str)` - (success, message)

**Requirements**: Player must have the science skill trained to at least level 1.

#### collect_datacores(entity, agent_id, days_elapsed)

Collect datacores from an active research project.

**Parameters**:
- `entity`: Entity object
- `agent_id`: Agent identifier
- `days_elapsed`: Number of days since last collection (or project start)

**Returns**: `(bool, int, str)` - (success, datacores_collected, message)

**Note**: 100 RP = 1 datacore. Minimum 100 RP required to collect.

## Testing

### Test Suite

8 comprehensive tests covering all invention mechanics:

1. ✅ **Basic Invention** - Standard invention without decryptor
2. ✅ **Invention with Decryptor** - Tests decryptor modifiers
3. ✅ **Skill Requirements** - Validates skill checks
4. ✅ **BPO Restriction** - Cannot invent from BPOs
5. ✅ **R&D Agent** - Research project and datacore collection
6. ✅ **Multiple Attempts** - Verifies randomness and probability
7. ✅ **Missing Materials** - Error handling for missing items
8. ✅ **Skill Bonus Scaling** - Confirms skill effects

**Run Tests**:
```bash
cd archive/python
python tests/test_invention.py
```

**Expected Output**:
```
============================================================
INVENTION SYSTEM TEST SUITE
============================================================
✓ Basic invention test passed
✓ Decryptor test passed
✓ Skill requirement test passed
✓ BPO restriction test passed
✓ R&D agent test passed
✓ Multiple attempts test passed
✓ Missing materials test passed
✓ Skill scaling test passed
============================================================
Test Results: 8 passed, 0 failed
============================================================
🎉 ALL TESTS PASSED! 🎉
```

## Files

### System Code
- `archive/python/engine/systems/invention_system.py` - Main invention system (294 lines)

### Data Files
- `data/industry/datacores.json` - 10 datacore definitions
- `data/industry/decryptors.json` - 10 decryptor definitions
- `data/skills/science_skills.json` - 15 science skill definitions

### Tests
- `archive/python/tests/test_invention.py` - Complete test suite (484 lines)

### Documentation
- `docs/development/PHASE7_RESEARCH_INVENTION.md` - This file

## Statistics

### Code
- **System Code**: 294 lines
- **Test Code**: 484 lines
- **Data Definitions**: 35 items (10 datacores + 10 decryptors + 15 skills)
- **Total**: ~800 lines

### Coverage
- **Test Functions**: 8
- **Success Rate**: 100% (8/8 passing)
- **Components Tested**: Invention, Datacores, Decryptors, R&D Agents, Skills

## Integration

The invention system integrates with:

1. **Inventory System** - Stores BPCs, datacores, decryptors
2. **Skill Training** - Checks skill levels, affects success rates
3. **Industry System** - T2 BPCs used for manufacturing
4. **Market System** - Datacores and decryptors tradeable (future)
5. **Exploration System** - Decryptors from data sites (future)

## Future Enhancements

Potential improvements for future phases:

- [ ] **Ancient Relics** - Reverse engineering for T2 components
- [ ] **Tech III Invention** - Advanced invention for T3 ships
- [ ] **Invention UI** - Visual invention interface with success calculator
- [ ] **Agent Reputation** - Better agents with higher RP generation
- [ ] **Meta Level Invention** - Invent faction/deadspace items
- [ ] **Invention Teams** - Temporary bonuses for invention jobs
- [ ] **Batch Invention** - Queue multiple invention attempts
- [ ] **Invention History** - Track success/failure statistics

## Conclusion

The Research & Invention system provides a complete, balanced implementation of EVE Online's Tech II blueprint invention mechanics. Players can:

- Invent T2 BPCs from T1 BPCs with skill-based success rates
- Use decryptors to optimize results for their needs
- Generate datacores passively through R&D agents
- Access all 10 science skills and 4 encryption methods
- Choose from 10 different decryptors with unique effects

The system is fully tested, balanced, and ready for integration with manufacturing and market systems.

---

**Implementation**: ✅ Complete  
**Testing**: ✅ 100% Pass Rate  
**Documentation**: ✅ Complete  
**Integration**: ✅ Ready
