"""
Manufacturing and Industry System
Handles blueprint research, manufacturing, and invention
"""

from typing import Optional, Dict, List, Tuple, Union
from dataclasses import dataclass
from engine.core.ecs import World, Entity
from engine.components.game_components import Position, Ship


@dataclass
class Blueprint:
    """Blueprint for manufacturing items"""
    blueprint_id: str
    item_type: str  # 'ship', 'module', 'ammo'
    item_id: str  # The item this blueprint creates
    material_efficiency: int = 0  # 0-10, reduces material cost
    time_efficiency: int = 0  # 0-20, reduces build time
    runs: int = -1  # -1 for BPO (infinite), >0 for BPC
    
    # Manufacturing requirements
    materials: Dict[str, int] = None  # {material_id: quantity}
    manufacturing_time: float = 3600.0  # seconds
    
    def __post_init__(self):
        if self.materials is None:
            self.materials = {}


@dataclass
class BlueprintInventory:
    """Component for storing blueprints"""
    blueprints: Dict[str, Blueprint] = None  # {blueprint_id: Blueprint}
    
    def __post_init__(self):
        if self.blueprints is None:
            self.blueprints = {}


@dataclass
class Inventory:
    """Component for storing materials and items"""
    items: Dict[str, int] = None  # {item_id: quantity}
    capacity: float = 1000.0  # m3
    used_capacity: float = 0.0
    
    def __post_init__(self):
        if self.items is None:
            self.items = {}
    
    def add_item(self, item_id: str, quantity: int, volume_per_item: float = 1.0) -> bool:
        """Add items to inventory if there's space"""
        required_volume = quantity * volume_per_item
        if self.used_capacity + required_volume <= self.capacity:
            self.items[item_id] = self.items.get(item_id, 0) + quantity
            self.used_capacity += required_volume
            return True
        return False
    
    def remove_item(self, item_id: str, quantity: int, volume_per_item: float = 1.0) -> bool:
        """Remove items from inventory"""
        if self.items.get(item_id, 0) >= quantity:
            self.items[item_id] -= quantity
            self.used_capacity -= quantity * volume_per_item
            if self.items[item_id] == 0:
                del self.items[item_id]
            return True
        return False
    
    def has_items(self, required_items: Dict[str, int]) -> bool:
        """Check if inventory has all required items"""
        for item_id, quantity in required_items.items():
            if self.items.get(item_id, 0) < quantity:
                return False
        return True


@dataclass
class ManufacturingJob:
    """Active manufacturing job"""
    job_id: str
    blueprint_id: str
    item_id: str
    quantity: int = 1
    start_time: float = 0.0
    end_time: float = 0.0
    status: str = "in_progress"  # 'in_progress', 'completed', 'delivered'


@dataclass
class ManufacturingQueue:
    """Component for active manufacturing jobs"""
    jobs: List[ManufacturingJob] = None
    max_jobs: int = 10
    
    def __post_init__(self):
        if self.jobs is None:
            self.jobs = []


class IndustrySystem:
    """
    Handles manufacturing, research, and invention
    Based on EVE Online's industry mechanics
    """
    
    def __init__(self, world: World):
        self.world = world
        self.job_counter = 0
    
    def _get_entity(self, entity: Union[Entity, str]) -> Optional[Entity]:
        """Helper to get Entity from either Entity or entity_id string"""
        if isinstance(entity, str):
            return self.world.get_entity(entity)
        return entity
    
    def start_manufacturing(
        self, 
        entity: Union[Entity, str], 
        blueprint_id: str, 
        quantity: int = 1,
        current_time: float = 0.0
    ) -> Optional[str]:
        """
        Start a manufacturing job
        
        Args:
            entity: Entity or entity_id with BlueprintInventory and Inventory
            blueprint_id: Blueprint to manufacture from
            quantity: Number of items to manufacture
            current_time: Current game time
            
        Returns:
            job_id if successful, None otherwise
        """
        entity = self._get_entity(entity)
        if not entity:
            return None
        
        bp_inv = entity.get_component(BlueprintInventory)
        inventory = entity.get_component(Inventory)
        mfg_queue = entity.get_component(ManufacturingQueue)
        
        if not all([bp_inv, inventory, mfg_queue]):
            return None
        
        # Get blueprint
        blueprint = bp_inv.blueprints.get(blueprint_id)
        if not blueprint:
            return None
        
        # Check if we have runs (for BPCs)
        if blueprint.runs != -1 and blueprint.runs < quantity:
            return None
        
        # Check materials
        required_materials = {}
        for material_id, base_qty in blueprint.materials.items():
            # Apply material efficiency (each level reduces waste by 1%)
            efficiency_multiplier = 1.0 - (blueprint.material_efficiency * 0.01)
            required_qty = int(base_qty * quantity * efficiency_multiplier)
            required_materials[material_id] = max(1, required_qty)  # Minimum 1
        
        if not inventory.has_items(required_materials):
            return None
        
        # Check queue space
        if len(mfg_queue.jobs) >= mfg_queue.max_jobs:
            return None
        
        # Consume materials
        for material_id, qty in required_materials.items():
            inventory.remove_item(material_id, qty)
        
        # Consume runs (for BPCs)
        if blueprint.runs != -1:
            blueprint.runs -= quantity
        
        # Calculate manufacturing time with time efficiency
        time_multiplier = 1.0 - (blueprint.time_efficiency * 0.01)
        job_time = blueprint.manufacturing_time * quantity * time_multiplier
        
        # Create job
        self.job_counter += 1
        job = ManufacturingJob(
            job_id=f"mfg_job_{self.job_counter}",
            blueprint_id=blueprint_id,
            item_id=blueprint.item_id,
            quantity=quantity,
            start_time=current_time,
            end_time=current_time + job_time,
            status="in_progress"
        )
        
        mfg_queue.jobs.append(job)
        return job.job_id
    
    def research_material_efficiency(
        self,
        entity: Union[Entity, str],
        blueprint_id: str,
        current_time: float = 0.0
    ) -> bool:
        """Research material efficiency on a blueprint"""
        entity = self._get_entity(entity)
        if not entity:
            return False
        bp_inv = entity.get_component( BlueprintInventory)
        if not bp_inv:
            return False
        
        blueprint = bp_inv.blueprints.get(blueprint_id)
        if not blueprint:
            return False
        
        # Can only research BPOs, not BPCs
        if blueprint.runs != -1:
            return False
        
        # Max ME is 10
        if blueprint.material_efficiency >= 10:
            return False
        
        blueprint.material_efficiency += 1
        return True
    
    def research_time_efficiency(
        self,
        entity: Union[Entity, str],
        blueprint_id: str,
        current_time: float = 0.0
    ) -> bool:
        """Research time efficiency on a blueprint"""
        entity = self._get_entity(entity)
        if not entity:
            return False
        bp_inv = entity.get_component( BlueprintInventory)
        if not bp_inv:
            return False
        
        blueprint = bp_inv.blueprints.get(blueprint_id)
        if not blueprint:
            return False
        
        # Can only research BPOs, not BPCs
        if blueprint.runs != -1:
            return False
        
        # Max TE is 20
        if blueprint.time_efficiency >= 20:
            return False
        
        blueprint.time_efficiency += 1
        return True
    
    def copy_blueprint(
        self,
        entity: Union[Entity, str],
        blueprint_id: str,
        runs: int = 10
    ) -> bool:
        """Create a blueprint copy (BPC) from an original (BPO)"""
        entity = self._get_entity(entity)
        if not entity:
            return False
        bp_inv = entity.get_component( BlueprintInventory)
        if not bp_inv:
            return False
        
        blueprint = bp_inv.blueprints.get(blueprint_id)
        if not blueprint:
            return False
        
        # Can only copy BPOs
        if blueprint.runs != -1:
            return False
        
        # Create copy
        self.job_counter += 1
        copy_id = f"{blueprint_id}_copy_{self.job_counter}"
        
        blueprint_copy = Blueprint(
            blueprint_id=copy_id,
            item_type=blueprint.item_type,
            item_id=blueprint.item_id,
            material_efficiency=blueprint.material_efficiency,
            time_efficiency=blueprint.time_efficiency,
            runs=runs,
            materials=blueprint.materials.copy(),
            manufacturing_time=blueprint.manufacturing_time
        )
        
        bp_inv.blueprints[copy_id] = blueprint_copy
        return True
    
    def update(self, delta_time: float, current_time: float):
        """Update all manufacturing jobs"""
        for entity in self.world.get_entities(ManufacturingQueue):
            mfg_queue = entity.get_component( ManufacturingQueue)
            inventory = entity.get_component( Inventory)
            
            if not inventory:
                continue
            
            # Check for completed jobs
            completed_jobs = []
            for job in mfg_queue.jobs:
                if job.status == "in_progress" and current_time >= job.end_time:
                    job.status = "completed"
                    completed_jobs.append(job)
            
            # Deliver completed jobs to inventory
            for job in completed_jobs:
                # Add manufactured items to inventory
                inventory.add_item(job.item_id, job.quantity)
                job.status = "delivered"
            
            # Remove delivered jobs
            mfg_queue.jobs = [job for job in mfg_queue.jobs if job.status != "delivered"]
    
    def get_active_jobs(self, entity: Entity) -> List[ManufacturingJob]:
        """Get all active manufacturing jobs for an entity"""
        entity = self._get_entity(entity)
        if not entity:
            return []
        mfg_queue = entity.get_component( ManufacturingQueue)
        if not mfg_queue:
            return []
        return [job for job in mfg_queue.jobs if job.status == "in_progress"]
    
    def cancel_job(self, entity: Union[Entity, str], job_id: str) -> bool:
        """Cancel a manufacturing job (no refund)"""
        mfg_queue = entity.get_component( ManufacturingQueue)
        if not mfg_queue:
            return False
        
        for i, job in enumerate(mfg_queue.jobs):
            if job.job_id == job_id and job.status == "in_progress":
                mfg_queue.jobs.pop(i)
                return True
        
        return False
    
    def reprocess_ore(
        self,
        entity: Union[Entity, str],
        ore_type: str,
        quantity: float,
        ore_data: Dict,
        station_efficiency: float = 0.50,
        skills_bonus: float = 0.0
    ) -> Optional[Dict[str, int]]:
        """
        Reprocess ore into minerals
        
        Args:
            entity: Entity with Inventory or OreHold
            ore_type: Type of ore to reprocess (e.g., 'veldspar')
            quantity: Amount of ore in m3 to reprocess
            ore_data: Ore type data dict with mineral yields
            station_efficiency: Base station efficiency (0.0-1.0, default 0.50)
            skills_bonus: Bonus from Reprocessing skills (0.0-0.20+)
            
        Returns:
            Dict of {mineral_id: quantity} if successful, None otherwise
        """
        entity = self._get_entity(entity)
        if not entity:
            return None
        
        from engine.components.game_components import OreHold, Inventory as InvComponent
        
        # Check for ore in ore hold first, then inventory
        ore_hold = entity.get_component(OreHold)
        inventory = entity.get_component(InvComponent)
        
        ore_available = 0.0
        source = None
        
        if ore_hold and ore_type in ore_hold.ore:
            ore_available = ore_hold.ore[ore_type]
            source = "ore_hold"
        elif inventory and ore_type in inventory.items:
            ore_available = float(inventory.items[ore_type])
            source = "inventory"
        
        if ore_available < quantity:
            return None
        
        # Calculate reprocessing yield
        # Base efficiency + Skills bonus
        total_efficiency = min(station_efficiency + skills_bonus, 1.0)
        
        # Get mineral yields from ore data
        if 'minerals' not in ore_data:
            return None
        
        # Calculate reprocessed units
        # In EVE, ore is reprocessed in batches based on reprocessing_base
        reprocessing_base = ore_data.get('reprocessing_base', 100)
        units_to_process = int(quantity / reprocessing_base)
        
        if units_to_process <= 0:
            return None
        
        # Calculate minerals gained
        minerals = {}
        for mineral_id, base_yield in ore_data['minerals'].items():
            mineral_amount = int(base_yield * units_to_process * total_efficiency)
            if mineral_amount > 0:
                minerals[mineral_id] = mineral_amount
        
        # Consume ore
        actual_ore_used = units_to_process * reprocessing_base
        if source == "ore_hold":
            ore_hold.ore[ore_type] -= actual_ore_used
            ore_hold.ore_hold_used -= actual_ore_used
            if ore_hold.ore[ore_type] <= 0:
                del ore_hold.ore[ore_type]
        else:
            inventory.items[ore_type] -= int(actual_ore_used)
            if inventory.items[ore_type] <= 0:
                del inventory.items[ore_type]
        
        # Add minerals to inventory
        if inventory:
            for mineral_id, qty in minerals.items():
                inventory.items[mineral_id] = inventory.items.get(mineral_id, 0) + qty
        
        print(f"[IndustrySystem] Reprocessed {actual_ore_used} {ore_type} into {minerals}")
        return minerals
    
    def calculate_reprocessing_efficiency(
        self,
        entity: Union[Entity, str],
        station_base: float = 0.50
    ) -> float:
        """
        Calculate total reprocessing efficiency from station and skills
        
        Args:
            entity: Entity with Skills component
            station_base: Base station efficiency (0.50 = 50%)
            
        Returns:
            Total efficiency from 0.0 to 1.0
        """
        entity = self._get_entity(entity)
        if not entity:
            return station_base
        
        from engine.components.game_components import Skills
        
        skills = entity.get_component(Skills)
        if not skills:
            return station_base
        
        # Reprocessing skill: 3% per level
        reprocessing_level = skills.skills.get("reprocessing", 0)
        reprocessing_bonus = reprocessing_level * 0.03
        
        # Reprocessing Efficiency: 2% per level
        efficiency_level = skills.skills.get("reprocessing_efficiency", 0)
        efficiency_bonus = efficiency_level * 0.02
        
        total_efficiency = min(station_base + reprocessing_bonus + efficiency_bonus, 1.0)
        return total_efficiency

