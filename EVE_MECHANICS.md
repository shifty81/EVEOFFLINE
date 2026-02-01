# EVE Online Mechanics Reference

This document provides detailed information about EVE Online game mechanics for implementation in EVE OFFLINE.

## 📋 Table of Contents
1. [Combat Mechanics](#combat-mechanics)
2. [Damage System](#damage-system)
3. [Tanking Strategies](#tanking-strategies)
4. [Capacitor System](#capacitor-system)
5. [Targeting System](#targeting-system)
6. [Drone Mechanics](#drone-mechanics)
7. [Electronic Warfare](#electronic-warfare)
8. [Aggression Mechanics](#aggression-mechanics)
9. [Mission Running](#mission-running)
10. [Skill Training](#skill-training)

---

## ⚔️ Combat Mechanics

### Weapon Systems

#### Turrets (Projectile, Energy, Hybrid)
**Damage Application Formula:**
```
base_damage = weapon_damage * (1 + skill_bonuses + ship_bonuses + module_bonuses)
actual_damage = base_damage * chance_to_hit * damage_multiplier
```

**Chance to Hit Factors:**
- Tracking speed
- Angular velocity of target
- Signature radius
- Optimal range and falloff

**Optimal Range:**
- 100% base damage within optimal range
- Damage falls off exponentially beyond optimal
- At optimal + falloff: ~50% damage
- At 2x falloff: ~6% damage

#### Missiles
**Damage Application:**
```
explosion_radius = missile_explosion_radius / (1 + missile_skill_bonuses)
signature_factor = min(1, target_signature / explosion_radius)
damage = base_damage * signature_factor
```

**Types:**
- **Light Missiles**: Anti-frigate, fast, short range
- **Heavy Missiles**: Anti-cruiser, medium range
- **Cruise Missiles**: Anti-battleship, long range, poor tracking
- **Torpedoes**: Anti-capital, very high damage, very poor tracking

---

## 💥 Damage System

### Damage Types
1. **EM (Electromagnetic)**
   - Amarr lasers
   - Blood Raiders, Sansha's Nation
   - Best vs: Shields (especially Caldari)
   - Worst vs: Armor (especially Amarr)

2. **Thermal**
   - Gallente blasters
   - Drones (most)
   - Best vs: Shields
   - Worst vs: Caldari shields

3. **Kinetic**
   - Caldari missiles, Gallente railguns
   - Guristas, Serpentis
   - Best vs: Shields (Caldari specialty)
   - Moderate vs: Armor

4. **Explosive**
   - Minmatar projectiles
   - Angel Cartel
   - Best vs: Shields (especially Minmatar)
   - Moderate vs: Armor

### Resistance Calculation
```
damage_taken = base_damage * (1 - resistance_percentage / 100)
effective_hp = raw_hp / (1 - average_resistance)
```

**Stacking Penalties:**
When multiple modules affect the same attribute:
- 1st module: 100% effective
- 2nd module: 86.9% effective
- 3rd module: 57.1% effective
- 4th module: 28.3% effective

**Formula:**
```
effectiveness = 0.5^((n-1)^2)
where n is the module number (1, 2, 3, etc.)
```

### Hit Points (Three Layers)
1. **Shield** (outermost)
   - Passive recharge (peak at 25-35% shield)
   - Active boosters
   - Best vs EM/thermal damage

2. **Armor** (middle layer)
   - No passive regen
   - Active repairers
   - Better base resists than shield

3. **Hull** (structure, last resort)
   - No passive regen
   - Rarely repaired in combat
   - Ship destroyed at 0% hull

---

## 🛡️ Tanking Strategies

### Shield Tanking
**Passive Shield Tank:**
- Shield recharge rate peaks at 25-35% capacity
- Use: Shield Power Relays, Shield Rechargers
- Best for: PVE ratting, low-attention activities
- Caldari ships excel at this

**Active Shield Tank:**
- Shield Boosters consume capacitor
- Instant HP restoration
- Best for: Burst tanking, PVP
- Requires good capacitor management

**Buffer Shield Tank:**
- Shield Extenders for maximum HP
- No capacitor needed
- Best for: Fleet PVP, alpha strikes
- High signature radius penalty

### Armor Tanking
**Passive Armor Tank:**
- Armor Plates for HP buffer
- Armor Hardeners for resistances
- No capacitor needed
- Velocity penalty from plates

**Active Armor Tank:**
- Armor Repairers consume capacitor
- Repairs at end of cycle
- Best for: Solo PVE, sustained combat
- Amarr/Gallente ships excel

**Damage Control:**
- Universal module
- +12.5% resists to ALL layers (shield, armor, hull)
- Highly recommended for every fit

### Hull Tanking
- Generally not recommended
- Emergency last-stand
- Some special ships (Brutix, etc.)

---

## ⚡ Capacitor System

### Capacitor Mechanics
**Recharge Rate:**
```
recharge_rate = capacitor_capacity / recharge_time
peak_recharge = 2.5 * average_recharge (at 25-30% cap)
```

**Capacitor Stability:**
- Cap stable: Recharge rate >= consumption rate
- Cap usage = sum of all active module consumption
- Flying "cap stable" is ideal for long missions

**Capacitor Skills:**
- **Capacitor Management**: +5% capacity per level
- **Capacitor Systems Operation**: -5% recharge time per level
- **Engineering**: Required for cap modules

### Energy Warfare
**Energy Neutralizers:**
- Drain target's capacitor
- Blood Raiders specialty
- Countered by: High cap pool, cap boosters

**Energy Vampires:**
- Drain target cap and add to your own
- Less effective than neuts at draining
- Useful for cap-hungry ships

**Nosferatu:**
- Only works if target has more cap % than you
- Free cap if it works

---

## 🎯 Targeting System

### Locking Targets
**Scan Resolution:**
- Time to lock = (40000 / (scan_resolution * (ship_signature / your_scan_resolution)))
- Faster scan resolution = faster locks
- Sensor Boosters increase scan resolution

**Maximum Targets:**
- Base: Ship-dependent (4-8 typically)
- **Targeting** skill: +1 target per level
- Locked targets remain until unlocked or out of range

**Targeting Range:**
- Ship base + skills + modules
- **Long Range Targeting**: +5% range per level
- Sensor Boosters increase range

### Signature Radius
- Smaller signature = harder to hit (turrets)
- Smaller signature = less missile damage
- Affects:
  - Turret tracking
  - Missile damage application
  - Scan resolution time
- Modified by:
  - Microwarpdrives (+500%)
  - Afterburners (+0-25%)
  - Target Painters (enemy debuff)

---

## 🤖 Drone Mechanics

### Drone Bandwidth
- Each ship has a bandwidth limit
- Each drone consumes bandwidth:
  - Light: 5 Mbit/s
  - Medium: 10 Mbit/s
  - Heavy: 25 Mbit/s
  - Sentry: 25 Mbit/s

**Example:**
- Vexor: 75 Mbit/s bandwidth
- Can control: 5 heavies OR 7 mediums + 1 light OR 15 lights

### Drone Bay
- Physical storage for drones
- Can carry more drones than you can control
- Swap drones mid-mission for different damage types

### Drone Commands
1. **Engage**: Attack current target
2. **Return to Bay**: Recall drones
3. **Return and Orbit**: Defensive mode
4. **Assist**: Help another player/ship
5. **Aggressive**: Auto-attack anything that attacks you
6. **Passive**: Only attack if commanded

### Drone Damage
```
drone_damage = base_damage * (1 + drone_skills + ship_bonuses)
drone_damage_per_second = drone_damage / cycle_time
```

**Key Skills:**
- **Drones**: +1 drone control per level (max 5 drones)
- **Light/Medium/Heavy Drone Operation**: +5% damage per level
- **Drone Interfacing**: +10% damage per level (advanced)
- **Drone Durability**: +5% HP per level
- **Drone Navigation**: +5% speed per level

---

## 📡 Electronic Warfare (EWAR)

### Stasis Webifiers
- Reduces target velocity by 50-60%
- Range: 10-20km
- Makes target easier to hit
- Stacks with multiple webs (with penalties)

### Warp Disruptors/Scramblers
- **Warp Disruptor**: Strength 1, 20-24km range
- **Warp Scrambler**: Strength 2, 7.5-10km range
- Prevents warp (need 1+ points)
- Scrams also shut off MWDs

### Target Painters
- Increases signature radius by 25-40%
- Makes target easier to hit with missiles/turrets
- Range: 24-60km
- Stackable with penalties

### Sensor Dampeners
- Reduces targeting range or scan resolution
- Choose one effect per module
- Gallente specialty

### ECM (Electronic Counter Measures)
- Jams target's sensors (breaks all locks)
- Chance-based (not guaranteed)
- 4 types (Gravimetric, Ladar, Magnetometric, RADAR)
- Caldari specialty

### Tracking Disruptors
- Reduces turret tracking speed or optimal range
- Choose one effect per module
- Amarr specialty

---

## 👊 Aggression Mechanics

### NPC Aggression
**Initial Aggression:**
- NPCs aggro first ship in range
- Or highest threat (damage dealers)

**Aggro Switching:**
- NPCs switch to highest DPS after initial aggro
- Frigates prioritize frigates
- Cruisers prioritize cruisers
- Battleships target anything

**Sleeper AI (Advanced):**
- Switches targets intelligently
- Focuses fire on single target
- Uses EWAR
- Assists other NPCs

### Aggro Range
- **Awareness Range**: NPCs become aware of you
- **Aggression Range**: NPCs actively engage
- Typically: 60-100km awareness, auto-aggro at 30-60km

### NPC Behavior States
1. **Idle**: Orbiting, not aggroed
2. **Approaching**: Moving toward player
3. **Orbiting**: Circling at optimal range
4. **Attacking**: Firing weapons
5. **Fleeing**: Low HP, warping out (rare)

---

## 🎯 Mission Running

### Mission Types
1. **Combat**: Kill specific NPCs
2. **Courier**: Transport items between stations
3. **Mining**: Collect ore
4. **Trade**: Buy/sell items (rare)
5. **Exploration**: Scan sites, retrieve items

### Mission Levels
- **Level 1**: Frigates, easy, 50-100k ISK
- **Level 2**: Frigates/Destroyers, 100-250k ISK
- **Level 3**: Cruisers, 250-500k ISK
- **Level 4**: Battleships, 1-5M ISK (EVE Online)
- **Level 5**: Fleet, 10M+ ISK (EVE Online)

### Standing Mechanics
- Completing missions increases standing
- Higher standing = better mission rewards
- Unlocks higher level agents
- Affects market fees and refining rates

### Blitz Missions
- Complete only required objectives
- Ignore unnecessary rats
- Faster ISK/hour
- May reduce loyalty points

---

## 📖 Skill Training

### Training Speed
```
SP_per_minute = primary_attribute + (secondary_attribute / 2)
SP_needed = 250 * multiplier * sqrt(32)^(level - 1)
```

**Example: Gunnery 5**
```
Multiplier: 1
Level 5 SP needed: 256,000 SP
With Perception 20, Willpower 20:
SP/min = 20 + 10 = 30
Time = 256,000 / 30 = 8,533 minutes = 5.9 days
```

### Skill Levels
- Level 1: Fast (base SP)
- Level 2: ~5.6x Level 1
- Level 3: ~31.6x Level 1
- Level 4: ~178x Level 1
- Level 5: ~1000x Level 1

**Training Priority:**
1. Core skills (CPU, PG, Cap, Shield/Armor, Navigation)
2. Weapon skills to 4 (5 for main weapon)
3. Spaceship command to fly desired ship
4. Support skills for chosen playstyle

### Attributes
- **Perception**: Gunnery, Missiles, Spaceship Command
- **Willpower**: Gunnery, Missiles, Combat
- **Intelligence**: Engineering, Electronics, Science
- **Memory**: Science, Drones, Manufacturing
- **Charisma**: Social, Trading

### Neural Remapping
- In EVE Online: Can remap once per year
- Optimize for skill plan
- EVE OFFLINE: Simplified or removed

---

## 🎮 Simplified Mechanics for EVE OFFLINE

### Recommendations for Implementation

1. **Combat**
   - Implement basic turret tracking
   - Simplified missile damage (no explosion mechanics)
   - Resistance system with 4 damage types

2. **Tanking**
   - Passive shield recharge
   - Active boosters/repairers
   - Buffer tanking with extenders/plates

3. **Capacitor**
   - Simple cap pool and regen
   - Active module consumption
   - Cap stability calculations

4. **Drones**
   - Bandwidth limits
   - Simple engage/return commands
   - Damage bonuses from skills/ship

5. **EWAR**
   - Webs and scrams (tacklingwebs)
   - Target painters
   - Skip complex ECM/dampeners for v1

6. **Skills**
   - Accelerated training (hours instead of days)
   - Simplified attribute system
   - Focus on 50-100 core skills

7. **Missions**
   - 4 mission levels
   - Simple completion objectives
   - Loyalty points and standing

---

## 📚 References

- [EVE University Wiki](https://wiki.eveuniversity.org/)
- [EVE Online Official Guide](https://www.eveonline.com/guides)
- Astrox Imperium mechanics (simplified EVE)

---

*This document is for implementation reference. Actual EVE Online is significantly more complex.*
*EVE OFFLINE aims to capture the feel while remaining accessible for small groups.*
