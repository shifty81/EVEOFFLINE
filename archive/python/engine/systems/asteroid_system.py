"""
Asteroid Field System
Manages asteroid belts, mining mechanics, and ore distribution
Based on EVE Online asteroid field mechanics
"""

import random
import math
import json
from typing import Dict, List, Tuple, Optional
from dataclasses import dataclass
from enum import Enum


class AsteroidSize(Enum):
    """Asteroid size categories"""
    SMALL = "small"
    MEDIUM = "medium"
    LARGE = "large"
    HUGE = "huge"


class BeltLayout(Enum):
    """Asteroid belt layout types"""
    SEMICIRCLE = "semicircle"
    SPHERICAL = "spherical"


@dataclass
class OreType:
    """Ore type definition"""
    name: str
    tier: str
    security_min: float
    security_max: float
    base_value: int
    minerals: Dict[str, int]


@dataclass
class Asteroid:
    """Individual asteroid in a belt"""
    id: str
    ore_type: str
    variant: str
    position: Tuple[float, float, float]
    size: AsteroidSize
    ore_remaining: int
    ore_total: int
    
    def is_depleted(self) -> bool:
        """Check if asteroid is fully mined"""
        return self.ore_remaining <= 0
    
    def mine(self, amount: int) -> int:
        """
        Mine ore from asteroid
        Returns actual amount mined
        """
        actual = min(amount, self.ore_remaining)
        self.ore_remaining -= actual
        return actual


class AsteroidBelt:
    """
    Asteroid belt with layout and mining mechanics
    """
    
    def __init__(
        self,
        belt_id: str,
        name: str,
        system: str,
        security: float,
        layout: BeltLayout = BeltLayout.SEMICIRCLE,
        radius_km: float = 50.0
    ):
        self.id = belt_id
        self.name = name
        self.system = system
        self.security = security
        self.layout = layout
        self.radius_km = radius_km
        self.warp_point = (0.0, 0.0, 0.0)
        self.asteroids: Dict[str, Asteroid] = {}
        self.npc_spawns = True
        self.last_respawn = None
        
    def generate_asteroids(
        self,
        ore_composition: Dict[str, float],
        asteroid_counts: Dict[str, int]
    ):
        """
        Generate asteroids in the belt based on composition
        
        Args:
            ore_composition: Dict of ore_type -> spawn_weight
            asteroid_counts: Dict of size -> count
        """
        asteroid_id = 0
        
        for size_str, count in asteroid_counts.items():
            size = AsteroidSize(size_str)
            
            for _ in range(count):
                # Choose ore type based on composition weights
                ore_type = random.choices(
                    list(ore_composition.keys()),
                    weights=list(ore_composition.values())
                )[0]
                
                # Generate position based on layout
                position = self._generate_position()
                
                # Determine ore amount based on size
                ore_amount = self._get_ore_amount(size)
                
                # Create asteroid
                asteroid = Asteroid(
                    id=f"{self.id}_ast_{asteroid_id}",
                    ore_type=ore_type,
                    variant="standard",
                    position=position,
                    size=size,
                    ore_remaining=ore_amount,
                    ore_total=ore_amount
                )
                
                self.asteroids[asteroid.id] = asteroid
                asteroid_id += 1
                
        print(f"[AsteroidBelt] Generated {len(self.asteroids)} asteroids in {self.name}")
    
    def _generate_position(self) -> Tuple[float, float, float]:
        """Generate position based on belt layout"""
        radius_m = self.radius_km * 1000  # Convert to meters
        
        if self.layout == BeltLayout.SEMICIRCLE:
            # Semicircle layout (arc)
            angle = random.uniform(0, math.pi)  # 0 to 180 degrees
            distance = random.uniform(radius_m * 0.5, radius_m)
            
            x = distance * math.cos(angle)
            y = distance * math.sin(angle)
            z = random.uniform(-radius_m * 0.2, radius_m * 0.2)
            
        else:  # SPHERICAL
            # Spherical layout (distributed in 3D sphere)
            theta = random.uniform(0, 2 * math.pi)
            phi = random.uniform(-math.pi/2, math.pi/2)
            distance = random.uniform(radius_m * 0.3, radius_m)
            
            x = distance * math.cos(phi) * math.cos(theta)
            y = distance * math.cos(phi) * math.sin(theta)
            z = distance * math.sin(phi)
        
        return (x, y, z)
    
    def _get_ore_amount(self, size: AsteroidSize) -> int:
        """Get ore amount based on asteroid size"""
        base_amounts = {
            AsteroidSize.SMALL: 7500,
            AsteroidSize.MEDIUM: 35000,
            AsteroidSize.LARGE: 150000,
            AsteroidSize.HUGE: 750000
        }
        
        base = base_amounts[size]
        # Add some randomness (±20%)
        return int(base * random.uniform(0.8, 1.2))
    
    def get_nearby_asteroids(
        self,
        position: Tuple[float, float, float],
        max_distance: float = 15000.0
    ) -> List[Asteroid]:
        """
        Get asteroids within range of position
        
        Args:
            position: (x, y, z) position
            max_distance: Maximum distance in meters
            
        Returns:
            List of asteroids within range
        """
        nearby = []
        
        for asteroid in self.asteroids.values():
            if asteroid.is_depleted():
                continue
            
            distance = self._distance(position, asteroid.position)
            if distance <= max_distance:
                nearby.append(asteroid)
        
        return nearby
    
    def _distance(
        self,
        pos1: Tuple[float, float, float],
        pos2: Tuple[float, float, float]
    ) -> float:
        """Calculate 3D distance between two positions"""
        dx = pos2[0] - pos1[0]
        dy = pos2[1] - pos1[1]
        dz = pos2[2] - pos1[2]
        return math.sqrt(dx*dx + dy*dy + dz*dz)
    
    def respawn_asteroids(self):
        """
        Respawn asteroids (called at daily downtime)
        Implements over-mining mechanics
        """
        # Calculate depletion percentage
        total_depleted = sum(
            1 for ast in self.asteroids.values() if ast.is_depleted()
        )
        depletion_ratio = total_depleted / len(self.asteroids) if self.asteroids else 0
        
        # If heavily mined (>70% depleted), respawn smaller
        if depletion_ratio > 0.7:
            # Reduce asteroid counts by 20%
            print(f"[AsteroidBelt] {self.name} over-mined, respawning smaller")
            # Would reduce counts here in full implementation
        
        # Reset all asteroids
        for asteroid in self.asteroids.values():
            asteroid.ore_remaining = asteroid.ore_total
        
        print(f"[AsteroidBelt] {self.name} respawned")


class AsteroidFieldManager:
    """Manages all asteroid fields in the game"""
    
    def __init__(self):
        self.belts: Dict[str, AsteroidBelt] = {}
        self.ore_types: Dict[str, OreType] = {}
        
    def load_ore_types(self, filepath: str):
        """Load ore type definitions from JSON"""
        with open(filepath, 'r') as f:
            data = json.load(f)
        
        for ore_id, ore_data in data['ore_types'].items():
            ore_type = OreType(
                name=ore_data['name'],
                tier=ore_data['tier'],
                security_min=ore_data.get('security_min', -1.0),
                security_max=ore_data.get('security_max', 1.0),
                base_value=ore_data['base_value'],
                minerals=ore_data['minerals']
            )
            self.ore_types[ore_id] = ore_type
        
        print(f"[AsteroidFieldManager] Loaded {len(self.ore_types)} ore types")
    
    def load_belts(self, filepath: str):
        """Load belt definitions from JSON"""
        with open(filepath, 'r') as f:
            data = json.load(f)
        
        # Load all belt categories
        for category, belts in data.items():
            for belt_data in belts:
                belt = AsteroidBelt(
                    belt_id=belt_data['id'],
                    name=belt_data['name'],
                    system=belt_data['system'],
                    security=belt_data['security'],
                    layout=BeltLayout(belt_data['layout']),
                    radius_km=belt_data['radius_km']
                )
                
                belt.warp_point = tuple(belt_data['warp_point'])
                belt.npc_spawns = belt_data.get('npc_spawns', True)
                
                # Generate asteroids
                belt.generate_asteroids(
                    ore_composition=belt_data['ore_composition'],
                    asteroid_counts=belt_data['asteroid_count']
                )
                
                self.belts[belt.id] = belt
        
        print(f"[AsteroidFieldManager] Loaded {len(self.belts)} asteroid belts")
    
    def get_belt(self, belt_id: str) -> Optional[AsteroidBelt]:
        """Get belt by ID"""
        return self.belts.get(belt_id)
    
    def get_belts_in_system(self, system: str) -> List[AsteroidBelt]:
        """Get all belts in a system"""
        return [belt for belt in self.belts.values() if belt.system == system]
    
    def daily_downtime(self):
        """Run daily downtime respawn for all belts"""
        print("[AsteroidFieldManager] Running daily downtime...")
        for belt in self.belts.values():
            belt.respawn_asteroids()
        print("[AsteroidFieldManager] Downtime complete")
