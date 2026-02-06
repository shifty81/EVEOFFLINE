"""
Wing Commander System
Core differentiating feature of EVE OFFLINE - players command their own wing
of NPC pilots flying player-owned ships. NPCs can autonomously mine, haul,
run trade routes, do combat, and more based on their skills and orders.

Max wing size: 26 (mix of real players + NPC pilots)
"""

from typing import Optional, Dict, List, Tuple
from dataclasses import dataclass, field
from enum import Enum


class PilotSpecialization(Enum):
    """NPC pilot specializations"""
    COMBAT = "combat"
    MINER = "miner"
    HAULER = "hauler"
    SCOUT = "scout"
    TRADER = "trader"       # Runs trade routes autonomously
    INDUSTRIALIST = "industrialist"  # Handles PI and manufacturing


class PilotStatus(Enum):
    """Current status of an NPC pilot"""
    AVAILABLE = "available"       # Idle at station, ready for orders
    ASSIGNED = "assigned"         # Assigned to ship, awaiting orders
    EXECUTING = "executing"       # Actively executing a task
    TRAVELING = "traveling"       # In transit (warp/gate)
    MINING = "mining"             # Actively mining
    HAULING = "hauling"           # Hauling cargo
    TRADING = "trading"           # Running trade route
    COMBAT = "in_combat"          # Engaged in combat
    DOCKED = "docked"             # Docked at station
    RETREATING = "retreating"     # Fleeing from danger
    TRAINING = "training"         # Being trained
    OFFLINE = "offline"           # Dismissed/inactive


class TaskType(Enum):
    """Types of tasks that can be assigned to NPC pilots"""
    FOLLOW = "follow"                   # Follow commander
    GUARD = "guard"                     # Guard a position or entity
    MINE_HERE = "mine_here"             # Mine current belt
    MINE_AND_HAUL = "mine_and_haul"     # Mine then deliver to station
    COLLECT_ORE = "collect_ore"         # Pick up jettisoned ore
    HAUL_TO_STATION = "haul_to_station" # Deliver cargo to station
    TRADE_ROUTE = "trade_route"         # Run a preset trade route
    ENGAGE_TARGETS = "engage_targets"   # Attack hostile NPCs
    HOLD_FIRE = "hold_fire"             # Don't attack
    RETURN_TO_BASE = "return_to_base"   # Dock at home station
    SCOUT_AHEAD = "scout_ahead"         # Scout the next system
    ORBIT = "orbit"                     # Orbit a target at range
    SELL_CARGO = "sell_cargo"           # Sell cargo at market
    PI_MANAGE = "pi_manage"             # Manage planetary colonies


class TaskPriority(Enum):
    """Task priority levels"""
    LOW = 0
    NORMAL = 1
    HIGH = 2
    EMERGENCY = 3  # Emergency retreat, etc.


@dataclass
class PilotTask:
    """A task assigned to an NPC pilot"""
    task_id: str
    task_type: TaskType
    priority: TaskPriority = TaskPriority.NORMAL
    parameters: Dict = field(default_factory=dict)
    status: str = "queued"      # queued, running, completed, failed
    progress: float = 0.0       # 0.0 to 1.0
    started_time: float = 0.0
    completed_time: float = 0.0
    result: Dict = field(default_factory=dict)


@dataclass
class TradeRoute:
    """A trade route that a hauler/trader NPC can run repeatedly"""
    route_id: str
    owner_id: str
    name: str
    # List of stops: [(station_id, action, item_id, quantity)]
    # action: "buy", "sell", "pickup", "deliver"
    stops: List[Dict] = field(default_factory=list)
    is_active: bool = True
    loop: bool = True           # Repeat the route continuously
    created_time: float = 0.0


@dataclass
class PilotStats:
    """Cumulative statistics for an NPC pilot"""
    tasks_completed: int = 0
    tasks_failed: int = 0
    ore_mined: float = 0.0          # m3 of ore mined
    ore_hauled: float = 0.0         # m3 of cargo hauled
    distance_traveled: float = 0.0  # meters traveled
    npcs_killed: int = 0
    isk_earned: float = 0.0         # ISK generated
    time_active: float = 0.0        # seconds active


@dataclass
class AIPilot:
    """
    An NPC pilot that can be hired to fly player-owned ships.
    Core component of the wing management system.
    """
    pilot_id: str
    name: str
    owner_id: str                   # Player who hired this pilot
    specialization: PilotSpecialization
    skill_level: int = 1            # 1 (Novice) to 5 (Elite)
    experience: int = 0             # XP toward next level
    loyalty: int = 50               # 0-100, affects performance and quit chance
    salary_per_hour: float = 10000  # ISK per hour
    hiring_fee: float = 100000      # One-time hiring cost

    # Ship assignment
    assigned_ship_id: Optional[str] = None
    assigned_ship_type: str = ""

    # Current state
    status: PilotStatus = PilotStatus.AVAILABLE
    current_task: Optional[PilotTask] = None
    task_queue: List[PilotTask] = field(default_factory=list)

    # Location
    system_id: str = ""
    station_id: Optional[str] = None  # None if in space
    position_x: float = 0.0
    position_y: float = 0.0
    position_z: float = 0.0

    # Statistics
    stats: PilotStats = field(default_factory=PilotStats)

    # Abilities unlocked based on level
    abilities: List[str] = field(default_factory=list)

    # Personality trait (affects behavior)
    personality: str = "balanced"    # cautious, aggressive, efficient, loyal

    # Timestamps
    hired_time: float = 0.0
    last_paid_time: float = 0.0

    # Entity reference (when undocked and active in world)
    entity_id: Optional[str] = None


# Salary and hiring costs per skill level
PILOT_COSTS = {
    1: {"salary": 10000,  "hiring_fee": 100000},
    2: {"salary": 25000,  "hiring_fee": 500000},
    3: {"salary": 50000,  "hiring_fee": 1500000},
    4: {"salary": 100000, "hiring_fee": 5000000},
    5: {"salary": 250000, "hiring_fee": 20000000},
}

# XP required to level up
LEVEL_UP_XP = {
    1: 10000,
    2: 25000,
    3: 50000,
    4: 100000,
    5: None,  # Max level
}

# Abilities unlocked at each level
LEVEL_ABILITIES = {
    1: ["basic_tasks"],
    2: ["efficient_routing", "cargo_optimization"],
    3: ["advanced_scanning", "threat_assessment", "multi_target"],
    4: ["predictive_behavior", "multi_tasking", "auto_repair"],
    5: ["perfect_execution", "autonomous_decision", "fleet_coordination"],
}

# Which ship classes each specialization can fly
SPECIALIZATION_SHIPS = {
    PilotSpecialization.COMBAT: [
        "Frigate", "Destroyer", "Cruiser", "Battlecruiser", "Battleship"
    ],
    PilotSpecialization.MINER: [
        "Frigate", "Mining Barge", "Exhumer"
    ],
    PilotSpecialization.HAULER: [
        "Industrial", "Frigate", "Destroyer"
    ],
    PilotSpecialization.SCOUT: [
        "Frigate", "Destroyer", "Cruiser"
    ],
    PilotSpecialization.TRADER: [
        "Industrial", "Frigate"
    ],
    PilotSpecialization.INDUSTRIALIST: [
        "Industrial", "Frigate"
    ],
}

# Maximum wing size (player + NPC pilots + other real players)
MAX_WING_SIZE = 26


class WingCommanderSystem:
    """
    Manages the player's wing of NPC pilots.
    Players hire NPC pilots, assign them to ships, and give them orders.
    NPCs execute tasks autonomously based on their skills and commands.
    """

    def __init__(self):
        # All hired pilots: {pilot_id: AIPilot}
        self.pilots: Dict[str, AIPilot] = {}
        # Player's wing roster: {owner_id: [pilot_id]}
        self.player_wings: Dict[str, List[str]] = {}
        # Trade routes: {route_id: TradeRoute}
        self.trade_routes: Dict[str, TradeRoute] = {}
        # Counters
        self._pilot_counter = 0
        self._task_counter = 0
        self._route_counter = 0

    def get_wing_size(self, owner_id: str) -> int:
        """Get current number of NPC pilots in player's wing"""
        return len(self.player_wings.get(owner_id, []))

    def can_hire(self, owner_id: str) -> bool:
        """Check if player can hire more NPC pilots"""
        return self.get_wing_size(owner_id) < MAX_WING_SIZE

    def hire_pilot(
        self,
        owner_id: str,
        name: str,
        specialization: PilotSpecialization,
        skill_level: int = 1,
        personality: str = "balanced",
        current_time: float = 0.0,
        wallet_isk: float = 0.0
    ) -> Optional[AIPilot]:
        """
        Hire a new NPC pilot.

        Args:
            owner_id: Player ID hiring the pilot
            name: Name for the NPC pilot
            specialization: Pilot's role specialization
            skill_level: Starting skill level (1-5)
            personality: Personality trait
            current_time: Current game timestamp
            wallet_isk: Player's current ISK

        Returns:
            AIPilot if hired successfully, None otherwise
        """
        # Validate
        if not self.can_hire(owner_id):
            return None

        skill_level = max(1, min(5, skill_level))
        costs = PILOT_COSTS[skill_level]

        if wallet_isk < costs["hiring_fee"]:
            return None

        self._pilot_counter += 1
        pilot_id = f"pilot_{self._pilot_counter}"

        pilot = AIPilot(
            pilot_id=pilot_id,
            name=name,
            owner_id=owner_id,
            specialization=specialization,
            skill_level=skill_level,
            salary_per_hour=costs["salary"],
            hiring_fee=costs["hiring_fee"],
            personality=personality,
            hired_time=current_time,
            last_paid_time=current_time,
            abilities=list(LEVEL_ABILITIES.get(skill_level, ["basic_tasks"])),
        )

        self.pilots[pilot_id] = pilot

        if owner_id not in self.player_wings:
            self.player_wings[owner_id] = []
        self.player_wings[owner_id].append(pilot_id)

        return pilot

    def dismiss_pilot(self, owner_id: str, pilot_id: str) -> bool:
        """Dismiss an NPC pilot from the wing."""
        pilot = self.pilots.get(pilot_id)
        if not pilot or pilot.owner_id != owner_id:
            return False

        pilot.status = PilotStatus.OFFLINE
        pilot.assigned_ship_id = None

        if owner_id in self.player_wings:
            if pilot_id in self.player_wings[owner_id]:
                self.player_wings[owner_id].remove(pilot_id)

        del self.pilots[pilot_id]
        return True

    def assign_ship(
        self,
        owner_id: str,
        pilot_id: str,
        ship_entity_id: str,
        ship_type: str,
        ship_class: str
    ) -> bool:
        """
        Assign an NPC pilot to a player-owned ship.

        Args:
            owner_id: Player ID (must own both pilot and ship)
            pilot_id: NPC pilot to assign
            ship_entity_id: Ship entity ID
            ship_type: Ship type name (e.g., 'Rifter')
            ship_class: Ship class (e.g., 'Frigate')
        """
        pilot = self.pilots.get(pilot_id)
        if not pilot or pilot.owner_id != owner_id:
            return False

        # Check if pilot can fly this ship class
        allowed_classes = SPECIALIZATION_SHIPS.get(pilot.specialization, [])
        if ship_class not in allowed_classes:
            return False

        # Unassign from previous ship
        if pilot.assigned_ship_id:
            pilot.assigned_ship_id = None

        pilot.assigned_ship_id = ship_entity_id
        pilot.assigned_ship_type = ship_type
        pilot.status = PilotStatus.ASSIGNED

        return True

    def unassign_ship(self, owner_id: str, pilot_id: str) -> bool:
        """Remove a pilot from their assigned ship."""
        pilot = self.pilots.get(pilot_id)
        if not pilot or pilot.owner_id != owner_id:
            return False

        pilot.assigned_ship_id = None
        pilot.assigned_ship_type = ""
        pilot.status = PilotStatus.AVAILABLE
        pilot.current_task = None
        pilot.task_queue.clear()

        return True

    def give_order(
        self,
        owner_id: str,
        pilot_id: str,
        task_type: TaskType,
        parameters: Optional[Dict] = None,
        priority: TaskPriority = TaskPriority.NORMAL,
        current_time: float = 0.0
    ) -> Optional[PilotTask]:
        """
        Give an order to an NPC pilot.

        Args:
            owner_id: Player giving the order
            pilot_id: NPC pilot receiving the order
            task_type: Type of task to execute
            parameters: Task-specific parameters
            priority: Priority level
            current_time: Current game timestamp

        Returns:
            PilotTask if order accepted, None otherwise
        """
        pilot = self.pilots.get(pilot_id)
        if not pilot or pilot.owner_id != owner_id:
            return None

        # Must have a ship assigned for most tasks
        if not pilot.assigned_ship_id and task_type != TaskType.RETURN_TO_BASE:
            return None

        # Validate task is appropriate for specialization
        if not self._can_execute_task(pilot, task_type):
            return None

        self._task_counter += 1
        task = PilotTask(
            task_id=f"task_{self._task_counter}",
            task_type=task_type,
            priority=priority,
            parameters=parameters or {},
            status="queued",
            started_time=current_time,
        )

        # Emergency priority replaces current task
        if priority == TaskPriority.EMERGENCY:
            pilot.current_task = task
            task.status = "running"
            pilot.status = self._task_to_status(task_type)
        elif pilot.current_task is None:
            pilot.current_task = task
            task.status = "running"
            pilot.status = self._task_to_status(task_type)
        else:
            pilot.task_queue.append(task)

        return task

    def order_all(
        self,
        owner_id: str,
        task_type: TaskType,
        parameters: Optional[Dict] = None,
        current_time: float = 0.0
    ) -> List[PilotTask]:
        """Give the same order to all NPC pilots in the wing."""
        tasks = []
        pilot_ids = self.player_wings.get(owner_id, [])
        for pid in pilot_ids:
            task = self.give_order(owner_id, pid, task_type, parameters,
                                  current_time=current_time)
            if task:
                tasks.append(task)
        return tasks

    def create_trade_route(
        self,
        owner_id: str,
        name: str,
        stops: List[Dict],
        loop: bool = True,
        current_time: float = 0.0
    ) -> Optional[TradeRoute]:
        """
        Create a trade route that NPC haulers/traders can run.

        Args:
            owner_id: Player creating the route
            name: Name of the route
            stops: List of stop dicts with keys: station_id, action, item_id, quantity
            loop: Whether to repeat the route
            current_time: Current timestamp

        Returns:
            TradeRoute if created
        """
        if not stops or len(stops) < 2:
            return None

        self._route_counter += 1
        route = TradeRoute(
            route_id=f"route_{self._route_counter}",
            owner_id=owner_id,
            name=name,
            stops=stops,
            loop=loop,
            created_time=current_time,
        )

        self.trade_routes[route.route_id] = route
        return route

    def assign_trade_route(
        self,
        owner_id: str,
        pilot_id: str,
        route_id: str,
        current_time: float = 0.0
    ) -> Optional[PilotTask]:
        """Assign a trade route to a hauler/trader NPC pilot."""
        route = self.trade_routes.get(route_id)
        if not route or route.owner_id != owner_id:
            return None

        return self.give_order(
            owner_id, pilot_id,
            TaskType.TRADE_ROUTE,
            parameters={"route_id": route_id},
            current_time=current_time,
        )

    def process_salaries(
        self,
        owner_id: str,
        current_time: float,
        wallet_isk: float
    ) -> Dict:
        """
        Process hourly salary payments for NPC pilots.

        Returns:
            Dict with 'total_cost', 'paid', 'unpaid_pilots'
        """
        pilot_ids = self.player_wings.get(owner_id, [])
        total_cost = 0.0
        unpaid = []

        for pid in pilot_ids:
            pilot = self.pilots.get(pid)
            if not pilot:
                continue

            # Calculate hours since last payment
            hours_elapsed = (current_time - pilot.last_paid_time) / 3600.0
            if hours_elapsed < 1.0:
                continue  # Not yet due

            cost = pilot.salary_per_hour * int(hours_elapsed)
            total_cost += cost

        if wallet_isk >= total_cost:
            # Pay all pilots
            for pid in pilot_ids:
                pilot = self.pilots.get(pid)
                if not pilot:
                    continue
                hours = (current_time - pilot.last_paid_time) / 3600.0
                if hours >= 1.0:
                    pilot.last_paid_time = current_time
                    # Loyalty boost for being paid on time
                    pilot.loyalty = min(100, pilot.loyalty + 1)

            return {"total_cost": total_cost, "paid": True, "unpaid_pilots": []}
        else:
            # Can't afford - loyalty decreases
            for pid in pilot_ids:
                pilot = self.pilots.get(pid)
                if not pilot:
                    continue
                pilot.loyalty = max(0, pilot.loyalty - 5)
                if pilot.loyalty <= 0:
                    unpaid.append(pid)

            return {"total_cost": total_cost, "paid": False, "unpaid_pilots": unpaid}

    def gain_experience(
        self,
        pilot_id: str,
        xp_amount: int,
        task_success: bool = True
    ) -> Dict:
        """
        Award experience to an NPC pilot.

        Returns:
            Dict with 'leveled_up', 'new_level', 'new_abilities'
        """
        pilot = self.pilots.get(pilot_id)
        if not pilot:
            return {"leveled_up": False}

        if not task_success:
            xp_amount = xp_amount // 2  # Half XP on failure

        pilot.experience += xp_amount

        # Check level up
        xp_needed = LEVEL_UP_XP.get(pilot.skill_level)
        if xp_needed and pilot.experience >= xp_needed:
            pilot.skill_level += 1
            pilot.experience = 0
            # Update salary
            costs = PILOT_COSTS.get(pilot.skill_level, PILOT_COSTS[5])
            pilot.salary_per_hour = costs["salary"]
            # Unlock new abilities
            new_abilities = LEVEL_ABILITIES.get(pilot.skill_level, [])
            for ability in new_abilities:
                if ability not in pilot.abilities:
                    pilot.abilities.append(ability)

            return {
                "leveled_up": True,
                "new_level": pilot.skill_level,
                "new_abilities": new_abilities,
            }

        return {"leveled_up": False}

    def complete_task(self, pilot_id: str, success: bool = True,
                      result: Optional[Dict] = None,
                      current_time: float = 0.0):
        """Mark current task as completed and advance to next in queue."""
        pilot = self.pilots.get(pilot_id)
        if not pilot or not pilot.current_task:
            return

        pilot.current_task.status = "completed" if success else "failed"
        pilot.current_task.completed_time = current_time
        if result:
            pilot.current_task.result = result

        # Update stats
        if success:
            pilot.stats.tasks_completed += 1
        else:
            pilot.stats.tasks_failed += 1

        # Award XP
        self.gain_experience(pilot_id, 100, success)

        # Advance to next task
        if pilot.task_queue:
            pilot.current_task = pilot.task_queue.pop(0)
            pilot.current_task.status = "running"
            pilot.current_task.started_time = current_time
            pilot.status = self._task_to_status(pilot.current_task.task_type)
        else:
            pilot.current_task = None
            pilot.status = PilotStatus.ASSIGNED if pilot.assigned_ship_id else PilotStatus.AVAILABLE

    def get_wing_roster(self, owner_id: str) -> List[AIPilot]:
        """Get all NPC pilots in a player's wing."""
        pilot_ids = self.player_wings.get(owner_id, [])
        return [self.pilots[pid] for pid in pilot_ids if pid in self.pilots]

    def get_wing_status(self, owner_id: str) -> Dict:
        """Get summary status of the wing."""
        pilots = self.get_wing_roster(owner_id)
        total_salary = sum(p.salary_per_hour for p in pilots)

        status_counts = {}
        for p in pilots:
            s = p.status.value
            status_counts[s] = status_counts.get(s, 0) + 1

        spec_counts = {}
        for p in pilots:
            s = p.specialization.value
            spec_counts[s] = spec_counts.get(s, 0) + 1

        return {
            "total_pilots": len(pilots),
            "max_wing_size": MAX_WING_SIZE,
            "slots_available": MAX_WING_SIZE - len(pilots),
            "total_salary_per_hour": total_salary,
            "status_breakdown": status_counts,
            "specialization_breakdown": spec_counts,
        }

    def get_pilot(self, pilot_id: str) -> Optional[AIPilot]:
        """Get a specific NPC pilot."""
        return self.pilots.get(pilot_id)

    def _can_execute_task(self, pilot: AIPilot, task_type: TaskType) -> bool:
        """Check if a pilot's specialization allows this task."""
        # Universal tasks any pilot can do
        universal = {TaskType.FOLLOW, TaskType.RETURN_TO_BASE,
                     TaskType.HOLD_FIRE, TaskType.ORBIT}
        if task_type in universal:
            return True

        task_map = {
            PilotSpecialization.COMBAT: {
                TaskType.GUARD, TaskType.ENGAGE_TARGETS,
            },
            PilotSpecialization.MINER: {
                TaskType.MINE_HERE, TaskType.MINE_AND_HAUL,
            },
            PilotSpecialization.HAULER: {
                TaskType.COLLECT_ORE, TaskType.HAUL_TO_STATION,
                TaskType.TRADE_ROUTE, TaskType.SELL_CARGO,
            },
            PilotSpecialization.SCOUT: {
                TaskType.SCOUT_AHEAD, TaskType.GUARD,
            },
            PilotSpecialization.TRADER: {
                TaskType.TRADE_ROUTE, TaskType.SELL_CARGO,
                TaskType.HAUL_TO_STATION,
            },
            PilotSpecialization.INDUSTRIALIST: {
                TaskType.PI_MANAGE, TaskType.HAUL_TO_STATION,
            },
        }

        allowed = task_map.get(pilot.specialization, set())
        return task_type in allowed

    def _task_to_status(self, task_type: TaskType) -> PilotStatus:
        """Map a task type to the appropriate pilot status."""
        mapping = {
            TaskType.FOLLOW: PilotStatus.EXECUTING,
            TaskType.GUARD: PilotStatus.EXECUTING,
            TaskType.MINE_HERE: PilotStatus.MINING,
            TaskType.MINE_AND_HAUL: PilotStatus.MINING,
            TaskType.COLLECT_ORE: PilotStatus.HAULING,
            TaskType.HAUL_TO_STATION: PilotStatus.HAULING,
            TaskType.TRADE_ROUTE: PilotStatus.TRADING,
            TaskType.ENGAGE_TARGETS: PilotStatus.COMBAT,
            TaskType.HOLD_FIRE: PilotStatus.EXECUTING,
            TaskType.RETURN_TO_BASE: PilotStatus.TRAVELING,
            TaskType.SCOUT_AHEAD: PilotStatus.EXECUTING,
            TaskType.ORBIT: PilotStatus.EXECUTING,
            TaskType.SELL_CARGO: PilotStatus.TRADING,
            TaskType.PI_MANAGE: PilotStatus.EXECUTING,
        }
        return mapping.get(task_type, PilotStatus.EXECUTING)

    def update(self, delta_time: float, current_time: float):
        """
        Update all NPC pilots - process AI decisions, task progress, etc.
        Called each game tick by the server.
        """
        for pilot in self.pilots.values():
            if pilot.status == PilotStatus.OFFLINE:
                continue

            # Update active time
            pilot.stats.time_active += delta_time

            # Emergency behavior - retreat if health is very low
            # (Actual health check would integrate with Health component)

            # Process current task progress
            if pilot.current_task and pilot.current_task.status == "running":
                self._update_task_progress(pilot, delta_time, current_time)

    def _update_task_progress(self, pilot: AIPilot, delta_time: float,
                              current_time: float):
        """Update progress on a pilot's current task."""
        task = pilot.current_task
        if not task:
            return

        # Simulate task progress based on skill level
        # Higher skill = faster completion
        progress_rate = 0.01 * pilot.skill_level  # Base rate per second
        task.progress = min(1.0, task.progress + progress_rate * delta_time)

        # Task-specific progress tracking
        if task.task_type == TaskType.MINE_HERE:
            # Mining produces ore over time
            ore_per_second = 5.0 * pilot.skill_level  # m3/s based on skill
            pilot.stats.ore_mined += ore_per_second * delta_time

        elif task.task_type == TaskType.HAUL_TO_STATION:
            # Hauling covers distance
            speed = 500.0 * pilot.skill_level  # m/s
            pilot.stats.distance_traveled += speed * delta_time
