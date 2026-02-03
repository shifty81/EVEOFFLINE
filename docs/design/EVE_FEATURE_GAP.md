# EVE Online Feature Gap Analysis

## Overview
This document analyzes gameplay features from EVE Online that are missing or incomplete in EVE OFFLINE, prioritized by importance for a complete PVE experience.

## Critical Missing Features (High Priority)

### 1. **Sovereignty & Security Status System**
**EVE Online Implementation:**
- Security status ranges from 1.0 (high-sec) to -1.0 (null-sec)
- CONCORD response in high-sec
- Gate guns and faction police
- Criminal flagging and suspect timers

**Current Status:** Basic security levels exist but no enforcement
**Implementation Needed:**
- [ ] CONCORD NPC response system for high-sec
- [ ] Criminal flagging mechanics
- [ ] Security status tracking per player
- [ ] Faction standing system with consequences

---

### 2. **Advanced NPC AI & Aggression**
**EVE Online Implementation:**
- NPC rats spawn in belts and anomalies
- Aggression tables and priority targeting
- Rat types (frigates, cruisers, battleships) with behaviors
- Officer spawns and faction warfare NPCs

**Current Status:** Basic AI exists (approach, orbit, attack)
**Implementation Needed:**
- [ ] Aggression switching based on threat/DPS
- [ ] NPC fleet compositions
- [ ] Officer/Commander spawns with better loot
- [ ] Sleeper AI (wormhole space)
- [ ] Drifter/Triglavian equivalents

---

### 3. **Bounty & Reward System**
**EVE Online Implementation:**
- ISK bounties on NPC kills
- Mission rewards (ISK + LP + items)
- Insurance payouts
- Salvage from wrecks

**Current Status:** Basic bounty values in NPC data
**Implementation Needed:**
- [ ] Automatic bounty payout on NPC destruction
- [ ] Loyalty Points (LP) system
- [ ] LP stores with unique items
- [ ] Insurance contracts
- [ ] Salvaging mechanics with modules

---

### 4. **Wormhole & Null-Sec Mechanics**
**EVE Online Implementation:**
- Wormhole space with no local chat
- Sleeper NPCs and blue loot
- Null-sec with no CONCORD
- Anomalies and signatures that spawn dynamically

**Current Status:** Basic exploration signatures exist
**Implementation Needed:**
- [ ] Wormhole generation and connections
- [ ] Wormhole mass/time limits
- [ ] Sleeper cache sites
- [ ] Null-sec specific mechanics (bubbles, cynos)
- [ ] Dynamic anomaly spawning

---

### 5. **Market & Economy**
**EVE Online Implementation:**
- Buy/sell orders with escrow
- Regional markets
- Market hubs (Jita, Amarr)
- Order expiration and fees
- Hauling and logistics

**Current Status:** Basic market data structure
**Implementation Needed:**
- [ ] Order matching engine
- [ ] Broker fees and sales tax
- [ ] Order modification/cancellation
- [ ] Regional price differences
- [ ] Market API for clients

---

## Medium Priority Features

### 6. **Clone & Medical System**
**EVE Online Implementation:**
- Jump clones for fast travel
- Medical clones with implants
- Skill point loss on pod death
- Clone grades

**Current Status:** Not implemented
**Implementation Needed:**
- [ ] Clone bay stations
- [ ] Jump clone timer (24 hours)
- [ ] Implant system
- [ ] Death penalties

---

### 7. **Planetary Interaction (PI)**
**EVE Online Implementation:**
- Extract resources from planets
- Build command centers
- Factory planets for production chains
- Export products

**Current Status:** Not implemented
**Implementation Needed:**
- [ ] Planet scanning
- [ ] Resource extraction
- [ ] Production chains
- [ ] Customs offices

---

### 8. **Incursions & Live Events**
**EVE Online Implementation:**
- Sansha's Nation incursions
- High/Mid/Low-sec sites
- Influence levels
- Community goals

**Current Status:** Static mission system only
**Implementation Needed:**
- [ ] Dynamic incursion spawning
- [ ] Multiple difficulty tiers
- [ ] Fleet coordination rewards
- [ ] Influence mechanics

---

### 9. **Advanced Fitting & Modules**
**EVE Online Implementation:**
- Tech II modules (better stats, higher requirements)
- Faction modules (rare drops)
- Deadspace modules (from exploration)
- Mutated modules (Abyssal)
- Implants and boosters

**Current Status:** Basic modules exist, no tiers
**Implementation Needed:**
- [ ] Tech II module variants
- [ ] Faction module drops
- [ ] Deadspace loot tables
- [ ] Meta level system (0-5+)
- [ ] Implant slots

---

### 10. **Industry & Manufacturing**
**EVE Online Implementation:**
- Blueprint research (ME/TE)
- Invention (T2 blueprints)
- Reactions (moon goo)
- Capital ship construction
- Structure building

**Current Status:** Basic blueprint system
**Implementation Needed:**
- [ ] Material Efficiency research
- [ ] Time Efficiency research
- [ ] Invention mechanics
- [ ] Reaction formulas
- [ ] Capital components

---

## Lower Priority (Polish Features)

### 11. **Character Customization**
- Ship skins and paint jobs
- Character portraits
- Corporation logos
- Structure skins

### 12. **Social Features (Enhanced)**
- Alliance system
- Corporation wars
- Wardec mechanics
- Citadel structures

### 13. **Advanced Movement**
- Microwarp drive signature bloom
- Afterburner mechanics
- Inertia modifiers
- Warp disruption bubbles (null-sec)

### 14. **EWAR Expanded**
- Remote sensor dampening
- ECM jamming
- Tracking disruption
- Guidance disruption (missiles)

### 15. **Abyssal Deadspace**
- Filament entry system
- Time-limited pockets
- Escalating difficulty
- Mutaplasmid loot

---

## Recommended Implementation Order

### Phase 1: Core Economy & Rewards
1. Bounty payout system
2. Market order execution
3. Salvaging mechanics
4. Loot system improvements

### Phase 2: Advanced Combat
1. Improved NPC AI
2. Officer spawns
3. Security status system
4. CONCORD mechanics

### Phase 3: Exploration & Space
1. Wormhole system
2. Dynamic anomaly spawning
3. Sleeper NPCs
4. Null-sec mechanics

### Phase 4: Industry & Progression
1. Blueprint research
2. Invention system
3. Clone system
4. Implants

### Phase 5: Social & Endgame
1. Incursions
2. Planetary Interaction
3. Advanced corporation features
4. Alliance mechanics

---

## EVE-Specific Mechanics to Avoid

These EVE features are not suitable for a small-group PVE focus:

❌ **Player vs Player (PvP)**
- Ganking mechanics
- War declarations
- FW (Faction Warfare)
- Sov warfare

❌ **Extreme Grind**
- Months-long skill training
- Capital ship requirements
- Alliance politics

❌ **Market PvP**
- Market manipulation
- Station trading bots
- Scamming contracts

---

## Conclusion

To achieve a complete EVE-like PVE experience, focus on:
1. **Economy loop**: Kill NPCs → Get bounties → Buy better ships/modules
2. **Risk vs Reward**: High-sec (safe) → Null-sec (dangerous but profitable)
3. **Progression**: Frigates → Cruisers → Battleships → Capitals
4. **Content variety**: Missions, Exploration, Mining, Industry
5. **Social features**: Corporations, Fleet operations, Shared goals

The current implementation has ~40% of EVE's PVE features. Completing Phase 1-3 would bring it to ~75% feature parity.
