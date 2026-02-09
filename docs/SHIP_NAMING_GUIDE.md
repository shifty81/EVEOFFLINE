# Ship Naming Guide - Fan Nicknames & Community Slang

This document maps EVE Online ships to community-inspired names to avoid copyright issues while keeping the spirit of the game alive.

## Naming Philosophy

1. **Use Community Nicknames**: Leverage what players actually call ships in EVE
2. **Ironic Twists**: Add humor and personality
3. **Keep the Spirit**: Maintain the feel and character of each ship
4. **Avoid Direct Copies**: Never use exact EVE Online ship names

## Reference Resources

- EVE Online Ships Database (eveonlineships.com) - Visual references
- EVE University Wiki - Technical specs and characteristics
- ArtStation concepts by Elijah Sage - Faction ship designs
- Community forums and slang dictionaries

## Ship Name Mapping by Faction

### Minmatar Ships (Rustic, Asymmetric, "Rustbucket" Aesthetic)

| Original | Community Name | Our Name | Notes |
|----------|---------------|----------|-------|
| Rifter | "Rustbucket", "Flying Trashcan" | **Drifter** | The classic newbie frigate |
| Stabber | "Stabby" | **Stabby** | Perfect community nickname |
| Thrasher | - | **Thrasher** (keep, generic enough) | - |
| Rupture | - | **Fracture** | Play on words |
| Cyclone | - | **Typhoon Jr** (ironic) | Smaller than Typhoon |
| Tempest | "Autopest" | **Tempest** (keep, generic) | With autocannons |
| Typhoon | "Phoon" | **Monsoon** | Weather theme |
| Nidhoggur | "Niddy", "Nid" | **Nidhog** | Shortened version |
| Naglfar | - | **Nailgun** | Ironic twist on the name |
| Ragnarok | - | **Ragnarok** (keep, mythology) | - |

### Caldari Ships (Industrial, Blocky, "Brick" Aesthetic)

| Original | Community Name | Our Name | Notes |
|----------|---------------|----------|-------|
| Merlin | - | **Wizard** | Magic theme twist |
| Cormorant | - | **Cormorant** (keep, bird name) | - |
| Caracal | - | **Wildcat** | Similar cat species |
| Moa | - | **Moa** (keep, extinct bird) | - |
| Drake | "Brick", "Brickdrake" | **Brick** | Perfect community name |
| Ferox | - | **Ferro** | Shortened |
| Raven | "Caw-Caw", "Carebear Raven" | **Crow** | Smaller raven bird |
| Scorpion | - | **Scorpion** (keep, generic) | - |
| Chimera | - | **Chimera** (keep, mythology) | - |
| Phoenix | - | **Phoenix** (keep, mythology) | - |
| Leviathan | - | **Behemoth** | Biblical sea creature |

### Gallente Ships (Organic, Curved, Drone-focused)

| Original | Community Name | Our Name | Notes |
|----------|---------------|----------|-------|
| Tristan | - | **Isolde** | Other part of legend |
| Catalyst | - | **Catalyst** (keep, generic) | - |
| Vexor | - | **Vexer** | Slight variation |
| Thorax | - | **Thorax** (keep, anatomy) | - |
| Brutix | - | **Brutus** | Roman name |
| Myrmidon | - | **Spartan** | Greek warrior theme |
| Dominix | "Domi" | **Dominus** | Latin variation |
| Megathron | "Blasterthron" | **Megatron** | Pop culture twist |
| Thanatos | "Thanny" | **Reaper** | Death theme |
| Moros | - | **Doom** | Greek fate/doom |
| Erebus | - | **Abyss** | Greek darkness |

### Amarr Ships (Golden, Ornate, Cathedral-like)

| Original | Community Name | Our Name | Notes |
|----------|---------------|----------|-------|
| Punisher | - | **Inquisitor** | Religious theme |
| Coercer | - | **Enforcer** | Similar meaning |
| Maller | - | **Hammer** | Tool theme |
| Omen | - | **Omen** (keep, generic) | - |
| Harbinger | - | **Herald** | Similar meaning |
| Oracle | - | **Oracle** (keep, generic) | - |
| Apocalypse | "Apoc" | **Revelation** | Biblical theme |
| Armageddon | "Geddon" | **Armageddon** (keep, generic) | - |
| Archon | - | **Archon** (keep, generic) | - |
| Revelation | - | **Prophecy** | Similar meaning |
| Avatar | - | **Colossus** | Massive theme |

### Pirate/Special Faction Ships

| Original | Community Name | Our Name | Notes |
|----------|---------------|----------|-------|
| Vagabond | "Vaga" | **Nomad** | Wanderer theme |
| Cerberus | - | **Hound** | Three-headed dog simplified |
| Ishtar | - | **Astarte** | Similar goddess |
| Zealot | - | **Fanatic** | Similar meaning |

## Ship Class Slang Terms

Keep these generic community terms:
- **Frig** = Frigate
- **Dessie** = Destroyer  
- **Cruiser** = Cruiser (no change)
- **BC** = Battlecruiser
- **BS** = Battleship
- **Carrier** / "Carrier" = Carrier (no change)
- **Dread** = Dreadnought
- **Titan** = Titan (no change)
- **Ceptor** = Interceptor
- **Logi** = Logistics ship

## Implementation Strategy

1. **Phase 1**: Update ship JSON data files with new names
2. **Phase 2**: Update model.cpp ship type checking functions
3. **Phase 3**: Update documentation and comments
4. **Phase 4**: Test all references and ensure consistency

## Visual Design References

When implementing procedural generation, reference these sources:
- **eveonlineships.com**: Official ship screenshots for each faction
- **ArtStation (Elijah Sage)**: Custom concepts (Riptide, Intervention, Cockatrice, Spartan)
- **EVE University Wiki**: Faction aesthetic descriptions
- **Pinshape faction logos**: Visual motifs and iconography

### Faction Aesthetic Guidelines

**Minmatar**: Rusty metal, exposed framework, asymmetric, industrial, welded plates  
**Caldari**: Angular, city-block architecture, industrial blue-grey, blocky, functional  
**Gallente**: Smooth curves, organic forms, green tones, drone aesthetics, flowing  
**Amarr**: Golden armor, spires, vertical emphasis, cathedral-like, ornate panels

---

*Last Updated: February 9, 2026*  
*Based on community feedback and slang from EVE Online players*
