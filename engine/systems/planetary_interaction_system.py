"""
Planetary Interaction (PI) System
Handles planetary colony management, resource extraction, and processing
Based on EVE Online PI mechanics
"""

import math
from typing import Optional, Dict, List, Tuple
from engine.core.ecs import System
from engine.components.game_components import (
    PlanetaryColony, PIStructure, PIResource, ExtractorQueue,
    Inventory, Skills, Player
)


class PlanetaryInteractionSystem(System):
    """
    Manages planetary interaction operations including:
    - Colony creation and management
    - Resource extraction from planets
    - Material processing in factories
    - Product storage and export via launchpads
    """
    
    def __init__(self, world):
        super().__init__(world)
        self.planet_resources: Dict[str, List[PIResource]] = {}  # {planet_id: [resources]}
        
    def update(self, delta_time: float):
        """Update all active PI operations"""
        # Update extractors
        self._update_extractors(delta_time)
        
        # Update processors
        self._update_processors(delta_time)
        
    def _update_extractors(self, delta_time: float):
        """Update resource extraction for all active extractors"""
        entities = self.get_entities(PIStructure, ExtractorQueue)
        
        for entity in entities:
            structure = entity.get_component(PIStructure)
            queue = entity.get_component(ExtractorQueue)
            
            if not structure.is_active or structure.structure_type != "extractor":
                continue
            
            # Check if extraction program is still running
            current_time = self.world.game_time if hasattr(self.world, 'game_time') else 0
            if current_time >= queue.program_end_time:
                structure.is_active = False
                continue
            
            # Update extraction cycle
            structure.current_cycle_progress += delta_time
            
            # Check if cycle complete
            if structure.current_cycle_progress >= structure.extraction_cycle_time:
                self._complete_extraction_cycle(entity)
                structure.current_cycle_progress = 0.0
                queue.cycles_completed += 1
    
    def _update_processors(self, delta_time: float):
        """Update material processing for all active processors"""
        entities = self.get_entities(PIStructure)
        
        for entity in entities:
            structure = entity.get_component(PIStructure)
            
            if not structure.is_active:
                continue
                
            if structure.structure_type not in ["basic_processor", "advanced_processor"]:
                continue
            
            # Check if recipe is set
            if not structure.recipe_id:
                continue
            
            # Update processing cycle
            structure.current_cycle_progress += delta_time
            
            # Check if cycle complete
            if structure.current_cycle_progress >= structure.processing_cycle_time:
                self._complete_processing_cycle(entity)
                structure.current_cycle_progress = 0.0
    
    def create_colony(
        self,
        player_entity,
        planet_id: str,
        planet_type: str
    ) -> Optional[str]:
        """
        Create a new planetary colony for a player
        
        Args:
            player_entity: The player entity
            planet_id: ID of the planet
            planet_type: Type of planet (barren, temperate, etc.)
            
        Returns:
            Colony entity ID if successful, None otherwise
        """
        player = player_entity.get_component(Player)
        if not player:
            return None
        
        # Create colony entity
        colony_entity = self.world.create_entity()
        colony = PlanetaryColony(
            planet_id=planet_id,
            planet_type=planet_type,
            owner_id=player.character_id
        )
        colony_entity.add_component(colony)
        
        # Create command center
        cc_entity = self._create_command_center(colony_entity.id, planet_id)
        if cc_entity:
            colony.command_center_id = cc_entity.id
        
        return colony_entity.id
    
    def _create_command_center(
        self,
        colony_id: str,
        planet_id: str
    ):
        """Create command center structure for a colony"""
        cc_entity = self.world.create_entity()
        
        cc_structure = PIStructure(
            structure_type="command_center",
            colony_id=colony_id,
            planet_x=0.5,
            planet_y=0.5,
            cpu_usage=0.0,  # Command center provides CPU
            powergrid_usage=0.0,  # Command center provides power
            storage_capacity=500.0,
            is_active=True
        )
        
        cc_entity.add_component(cc_structure)
        return cc_entity
    
    def place_extractor(
        self,
        colony_entity,
        planet_x: float,
        planet_y: float,
        resource_id: str
    ) -> Optional[str]:
        """
        Place an extractor on a planet
        
        Args:
            colony_entity: The colony entity
            planet_x: X coordinate on planet (0.0-1.0)
            planet_y: Y coordinate on planet (0.0-1.0)
            resource_id: Resource to extract
            
        Returns:
            Extractor entity ID if successful
        """
        colony = colony_entity.get_component(PlanetaryColony)
        if not colony:
            return None
        
        # Check CPU/PG availability
        cpu_required = 400.0  # Standard extractor CPU
        pg_required = 2100.0  # Standard extractor PG
        
        if colony.cpu_used + cpu_required > colony.cpu_max:
            return None
        if colony.powergrid_used + pg_required > colony.powergrid_max:
            return None
        
        # Create extractor entity
        extractor_entity = self.world.create_entity()
        
        extractor = PIStructure(
            structure_type="extractor",
            colony_id=colony_entity.id,
            planet_x=planet_x,
            planet_y=planet_y,
            cpu_usage=cpu_required,
            powergrid_usage=pg_required,
            extraction_resource=resource_id,
            extraction_rate=100.0,  # Base rate
            extraction_cycle_time=3600.0,  # 1 hour cycles
            extraction_heads=5,  # Standard extractor
            extraction_area_radius=0.05,  # 5% of planet surface
            storage_capacity=10000.0
        )
        
        extractor_entity.add_component(extractor)
        
        # Create extraction queue
        queue = ExtractorQueue(
            extractor_id=extractor_entity.id,
            resource_id=resource_id
        )
        extractor_entity.add_component(queue)
        
        # Update colony resources
        colony.cpu_used += cpu_required
        colony.powergrid_used += pg_required
        colony.installations[extractor_entity.id] = "extractor"
        
        return extractor_entity.id
    
    def start_extraction_program(
        self,
        extractor_entity,
        duration_hours: float = 24.0
    ) -> bool:
        """
        Start an extraction program on an extractor
        
        Args:
            extractor_entity: The extractor entity
            duration_hours: Duration in hours (1-336, max 14 days)
            
        Returns:
            True if extraction started successfully
        """
        structure = extractor_entity.get_component(PIStructure)
        queue = extractor_entity.get_component(ExtractorQueue)
        
        if not all([structure, queue]):
            return False
        
        if structure.structure_type != "extractor":
            return False
        
        # Validate duration
        if duration_hours < 1 or duration_hours > 336:  # Max 14 days
            return False
        
        # Calculate extraction parameters
        current_time = self.world.game_time if hasattr(self.world, 'game_time') else 0
        duration_seconds = duration_hours * 3600
        
        # Start extraction program
        structure.is_active = True
        structure.current_cycle_progress = 0.0
        
        queue.program_start_time = current_time
        queue.program_duration = duration_seconds
        queue.program_end_time = current_time + duration_seconds
        queue.cycles_completed = 0
        queue.total_cycles = int(duration_seconds / structure.extraction_cycle_time)
        queue.total_extracted = 0
        
        # Calculate yield per cycle based on extraction rate
        base_yield = structure.extraction_rate
        queue.extraction_yield_per_cycle = int(base_yield * (structure.extraction_cycle_time / 3600.0))
        
        return True
    
    def _complete_extraction_cycle(self, extractor_entity):
        """Complete an extraction cycle and store extracted resources"""
        structure = extractor_entity.get_component(PIStructure)
        queue = extractor_entity.get_component(ExtractorQueue)
        
        if not all([structure, queue]):
            return
        
        # Extract resources
        extracted_amount = queue.extraction_yield_per_cycle
        
        # Check storage capacity
        current_volume = sum(structure.storage.values())
        if current_volume + extracted_amount > structure.storage_capacity:
            # Storage full, extract what fits
            extracted_amount = int(structure.storage_capacity - current_volume)
        
        if extracted_amount > 0:
            # Add to storage
            resource_id = structure.extraction_resource
            structure.storage[resource_id] = structure.storage.get(resource_id, 0) + extracted_amount
            queue.total_extracted += extracted_amount
    
    def place_processor(
        self,
        colony_entity,
        planet_x: float,
        planet_y: float,
        processor_type: str = "basic_processor"
    ) -> Optional[str]:
        """
        Place a processor facility on a planet
        
        Args:
            colony_entity: The colony entity
            planet_x: X coordinate on planet
            planet_y: Y coordinate on planet
            processor_type: Type of processor (basic or advanced)
            
        Returns:
            Processor entity ID if successful
        """
        colony = colony_entity.get_component(PlanetaryColony)
        if not colony:
            return None
        
        # Determine CPU/PG requirements
        if processor_type == "basic_processor":
            cpu_required = 200.0
            pg_required = 800.0
        elif processor_type == "advanced_processor":
            cpu_required = 500.0
            pg_required = 1100.0
        else:
            return None
        
        # Check availability
        if colony.cpu_used + cpu_required > colony.cpu_max:
            return None
        if colony.powergrid_used + pg_required > colony.powergrid_max:
            return None
        
        # Create processor entity
        processor_entity = self.world.create_entity()
        
        processor = PIStructure(
            structure_type=processor_type,
            colony_id=colony_entity.id,
            planet_x=planet_x,
            planet_y=planet_y,
            cpu_usage=cpu_required,
            powergrid_usage=pg_required,
            processing_cycle_time=600.0,  # 10 minutes
            storage_capacity=10000.0
        )
        
        processor_entity.add_component(processor)
        
        # Update colony resources
        colony.cpu_used += cpu_required
        colony.powergrid_used += pg_required
        colony.installations[processor_entity.id] = processor_type
        
        return processor_entity.id
    
    def set_processor_recipe(
        self,
        processor_entity,
        recipe_id: str
    ) -> bool:
        """
        Set the production recipe for a processor
        
        Args:
            processor_entity: The processor entity
            recipe_id: Recipe to produce
            
        Returns:
            True if recipe set successfully
        """
        structure = processor_entity.get_component(PIStructure)
        if not structure:
            return False
        
        if structure.structure_type not in ["basic_processor", "advanced_processor"]:
            return False
        
        structure.recipe_id = recipe_id
        structure.is_active = True
        structure.current_cycle_progress = 0.0
        
        return True
    
    def _complete_processing_cycle(self, processor_entity):
        """Complete a processing cycle and produce output"""
        structure = processor_entity.get_component(PIStructure)
        if not structure:
            return
        
        # In a full implementation, this would:
        # 1. Check for required input materials
        # 2. Consume inputs
        # 3. Produce outputs based on recipe
        # 4. Store outputs in processor storage
        
        # For now, just mark cycle as complete
        pass
    
    def place_launchpad(
        self,
        colony_entity,
        planet_x: float,
        planet_y: float
    ) -> Optional[str]:
        """
        Place a launchpad on a planet for exporting products
        
        Args:
            colony_entity: The colony entity
            planet_x: X coordinate
            planet_y: Y coordinate
            
        Returns:
            Launchpad entity ID if successful
        """
        colony = colony_entity.get_component(PlanetaryColony)
        if not colony:
            return None
        
        cpu_required = 3600.0
        pg_required = 700.0
        
        if colony.cpu_used + cpu_required > colony.cpu_max:
            return None
        if colony.powergrid_used + pg_required > colony.powergrid_max:
            return None
        
        # Create launchpad entity
        launchpad_entity = self.world.create_entity()
        
        launchpad = PIStructure(
            structure_type="launchpad",
            colony_id=colony_entity.id,
            planet_x=planet_x,
            planet_y=planet_y,
            cpu_usage=cpu_required,
            powergrid_usage=pg_required,
            storage_capacity=10000.0,
            is_active=True
        )
        
        launchpad_entity.add_component(launchpad)
        
        # Update colony
        colony.cpu_used += cpu_required
        colony.powergrid_used += pg_required
        colony.installations[launchpad_entity.id] = "launchpad"
        
        return launchpad_entity.id
    
    def transfer_materials(
        self,
        from_structure_entity,
        to_structure_entity,
        material_id: str,
        quantity: int
    ) -> bool:
        """
        Transfer materials between two PI structures
        
        Args:
            from_structure_entity: Source structure
            to_structure_entity: Destination structure
            material_id: Material to transfer
            quantity: Amount to transfer
            
        Returns:
            True if transfer successful
        """
        from_structure = from_structure_entity.get_component(PIStructure)
        to_structure = to_structure_entity.get_component(PIStructure)
        
        if not all([from_structure, to_structure]):
            return False
        
        # Check source has material
        if from_structure.storage.get(material_id, 0) < quantity:
            return False
        
        # Check destination capacity
        to_volume = sum(to_structure.storage.values())
        if to_volume + quantity > to_structure.storage_capacity:
            return False
        
        # Transfer
        from_structure.storage[material_id] -= quantity
        if from_structure.storage[material_id] == 0:
            del from_structure.storage[material_id]
        
        to_structure.storage[material_id] = to_structure.storage.get(material_id, 0) + quantity
        
        return True
    
    def export_from_launchpad(
        self,
        launchpad_entity,
        player_entity,
        material_id: str,
        quantity: int
    ) -> bool:
        """
        Export materials from a launchpad to player's inventory
        
        Args:
            launchpad_entity: The launchpad entity
            player_entity: The player entity
            material_id: Material to export
            quantity: Amount to export
            
        Returns:
            True if export successful
        """
        launchpad = launchpad_entity.get_component(PIStructure)
        inventory = player_entity.get_component(Inventory)
        
        if not all([launchpad, inventory]):
            return False
        
        if launchpad.structure_type != "launchpad":
            return False
        
        # Check launchpad has material
        if launchpad.storage.get(material_id, 0) < quantity:
            return False
        
        # Transfer to player inventory
        launchpad.storage[material_id] -= quantity
        if launchpad.storage[material_id] == 0:
            del launchpad.storage[material_id]
        
        inventory.items[material_id] = inventory.items.get(material_id, 0) + quantity
        
        return True
    
    def get_colony_info(self, colony_entity) -> Dict:
        """Get detailed information about a colony"""
        colony = colony_entity.get_component(PlanetaryColony)
        if not colony:
            return {}
        
        # Count installations by type
        installation_counts = {}
        for install_id, install_type in colony.installations.items():
            installation_counts[install_type] = installation_counts.get(install_type, 0) + 1
        
        return {
            "planet_id": colony.planet_id,
            "planet_type": colony.planet_type,
            "owner_id": colony.owner_id,
            "cpu_used": colony.cpu_used,
            "cpu_max": colony.cpu_max,
            "cpu_percent": (colony.cpu_used / colony.cpu_max * 100) if colony.cpu_max > 0 else 0,
            "powergrid_used": colony.powergrid_used,
            "powergrid_max": colony.powergrid_max,
            "pg_percent": (colony.powergrid_used / colony.powergrid_max * 100) if colony.powergrid_max > 0 else 0,
            "installations": installation_counts,
            "total_installations": len(colony.installations)
        }
