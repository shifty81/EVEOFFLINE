"""
3D Health Bars for Ships
Displays health bars above ships in 3D space
"""

from panda3d.core import Vec3, Vec4, NodePath, CardMaker, TextNode
from direct.gui.OnscreenText import OnscreenText
from typing import Dict, Optional, Any


class HealthBarManager:
    """
    Manages 3D health bars that appear above ships in the game world
    Shows shield/armor/hull status visually in 3D space
    """
    
    def __init__(self, render):
        self.render = render
        self.health_bars = {}  # entity_id -> health bar NodePath
        
        # Health bar settings
        self.bar_width = 10.0
        self.bar_height = 0.8
        self.bar_spacing = 0.2
        self.bar_offset_z = 8.0  # Height above ship
        
        print("[HealthBars] Health bar manager initialized")
    
    def _create_health_bar(self, entity_id: str, position: Vec3) -> NodePath:
        """
        Create a health bar for an entity
        
        Args:
            entity_id: Entity identifier
            position: Position of the entity
            
        Returns:
            NodePath of the health bar root
        """
        # Create root node for health bar
        bar_root = self.render.attachNewNode(f"healthbar_{entity_id}")
        bar_root.setPos(position.x, position.y, position.z + self.bar_offset_z)
        bar_root.setBillboardPointEye()  # Always face camera
        
        # Create background (black)
        bg_card = CardMaker(f"bg_{entity_id}")
        bg_card.setFrame(-self.bar_width/2, self.bar_width/2, 
                        -(self.bar_height * 3 + self.bar_spacing * 2) / 2,
                        (self.bar_height * 3 + self.bar_spacing * 2) / 2)
        bg_node = bar_root.attachNewNode(bg_card.generate())
        bg_node.setColor(0, 0, 0, 0.7)
        bg_node.setTransparency(True)
        
        # Create shield bar (blue)
        shield_card = CardMaker(f"shield_{entity_id}")
        shield_card.setFrame(-self.bar_width/2, self.bar_width/2, 
                            self.bar_height + self.bar_spacing,
                            self.bar_height * 2 + self.bar_spacing)
        shield_node = bar_root.attachNewNode(shield_card.generate())
        shield_node.setColor(0.3, 0.5, 1, 0.8)
        shield_node.setTransparency(True)
        shield_node.setName("shield_bar")
        
        # Create armor bar (yellow)
        armor_card = CardMaker(f"armor_{entity_id}")
        armor_card.setFrame(-self.bar_width/2, self.bar_width/2,
                           0, self.bar_height)
        armor_node = bar_root.attachNewNode(armor_card.generate())
        armor_node.setColor(0.8, 0.8, 0.2, 0.8)
        armor_node.setTransparency(True)
        armor_node.setName("armor_bar")
        
        # Create hull bar (red)
        hull_card = CardMaker(f"hull_{entity_id}")
        hull_card.setFrame(-self.bar_width/2, self.bar_width/2,
                          -(self.bar_height + self.bar_spacing),
                          -self.bar_spacing)
        hull_node = bar_root.attachNewNode(hull_card.generate())
        hull_node.setColor(0.8, 0.3, 0.3, 0.8)
        hull_node.setTransparency(True)
        hull_node.setName("hull_bar")
        
        return bar_root
    
    def update_health_bar(self, entity_id: str, entity_data: dict):
        """
        Update or create health bar for an entity
        
        Args:
            entity_id: Entity identifier
            entity_data: Dictionary with entity data
                - position: (x, y, z) tuple
                - shield_current: Current shield HP
                - shield_max: Max shield HP
                - armor_current: Current armor HP
                - armor_max: Max armor HP
                - hull_current: Current hull HP
                - hull_max: Max hull HP
        """
        position = entity_data.get('position', (0, 0, 0))
        pos_vec = Vec3(position[0], position[1], position[2])
        
        # Create health bar if it doesn't exist
        if entity_id not in self.health_bars:
            bar_root = self._create_health_bar(entity_id, pos_vec)
            self.health_bars[entity_id] = bar_root
        else:
            bar_root = self.health_bars[entity_id]
        
        # Update position
        bar_root.setPos(pos_vec.x, pos_vec.y, pos_vec.z + self.bar_offset_z)
        
        # Update bar scales based on health percentages
        shield_current = entity_data.get('shield_current', 0)
        shield_max = entity_data.get('shield_max', 1)
        shield_percent = shield_current / shield_max if shield_max > 0 else 0
        
        armor_current = entity_data.get('armor_current', 0)
        armor_max = entity_data.get('armor_max', 1)
        armor_percent = armor_current / armor_max if armor_max > 0 else 0
        
        hull_current = entity_data.get('hull_current', 0)
        hull_max = entity_data.get('hull_max', 1)
        hull_percent = hull_current / hull_max if hull_max > 0 else 0
        
        # Find bar nodes and update their scales
        shield_bar = bar_root.find("**/shield_bar")
        if shield_bar:
            shield_bar.setScale(shield_percent, 1, 1)
        
        armor_bar = bar_root.find("**/armor_bar")
        if armor_bar:
            armor_bar.setScale(armor_percent, 1, 1)
        
        hull_bar = bar_root.find("**/hull_bar")
        if hull_bar:
            hull_bar.setScale(hull_percent, 1, 1)
    
    def remove_health_bar(self, entity_id: str):
        """Remove health bar for an entity"""
        if entity_id in self.health_bars:
            bar_root = self.health_bars[entity_id]
            bar_root.removeNode()
            del self.health_bars[entity_id]
    
    def update_all_health_bars(self, entities: Dict[str, Any]):
        """
        Update all health bars based on entity data
        
        Args:
            entities: Dictionary of entity_id -> entity object
        """
        # Remove health bars for entities that no longer exist
        current_ids = set(self.health_bars.keys())
        entity_ids = set(entities.keys())
        
        for entity_id in current_ids - entity_ids:
            self.remove_health_bar(entity_id)
        
        # Update or create health bars for all entities
        for entity_id, entity in entities.items():
            entity_data = {
                'position': entity.get_position(),
                'shield_current': getattr(entity, 'shield', 0),
                'shield_max': getattr(entity, 'max_shield', 1),
                'armor_current': getattr(entity, 'armor', 0),
                'armor_max': getattr(entity, 'max_armor', 1),
                'hull_current': getattr(entity, 'hull', 0),
                'hull_max': getattr(entity, 'max_hull', 1),
            }
            self.update_health_bar(entity_id, entity_data)
    
    def set_health_bar_visibility(self, entity_id: str, visible: bool):
        """Show or hide a specific health bar"""
        if entity_id in self.health_bars:
            if visible:
                self.health_bars[entity_id].show()
            else:
                self.health_bars[entity_id].hide()
    
    def show_all(self):
        """Show all health bars"""
        for bar in self.health_bars.values():
            bar.show()
    
    def hide_all(self):
        """Hide all health bars"""
        for bar in self.health_bars.values():
            bar.hide()
    
    def clear_all(self):
        """Remove all health bars"""
        for entity_id in list(self.health_bars.keys()):
            self.remove_health_bar(entity_id)
        print("[HealthBars] All health bars cleared")
    
    def cleanup(self):
        """Cleanup all resources"""
        self.clear_all()
        print("[HealthBars] Health bar manager cleaned up")
