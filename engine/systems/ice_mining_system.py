"""
Ice Mining System
Handles ice harvester operations, ice extraction, and ice storage
Based on EVE Online ice mining mechanics
"""

import json
import math
import os
from typing import Optional, Dict, List, Tuple
from engine.core.ecs import System
from engine.components.game_components import (
    Position, IceHarvester, MiningYield, OreHold, Inventory,
    Capacitor, Skills, Player, Fitting
)


class IceField:
    """Represents a field of ice for harvesting"""
    
    def __init__(self, field_id: str, ice_type: str, system_id: str, position: Tuple[float, float, float]):
        self.id = field_id
        self.ice_type = ice_type
        self.system_id = system_id
        self.position = position
        self.remaining_units = 1000  # Ice fields have a large but finite number of units
        self.spawn_time = 0.0
        
    def is_depleted(self) -> bool:
        """Check if ice field is depleted"""
        return self.remaining_units <= 0
    
    def harvest(self, amount: int = 1) -> int:
        """
        Harvest ice from the field
        
        Args:
            amount: Number of units to harvest
            
        Returns:
            Actual units harvested
        """
        harvested = min(amount, self.remaining_units)
        self.remaining_units -= harvested
        return harvested


class IceFieldManager:
    """Manages ice fields in the game world"""
    
    def __init__(self):
        self.fields: Dict[str, IceField] = {}
        self.system_fields: Dict[str, List[str]] = {}  # system_id -> [field_ids]
        self.ice_types_data = self._load_ice_types()
        
    def _load_ice_types(self) -> Dict:
        """Load ice types from data file"""
        data_path = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(__file__))), 
                                'data', 'ice_types.json')
        try:
            with open(data_path, 'r') as f:
                data = json.load(f)
                return data.get('ice_types', {})
        except FileNotFoundError:
            print(f"Warning: ice_types.json not found at {data_path}")
            return {}
    
    def create_ice_field(
        self,
        field_id: str,
        ice_type: str,
        system_id: str,
        position: Tuple[float, float, float]
    ) -> IceField:
        """
        Create a new ice field
        
        Args:
            field_id: Unique identifier for the field
            ice_type: Type of ice in this field
            system_id: Solar system containing the field
            position: (x, y, z) position of the field
            
        Returns:
            Created IceField
        """
        field = IceField(field_id, ice_type, system_id, position)
        self.fields[field_id] = field
        
        if system_id not in self.system_fields:
            self.system_fields[system_id] = []
        self.system_fields[system_id].append(field_id)
        
        return field
    
    def get_ice_field(self, field_id: str) -> Optional[IceField]:
        """Get an ice field by ID"""
        return self.fields.get(field_id)
    
    def get_fields_in_system(self, system_id: str) -> List[IceField]:
        """Get all ice fields in a system"""
        field_ids = self.system_fields.get(system_id, [])
        return [self.fields[fid] for fid in field_ids if fid in self.fields]
    
    def remove_field(self, field_id: str):
        """Remove a depleted or destroyed field"""
        if field_id in self.fields:
            field = self.fields[field_id]
            if field.system_id in self.system_fields:
                self.system_fields[field.system_id].remove(field_id)
            del self.fields[field_id]


class IceMiningSystem(System):
    """
    Manages ice harvesting operations including:
    - Ice harvester cycles
    - Ice extraction from fields
    - Cycle time calculations with skill/module bonuses
    - Ice storage in cargo/ore holds
    """
    
    def __init__(self, world, ice_field_manager: IceFieldManager):
        super().__init__(world)
        self.ice_field_manager = ice_field_manager
        
    def update(self, delta_time: float):
        """Update ice harvesting cycles for all active harvesters"""
        entities = self.get_entities(IceHarvester, Position, Capacitor)
        
        for entity in entities:
            ice_harvester = entity.get_component(IceHarvester)
            
            if not ice_harvester.is_active:
                continue
                
            # Update harvesting cycle
            ice_harvester.current_cycle += delta_time
            
            # Check if cycle complete
            if ice_harvester.current_cycle >= ice_harvester.cycle_time:
                self._complete_harvesting_cycle(entity)
                ice_harvester.current_cycle = 0.0
    
    def start_harvesting(
        self,
        entity,
        target_field_id: str
    ) -> bool:
        """
        Start harvesting ice from a field
        
        Args:
            entity: The harvesting entity
            target_field_id: ID of the ice field to harvest
            
        Returns:
            True if harvesting started successfully
        """
        ice_harvester = entity.get_component(IceHarvester)
        position = entity.get_component(Position)
        capacitor = entity.get_component(Capacitor)
        
        if not all([ice_harvester, position, capacitor]):
            return False
        
        # Find the ice field
        ice_field = self.ice_field_manager.get_ice_field(target_field_id)
        if not ice_field:
            return False
        
        # Check if field is depleted
        if ice_field.is_depleted():
            return False
        
        # Check range
        distance = self._calculate_distance(
            (position.x, position.y, position.z),
            ice_field.position
        )
        
        if distance > ice_harvester.optimal_range:
            return False
        
        # Check capacitor
        if capacitor.capacitor < ice_harvester.capacitor_usage:
            return False
        
        # Start harvesting
        ice_harvester.is_active = True
        ice_harvester.target_ice_id = target_field_id
        ice_harvester.current_cycle = 0.0
        
        # Update yield tracking
        mining_yield = entity.get_component(MiningYield)
        if mining_yield:
            mining_yield.active_mining = True
            mining_yield.target_ice_id = target_field_id
        
        return True
    
    def stop_harvesting(self, entity) -> bool:
        """Stop harvesting ice"""
        ice_harvester = entity.get_component(IceHarvester)
        mining_yield = entity.get_component(MiningYield)
        
        if not ice_harvester:
            return False
        
        ice_harvester.is_active = False
        ice_harvester.target_ice_id = None
        ice_harvester.current_cycle = 0.0
        
        if mining_yield:
            mining_yield.active_mining = False
            mining_yield.target_ice_id = None
        
        return True
    
    def _complete_harvesting_cycle(self, entity):
        """Complete one harvesting cycle and extract ice"""
        ice_harvester = entity.get_component(IceHarvester)
        capacitor = entity.get_component(Capacitor)
        mining_yield = entity.get_component(MiningYield)
        
        # Check capacitor
        if capacitor.capacitor < ice_harvester.capacitor_usage:
            self.stop_harvesting(entity)
            return
        
        # Consume capacitor
        capacitor.capacitor -= ice_harvester.capacitor_usage
        
        # Find the ice field
        ice_field = self.ice_field_manager.get_ice_field(ice_harvester.target_ice_id)
        if not ice_field or ice_field.is_depleted():
            self.stop_harvesting(entity)
            return
        
        # Calculate cycle time bonus from skills
        cycle_time_multiplier = self._calculate_cycle_time_multiplier(entity)
        actual_cycle_time = ice_harvester.cycle_time * cycle_time_multiplier
        
        # Always harvest 1 unit of ice (EVE mechanics)
        units_harvested = ice_field.harvest(1)
        
        # Store ice (prioritize ore hold, then cargo)
        stored = self._store_ice(entity, ice_field.ice_type, units_harvested)
        
        if not stored:
            # No space, stop harvesting
            self.stop_harvesting(entity)
            return
        
        # Update statistics
        if mining_yield:
            if ice_field.ice_type not in mining_yield.total_ice_mined:
                mining_yield.total_ice_mined[ice_field.ice_type] = 0
            mining_yield.total_ice_mined[ice_field.ice_type] += units_harvested
    
    def _calculate_cycle_time_multiplier(self, entity) -> float:
        """Calculate cycle time multiplier from skills and modules"""
        multiplier = 1.0
        
        # Ice Harvesting skill: -5% cycle time per level
        skills = entity.get_component(Skills)
        if skills and 'ice_harvesting' in skills.trained_skills:
            level = skills.trained_skills['ice_harvesting']['level']
            multiplier -= (level * 0.05)
        
        # Ice Harvester Upgrade modules (apply stacking penalty)
        fitting = entity.get_component(Fitting)
        if fitting:
            upgrade_bonuses = []
            for module_id, module in fitting.low_slots.items():
                if module and 'ice_harvester_upgrade' in module.get('id', ''):
                    bonus = module.get('bonuses', {}).get('ice_harvesting_cycle_time', 0)
                    if bonus:
                        upgrade_bonuses.append(abs(bonus))  # Convert negative to positive for calculation
            
            # Apply stacking penalty
            upgrade_bonuses.sort(reverse=True)
            for i, bonus in enumerate(upgrade_bonuses):
                penalty = math.exp(-((i / 2.67) ** 2))
                multiplier -= (bonus * penalty)
        
        # Minimum cycle time is 50% of base
        multiplier = max(0.5, multiplier)
        return multiplier
    
    def _store_ice(self, entity, ice_type: str, units: int) -> bool:
        """
        Store harvested ice in ore hold or cargo
        
        Args:
            entity: Entity doing the harvesting
            ice_type: Type of ice harvested
            units: Number of units to store
            
        Returns:
            True if storage successful
        """
        # Try ore hold first
        ore_hold = entity.get_component(OreHold)
        if ore_hold:
            # Ice takes 1000 m3 per unit
            volume_needed = units * 1000.0
            available = ore_hold.ore_hold_capacity - ore_hold.ore_hold_used
            
            if available >= volume_needed:
                if ice_type not in ore_hold.ice:
                    ore_hold.ice[ice_type] = 0
                ore_hold.ice[ice_type] += units
                ore_hold.ore_hold_used += volume_needed
                return True
        
        # Try regular cargo
        inventory = entity.get_component(Inventory)
        if inventory:
            volume_needed = units * 1000.0
            available = inventory.capacity - inventory.used_capacity
            
            if available >= volume_needed:
                if ice_type not in inventory.items:
                    inventory.items[ice_type] = 0
                inventory.items[ice_type] += units
                inventory.used_capacity += volume_needed
                return True
        
        return False
    
    def _calculate_distance(self, pos1: Tuple[float, float, float], pos2: Tuple[float, float, float]) -> float:
        """Calculate distance between two positions"""
        dx = pos1[0] - pos2[0]
        dy = pos1[1] - pos2[1]
        dz = pos1[2] - pos2[2]
        return math.sqrt(dx*dx + dy*dy + dz*dz)
    
    def get_nearby_ice_fields(
        self,
        entity,
        max_distance: float = 100000.0
    ) -> List[Tuple[IceField, float]]:
        """
        Get list of ice fields near an entity
        
        Args:
            entity: Entity to check from
            max_distance: Maximum distance to search
            
        Returns:
            List of (ice_field, distance) tuples, sorted by distance
        """
        position = entity.get_component(Position)
        if not position:
            return []
        
        nearby = []
        entity_pos = (position.x, position.y, position.z)
        
        for field in self.ice_field_manager.fields.values():
            if field.is_depleted():
                continue
            
            distance = self._calculate_distance(entity_pos, field.position)
            if distance <= max_distance:
                nearby.append((field, distance))
        
        # Sort by distance
        nearby.sort(key=lambda x: x[1])
        return nearby
    
    def get_harvesting_info(self, entity) -> Dict:
        """Get current ice harvesting status and information"""
        ice_harvester = entity.get_component(IceHarvester)
        mining_yield = entity.get_component(MiningYield)
        
        if not ice_harvester:
            return {"error": "No ice harvester equipped"}
        
        cycle_time_multiplier = self._calculate_cycle_time_multiplier(entity)
        actual_cycle_time = ice_harvester.cycle_time * cycle_time_multiplier
        
        info = {
            "is_active": ice_harvester.is_active,
            "base_cycle_time": ice_harvester.cycle_time,
            "actual_cycle_time": actual_cycle_time,
            "current_cycle": ice_harvester.current_cycle,
            "cycle_progress": (ice_harvester.current_cycle / actual_cycle_time * 100) if ice_harvester.is_active else 0,
            "yield_per_cycle": ice_harvester.yield_amount,  # Always 1 unit
            "optimal_range": ice_harvester.optimal_range,
            "capacitor_usage": ice_harvester.capacitor_usage,
            "target_field_id": ice_harvester.target_ice_id
        }
        
        if mining_yield:
            info["total_ice_mined"] = mining_yield.total_ice_mined
            info["active_harvesting"] = mining_yield.active_mining
        
        return info
    
    def reprocess_ice(
        self,
        entity,
        ice_type: str,
        units: int
    ) -> Dict[str, int]:
        """
        Reprocess ice into fuel and materials
        
        Args:
            entity: Entity doing the reprocessing
            ice_type: Type of ice to reprocess
            units: Number of units to reprocess
            
        Returns:
            Dictionary of {material: units} produced
        """
        # Load ice type data
        ice_data = self.ice_field_manager.ice_types_data.get(ice_type)
        if not ice_data:
            return {}
        
        # Check if entity has the ice
        ore_hold = entity.get_component(OreHold)
        inventory = entity.get_component(Inventory)
        
        available_units = 0
        if ore_hold and ice_type in ore_hold.ice:
            available_units += ore_hold.ice[ice_type]
        if inventory and ice_type in inventory.items:
            available_units += inventory.items[ice_type]
        
        if available_units < units:
            return {}
        
        # Calculate yield with skills
        skills = entity.get_component(Skills)
        yield_multiplier = 1.0
        
        # Ice Processing skill: +2% yield per level
        if skills and 'ice_processing' in skills.trained_skills:
            level = skills.trained_skills['ice_processing']['level']
            yield_multiplier += (level * 0.02)
        
        # Base reprocessing skill: +3% yield per level
        if skills and 'reprocessing' in skills.trained_skills:
            level = skills.trained_skills['reprocessing']['level']
            yield_multiplier += (level * 0.03)
        
        # Calculate products
        products = {}
        base_products = ice_data.get('reprocessing', {}).get('products', {})
        
        for material, base_amount in base_products.items():
            amount = int(base_amount * units * yield_multiplier)
            if amount > 0:
                products[material] = amount
        
        # Remove ice from storage
        units_removed = 0
        if ore_hold and ice_type in ore_hold.ice:
            remove = min(units - units_removed, ore_hold.ice[ice_type])
            ore_hold.ice[ice_type] -= remove
            ore_hold.ore_hold_used -= (remove * 1000.0)
            units_removed += remove
        
        if units_removed < units and inventory and ice_type in inventory.items:
            remove = units - units_removed
            inventory.items[ice_type] -= remove
            inventory.used_capacity -= (remove * 1000.0)
        
        return products
