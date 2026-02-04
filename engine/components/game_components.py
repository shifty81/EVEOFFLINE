"""
Game Components
Components represent data for entities (ships, NPCs, projectiles, etc.)
"""

from dataclasses import dataclass, field
from typing import List, Dict, Optional, Tuple
from engine.core.ecs import Component


@dataclass
class Position(Component):
    """Position and orientation in 3D space"""
    x: float = 0.0
    y: float = 0.0
    z: float = 0.0
    rotation: float = 0.0  # radians
    

@dataclass
class Velocity(Component):
    """Velocity and movement"""
    vx: float = 0.0
    vy: float = 0.0
    vz: float = 0.0
    angular_velocity: float = 0.0
    max_speed: float = 100.0
    

@dataclass  
class Health(Component):
    """Health pools similar to EVE ONLINE"""
    hull_hp: float = 100.0
    hull_max: float = 100.0
    armor_hp: float = 100.0
    armor_max: float = 100.0
    shield_hp: float = 100.0
    shield_max: float = 100.0
    shield_recharge_rate: float = 1.0  # HP per second
    
    # Resistance profiles (0.0 = no resist, 0.5 = 50% resist, etc.)
    hull_em_resist: float = 0.0
    hull_thermal_resist: float = 0.0
    hull_kinetic_resist: float = 0.0
    hull_explosive_resist: float = 0.0
    
    armor_em_resist: float = 0.0
    armor_thermal_resist: float = 0.0
    armor_kinetic_resist: float = 0.0
    armor_explosive_resist: float = 0.0
    
    shield_em_resist: float = 0.0
    shield_thermal_resist: float = 0.0
    shield_kinetic_resist: float = 0.0
    shield_explosive_resist: float = 0.0
    
    def is_alive(self) -> bool:
        """Check if entity is still alive"""
        return self.hull_hp > 0.0
    

@dataclass
class Capacitor(Component):
    """Energy capacitor like EVE ONLINE"""
    capacitor: float = 100.0
    capacitor_max: float = 100.0
    recharge_rate: float = 2.0  # GJ per second
    

@dataclass
class Ship(Component):
    """Ship-specific data"""
    ship_type: str = "Frigate"
    ship_class: str = "Frigate"  # Frigate, Destroyer, Cruiser, etc.
    ship_name: str = "Rifter"
    race: str = "Minmatar"  # Caldari, Gallente, Amarr, Minmatar
    
    # Fitting resources
    cpu: float = 0.0
    cpu_max: float = 100.0
    powergrid: float = 0.0
    powergrid_max: float = 50.0
    
    # Signature and targeting
    signature_radius: float = 35.0  # meters
    scan_resolution: float = 400.0  # mm
    max_locked_targets: int = 3
    max_targeting_range: float = 20000.0  # meters
    

@dataclass
class Fitting(Component):
    """Fitted modules on a ship"""
    high_slots: List[Optional[str]] = field(default_factory=lambda: [None, None, None])
    mid_slots: List[Optional[str]] = field(default_factory=lambda: [None, None, None])
    low_slots: List[Optional[str]] = field(default_factory=lambda: [None, None, None])
    rig_slots: List[Optional[str]] = field(default_factory=lambda: [None, None, None])
    cargo: Dict[str, int] = field(default_factory=dict)  # item_id: quantity
    cargo_capacity: float = 100.0  # m3
    

@dataclass
class Skills(Component):
    """Character skills"""
    skills: Dict[str, int] = field(default_factory=dict)  # skill_name: level (0-5)
    skill_queue: List[Tuple[str, int]] = field(default_factory=list)  # [(skill, target_level), ...]
    skill_points: Dict[str, int] = field(default_factory=dict)  # skill_name: SP
    

@dataclass
class Target(Component):
    """Targeting information"""
    locked_targets: List[str] = field(default_factory=list)  # entity IDs
    locking_targets: Dict[str, float] = field(default_factory=dict)  # entity_id: progress (0-1)
    

@dataclass
class Weapon(Component):
    """Weapon system"""
    weapon_type: str = "Projectile"  # Projectile, Energy, Missile, Hybrid
    damage_type: str = "kinetic"  # em, thermal, kinetic, explosive
    damage: float = 10.0
    optimal_range: float = 5000.0  # meters
    falloff_range: float = 2500.0  # meters
    tracking_speed: float = 0.5  # radians per second
    rate_of_fire: float = 3.0  # seconds between shots
    cooldown: float = 0.0  # current cooldown timer
    ammo_type: str = "EMP"
    ammo_count: int = 100
    

@dataclass
class AI(Component):
    """AI behavior for NPCs"""
    behavior: str = "aggressive"  # aggressive, defensive, passive, flee
    state: str = "idle"  # idle, approaching, orbiting, fleeing, attacking
    target_entity_id: Optional[str] = None
    orbit_distance: float = 1000.0  # preferred orbit distance
    awareness_range: float = 50000.0  # meters
    

@dataclass
class Player(Component):
    """Player-controlled entity"""
    player_id: str = ""
    character_name: str = "Pilot"
    isk: float = 1000000.0  # Starting ISK
    corporation: str = "NPC Corp"
    

@dataclass
class NPC(Component):
    """NPC-specific data"""
    npc_type: str = "Serpentis Scout"
    faction: str = "Serpentis"
    bounty: float = 10000.0  # ISK reward for killing
    loot_table: List[str] = field(default_factory=list)  # possible loot items


@dataclass
class Module(Component):
    """Active module component for fitted modules"""
    module_id: str = ""
    module_type: str = "weapon"  # weapon, shield_booster, armor_repairer, etc.
    slot_type: str = "high"  # high, mid, low, rig
    active: bool = False
    cycle_time: float = 5.0  # seconds
    current_cycle: float = 0.0
    cpu_usage: float = 0.0
    powergrid_usage: float = 0.0
    capacitor_usage: float = 0.0  # cap per cycle
    
    # Effect parameters (type-specific)
    # Examples: {'damage_multiplier': 1.2, 'range_bonus': 0.15, 'repair_amount': 100}
    # For shield boosters: {'shield_boost_amount': 200}
    # For damage mods: {'damage_multiplier': 1.1}
    # For prop mods: {'velocity_bonus': 500}
    effect_params: Dict[str, float] = field(default_factory=dict)


@dataclass  
class Drone(Component):
    """Drone component"""
    drone_type: str = "Light Scout Drone"
    size: str = "light"  # light, medium, heavy, sentry
    bandwidth: float = 5.0  # Mbit/s
    damage: float = 5.0
    damage_type: str = "thermal"
    optimal_range: float = 10000.0
    tracking: float = 1.0
    speed: float = 3000.0  # m/s
    hp: float = 50.0
    hp_max: float = 50.0
    owner_entity_id: Optional[str] = None
    target_entity_id: Optional[str] = None
    state: str = "in_bay"  # in_bay, launching, active, returning


@dataclass
class DroneBay(Component):
    """Drone bay for ships"""
    drones: List[str] = field(default_factory=list)  # entity IDs of drones
    drone_bay_capacity: float = 50.0  # m3
    drone_bandwidth: float = 25.0  # Mbit/s
    active_drones: List[str] = field(default_factory=list)  # currently deployed
    bandwidth_used: float = 0.0


@dataclass
class SkillTraining(Component):
    """Skill training queue and progress"""
    current_skill: Optional[str] = None
    current_target_level: int = 0
    current_sp: int = 0  # SP in current skill
    training_queue: List[Tuple[str, int]] = field(default_factory=list)  # (skill_name, level)
    sp_per_minute: float = 30.0  # Based on attributes


@dataclass
class EWAREffect(Component):
    """Electronic warfare effects on a ship"""
    webbed_velocity_reduction: float = 0.0  # 0.0 to 1.0
    scrambled: bool = False
    disrupted: bool = False
    signature_multiplier: float = 1.0  # From target painters
    tracking_reduction: float = 0.0  # From tracking disruptors


@dataclass
class Mission(Component):
    """Active mission tracking"""
    mission_id: str = ""
    mission_type: str = "combat"  # combat, courier, mining, exploration
    objectives: List[Dict[str, any]] = field(default_factory=list)
    completed_objectives: List[int] = field(default_factory=list)
    rewards: Dict[str, float] = field(default_factory=dict)  # isk, lp, items
    status: str = "active"  # active, completed, failed


@dataclass
class Inventory(Component):
    """Ship/station inventory"""
    items: Dict[str, int] = field(default_factory=dict)  # item_id: quantity
    cargo_capacity: float = 100.0  # m3
    cargo_used: float = 0.0  # m3


@dataclass
class WarpDrive(Component):
    """Warp drive component"""
    warp_speed: float = 3.0  # AU per second
    max_warp_distance: float = 100.0  # AU
    align_time: float = 5.0  # seconds to align
    is_warping: bool = False
    warp_destination: Optional[Tuple[float, float, float]] = None  # (x, y, z)
    warp_progress: float = 0.0
    is_aligning: bool = False
    align_progress: float = 0.0


@dataclass
class Celestial(Component):
    """Celestial object (station, gate, asteroid belt, etc.)"""
    celestial_type: str = "station"  # station, stargate, asteroid_belt, planet, moon
    celestial_name: str = "Jita IV - Moon 4"
    system_id: str = "jita"
    
    # For stargates
    destination_system: Optional[str] = None
    destination_gate: Optional[str] = None
    
    # For stations
    dockable: bool = False
    services: List[str] = field(default_factory=list)  # repair, market, fitting, etc.


@dataclass
class Docking(Component):
    """Docking status"""
    is_docked: bool = False
    docked_at: Optional[str] = None  # celestial entity ID
    can_undock: bool = True


@dataclass
class CorporationMembership(Component):
    """Player's corporation membership"""
    corporation_id: str = "NPC_Corp"
    corporation_name: str = "NPC Corporation"
    role: str = "member"  # member, director, ceo
    joined_date: float = 0.0  # timestamp
    title: str = ""


@dataclass
class Contacts(Component):
    """Player contacts and standings"""
    contacts: Dict[str, float] = field(default_factory=dict)  # {character_id: standing (-10 to +10)}
    blocked: List[str] = field(default_factory=list)  # blocked character IDs
    
    
@dataclass
class Mail(Component):
    """In-game mail system"""
    inbox: List[Dict[str, any]] = field(default_factory=list)  # List of mail messages
    sent: List[Dict[str, any]] = field(default_factory=list)  # Sent mail messages
    unread_count: int = 0
    

@dataclass
class Chat(Component):
    """Chat channels and messages"""
    channels: Dict[str, List[Dict[str, any]]] = field(default_factory=dict)  # {channel_id: [messages]}
    active_channels: List[str] = field(default_factory=list)  # Currently subscribed channels
    muted_characters: List[str] = field(default_factory=list)  # Muted character IDs


@dataclass
class Contract(Component):
    """Player contracts"""
    active_contracts: List[str] = field(default_factory=list)  # Contract IDs
    completed_contracts: List[str] = field(default_factory=list)
    failed_contracts: List[str] = field(default_factory=list)


@dataclass
class MiningLaser(Component):
    """Mining laser component for mining operations"""
    laser_type: str = "Miner I"  # Miner I, Miner II, Strip Miner I, etc.
    cycle_time: float = 60.0  # seconds per cycle
    yield_amount: float = 40.0  # m3 per cycle
    optimal_range: float = 10000.0  # meters
    capacitor_usage: float = 60.0  # GJ per cycle
    is_active: bool = False
    current_cycle: float = 0.0  # current cycle progress
    target_asteroid_id: Optional[str] = None


@dataclass
class MiningYield(Component):
    """Tracks active mining operations and yields"""
    total_ore_mined: Dict[str, float] = field(default_factory=dict)  # {ore_type: m3}
    active_mining: bool = False
    target_asteroid_id: Optional[str] = None
    mining_start_time: float = 0.0
    yield_multiplier: float = 1.0  # From skills and modules


@dataclass
class OreHold(Component):
    """Specialized cargo hold for ore (mining barges/exhumers)"""
    ore: Dict[str, float] = field(default_factory=dict)  # {ore_type: m3}
    ore_hold_capacity: float = 5000.0  # m3
    ore_hold_used: float = 0.0  # m3
