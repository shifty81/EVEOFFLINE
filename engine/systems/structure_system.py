"""
Structure System
Manages player-owned structures including deployment, maintenance, and services.
"""

import time
from typing import Dict, List, Optional, Set
from engine.core.ecs import World
from engine.components.game_components import (
    Structure, StructureService, Position, Player, 
    CorporationMembership, Inventory
)


class StructureSystem:
    """System for managing player-owned structures"""
    
    def __init__(self, world: World):
        self.world = world
        self.structures: Dict[str, any] = {}  # structure_id: structure_data
        self.structure_locations: Dict[str, List[str]] = {}  # system_id: [structure_ids]
        
    def update(self, dt: float):
        """Update all structures"""
        current_time = time.time()
        
        # Get all entities with Structure component
        structure_entities = []
        for entity_id, entity in self.world.entities.items():
            if entity.has_component(Structure):
                structure_entities.append(entity_id)
        
        # Update anchoring structures
        for entity_id in structure_entities:
            entity = self.world.get_entity(entity_id)
            structure_comp = entity.get_component(Structure)
            
            if structure_comp.state == "anchoring":
                self._update_anchoring(entity_id, structure_comp, dt)
            elif structure_comp.state == "online":
                self._update_fuel_consumption(entity_id, structure_comp, current_time)
                self._update_services(entity_id, structure_comp)
            elif structure_comp.state == "reinforced":
                self._update_reinforcement(entity_id, structure_comp, current_time)
    
    def _update_anchoring(self, entity_id: str, structure: Structure, dt: float):
        """Update anchoring process"""
        structure.anchoring_time_remaining -= dt
        
        if structure.anchoring_time_remaining <= 0:
            structure.state = "online"
            structure.online_time = time.time()
            structure.anchoring_time_remaining = 0.0
            print(f"Structure {structure.structure_name} is now online!")
    
    def _update_fuel_consumption(self, entity_id: str, structure: Structure, current_time: float):
        """Update fuel consumption"""
        if structure.last_fuel_check == 0.0:
            structure.last_fuel_check = current_time
            return
        
        time_elapsed = current_time - structure.last_fuel_check
        hours_elapsed = time_elapsed / 3600.0
        
        fuel_needed = structure.fuel_usage_rate * hours_elapsed
        
        # Check if we have enough fuel
        total_fuel = sum(structure.fuel_bay.values())
        if total_fuel >= fuel_needed:
            # Consume fuel proportionally
            for fuel_type in structure.fuel_bay:
                if structure.fuel_bay[fuel_type] > 0:
                    consumed = min(fuel_needed, structure.fuel_bay[fuel_type])
                    structure.fuel_bay[fuel_type] -= consumed
                    fuel_needed -= consumed
                    if fuel_needed <= 0:
                        break
            structure.last_fuel_check = current_time
        else:
            # Out of fuel - go offline
            print(f"Structure {structure.structure_name} is out of fuel!")
            # Disable all services
            for service in structure.services:
                structure.services[service] = False
    
    def _update_services(self, entity_id: str, structure: Structure):
        """Update structure services"""
        # Services consume additional fuel based on what's active
        active_services = [s for s, active in structure.services.items() if active]
        
        # Each service entity has its own fuel cost
        for entity_id, entity in self.world.entities.items():
            if entity.has_component(StructureService):
                service_comp = entity.get_component(StructureService)
                if service_comp.is_online:
                    # Service is consuming fuel (already handled in fuel consumption)
                    pass
    
    def _update_reinforcement(self, entity_id: str, structure: Structure, current_time: float):
        """Update reinforcement timer"""
        # Reinforcement mechanics would go here
        # For now, just a placeholder
        pass
    
    def deploy_structure(self, player_entity_id: str, structure_type: str,
                        structure_name: str, system_id: str, x: float, y: float, z: float) -> Optional[str]:
        """
        Deploy a new structure
        
        Args:
            player_entity_id: Player deploying the structure
            structure_type: Type of structure (astrahus, orbital_skyhook, etc.)
            structure_name: Name for the structure
            system_id: Solar system to deploy in
            x, y, z: Position coordinates
            
        Returns:
            Structure entity ID if successful, None otherwise
        """
        # Get player info
        player_entity = self.world.get_entity(player_entity_id)
        if not player_entity:
            return None
        
        player_comp = player_entity.get_component(Player)
        if not player_comp:
            return None
        
        # Check if player has permission (corp member, etc.)
        corp_comp = player_entity.get_component(CorporationMembership)
        owner_corp_id = corp_comp.corporation_id if corp_comp else None
        
        # Load structure data
        structure_data = self._load_structure_data(structure_type)
        if not structure_data:
            print(f"Unknown structure type: {structure_type}")
            return None
        
        # Create structure entity
        structure_entity = self.world.create_entity()
        
        # Add Position component
        pos_comp = Position(x=x, y=y, z=z)
        structure_entity.add_component(pos_comp)
        
        # Add Structure component
        structure_comp = Structure(
            structure_type=structure_type,
            structure_name=structure_name,
            owner_corporation_id=owner_corp_id,
            owner_player_id=player_entity_id,
            state="anchoring",
            anchoring_time_remaining=structure_data.get("anchoring_time", 86400.0),
            hull_max=structure_data.get("hull_hp", 1000000.0),
            hull_hp=structure_data.get("hull_hp", 1000000.0),
            shield_max=structure_data.get("shield_hp", 500000.0),
            shield_hp=structure_data.get("shield_hp", 500000.0),
            armor_max=structure_data.get("armor_hp", 750000.0),
            armor_hp=structure_data.get("armor_hp", 750000.0),
            cpu_max=structure_data.get("cpu", 1000.0),
            powergrid_max=structure_data.get("powergrid", 500.0),
            hangar_capacity=structure_data.get("hangar_capacity", 50000.0),
            fuel_bay_capacity=structure_data.get("fuel_bay_capacity", 10000.0),
            fuel_usage_rate=structure_data.get("fuel_usage_rate", 1.0)
        )
        structure_entity.add_component(structure_comp)
        
        # Track structure location
        if system_id not in self.structure_locations:
            self.structure_locations[system_id] = []
        self.structure_locations[system_id].append(structure_entity.id)
        
        self.structures[structure_entity.id] = {
            "system_id": system_id,
            "deployed_time": time.time()
        }
        
        print(f"Deployed {structure_type} '{structure_name}' at ({x}, {y}, {z})")
        print(f"Anchoring time: {structure_comp.anchoring_time_remaining / 3600.0:.1f} hours")
        
        return structure_entity.id
    
    def _load_structure_data(self, structure_type: str) -> Optional[Dict]:
        """Load structure data from data files"""
        # This would load from data/universe/player_structures.json
        # For now, return default values
        structure_defaults = {
            "astrahus": {
                "anchoring_time": 86400.0,  # 24 hours
                "hull_hp": 5000000.0,
                "shield_hp": 2500000.0,
                "armor_hp": 3750000.0,
                "cpu": 2000.0,
                "powergrid": 1000.0,
                "hangar_capacity": 100000.0,
                "fuel_bay_capacity": 20000.0,
                "fuel_usage_rate": 5.0
            },
            "orbital_skyhook": {
                "anchoring_time": 86400.0,  # 24 hours
                "hull_hp": 3000000.0,
                "shield_hp": 1500000.0,
                "armor_hp": 2000000.0,
                "cpu": 1500.0,
                "powergrid": 800.0,
                "hangar_capacity": 50000.0,
                "fuel_bay_capacity": 15000.0,
                "fuel_usage_rate": 3.0
            },
            "sovereignty_hub": {
                "anchoring_time": 172800.0,  # 48 hours
                "hull_hp": 10000000.0,
                "shield_hp": 5000000.0,
                "armor_hp": 7500000.0,
                "cpu": 3000.0,
                "powergrid": 1500.0,
                "hangar_capacity": 150000.0,
                "fuel_bay_capacity": 30000.0,
                "fuel_usage_rate": 10.0
            },
            "metenox_moon_drill": {
                "anchoring_time": 43200.0,  # 12 hours
                "hull_hp": 2000000.0,
                "shield_hp": 1000000.0,
                "armor_hp": 1500000.0,
                "cpu": 1000.0,
                "powergrid": 600.0,
                "hangar_capacity": 30000.0,
                "fuel_bay_capacity": 10000.0,
                "fuel_usage_rate": 2.0
            }
        }
        
        return structure_defaults.get(structure_type.lower())
    
    def add_fuel(self, structure_entity_id: str, fuel_type: str, amount: int) -> bool:
        """Add fuel to a structure"""
        entity = self.world.get_entity(structure_entity_id)
        if not entity:
            return False
        
        structure_comp = entity.get_component(Structure)
        if not structure_comp:
            return False
        
        # Check capacity
        current_fuel = sum(structure_comp.fuel_bay.values())
        if current_fuel + amount > structure_comp.fuel_bay_capacity:
            print("Fuel bay is full!")
            return False
        
        # Add fuel
        if fuel_type not in structure_comp.fuel_bay:
            structure_comp.fuel_bay[fuel_type] = 0
        structure_comp.fuel_bay[fuel_type] += amount
        
        print(f"Added {amount} units of {fuel_type} to structure")
        return True
    
    def activate_service(self, structure_entity_id: str, service_type: str) -> bool:
        """Activate a service on a structure"""
        entity = self.world.get_entity(structure_entity_id)
        if not entity:
            return False
        
        structure_comp = entity.get_component(Structure)
        if not structure_comp:
            return False
        
        if structure_comp.state != "online":
            print("Structure must be online to activate services")
            return False
        
        # Create service entity
        service_entity = self.world.create_entity()
        service_comp = StructureService(
            service_type=service_type,
            is_online=True
        )
        service_entity.add_component(service_comp)
        
        # Update structure services dict
        structure_comp.services[service_type] = True
        
        print(f"Activated {service_type} service on structure")
        return True
    
    def get_structures_in_system(self, system_id: str) -> List[str]:
        """Get all structures in a system"""
        return self.structure_locations.get(system_id, [])
    
    def get_structure_info(self, structure_entity_id: str) -> Optional[Dict]:
        """Get information about a structure"""
        entity = self.world.get_entity(structure_entity_id)
        if not entity:
            return None
        
        structure_comp = entity.get_component(Structure)
        if not structure_comp:
            return None
        
        pos_comp = entity.get_component(Position)
        
        return {
            "entity_id": structure_entity_id,
            "name": structure_comp.structure_name,
            "type": structure_comp.structure_type,
            "state": structure_comp.state,
            "owner_corp": structure_comp.owner_corporation_id,
            "owner_player": structure_comp.owner_player_id,
            "position": (pos_comp.x, pos_comp.y, pos_comp.z) if pos_comp else None,
            "hull_hp": structure_comp.hull_hp,
            "hull_max": structure_comp.hull_max,
            "shield_hp": structure_comp.shield_hp,
            "shield_max": structure_comp.shield_max,
            "armor_hp": structure_comp.armor_hp,
            "armor_max": structure_comp.armor_max,
            "fuel": sum(structure_comp.fuel_bay.values()),
            "fuel_capacity": structure_comp.fuel_bay_capacity,
            "services": structure_comp.services,
            "anchoring_time_remaining": structure_comp.anchoring_time_remaining if structure_comp.state == "anchoring" else 0
        }
    
    def unanchor_structure(self, structure_entity_id: str, player_entity_id: str) -> bool:
        """Begin unanchoring a structure"""
        structure_entity = self.world.get_entity(structure_entity_id)
        if not structure_entity:
            return False
        
        structure_comp = structure_entity.get_component(Structure)
        if not structure_comp:
            return False
        
        # Check permissions
        player_entity = self.world.get_entity(player_entity_id)
        if not player_entity:
            return False
        
        player_comp = player_entity.get_component(Player)
        if not player_comp:
            return False
        
        # Check if player is owner or has corp permissions
        if structure_comp.owner_player_id != player_entity_id:
            corp_comp = player_entity.get_component(CorporationMembership)
            if not corp_comp or corp_comp.corporation_id != structure_comp.owner_corporation_id:
                print("You don't have permission to unanchor this structure")
                return False
        
        if structure_comp.state != "online":
            print("Structure must be online to unanchor")
            return False
        
        # Begin unanchoring process
        structure_comp.state = "unanchoring"
        structure_comp.anchoring_time_remaining = 86400.0  # 24 hours to unanchor
        
        print(f"Unanchoring {structure_comp.structure_name}...")
        return True
    
    def destroy_structure(self, structure_entity_id: str):
        """Destroy a structure (combat or unanchoring complete)"""
        entity = self.world.get_entity(structure_entity_id)
        if entity:
            structure_comp = entity.get_component(Structure)
            if structure_comp:
                # Drop loot from hangar
                # TODO: Implement loot dropping
                
                # Remove from tracking
                for system_id, structures in self.structure_locations.items():
                    if structure_entity_id in structures:
                        structures.remove(structure_entity_id)
                        break
                
                if structure_entity_id in self.structures:
                    del self.structures[structure_entity_id]
        
        # Remove entity
        self.world.destroy_entity(structure_entity_id)
        print(f"Structure destroyed")
