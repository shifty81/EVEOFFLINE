# AI Companion System - Design Document

## Overview

The AI Companion System allows players to hire persistent AI pilots that can be assigned to player-owned ships and commanded to perform various tasks autonomously. These AI companions act as NPC assistants in the player's wing/fleet, following orders and automating routine tasks.

## Core Concept

### Player's Wing Structure

```
Player (Commander)
├── Player Ship (you control directly)
└── AI Companions (autonomous)
    ├── AI Pilot 1 → Ship A (Mining Barge)
    ├── AI Pilot 2 → Ship B (Hauler)
    └── AI Pilot 3 → Ship C (Combat Ship)
```

### Key Features

1. **Persistent AI Pilots**: Hired permanently, retain skills/experience
2. **Ship Assignment**: Assign AI pilots to ships you own
3. **Command System**: Give orders to AI companions
4. **Task Automation**: AI handles routine tasks (hauling, mining, combat support)
5. **Fleet Integration**: AI companions are part of your wing
6. **Progression**: AI pilots gain experience and improve over time

## AI Companion Types

### 1. Hauler Pilot
**Primary Role**: Transport cargo and materials

**Capabilities**:
- Pick up jettisoned cargo/ore
- Transport items between locations
- Follow player and collect loot
- Return to station when cargo full
- Auto-sell items at market

**Commands**:
- `"Collect ore"` - Pick up nearby jettisoned cargo
- `"Follow me"` - Stay within X km of player
- `"Return to station"` - Dock and unload cargo
- `"Sell cargo"` - Sell all cargo at best prices
- `"Guard position"` - Stay at current location

### 2. Miner Pilot
**Primary Role**: Mine asteroids autonomously

**Capabilities**:
- Mine asteroids in current belt
- Fill cargo hold
- Return to station when full
- Resume mining after unloading

**Commands**:
- `"Mine here"` - Mine current asteroid belt
- `"Mine and haul"` - Mine then deliver to station
- `"Target [asteroid]"` - Mine specific asteroid type
- `"Stop mining"` - Cease operations

### 3. Combat Pilot
**Primary Role**: Provide combat support

**Capabilities**:
- Engage enemies attacking player
- Defend other AI companions
- Target selection and prioritization
- Use weapons and modules effectively
- Retreat when heavily damaged

**Commands**:
- `"Guard me"` - Stay close and defend player
- `"Engage targets"` - Attack hostile NPCs
- `"Hold fire"` - Don't attack unless attacked
- `"Return to base"` - Retreat to safety

### 4. Scout Pilot
**Primary Role**: Reconnaissance and information gathering

**Capabilities**:
- Scout ahead of player
- Report enemy positions
- Scan anomalies and signatures
- Mark locations of interest
- Monitor local space

**Commands**:
- `"Scout ahead"` - Move to next system and report
- `"Scan system"` - Scan for anomalies
- `"Watch for hostiles"` - Alert on enemy presence
- `"Return"` - Come back to player location

## AI Pilot Management

### Hiring System

#### Recruitment
```python
class AIPilot:
    def __init__(self, name, specialization, skill_level):
        self.id = generate_id()
        self.name = name
        self.specialization = specialization  # Hauler, Miner, Combat, Scout
        self.skill_level = skill_level  # 1-5
        self.experience = 0
        self.loyalty = 50  # 0-100
        self.salary = calculate_salary(skill_level)
        self.assigned_ship = None
        self.current_task = None
        self.status = "Available"  # Available, Busy, Docked, Undocking, etc.
```

#### Hiring Locations
- Space stations
- NPC corporations
- Player markets
- Mission rewards

#### Cost Structure
| Skill Level | Base Salary (ISK/hour) | Hiring Fee |
|-------------|------------------------|------------|
| 1 (Novice) | 10,000 | 100,000 |
| 2 (Competent) | 25,000 | 500,000 |
| 3 (Skilled) | 50,000 | 1,500,000 |
| 4 (Expert) | 100,000 | 5,000,000 |
| 5 (Elite) | 250,000 | 20,000,000 |

### Ship Assignment

```python
class ShipAssignment:
    def assign_pilot(self, pilot, ship):
        """Assign AI pilot to a player-owned ship"""
        if ship.owner != player.id:
            raise Exception("You don't own this ship")
        
        if pilot.assigned_ship:
            pilot.assigned_ship.pilot = None
        
        pilot.assigned_ship = ship
        ship.pilot = pilot
        pilot.status = "Assigned"
        
        # Verify pilot can fly the ship
        if not pilot.can_fly(ship.type):
            raise Exception(f"{pilot.name} cannot fly {ship.type}")
```

#### Requirements
- Player must own the ship
- AI pilot must have skills for that ship type
- Ship must be in same station/location
- Ship must not be in use by another pilot

### Experience & Progression

```python
class AIPilotProgression:
    def gain_experience(self, pilot, task_type, success):
        """AI pilots improve with experience"""
        base_xp = 100
        bonus_xp = 50 if success else 0
        
        pilot.experience += base_xp + bonus_xp
        
        # Level up check
        xp_needed = (pilot.skill_level + 1) * 10000
        if pilot.experience >= xp_needed:
            pilot.skill_level += 1
            pilot.experience = 0
            print(f"{pilot.name} leveled up to {pilot.skill_level}!")
            
            # Unlock new abilities
            self.unlock_abilities(pilot)
    
    def unlock_abilities(self, pilot):
        """Higher level pilots gain new abilities"""
        abilities = {
            2: ["efficient_routing", "cargo_optimization"],
            3: ["advanced_scanning", "threat_assessment"],
            4: ["predictive_behavior", "multi_tasking"],
            5: ["perfect_execution", "autonomous_decision_making"]
        }
        
        if pilot.skill_level in abilities:
            pilot.abilities.extend(abilities[pilot.skill_level])
```

## Command System

### Command Interface

#### In-Game UI
```
┌─────────────────────────────────────┐
│  AI Wing Management                 │
├─────────────────────────────────────┤
│ ┌─────────────────────────────────┐ │
│ │ AI Pilot: Marcus (Hauler)       │ │
│ │ Ship: Bestower                  │ │
│ │ Status: Following (2.5km)       │ │
│ │ Cargo: 4,500 / 20,000 m³       │ │
│ │                                 │ │
│ │ Commands:                       │ │
│ │ [Collect Ore] [Follow] [Dock]  │ │
│ │ [Guard Here] [Sell Cargo]      │ │
│ └─────────────────────────────────┘ │
│                                     │
│ ┌─────────────────────────────────┐ │
│ │ AI Pilot: Sarah (Combat)        │ │
│ │ Ship: Caracal                   │ │
│ │ Status: Guarding (1.2km)        │ │
│ │ Target: Guristas Frigate        │ │
│ │                                 │ │
│ │ Commands:                       │ │
│ │ [Guard] [Engage] [Hold Fire]   │ │
│ │ [Follow] [Return to Base]      │ │
│ └─────────────────────────────────┘ │
└─────────────────────────────────────┘
```

#### Chat Commands
```
/wing Marcus collect ore
/wing Sarah engage targets
/wing all follow me
/wing Marcus return to station
/fleet status
```

#### Quick Commands (Hotkeys)
- F1-F4: Command slots (customizable)
- Ctrl+1-9: Quick orders for specific pilots
- Alt+Click: "Go here" command

### Command Processing

```python
class CommandProcessor:
    def execute_command(self, pilot, command, parameters=None):
        """Process and execute AI pilot commands"""
        if not pilot.is_available():
            return {"success": False, "message": f"{pilot.name} is busy"}
        
        # Validate command
        if command not in pilot.available_commands():
            return {"success": False, "message": "Invalid command"}
        
        # Check if pilot can execute (skill, resources, etc.)
        if not pilot.can_execute(command, parameters):
            return {"success": False, "message": "Cannot execute command"}
        
        # Queue command
        task = Task(command, parameters, priority=1)
        pilot.task_queue.append(task)
        pilot.status = "Executing"
        
        return {"success": True, "message": f"{pilot.name} acknowledged"}
```

## AI Behavior System

### Behavior Tree Architecture

```
Root
├── Sequence: Active Task
│   ├── Check: Has Task?
│   ├── Execute: Current Task
│   └── Complete: Task Done
├── Selector: Emergency Response
│   ├── Flee: Low Health
│   ├── Evade: Under Attack
│   └── Repair: Damaged Modules
└── Sequence: Idle Behavior
    ├── Follow: Stay Near Player
    ├── Scan: Look for Opportunities
    └── Maintain: Optimal Position
```

### Task Execution

#### Hauler: Collect Ore Task
```python
class CollectOreTask(Task):
    def execute(self, pilot, ship, world):
        """Autonomous ore collection behavior"""
        # 1. Find jettisoned cargo
        nearby_cargo = world.find_jettisoned_cargo(
            position=ship.position,
            radius=50000,  # 50km
            owner=pilot.player_owner
        )
        
        if not nearby_cargo:
            return TaskStatus.COMPLETE
        
        # 2. Prioritize cargo (closest first)
        cargo = sorted(nearby_cargo, key=lambda c: distance(ship.position, c.position))
        target = cargo[0]
        
        # 3. Navigate to cargo
        if distance(ship.position, target.position) > 2500:  # 2.5km
            pilot.navigate_to(target.position)
            return TaskStatus.RUNNING
        
        # 4. Pick up cargo
        if ship.cargo_space_available() >= target.volume:
            ship.add_cargo(target.contents)
            world.remove_cargo_container(target.id)
            return TaskStatus.RUNNING
        else:
            # Cargo full - need to dock
            pilot.queue_task(DockAndUnloadTask())
            return TaskStatus.COMPLETE
```

#### Combat: Guard Player Task
```python
class GuardPlayerTask(Task):
    def execute(self, pilot, ship, world):
        """Combat AI protecting player"""
        player_ship = world.get_entity(pilot.player_owner)
        
        # 1. Stay within range of player
        max_distance = 5000  # 5km
        if distance(ship.position, player_ship.position) > max_distance:
            pilot.approach(player_ship.position, max_distance * 0.8)
            return TaskStatus.RUNNING
        
        # 2. Scan for threats
        threats = world.find_hostile_entities(
            position=player_ship.position,
            radius=50000
        )
        
        if not threats:
            return TaskStatus.RUNNING
        
        # 3. Prioritize threats by danger level
        priority_target = self.select_target(threats, player_ship)
        
        # 4. Engage target
        if pilot.can_reach_target(priority_target):
            pilot.lock_target(priority_target.id)
            pilot.fire_weapons(priority_target.id)
        else:
            pilot.approach(priority_target.position, ship.optimal_range)
        
        return TaskStatus.RUNNING
    
    def select_target(self, threats, player_ship):
        """Intelligent target selection"""
        scored_threats = []
        for threat in threats:
            score = 0
            
            # Closer threats are higher priority
            dist = distance(threat.position, player_ship.position)
            score += (50000 - dist) / 1000
            
            # Threats attacking player are highest priority
            if threat.target == player_ship.id:
                score += 100
            
            # Consider threat level
            score += threat.threat_rating * 10
            
            scored_threats.append((score, threat))
        
        # Return highest priority target
        return max(scored_threats, key=lambda x: x[0])[1]
```

### Autonomous Decision Making

```python
class AIDecisionMaker:
    def update(self, pilot, ship, world, dt):
        """AI makes autonomous decisions based on situation"""
        
        # Emergency checks (always highest priority)
        if ship.health_percentage() < 0.25:
            pilot.override_task(RetreatTask())
            return
        
        if ship.capacitor_percentage() < 0.10:
            pilot.stop_modules()
            return
        
        # Task-based behavior
        if pilot.has_active_task():
            pilot.execute_current_task()
        else:
            # No task - use idle behavior
            self.idle_behavior(pilot, ship, world)
    
    def idle_behavior(self, pilot, ship, world):
        """What AI does when idle"""
        player_ship = world.get_entity(pilot.player_owner)
        
        # Stay near player
        if distance(ship.position, player_ship.position) > 10000:
            pilot.approach(player_ship.position, 8000)
        
        # Opportunistic behavior based on specialization
        if pilot.specialization == "Hauler":
            # Look for jettisoned cargo to collect
            cargo = world.find_jettisoned_cargo(ship.position, 25000)
            if cargo:
                pilot.queue_task(CollectOreTask())
        
        elif pilot.specialization == "Combat":
            # Scan for threats
            threats = world.find_hostile_entities(player_ship.position, 30000)
            if threats:
                pilot.queue_task(GuardPlayerTask())
```

## Integration Points

### 1. Entity System

```python
# Extend Entity class
class Entity:
    def __init__(self):
        # ... existing fields ...
        self.ai_pilot = None  # Reference to AI pilot if controlled by AI
        self.is_ai_controlled = False
        self.player_owner = None  # Player who owns this AI
```

### 2. Fleet System

```python
class FleetSystem:
    def add_ai_companion(self, fleet, pilot, ship):
        """Add AI companion to player's wing"""
        member = FleetMember(
            entity_id=ship.id,
            player_id=pilot.player_owner,
            role="Support",
            is_ai=True,
            ai_pilot=pilot
        )
        fleet.members.append(member)
```

### 3. Network Protocol

```python
# New message types
class MessageType:
    AI_COMMAND = "ai_command"
    AI_STATUS = "ai_status"
    AI_REPORT = "ai_report"

# Message format
{
    "type": "ai_command",
    "pilot_id": "ai_123",
    "command": "collect_ore",
    "parameters": {}
}
```

### 4. UI Integration

```python
class AIWingPanel:
    def __init__(self, ui_system):
        self.ui_system = ui_system
        self.ai_pilots = []
        
    def render(self):
        """Render AI wing management panel"""
        for pilot in self.ai_pilots:
            self.render_pilot_card(pilot)
    
    def render_pilot_card(self, pilot):
        """Show individual AI pilot status"""
        panel = self.create_panel()
        
        # Pilot info
        panel.add_text(f"Name: {pilot.name}")
        panel.add_text(f"Type: {pilot.specialization}")
        panel.add_text(f"Ship: {pilot.assigned_ship.name if pilot.assigned_ship else 'None'}")
        panel.add_text(f"Status: {pilot.status}")
        
        # Current task
        if pilot.current_task:
            panel.add_text(f"Task: {pilot.current_task.description}")
            panel.add_progress_bar(pilot.current_task.progress())
        
        # Command buttons
        for cmd in pilot.available_commands():
            panel.add_button(cmd, lambda: self.send_command(pilot, cmd))
```

## Data Structures

### AI Pilot Data

```json
{
    "id": "ai_pilot_001",
    "name": "Marcus Stone",
    "specialization": "Hauler",
    "skill_level": 3,
    "experience": 5420,
    "loyalty": 75,
    "salary_per_hour": 50000,
    "hired_date": "2026-01-15T10:30:00Z",
    "assigned_ship_id": "ship_789",
    "abilities": [
        "efficient_routing",
        "cargo_optimization",
        "advanced_scanning"
    ],
    "stats": {
        "tasks_completed": 156,
        "ore_hauled": 5600000,
        "distance_traveled": 12500000,
        "profits_generated": 45000000
    },
    "current_task": {
        "type": "collect_ore",
        "status": "running",
        "progress": 0.65,
        "started": "2026-02-03T14:20:00Z"
    }
}
```

### Task Queue

```json
{
    "pilot_id": "ai_pilot_001",
    "tasks": [
        {
            "id": "task_001",
            "type": "collect_ore",
            "priority": 1,
            "parameters": {
                "radius": 50000,
                "ore_type": "any"
            },
            "status": "running"
        },
        {
            "id": "task_002",
            "type": "dock_and_unload",
            "priority": 2,
            "parameters": {
                "station_id": "station_123"
            },
            "status": "queued"
        }
    ]
}
```

## Persistence

### Database Schema

```sql
-- AI Pilots table
CREATE TABLE ai_pilots (
    id VARCHAR(50) PRIMARY KEY,
    player_id VARCHAR(50) NOT NULL,
    name VARCHAR(100) NOT NULL,
    specialization VARCHAR(20) NOT NULL,
    skill_level INT NOT NULL,
    experience INT DEFAULT 0,
    loyalty INT DEFAULT 50,
    salary_per_hour INT NOT NULL,
    hired_date TIMESTAMP NOT NULL,
    assigned_ship_id VARCHAR(50),
    status VARCHAR(20) DEFAULT 'Available',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (player_id) REFERENCES players(id),
    FOREIGN KEY (assigned_ship_id) REFERENCES ships(id)
);

-- AI Pilot Abilities
CREATE TABLE ai_pilot_abilities (
    pilot_id VARCHAR(50) NOT NULL,
    ability VARCHAR(50) NOT NULL,
    unlocked_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (pilot_id, ability),
    FOREIGN KEY (pilot_id) REFERENCES ai_pilots(id)
);

-- AI Pilot Stats
CREATE TABLE ai_pilot_stats (
    pilot_id VARCHAR(50) PRIMARY KEY,
    tasks_completed INT DEFAULT 0,
    ore_hauled BIGINT DEFAULT 0,
    distance_traveled BIGINT DEFAULT 0,
    profits_generated BIGINT DEFAULT 0,
    FOREIGN KEY (pilot_id) REFERENCES ai_pilots(id)
);

-- AI Task History
CREATE TABLE ai_task_history (
    id VARCHAR(50) PRIMARY KEY,
    pilot_id VARCHAR(50) NOT NULL,
    task_type VARCHAR(50) NOT NULL,
    started_at TIMESTAMP NOT NULL,
    completed_at TIMESTAMP,
    success BOOLEAN,
    details JSON,
    FOREIGN KEY (pilot_id) REFERENCES ai_pilots(id)
);
```

## Economy Integration

### Cost Management

```python
class AIEconomy:
    def calculate_hourly_costs(self, player):
        """Calculate total AI pilot costs"""
        total_cost = 0
        for pilot in player.ai_pilots:
            total_cost += pilot.salary
        return total_cost
    
    def process_salaries(self, world):
        """Deduct salaries from player wallets (hourly)"""
        for player in world.players:
            cost = self.calculate_hourly_costs(player)
            if player.wallet < cost:
                # Handle insufficient funds
                self.handle_unpaid_pilots(player)
            else:
                player.wallet -= cost
    
    def handle_unpaid_pilots(self, player):
        """Handle AI pilots when player can't pay"""
        for pilot in player.ai_pilots:
            pilot.loyalty -= 10
            if pilot.loyalty <= 0:
                # Pilot quits
                self.remove_pilot(player, pilot)
                player.send_message(f"{pilot.name} has quit due to unpaid salary!")
```

### Profit Sharing

```python
class ProfitSharing:
    def calculate_pilot_contribution(self, pilot):
        """Calculate value generated by AI pilot"""
        # Ore value
        ore_value = pilot.stats.ore_hauled * avg_ore_price
        
        # Combat contribution (kills * bounty)
        combat_value = pilot.stats.npcs_killed * avg_bounty
        
        # Bonus for efficiency
        efficiency_bonus = pilot.skill_level * 0.1  # 10% per level
        
        return (ore_value + combat_value) * (1 + efficiency_bonus)
```

## Advanced Features

### 1. AI Pilot Personalities

```python
class PersonalityTraits:
    CAUTIOUS = "cautious"      # Retreats early, avoids risks
    AGGRESSIVE = "aggressive"  # Engages more readily
    EFFICIENT = "efficient"    # Optimizes routes and tasks
    LOYAL = "loyal"           # Bonus loyalty, costs more
```

### 2. Training System

```python
class TrainingSystem:
    def train_pilot(self, pilot, training_type, duration_hours):
        """Train AI pilot to improve specific skills"""
        cost = duration_hours * 100000  # 100k per hour
        
        training = Training(
            pilot=pilot,
            type=training_type,
            duration=duration_hours,
            cost=cost
        )
        
        pilot.status = "Training"
        pilot.training_completion = time.now() + timedelta(hours=duration_hours)
```

### 3. Equipment & Fitting

```python
class AIFitting:
    def optimize_fitting(self, pilot, ship, role):
        """AI can suggest/apply fittings for their ship"""
        if pilot.skill_level < 3:
            return  # Only skilled pilots can optimize
        
        fitting = self.generate_optimal_fitting(ship.type, role)
        ship.apply_fitting(fitting)
```

### 4. Voice Commands (Future)

```python
class VoiceCommandSystem:
    def process_voice_command(self, audio_input):
        """Process voice commands for AI pilots"""
        # Speech-to-text
        text = self.transcribe(audio_input)
        
        # Parse command
        command = self.parse_natural_language(text)
        # "Marcus, collect the ore" → collect_ore command
        
        # Execute
        self.execute_command(command)
```

## Implementation Priority

### Phase 1: Core System (Weeks 1-2)
- [ ] AI Pilot data structures
- [ ] Basic command system
- [ ] Ship assignment
- [ ] Simple task execution (follow, guard)

### Phase 2: Hauler AI (Weeks 3-4)
- [ ] Collect ore task
- [ ] Dock and unload task
- [ ] Follow player behavior
- [ ] UI for hauler commands

### Phase 3: Combat AI (Weeks 5-6)
- [ ] Guard player task
- [ ] Engage targets task
- [ ] Threat assessment
- [ ] UI for combat commands

### Phase 4: Progression (Weeks 7-8)
- [ ] Experience system
- [ ] Skill leveling
- [ ] Ability unlocks
- [ ] Loyalty system

### Phase 5: Management (Weeks 9-10)
- [ ] Hiring system
- [ ] UI panels
- [ ] Statistics tracking
- [ ] Economy integration

### Phase 6: Advanced (Weeks 11-12)
- [ ] Miner AI
- [ ] Scout AI
- [ ] Training system
- [ ] Personality traits

## Testing Scenarios

### 1. Basic Hauler Test
```python
def test_basic_hauler():
    # 1. Hire hauler pilot
    pilot = hire_pilot("Marcus", "Hauler", skill_level=1)
    
    # 2. Assign to ship
    assign_pilot(pilot, player_ship("Bestower"))
    
    # 3. Player mines and jettisons ore
    mine_asteroid()
    jettison_ore(1000)  # 1000 m³
    
    # 4. Command hauler to collect
    command(pilot, "collect_ore")
    
    # 5. Verify hauler picks up ore
    wait(30)  # 30 seconds
    assert pilot.ship.cargo_volume() == 1000
    
    # 6. Command return to station
    command(pilot, "return_to_station")
    
    # 7. Verify docking and unload
    wait(60)
    assert pilot.status == "Docked"
    assert station_hangar_has_ore(1000)
```

### 2. Combat Support Test
```python
def test_combat_support():
    # 1. Hire combat pilot
    pilot = hire_pilot("Sarah", "Combat", skill_level=2)
    
    # 2. Assign combat ship
    assign_pilot(pilot, player_ship("Caracal"))
    
    # 3. Command to guard
    command(pilot, "guard_me")
    
    # 4. Spawn hostile NPC
    spawn_npc("Guristas Frigate", hostile=True, distance=10000)
    
    # 5. Verify AI engages
    wait(10)
    assert pilot.status == "In Combat"
    assert pilot.target is not None
    
    # 6. Verify AI retreats when low health
    pilot.ship.take_damage(pilot.ship.hull_max * 0.8)
    wait(5)
    assert pilot.status == "Retreating"
```

## Success Metrics

### Player Engagement
- % of players who hire AI pilots
- Average number of AI pilots per player
- Average time AI pilots are active
- Task completion rates

### Economic Impact
- Total ISK spent on AI salaries
- ISK earned through AI assistance
- ROI for players using AI pilots

### System Performance
- AI decision making latency
- Server load with AI pilots
- Path finding performance

## Future Expansion Ideas

1. **AI Pilot Market**: Trade trained AI pilots
2. **Clone Technology**: Backup AI pilot skills
3. **AI Corporations**: Form AI-run corporations
4. **Advanced Training**: Specialized skill trees
5. **AI vs AI**: AI pilots can compete
6. **Reputation System**: AI pilots have faction standings
7. **Dynamic Contracts**: AI pilots accept missions
8. **Fleet Commanders**: High-level AI that command other AI

## Conclusion

The AI Companion System adds a new dimension to EVEOFFLINE, allowing players to build their own crew and automate routine tasks while maintaining the challenge and engagement of the game. The system is designed to be:

- **Intuitive**: Simple commands, clear feedback
- **Scalable**: From 1 to many AI pilots
- **Balanced**: Costs and benefits carefully tuned
- **Immersive**: AI pilots feel like crew members
- **Extensible**: Room for many future features

This system integrates seamlessly with existing game systems and provides a foundation for rich emergent gameplay.
