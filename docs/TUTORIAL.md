# EVE OFFLINE - Getting Started Tutorial

Welcome to EVE OFFLINE! This tutorial will guide you through your first steps in the game.

## Table of Contents
1. [First Launch](#first-launch)
2. [Character Creation](#character-creation)
3. [Basic Controls](#basic-controls)
4. [Your First Ship](#your-first-ship)
5. [Combat Basics](#combat-basics)
6. [Fitting Your Ship](#fitting-your-ship)
7. [Your First Mission](#your-first-mission)
8. [Skills and Training](#skills-and-training)
9. [Making ISK](#making-isk)
10. [Next Steps](#next-steps)

---

## First Launch

### Starting the Client

**Linux/macOS:**
```bash
cd build/bin
./eve_client "YourName"
```

**Windows:**
```cmd
cd cpp_client\build_vs\bin\Release
eve_client.exe YourName
```

### Connecting to Server

The client will attempt to connect to:
- **Local server**: `localhost:7777`
- Or specify with: `./eve_client "YourName" --server 192.168.1.100:7777`

**First time?** You may need to start the server first:
```bash
cd cpp_server/build/bin
./eve_server
```

---

## Character Creation

When you first connect, you'll create a character:

### Choose Your Race

Each race has unique ship bonuses and aesthetics:

**Minmatar** 🔧
- Strengths: Speed, flexibility, projectile weapons
- Ships: Rifter, Thrasher, Stabber
- Best for: Fast combat, hit-and-run tactics

**Caldari** 🛡️
- Strengths: Shields, missiles, range
- Ships: Merlin, Cormorant, Caracal
- Best for: Long-range combat, defensive play

**Gallente** 🤖
- Strengths: Armor, drones, hybrid weapons
- Ships: Tristan, Catalyst, Vexor
- Best for: Drone tactics, versatile combat

**Amarr** ⚡
- Strengths: Armor, energy weapons, capacitor
- Ships: Punisher, Coercer, Maller
- Best for: Sustained combat, laser weapons

**Recommendation**: For beginners, try **Caldari** (easy shield tanking) or **Minmatar** (fast and forgiving).

### Starting Attributes

You'll start with basic attributes that affect:
- **Perception**: Weapon skills
- **Willpower**: Defensive skills
- **Intelligence**: Science/Industry
- **Charisma**: Social/Trade
- **Memory**: Engineering/Navigation

Don't worry too much about these initially - you can train skills regardless of attributes.

---

## Basic Controls

### Camera Controls
- **Mouse Wheel**: Zoom in/out
- **Right Click + Drag**: Rotate camera
- **Middle Click + Drag**: Pan camera
- **Double-click entity**: Focus camera

### Movement
- **Right-click in space**: Context menu
  - "Approach" - Move towards
  - "Orbit" - Circle at distance
  - "Keep at Range" - Maintain distance
- **WASD keys**: Manual flight (if enabled)
- **Double-click destination**: Warp to location

### Combat
- **Left-click entity**: Select target
- **Ctrl + Left-click**: Add to target list
- **Alt + Left-click**: Lock target
- **F1-F8**: Activate modules
- **Ctrl + F1-F8**: Group modules

### Interface
- **ESC**: Main menu
- **Tab**: Switch panels
- **I**: Inventory
- **C**: Character sheet
- **F**: Fitting window
- **M**: Market
- **L**: Mission log

---

## Your First Ship

You start with a basic **frigate** appropriate for your race:

### Ship Overview

Your ship has several key attributes:

**Hit Points (HP)**
- 🛡️ **Shield**: First line of defense, recharges automatically
- 🔧 **Armor**: Second layer, requires repairs
- ❤️ **Hull**: Last resort, ship explodes at 0%

**Capacitor** ⚡
- Powers active modules
- Recharges over time
- Watch capacitor % - running out is bad!

**Cargo Hold** 📦
- Stores items and loot
- Has limited capacity (m³)
- Can be expanded with mods

**Module Slots**
- **High**: Weapons, mining lasers
- **Medium**: Shields, tackle, EWAR
- **Low**: Armor, damage mods
- **Rigs**: Permanent upgrades

### Checking Your Ship

1. Open **Character Sheet** (C key)
2. View current ship stats
3. Check modules fitted
4. See cargo capacity

---

## Combat Basics

### Your First Fight

Let's hunt some pirates!

#### Step 1: Find Targets
- Undock from station
- Open **Overview** panel
- Look for red (hostile) NPCs
- Common targets: "Serpentis Scout", "Guristas Rookie"

#### Step 2: Approach Target
1. Select enemy in Overview
2. Right-click → "Approach"
3. Your ship will move towards target

#### Step 3: Lock Target
1. Click "Lock Target" button
2. Or press **Ctrl + Left-click** on target
3. Wait for lock to complete (watch scan resolution)

#### Step 4: Engage
1. Activate weapons (F1, F2, F3 keys)
2. Watch target's health bars
3. Monitor your own shields/armor
4. Keep firing until enemy is destroyed

#### Step 5: Loot
1. Target destroyed ship creates **wreck**
2. Approach wreck
3. Right-click → "Open Cargo"
4. Drag items to your cargo hold
5. Return to station to sell

### Combat Tips

**Do's** ✅
- Keep moving (orbit or approach)
- Watch capacitor usage
- Monitor your shields/armor
- Use appropriate ammo/damage type
- Warp out if taking heavy damage

**Don'ts** ❌
- Don't sit still (easy target)
- Don't run out of capacitor
- Don't let hull get damaged
- Don't engage multiple enemies alone (yet)
- Don't forget to loot!

---

## Fitting Your Ship

### Opening the Fitting Window

Press **F** or click "Fitting" in station menu.

### Understanding Fitting

Your ship has **limited resources**:
- **CPU** (tf): Required by all modules
- **PowerGrid** (MW): Required by all modules
- **Capacitor** (GJ): Consumed by active modules

**Rule**: Total module requirements ≤ Ship resources

### Basic Fitting Guide

**Frigate Combat Fit:**

**High Slots** (Weapons)
- 2-3x Small turrets (your race's weapon type)
- 1x Utility (salvager, probe launcher, etc.)

**Mid Slots** (Shield/EWAR)
- 1-2x Shield Extender (more HP)
- 1x Afterburner (speed boost)
- 1x Stasis Webifier (slow enemies)

**Low Slots** (Armor/Damage)
- 1-2x Damage mods (increase turret damage)
- 1x Armor plate or Nanofiber (HP or speed)

**Rigs** (Permanent)
- Whatever fits your strategy

### Fitting Process

1. Open Fitting window
2. Click empty slot
3. Browse available modules
4. Double-click to fit
5. Check CPU/PG bars (must not be red!)
6. Click "Apply" to save

### Saving Fits

1. Fit your ship
2. Click "Save Fitting"
3. Name it (e.g., "PVE Combat Frigate")
4. Load later with "Load Fitting"

---

## Your First Mission

Missions are PVE content that rewards ISK and LP (Loyalty Points).

### Getting a Mission

1. Dock at station
2. Open "Mission" panel
3. Browse available missions
4. Click "Accept"
5. Read the briefing

### Mission Types

**Level 1 Missions** (Start here!)
- **Combat**: Kill pirates
- **Courier**: Transport items
- **Mining**: Mine ore
- **Trade**: Buy/sell items

### Running a Combat Mission

1. Accept mission
2. Undock from station
3. Warp to mission location
4. Kill all hostiles
5. Complete objectives
6. Return to station
7. Click "Complete Mission"
8. Collect rewards!

### Mission Tips

- Start with **Level 1** missions
- Read briefing for enemy types
- Fit appropriate tank (shield/armor)
- Bring extra ammo
- Watch for escalations (harder enemies)
- Don't be afraid to warp out and repair

---

## Skills and Training

### Skill System Overview

Skills improve your character permanently:
- **Level 1-5** per skill
- Training happens in real-time (or accelerated)
- Prerequisites required for advanced skills
- Bonuses apply immediately

### Starting Skills

You begin with basic skills:
- **Spaceship Command**: Fly ships
- **Gunnery**: Use weapons  
- **Engineering**: Capacitor/CPU/PG
- **Shield/Armor Operation**: Defense
- **Navigation**: Speed/agility

### Training Your First Skill

1. Open **Character Sheet** (C key)
2. Click "Skills" tab
3. Browse skill tree
4. Select a skill
5. Click "Train to Level X"
6. Skill will train automatically

### Recommended Training Path

**First 24 Hours:**
1. Train racial frigate to 3
2. Train gunnery skills to 3
3. Train shield/armor skills to 3
4. Train weapon specialization to 3

**First Week:**
1. Train destroyer to 1
2. Train weapon skills to 4
3. Train tank skills to 4
4. Train navigation to 4

**First Month:**
1. Train cruiser to 1
2. Train all core skills to 4-5
3. Specialize in weapon type
4. Train fitting skills (CPU/PG)

---

## Making ISK

ISK (Interstellar Kredits) is the game currency. Here's how to earn it:

### Method 1: Missions (Easiest)
- **Effort**: Medium
- **ISK/hour**: 1-5M for Level 1-2
- **Requirements**: Combat ship
- Accept missions, complete objectives, collect rewards

### Method 2: Ratting (Simplest)
- **Effort**: Low
- **ISK/hour**: 0.5-2M
- **Requirements**: Combat ship
- Hunt NPCs in asteroid belts, loot wrecks, sell loot

### Method 3: Mining (Peaceful)
- **Effort**: Low
- **ISK/hour**: 1-3M
- **Requirements**: Mining ship, mining lasers
- Mine asteroids, refine ore, sell minerals

### Method 4: Trading (Advanced)
- **Effort**: High
- **ISK/hour**: 5-50M (with capital)
- **Requirements**: Market knowledge, ISK
- Buy low, transport, sell high

### Method 5: Exploration (Exciting)
- **Effort**: High
- **ISK/hour**: 0-20M (luck-based)
- **Requirements**: Probe scanner, exploration ship
- Scan signatures, run sites, loot valuable items

### Spending ISK Wisely

**Priorities:**
1. **Skills** - Many skills cost ISK to inject (if applicable)
2. **Ships** - Buy better ships as you progress
3. **Modules** - Upgrade your fitting
4. **Ammo** - Always have spare ammunition
5. **Insurance** - Protect your investment

**Don't:**
- Buy what you can't afford to lose
- Fly expensive ships without proper skills
- Neglect repairs and ammunition
- Gamble ISK in risky situations

---

## Next Steps

Congratulations! You've mastered the basics. Here's what to do next:

### Short Term Goals (Week 1)

- [ ] Complete 10 Level 1 missions
- [ ] Train core skills to level 3
- [ ] Earn your first 10M ISK
- [ ] Buy and fit a destroyer
- [ ] Try mining for an hour
- [ ] Join a corporation (optional)

### Medium Term Goals (Month 1)

- [ ] Complete Level 2 missions
- [ ] Train into a cruiser
- [ ] Earn 100M ISK
- [ ] Try exploration
- [ ] Experiment with different fits
- [ ] Run missions in a group

### Long Term Goals (Month 3+)

- [ ] Fly Tech II ships
- [ ] Complete Level 4 missions
- [ ] Try wormhole exploration
- [ ] Master market trading
- [ ] Build ships via manufacturing
- [ ] Lead a fleet

---

## Common Questions

### Q: How do I make ISK faster?
**A**: Run missions, upgrade your ship, train skills, use efficient fits. Don't rush - enjoy the journey!

### Q: What's the best ship?
**A**: No "best" ship - each ship has a role. Start with frigates, move to cruisers, then specialize.

### Q: How do I join a corporation?
**A**: Look for corp recruitment in social channels, or create your own!

### Q: What skills should I train first?
**A**: Core skills (CPU/PG, capacitor, shields, weapons). See [Training Path](#recommended-training-path).

### Q: I keep dying, what am I doing wrong?
**A**: Start with easier content (Level 1 missions), fit proper tank, watch your capacitor, warp out if low on HP.

### Q: How do I get better modules?
**A**: Buy from market, loot from missions, or build via manufacturing.

### Q: Can I play solo?
**A**: Yes! The game is designed for solo and small group play.

### Q: What's the endgame?
**A**: Whatever you want! Capital ships, manufacturing empire, exploration master, mission runner, trader.

---

## Getting Help

### In-Game
- Read tooltips (hover over anything)
- Check your mission briefings
- Ask in "Help" chat channel

### Out of Game
- Read [MODDING_GUIDE.md](MODDING_GUIDE.md) for content creation
- Check [ROADMAP.md](ROADMAP.md) for planned features
- Review [README.md](../README.md) for technical info

### Community
- GitHub Issues - Report bugs
- GitHub Discussions - Ask questions
- Discord - Chat with players

---

## Final Tips

1. **Take Your Time** - EVE OFFLINE is complex, don't rush
2. **Experiment** - Try different ships, fits, activities
3. **Ask Questions** - Community is helpful
4. **Have Fun** - It's a game, enjoy it!
5. **Stay Safe** - Don't fly what you can't afford to lose

**Most Important**: Everyone was a newbie once. Learn, adapt, and fly safe! o7

---

*Welcome to New Eden, capsuleer. Your adventure begins now.* 🚀

---

*Last Updated: February 7, 2026*
*Tutorial Version: 1.0*
