"""
Mining System
Handles mining laser operations, ore extraction, and yield calculations
Based on EVE Online mining mechanics
"""

import math
from typing import Optional, Dict, List, Tuple
from engine.core.ecs import System
from engine.components.game_components import (
    Position, MiningLaser, MiningYield, OreHold, Inventory,
    Capacitor, Skills, Player, Fitting
)
from engine.systems.asteroid_system import AsteroidFieldManager, Asteroid


class MiningSystem(System):
    """
    Manages mining operations including:
    - Mining laser cycles
    - Ore extraction from asteroids
    - Yield calculations with skill/module bonuses
    - Ore storage in cargo/ore holds
    """
    
    def __init__(self, world, asteroid_manager: AsteroidFieldManager):
        super().__init__(world)
        self.asteroid_manager = asteroid_manager
        
    def update(self, delta_time: float):
        """Update mining cycles for all active miners"""
        entities = self.get_entities(MiningLaser, Position, Capacitor)
        
        for entity in entities:
            mining_laser = entity.get_component(MiningLaser)
            
            if not mining_laser.is_active:
                continue
                
            # Update mining cycle
            mining_laser.current_cycle += delta_time
            
            # Check if cycle complete
            if mining_laser.current_cycle >= mining_laser.cycle_time:
                self._complete_mining_cycle(entity)
                mining_laser.current_cycle = 0.0
    
    def start_mining(
        self,
        entity,
        target_asteroid_id: str,
        belt_id: Optional[str] = None
    ) -> bool:
        """
        Start mining an asteroid
        
        Args:
            entity: The mining entity
            target_asteroid_id: ID of the asteroid to mine
            belt_id: Optional belt ID (will search all belts if not provided)
            
        Returns:
            True if mining started successfully
        """
        mining_laser = entity.get_component(MiningLaser)
        position = entity.get_component(Position)
        capacitor = entity.get_component(Capacitor)
        
        if not all([mining_laser, position, capacitor]):
            return False
        
        # Find the asteroid
        asteroid = self._find_asteroid(target_asteroid_id, belt_id)
        if not asteroid:
            return False
        
        # Check if asteroid is depleted
        if asteroid.is_depleted():
            return False
        
        # Check range
        distance = self._calculate_distance(
            (position.x, position.y, position.z),
            asteroid.position
        )
        
        if distance > mining_laser.optimal_range:
            return False
        
        # Check capacitor
        if capacitor.capacitor < mining_laser.capacitor_usage:
            return False
        
        # Start mining
        mining_laser.is_active = True
        mining_laser.target_asteroid_id = target_asteroid_id
        mining_laser.current_cycle = 0.0
        
        # Update yield tracking
        mining_yield = entity.get_component(MiningYield)
        if mining_yield:
            mining_yield.active_mining = True
            mining_yield.target_asteroid_id = target_asteroid_id
        
        return True
    
    def stop_mining(self, entity) -> bool:
        """Stop mining operations"""
        mining_laser = entity.get_component(MiningLaser)
        if not mining_laser:
            return False
        
        mining_laser.is_active = False
        mining_laser.target_asteroid_id = None
        mining_laser.current_cycle = 0.0
        
        # Update yield tracking
        mining_yield = entity.get_component(MiningYield)
        if mining_yield:
            mining_yield.active_mining = False
            mining_yield.target_asteroid_id = None
        
        return True
    
    def _complete_mining_cycle(self, entity):
        """Complete a mining cycle and extract ore"""
        mining_laser = entity.get_component(MiningLaser)
        capacitor = entity.get_component(Capacitor)
        
        if not mining_laser.target_asteroid_id:
            mining_laser.is_active = False
            return
        
        # Consume capacitor
        capacitor.capacitor -= mining_laser.capacitor_usage
        if capacitor.capacitor < 0:
            capacitor.capacitor = 0
            mining_laser.is_active = False
            return
        
        # Find asteroid
        asteroid = self._find_asteroid(mining_laser.target_asteroid_id)
        if not asteroid or asteroid.is_depleted():
            mining_laser.is_active = False
            return
        
        # Calculate yield with bonuses
        base_yield = mining_laser.yield_amount
        yield_multiplier = self._calculate_yield_multiplier(entity)
        final_yield = base_yield * yield_multiplier
        
        # Extract ore from asteroid
        ore_mined = asteroid.mine(int(final_yield))
        
        if ore_mined <= 0:
            mining_laser.is_active = False
            return
        
        # Store ore
        self._store_ore(entity, asteroid.ore_type, float(ore_mined))
        
        # Update statistics
        mining_yield = entity.get_component(MiningYield)
        if mining_yield:
            if asteroid.ore_type not in mining_yield.total_ore_mined:
                mining_yield.total_ore_mined[asteroid.ore_type] = 0.0
            mining_yield.total_ore_mined[asteroid.ore_type] += float(ore_mined)
    
    def _calculate_yield_multiplier(self, entity) -> float:
        """Calculate total yield multiplier from skills and modules"""
        multiplier = 1.0
        
        # Skill bonuses
        skills = entity.get_component(Skills)
        if skills:
            # Mining skill: 5% per level
            mining_level = skills.skills.get("mining", 0)
            multiplier *= (1.0 + mining_level * 0.05)
            
            # Astrogeology: 5% per level
            astro_level = skills.skills.get("astrogeology", 0)
            multiplier *= (1.0 + astro_level * 0.05)
            
            # Exhumers skill: 3% per level
            exhumer_level = skills.skills.get("exhumers", 0)
            multiplier *= (1.0 + exhumer_level * 0.03)
        
        # Module bonuses (Mining Laser Upgrades)
        fitting = entity.get_component(Fitting)
        if fitting:
            # Count mining upgrades in low slots
            upgrade_count = 0
            for module_id in fitting.low_slots:
                if module_id and "mining_laser_upgrade" in module_id:
                    upgrade_count += 1
            
            # Apply stacking penalties (simplified)
            for i in range(upgrade_count):
                penalty = math.pow(0.5, i * i)
                bonus = 0.05  # Base 5% bonus
                if "upgrade_ii" in str(fitting.low_slots):
                    bonus = 0.09  # Tech II is 9%
                multiplier *= (1.0 + bonus * penalty)
        
        # Mining yield component multiplier
        mining_yield = entity.get_component(MiningYield)
        if mining_yield:
            multiplier *= mining_yield.yield_multiplier
        
        return multiplier
    
    def _store_ore(self, entity, ore_type: str, amount: float):
        """Store mined ore in cargo or ore hold"""
        # Try ore hold first (mining barges/exhumers)
        ore_hold = entity.get_component(OreHold)
        if ore_hold:
            available_space = ore_hold.ore_hold_capacity - ore_hold.ore_hold_used
            if available_space >= amount:
                if ore_type not in ore_hold.ore:
                    ore_hold.ore[ore_type] = 0.0
                ore_hold.ore[ore_type] += amount
                ore_hold.ore_hold_used += amount
                return
        
        # Fall back to regular cargo
        inventory = entity.get_component(Inventory)
        if inventory:
            available_space = inventory.cargo_capacity - inventory.cargo_used
            if available_space >= amount:
                if ore_type not in inventory.items:
                    inventory.items[ore_type] = 0
                inventory.items[ore_type] += int(amount)
                inventory.cargo_used += amount
    
    def _find_asteroid(
        self,
        asteroid_id: str,
        belt_id: Optional[str] = None
    ) -> Optional[Asteroid]:
        """Find an asteroid by ID"""
        if belt_id:
            belt = self.asteroid_manager.get_belt(belt_id)
            if belt:
                return belt.asteroids.get(asteroid_id)
        
        # Search all belts
        for belt in self.asteroid_manager.belts.values():
            if asteroid_id in belt.asteroids:
                return belt.asteroids[asteroid_id]
        
        return None
    
    def _calculate_distance(
        self,
        pos1: Tuple[float, float, float],
        pos2: Tuple[float, float, float]
    ) -> float:
        """Calculate 3D distance between two positions"""
        dx = pos2[0] - pos1[0]
        dy = pos2[1] - pos1[1]
        dz = pos2[2] - pos1[2]
        return math.sqrt(dx*dx + dy*dy + dz*dz)
    
    def get_nearby_asteroids(
        self,
        entity,
        max_distance: Optional[float] = None
    ) -> List[Tuple[str, Asteroid, float]]:
        """
        Get all asteroids near an entity
        
        Returns:
            List of (belt_id, asteroid, distance) tuples
        """
        position = entity.get_component(Position)
        if not position:
            return []
        
        mining_laser = entity.get_component(MiningLaser)
        if not max_distance and mining_laser:
            max_distance = mining_laser.optimal_range
        elif not max_distance:
            max_distance = 15000.0  # Default range
        
        nearby = []
        entity_pos = (position.x, position.y, position.z)
        
        for belt in self.asteroid_manager.belts.values():
            for asteroid in belt.asteroids.values():
                if asteroid.is_depleted():
                    continue
                
                distance = self._calculate_distance(entity_pos, asteroid.position)
                if distance <= max_distance:
                    nearby.append((belt.id, asteroid, distance))
        
        # Sort by distance
        nearby.sort(key=lambda x: x[2])
        return nearby
    
    def get_mining_info(self, entity) -> Dict:
        """Get current mining status and information"""
        mining_laser = entity.get_component(MiningLaser)
        mining_yield = entity.get_component(MiningYield)
        
        if not mining_laser:
            return {"error": "No mining laser equipped"}
        
        info = {
            "is_active": mining_laser.is_active,
            "cycle_time": mining_laser.cycle_time,
            "current_cycle": mining_laser.current_cycle,
            "cycle_progress": (mining_laser.current_cycle / mining_laser.cycle_time * 100) if mining_laser.is_active else 0,
            "base_yield": mining_laser.yield_amount,
            "yield_multiplier": self._calculate_yield_multiplier(entity),
            "final_yield": mining_laser.yield_amount * self._calculate_yield_multiplier(entity),
            "optimal_range": mining_laser.optimal_range,
            "capacitor_usage": mining_laser.capacitor_usage,
            "target_asteroid_id": mining_laser.target_asteroid_id
        }
        
        if mining_yield:
            info["total_ore_mined"] = mining_yield.total_ore_mined
            info["active_mining"] = mining_yield.active_mining
        
        return info
