"""
Entity Manager for 3D Client
Manages game entities and their state
"""

from typing import Dict, Any, Optional
import time


class Entity:
    """Represents a game entity with interpolated state"""
    
    def __init__(self, entity_id: str, data: Dict[str, Any]):
        self.id = entity_id
        self.data = data
        
        # Position (current and target for interpolation)
        pos = data.get('position', {})
        self.x = pos.get('x', 0.0)
        self.y = pos.get('y', 0.0)
        self.z = pos.get('z', 0.0)
        
        # Previous position (for interpolation)
        self.prev_x = self.x
        self.prev_y = self.y
        self.prev_z = self.z
        
        # Target position (from latest server update)
        self.target_x = self.x
        self.target_y = self.y
        self.target_z = self.z
        
        # Velocity
        vel = data.get('velocity', {})
        self.vx = vel.get('x', 0.0)
        self.vy = vel.get('y', 0.0)
        self.vz = vel.get('z', 0.0)
        
        # Health
        health = data.get('health', {})
        self.shield = health.get('shield', 0)
        self.armor = health.get('armor', 0)
        self.hull = health.get('hull', 0)
        
        # Ship info
        ship = data.get('ship', {})
        self.ship_type = ship.get('type', 'Unknown')
        self.faction = ship.get('faction', 'Unknown')
        
        # Other
        self.target_id = data.get('target')
        
        # Timing
        self.last_update = time.time()
        self.interpolation_time = 0.1  # 100ms interpolation
        
    def update_from_server(self, data: Dict[str, Any]):
        """Update entity state from server data"""
        # Save current position as previous
        self.prev_x = self.x
        self.prev_y = self.y
        self.prev_z = self.z
        
        # Update target position
        pos = data.get('position', {})
        self.target_x = pos.get('x', self.target_x)
        self.target_y = pos.get('y', self.target_y)
        self.target_z = pos.get('z', self.target_z)
        
        # Update velocity
        vel = data.get('velocity', {})
        self.vx = vel.get('x', self.vx)
        self.vy = vel.get('y', self.vy)
        self.vz = vel.get('z', self.vz)
        
        # Update health
        health = data.get('health', {})
        self.shield = health.get('shield', self.shield)
        self.armor = health.get('armor', self.armor)
        self.hull = health.get('hull', self.hull)
        
        # Update ship info
        ship = data.get('ship', {})
        self.ship_type = ship.get('type', self.ship_type)
        self.faction = ship.get('faction', self.faction)
        
        # Update other
        self.target_id = data.get('target')
        
        # Update timestamp
        self.last_update = time.time()
        self.data = data
        
    def interpolate(self, dt: float):
        """
        Interpolate entity position for smooth rendering
        
        Args:
            dt: Delta time since last update (seconds)
        """
        # Calculate interpolation factor (0 to 1)
        elapsed = time.time() - self.last_update
        t = min(elapsed / self.interpolation_time, 1.0)
        
        # Linear interpolation (lerp)
        self.x = self.prev_x + (self.target_x - self.prev_x) * t
        self.y = self.prev_y + (self.target_y - self.prev_y) * t
        self.z = self.prev_z + (self.target_z - self.prev_z) * t
        
        # Alternative: Extrapolate based on velocity (commented out for now)
        # self.x = self.target_x + self.vx * elapsed
        # self.y = self.target_y + self.vy * elapsed
        # self.z = self.target_z + self.vz * elapsed
    
    def get_position(self) -> tuple:
        """Get current interpolated position"""
        return (self.x, self.y, self.z)
    
    def get_health_percentage(self) -> Dict[str, float]:
        """Get health as percentages (0-1)"""
        # Note: We don't have max values, so we'll need to track them
        # For now, assume some reasonable defaults
        return {
            'shield': self.shield / 1000.0 if self.shield else 0.0,
            'armor': self.armor / 1000.0 if self.armor else 0.0,
            'hull': self.hull / 1000.0 if self.hull else 0.0,
        }


class EntityManager:
    """Manages all game entities"""
    
    def __init__(self):
        self.entities: Dict[str, Entity] = {}
        self.player_id: Optional[str] = None
        
    def set_player_id(self, player_id: str):
        """Set the local player's entity ID"""
        self.player_id = player_id
        
    def get_player_entity(self) -> Optional[Entity]:
        """Get the player's entity"""
        if self.player_id:
            return self.entities.get(self.player_id)
        return None
        
    def update_from_state(self, state_data: Dict[str, Any]):
        """
        Update entities from server state update
        
        Args:
            state_data: State update data from server
        """
        entities_data = state_data.get('entities', [])
        
        # Track which entities we received
        received_ids = set()
        
        for entity_data in entities_data:
            entity_id = entity_data.get('id')
            if not entity_id:
                continue
                
            received_ids.add(entity_id)
            
            if entity_id in self.entities:
                # Update existing entity
                self.entities[entity_id].update_from_server(entity_data)
            else:
                # Create new entity
                self.entities[entity_id] = Entity(entity_id, entity_data)
                print(f"[EntityManager] New entity: {entity_id} ({entity_data.get('ship', {}).get('type', 'Unknown')})")
        
        # Remove entities that weren't in the update
        # (they were likely destroyed)
        entities_to_remove = []
        for entity_id in self.entities:
            if entity_id not in received_ids:
                entities_to_remove.append(entity_id)
        
        for entity_id in entities_to_remove:
            print(f"[EntityManager] Removing entity: {entity_id}")
            del self.entities[entity_id]
    
    def update_interpolation(self, dt: float):
        """
        Update entity interpolation
        
        Args:
            dt: Delta time since last frame (seconds)
        """
        for entity in self.entities.values():
            entity.interpolate(dt)
    
    def get_entity(self, entity_id: str) -> Optional[Entity]:
        """Get entity by ID"""
        return self.entities.get(entity_id)
    
    def get_all_entities(self) -> Dict[str, Entity]:
        """Get all entities"""
        return self.entities
    
    def get_entities_by_type(self, entity_type: str) -> list:
        """Get entities of a specific type (e.g., 'player', 'npc')"""
        # This would need entity type tracking in the data
        # For now, return all entities
        return list(self.entities.values())
    
    def clear(self):
        """Clear all entities"""
        self.entities.clear()
        print("[EntityManager] All entities cleared")
