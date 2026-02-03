"""
Performance Optimization System for 3D Client
Handles LOD (Level of Detail), culling, and rendering optimization
"""

from panda3d.core import Vec3, NodePath
import math


class PerformanceOptimizer:
    """
    Manages performance optimizations for 3D rendering
    
    Features:
    - Level of Detail (LOD) management
    - Frustum culling
    - Distance-based entity culling
    - Update rate throttling
    """
    
    # LOD distance thresholds (in world units)
    LOD_DISTANCES = {
        'high': 100.0,      # Full detail
        'medium': 300.0,    # Reduced detail
        'low': 600.0,       # Minimal detail
        'cull': 1000.0      # Don't render
    }
    
    # Update rate based on distance (updates per second)
    UPDATE_RATES = {
        'high': 30,         # 30 Hz
        'medium': 15,       # 15 Hz
        'low': 5,           # 5 Hz
        'cull': 0           # No updates
    }
    
    def __init__(self, camera):
        """
        Initialize the performance optimizer
        
        Args:
            camera: CameraSystem instance for position/frustum checks
        """
        self.camera = camera
        self.entity_lod_states = {}  # entity_id -> LOD level
        self.entity_update_timers = {}  # entity_id -> time since last update
        self.visible_entities = set()  # Set of currently visible entity IDs
        
        # Performance stats
        self.stats = {
            'total_entities': 0,
            'visible_entities': 0,
            'culled_entities': 0,
            'high_lod': 0,
            'medium_lod': 0,
            'low_lod': 0
        }
    
    def get_lod_level(self, distance: float) -> str:
        """
        Determine LOD level based on distance
        
        Args:
            distance: Distance from camera to entity
        
        Returns:
            LOD level: 'high', 'medium', 'low', or 'cull'
        """
        if distance < self.LOD_DISTANCES['high']:
            return 'high'
        elif distance < self.LOD_DISTANCES['medium']:
            return 'medium'
        elif distance < self.LOD_DISTANCES['low']:
            return 'low'
        else:
            return 'cull'
    
    def calculate_distance(self, entity_pos: tuple[float, float, float]) -> float:
        """
        Calculate distance between camera and entity
        
        Args:
            entity_pos: Entity position (x, y, z)
        
        Returns:
            Distance in world units
        """
        cam_pos = self.camera.get_position()
        dx = entity_pos[0] - cam_pos[0]
        dy = entity_pos[1] - cam_pos[1]
        dz = entity_pos[2] - cam_pos[2]
        
        return math.sqrt(dx*dx + dy*dy + dz*dz)
    
    def should_update_entity(self, entity_id: str, dt: float) -> bool:
        """
        Determine if entity should be updated this frame based on LOD
        
        Args:
            entity_id: Entity identifier
            dt: Delta time since last frame
        
        Returns:
            True if entity should be updated
        """
        # Get current LOD level
        lod_level = self.entity_lod_states.get(entity_id, 'high')
        
        # Culled entities don't update
        if lod_level == 'cull':
            return False
        
        # Get update rate for this LOD level
        update_rate = self.UPDATE_RATES[lod_level]
        if update_rate == 0:
            return False
        
        # Calculate update interval
        update_interval = 1.0 / update_rate
        
        # Check if enough time has passed
        timer = self.entity_update_timers.get(entity_id, 0.0)
        timer += dt
        
        if timer >= update_interval:
            self.entity_update_timers[entity_id] = 0.0
            return True
        else:
            self.entity_update_timers[entity_id] = timer
            return False
    
    def update_entity_lod(self, entity_id: str, entity_pos: tuple[float, float, float]) -> str:
        """
        Update LOD state for an entity based on distance
        
        Args:
            entity_id: Entity identifier
            entity_pos: Entity position (x, y, z)
        
        Returns:
            Current LOD level
        """
        distance = self.calculate_distance(entity_pos)
        lod_level = self.get_lod_level(distance)
        
        # Store LOD state
        old_level = self.entity_lod_states.get(entity_id, 'high')
        self.entity_lod_states[entity_id] = lod_level
        
        # Reset update timer if LOD changed
        if old_level != lod_level:
            self.entity_update_timers[entity_id] = 0.0
        
        return lod_level
    
    def is_entity_visible(self, entity_id: str, entity_pos: tuple[float, float, float]) -> bool:
        """
        Check if entity should be rendered based on LOD and culling
        
        Args:
            entity_id: Entity identifier
            entity_pos: Entity position (x, y, z)
        
        Returns:
            True if entity should be visible
        """
        # Update LOD state
        lod_level = self.update_entity_lod(entity_id, entity_pos)
        
        # Cull entities that are too far
        if lod_level == 'cull':
            return False
        
        # Could add frustum culling here in the future
        # For now, just use distance-based culling
        
        return True
    
    def apply_lod_to_node(self, node: NodePath, lod_level: str):
        """
        Apply LOD-based visual changes to a node
        
        Args:
            node: NodePath to modify
            lod_level: LOD level ('high', 'medium', 'low')
        """
        if lod_level == 'high':
            # Full detail - nothing to change
            pass
        elif lod_level == 'medium':
            # Reduced detail - could disable some child nodes
            # For now, just ensure it's visible
            pass
        elif lod_level == 'low':
            # Minimal detail - could use simplified geometry
            # For now, just ensure it's visible
            pass
    
    def update_stats(self, entities: dict):
        """
        Update performance statistics
        
        Args:
            entities: Dictionary of entity_id -> Entity
        """
        self.stats['total_entities'] = len(entities)
        self.stats['visible_entities'] = 0
        self.stats['culled_entities'] = 0
        self.stats['high_lod'] = 0
        self.stats['medium_lod'] = 0
        self.stats['low_lod'] = 0
        
        for entity_id, entity in entities.items():
            entity_pos = entity.get_position()
            lod_level = self.entity_lod_states.get(entity_id, 'high')
            
            if lod_level == 'cull':
                self.stats['culled_entities'] += 1
            else:
                self.stats['visible_entities'] += 1
                
                if lod_level == 'high':
                    self.stats['high_lod'] += 1
                elif lod_level == 'medium':
                    self.stats['medium_lod'] += 1
                elif lod_level == 'low':
                    self.stats['low_lod'] += 1
    
    def get_stats(self) -> dict:
        """Get current performance statistics"""
        return self.stats.copy()
    
    def get_performance_info(self) -> str:
        """
        Get formatted performance information string
        
        Returns:
            Formatted string with performance stats
        """
        return (
            f"Entities: {self.stats['visible_entities']}/{self.stats['total_entities']} visible | "
            f"LOD: H:{self.stats['high_lod']} M:{self.stats['medium_lod']} L:{self.stats['low_lod']} | "
            f"Culled: {self.stats['culled_entities']}"
        )
    
    def optimize_entities(self, entities: dict, entity_nodes: dict, dt: float):
        """
        Apply optimizations to all entities
        
        Args:
            entities: Dictionary of entity_id -> Entity
            entity_nodes: Dictionary of entity_id -> NodePath
            dt: Delta time since last frame
        """
        for entity_id, entity in entities.items():
            entity_pos = entity.get_position()
            
            # Check visibility and update LOD
            is_visible = self.is_entity_visible(entity_id, entity_pos)
            lod_level = self.entity_lod_states.get(entity_id, 'high')
            
            # Get entity node
            node = entity_nodes.get(entity_id)
            if node:
                if is_visible:
                    # Show node and apply LOD
                    if node.isHidden():
                        node.show()
                    self.apply_lod_to_node(node, lod_level)
                else:
                    # Hide node
                    if not node.isHidden():
                        node.hide()
        
        # Update statistics
        self.update_stats(entities)
    
    def clear(self):
        """Clear all optimization state"""
        self.entity_lod_states.clear()
        self.entity_update_timers.clear()
        self.visible_entities.clear()
        
        # Reset stats
        self.stats = {
            'total_entities': 0,
            'visible_entities': 0,
            'culled_entities': 0,
            'high_lod': 0,
            'medium_lod': 0,
            'low_lod': 0
        }
