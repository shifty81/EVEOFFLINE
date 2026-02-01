"""
Entity Component System (ECS) Implementation
Inspired by EVE ONLINE's entity architecture
"""

from typing import Dict, List, Set, Type, Any, Optional
from dataclasses import dataclass, field
import uuid


class Component:
    """Base class for all components"""
    pass


class Entity:
    """
    Entity represents a game object (ship, NPC, projectile, etc.)
    Entities are just IDs with attached components
    """
    
    def __init__(self, entity_id: Optional[str] = None):
        self.id = entity_id or str(uuid.uuid4())
        self.components: Dict[Type[Component], Component] = {}
        
    def add_component(self, component: Component) -> 'Entity':
        """Add a component to this entity"""
        self.components[type(component)] = component
        return self
        
    def remove_component(self, component_type: Type[Component]) -> 'Entity':
        """Remove a component from this entity"""
        if component_type in self.components:
            del self.components[component_type]
        return self
        
    def get_component(self, component_type: Type[Component]) -> Optional[Component]:
        """Get a component from this entity"""
        return self.components.get(component_type)
        
    def has_component(self, component_type: Type[Component]) -> bool:
        """Check if entity has a component"""
        return component_type in self.components


class System:
    """
    Base class for all systems
    Systems operate on entities with specific component combinations
    """
    
    def __init__(self, world: 'World'):
        self.world = world
        
    def update(self, delta_time: float):
        """Update this system. Called every frame."""
        raise NotImplementedError
        
    def get_entities(self, *component_types: Type[Component]) -> List[Entity]:
        """Get all entities that have all specified components"""
        return self.world.get_entities(*component_types)


class World:
    """
    World manages all entities and systems
    Represents the game state
    """
    
    def __init__(self):
        self.entities: Dict[str, Entity] = {}
        self.systems: List[System] = []
        
    def create_entity(self, entity_id: Optional[str] = None) -> Entity:
        """Create a new entity in the world"""
        entity = Entity(entity_id)
        self.entities[entity.id] = entity
        return entity
        
    def destroy_entity(self, entity_id: str):
        """Remove an entity from the world"""
        if entity_id in self.entities:
            del self.entities[entity_id]
            
    def get_entity(self, entity_id: str) -> Optional[Entity]:
        """Get an entity by ID"""
        return self.entities.get(entity_id)
        
    def add_system(self, system: System):
        """Add a system to the world"""
        self.systems.append(system)
        
    def update(self, delta_time: float):
        """Update all systems"""
        for system in self.systems:
            system.update(delta_time)
            
    def get_entities(self, *component_types: Type[Component]) -> List[Entity]:
        """Get all entities that have all specified components"""
        if not component_types:
            return list(self.entities.values())
            
        result = []
        for entity in self.entities.values():
            if all(entity.has_component(comp_type) for comp_type in component_types):
                result.append(entity)
        return result
