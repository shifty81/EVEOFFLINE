# EVE Online Manual Reference

> This document maps chapters from the EVE Online manual (`eve_online_manual.pdf`) to their implementation in EVE OFFLINE. All game systems are structured around this manual.

> **Status**: In active R&D and development — actively testing until further notice.

## Manual Chapter to Implementation Mapping

| Ch. | Manual Chapter | Implementation | Data Files | Status |
|-----|---------------|----------------|------------|--------|
| 1 | About EVE Online | Core game design, lore | `docs/design/DESIGN.md` | ✅ Implemented |
| 3 | Character Creation | 4 races, bloodlines, attributes | `data/character_creation/races.json` | ✅ Data ready |
| 4 | Your First Days in Space | Tutorial flow, starter systems | `data/universe/systems.json` | ✅ Systems defined |
| 5 | Interface | ImGui EVE-styled UI | `cpp_client/src/ui/` | ✅ Implemented |
| 6 | Mining Guide | Mining lasers, ore, refining | `data/asteroid_fields/`, `data/modules/` | ✅ Implemented |
| 7 | Skill Guide | Attribute-based training, 18 categories | `data/skills/skills.json` | ✅ Implemented |
| 8 | Ship Fitting Guide | Slots, CPU/PG, turrets, launchers | `data/ships/`, `data/modules/` | ✅ Implemented |
| 9 | Fighter Guide | Combat skills, weapon types | `data/skills/skills.json`, `data/modules/` | ✅ Implemented |
| 10 | Pirate Hunting Guide | NPC pirates, bounties, belt rats | `data/npcs/pirates.json` | ✅ Implemented |
| 11 | Mission Guide | 7 mission types, agents, standings | `data/missions/`, `data/corporations/` | ✅ Implemented |
| 12 | Corporation Guide | NPC corps, player corps, research corps | `data/corporations/corporations.json` | ✅ Data ready |
| 13 | Manufacturing & Research | Blueprints, ME/TE, research agents | `data/industry/` | ✅ Implemented |
| 14 | Deadspace Complexes | 5 difficulty tiers, multi-room, escalation | `data/exploration/deadspace_complexes.json` | ✅ Data ready |
| 15 | Escrow | Contracts, escrow mechanics | `data/contracts/contracts.json` | ✅ Data ready |
| 17 | Legal System | CONCORD, security status, criminal flags | `data/security/concord_and_insurance.json` | ✅ Data ready |

## Systems Not in Manual (EVE OFFLINE Additions)

| System | Description | Data Files |
|--------|-------------|------------|
| Clone System | Medical clones, jump clones, death mechanics | `data/character_creation/clones.json` |
| Implant System | Attribute-boosting cybernetic implants | `data/character_creation/implants.json` |
| Insurance System | Ship hull insurance with coverage levels | `data/security/concord_and_insurance.json` |
| Planetary Interaction | Resource extraction from planets | `data/planetary_interaction/` |
| Ice Mining | Ice products for fuel blocks | `data/ice_types.json` |
| Gas Mining | Gas cloud harvesting | `data/gas_types.json` |
| Capital Ships | Carriers, Dreadnoughts, Titans | `data/ships/capitals.json` |

## Key Data Directories

```
data/
├── character_creation/     # Ch. 3 - Races, bloodlines, clones, implants
│   ├── races.json          # 4 races with bloodlines and attributes
│   ├── clones.json         # Medical and jump clone mechanics
│   └── implants.json       # Cybernetic implant system
├── ships/                  # Ch. 1, 8 - Ship definitions
│   ├── frigates.json       # Tech I frigates (Rifter, Merlin, Tristan, Punisher)
│   ├── destroyers.json     # Destroyers (Thrasher, Cormorant, Catalyst, Coercer)
│   ├── cruisers.json       # Tech I cruisers
│   ├── battlecruisers.json # Battlecruisers
│   ├── battleships.json    # Battleships
│   ├── mining_barges.json  # Mining Barges (Procurer, Retriever, Covetor)
│   ├── exhumers.json       # Exhumers (Skiff, Mackinaw, Hulk)
│   ├── tech2_frigates.json # Assault Ships, Interceptors, Covert Ops
│   ├── tech2_cruisers.json # HAC, HIC, Recon, Logistics
│   └── capitals.json       # Carriers, Dreadnoughts, Titans
├── modules/                # Ch. 8 - Module definitions
├── skills/                 # Ch. 7 - Skill definitions
│   ├── skills.json         # 100+ skills across 18 categories
│   └── science_skills.json # Research field skills
├── missions/               # Ch. 11 - Mission templates
│   ├── level1_missions.json
│   ├── level2_missions.json
│   ├── level3_missions.json
│   ├── level4_missions.json
│   └── level5_missions.json
├── npcs/                   # Ch. 9, 10 - NPC definitions
│   ├── pirates.json        # Pirate faction ships
│   └── hireable_pilots.json # NPC crew
├── universe/               # Ch. 4, 5 - Universe structure
│   ├── systems.json        # Solar systems with security ratings
│   ├── enhanced_systems.json
│   ├── station_types.json
│   └── warp_mechanics.json
├── security/               # Ch. 17 - Legal system
│   └── concord_and_insurance.json # CONCORD + insurance
├── corporations/           # Ch. 12 - Corporation system
│   └── corporations.json   # NPC and player corp mechanics
├── contracts/              # Ch. 15 - Escrow system
│   └── contracts.json      # Contract types and escrow
├── exploration/            # Ch. 14 - Deadspace complexes
│   └── deadspace_complexes.json
├── industry/               # Ch. 13 - Manufacturing
├── market/                 # Economy
├── asteroid_fields/        # Ch. 6 - Mining
├── planetary_interaction/  # PI resources
├── gas_types.json          # Gas mining
└── ice_types.json          # Ice mining
```

## Skill Categories (per EVE Manual Ch. 7)

| Category | Primary Attr | Secondary Attr | Skills |
|----------|-------------|----------------|--------|
| Gunnery | Perception | Willpower | Gunnery, Small/Medium/Large Turrets (Projectile, Hybrid, Energy), Weapon Upgrades, Motion Prediction, Rapid Firing, Sharpshooter |
| Missiles | Perception | Willpower | Missiles, Light/Heavy/Cruise Missiles, Torpedoes, Missile Launcher Operation |
| Spaceship Command | Perception | Willpower | Spaceship Command, Racial Frigate/Destroyer/Cruiser/BC/BS skills |
| Engineering | Intelligence | Memory | Engineering, Electronics, Shield Operation, Shield Management, Energy Management |
| Shields | Intelligence | Memory | Shield Operation, Shield Management, Shield Upgrades, Tactical Shield Manipulation |
| Armor | Intelligence | Memory | Armor Layering, EM/Thermal/Kinetic/Explosive Armor Compensation |
| Navigation | Intelligence | Perception | Navigation, Afterburner, Warp Drive Operation, Evasive Maneuvering |
| Targeting | Intelligence | Memory | Targeting, Long Range Targeting, Signature Analysis |
| Drones | Memory | Perception | Drones, Light/Medium/Heavy Drone Operation, Drone Interfacing |
| Electronic Warfare | Intelligence | Memory | Electronic Warfare, Propulsion Jamming, Signal Suppression |
| Learning | Memory | Intelligence | Learning, Instant Recall, Analytical Mind, Spatial Awareness, Iron Will, Empathy |
| Social | Charisma | Intelligence | Social, Connections, Negotiation, Fast Talk, Criminal Connections |
| Trade | Willpower/Charisma | Charisma/Memory | Trade, Retail, Accounting, Broker Relations, Marketing, Procurement |
| Leadership | Willpower | Charisma | Leadership, Mining Foreman, Skirmish/Information/Armored/Siege Warfare |
| Corp Management | Memory | Charisma | Corporation Management, Starbase Defense Management |
| Mechanic | Intelligence | Memory | Mechanic, Hull Upgrades, Repair Systems, Jury Rigging |
| Science | Intelligence | Memory | Science, Research, Cybernetics, Infomorph Psychology |
| Resource Processing | Memory | Intelligence | Reprocessing, Reprocessing Efficiency, Ore Processing specializations |

## Race Summary (per EVE Manual Ch. 3)

| Race | Weapon | Defense | Starting Ship | Starting System |
|------|--------|---------|---------------|-----------------|
| Amarr | Energy (Lasers) | Armor | Punisher | Amarr |
| Caldari | Hybrid + Missiles | Shields | Merlin | Jita |
| Gallente | Hybrid + Drones | Armor | Tristan | Dodixie |
| Minmatar | Projectile | Shield/Armor | Rifter | Hek |

## Mission Types (per EVE Manual Ch. 11)

| Type | Description | Example |
|------|-------------|---------|
| Combat (Kill) | Destroy specific NPC targets | "Serpentis Infestation" |
| Mining | Deliver ore or minerals to agent | "Mining Operation" |
| Courier | Transport items between stations | "Urgent Delivery" |
| Trade | Acquire items from market and deliver | "Supply Acquisition" |
| Scenario | Retrieve items from combat zones | "Contested Data Retrieval" |
| Exploration | Scan and retrieve items from signatures | Exploration sites |
| Storyline | Special missions affecting faction standing | "Proving Your Worth" |
