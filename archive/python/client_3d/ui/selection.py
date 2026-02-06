"""
Entity Selection System for 3D Client
Handles mouse picking, selection state, and visual feedback
"""

from panda3d.core import CollisionTraverser, CollisionHandlerQueue
from panda3d.core import CollisionRay, CollisionNode, CollisionSphere
from panda3d.core import GeomNode, NodePath, Vec3, Vec4, Point3
from typing import Optional


class SelectionSystem:
    """
    Manages entity selection via mouse picking
    Uses Panda3D collision system for ray casting
    """
    
    def __init__(self, camera, render, mouse_watcher):
        self.camera = camera
        self.render = render
        self.mouse_watcher = mouse_watcher
        
        # Selection state
        self.selected_entity_id: Optional[str] = None
        self.selected_node: Optional[NodePath] = None
        
        # Collision system for picking
        self.picker_traverser = CollisionTraverser()
        self.picker_queue = CollisionHandlerQueue()
        
        # Create picker ray
        self.picker_node = CollisionNode('mouseRay')
        self.picker_np = self.camera.attachNewNode(self.picker_node)
        self.picker_node.setFromCollideMask(GeomNode.getDefaultCollideMask())
        self.picker_ray = CollisionRay()
        self.picker_node.addSolid(self.picker_ray)
        
        self.picker_traverser.addCollider(self.picker_np, self.picker_queue)
        
        # Selection visual feedback
        self.selection_highlight = None
        
        # Track registered entities to avoid duplicates
        self.registered_entities = set()
        
        print("[Selection] Selection system initialized")
    
    def add_pickable_entity(self, entity_id: str, node: NodePath, radius: float = 5.0):
        """
        Add collision sphere to an entity node to make it pickable
        
        Args:
            entity_id: Entity identifier
            node: Entity's NodePath
            radius: Collision sphere radius
        """
        # Skip if already registered
        if entity_id in self.registered_entities:
            return
        
        # Create collision sphere for this entity
        collision_sphere = CollisionSphere(0, 0, 0, radius)
        collision_node = CollisionNode(f'entity_{entity_id}')
        collision_node.addSolid(collision_sphere)
        collision_node.setIntoCollideMask(GeomNode.getDefaultCollideMask())
        
        # Attach collision node to entity
        collision_np = node.attachNewNode(collision_node)
        
        # Store entity ID in the node's tag for retrieval
        collision_np.setTag('entity_id', entity_id)
        
        # Track registered entity
        self.registered_entities.add(entity_id)
    
    def pick_entity(self, mouse_x: float, mouse_y: float) -> Optional[str]:
        """
        Perform ray casting to pick entity at mouse position
        
        Args:
            mouse_x: Mouse X coordinate (-1 to 1)
            mouse_y: Mouse Y coordinate (-1 to 1)
            
        Returns:
            Entity ID if entity picked, None otherwise
        """
        # Set up collision ray from camera through mouse position
        self.picker_ray.setFromLens(self.camera.node(), mouse_x, mouse_y)
        
        # Perform collision traversal
        self.picker_traverser.traverse(self.render)
        
        # Check if we hit anything
        if self.picker_queue.getNumEntries() > 0:
            # Sort entries by distance
            self.picker_queue.sortEntries()
            
            # Get closest entry
            entry = self.picker_queue.getEntry(0)
            picked_node = entry.getIntoNodePath()
            
            # Get entity ID from tag
            if picked_node.hasTag('entity_id'):
                entity_id = picked_node.getTag('entity_id')
                return entity_id
        
        return None
    
    def select_entity(self, entity_id: Optional[str], entity_node: Optional[NodePath] = None):
        """
        Select an entity and provide visual feedback
        
        Args:
            entity_id: Entity ID to select (None to deselect)
            entity_node: Entity's NodePath for visual highlight
        """
        # Clear previous selection highlight
        if self.selection_highlight:
            self.selection_highlight.removeNode()
            self.selection_highlight = None
        
        # Update selection state
        self.selected_entity_id = entity_id
        self.selected_node = entity_node
        
        if entity_id and entity_node:
            # Create selection highlight (golden glow outline)
            self._create_selection_highlight(entity_node)
            print(f"[Selection] Selected entity: {entity_id}")
        else:
            print(f"[Selection] Deselected entity")
    
    def _create_selection_highlight(self, entity_node: NodePath):
        """
        Create visual highlight for selected entity
        
        Args:
            entity_node: Entity's NodePath to highlight
        """
        try:
            # Create a slightly larger copy of the entity for highlight effect
            # For now, just change the color to indicate selection
            # This is a simple approach - more sophisticated methods exist
            
            # Store original color
            if not entity_node.hasTag('original_color'):
                original_color = entity_node.getColorScale()
                entity_node.setTag('original_color', f"{original_color.x},{original_color.y},{original_color.z},{original_color.w}")
            
            # Set selection color (golden highlight)
            # Use color scale to brighten the entity
            entity_node.setColorScale(1.5, 1.5, 1.0, 1.0)  # Golden tint
            
            # Future: Add actual outline effect or glow shader
            
        except Exception as e:
            print(f"[Selection] Error creating highlight: {e}")
    
    def get_selected_entity_id(self) -> Optional[str]:
        """Get currently selected entity ID"""
        return self.selected_entity_id
    
    def get_selected_node(self) -> Optional[NodePath]:
        """Get currently selected entity node"""
        return self.selected_node
    
    def clear_selection(self):
        """Clear current selection"""
        if self.selected_node and self.selected_node.hasTag('original_color'):
            # Restore original color
            color_str = self.selected_node.getTag('original_color')
            try:
                r, g, b, a = map(float, color_str.split(','))
                self.selected_node.setColorScale(r, g, b, a)
            except:
                self.selected_node.setColorScale(1, 1, 1, 1)
        
        self.select_entity(None, None)
    
    def cleanup(self):
        """Cleanup resources"""
        self.clear_selection()
        if self.picker_np:
            self.picker_np.removeNode()
        print("[Selection] Selection system cleaned up")
