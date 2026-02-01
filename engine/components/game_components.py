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
