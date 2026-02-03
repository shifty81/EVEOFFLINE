"""
Minimap/Radar Display for 3D Client
Shows a top-down view of nearby entities
"""

from direct.gui.DirectGui import DirectFrame, DirectLabel
from panda3d.core import (
    TextNode, Vec3, Vec4, TransparencyAttrib,
    CardMaker, Geom, GeomNode, GeomVertexFormat,
    GeomVertexData, GeomVertexWriter, GeomLines, GeomPoints
)
from typing import Dict, List, Tuple, Optional
import math


class MinimapRadar:
    """
    Minimap/Radar display showing nearby entities
    Similar to EVE Online's tactical overlay
    """
    
    def __init__(self, parent, pos=(0.7, 0.5), size=0.25, max_range=500.0):
        """
        Initialize minimap radar
        
        Args:
            parent: Parent node for this UI element
            pos: (x, y) position in normalized screen coordinates
            size: Minimap size (width/height in normalized coords)
            max_range: Maximum radar range in game units
        """
        self.parent = parent
        self.pos = pos
        self.size = size
        self.max_range = max_range
        self.visible = True
        
        # Player position (center of radar)
        self.player_pos = Vec3(0, 0, 0)
        self.player_rotation = 0.0
        
        # Entities to display
        self.entities = {}  # entity_id: {pos, type, faction}
        
        # Create minimap
        self._create_minimap()
        
        print(f"[MinimapRadar] Minimap initialized (range: {max_range})")
    
    def _create_minimap(self):
        """Create minimap UI elements"""
        # Background frame
        self.frame = DirectFrame(
            frameColor=(0.02, 0.04, 0.08, 0.9),  # EVE dark blue-black
            frameSize=(-self.size/2, self.size/2, -self.size/2, self.size/2),
            pos=(self.pos[0], 0, self.pos[1]),
            parent=self.parent
        )
        self.frame.setTransparency(TransparencyAttrib.MAlpha)
        
        # Border
        border_frame = DirectFrame(
            frameColor=(0.2, 0.6, 0.8, 0.5),  # EVE teal border
            frameSize=(-self.size/2 - 0.005, self.size/2 + 0.005, 
                       -self.size/2 - 0.005, self.size/2 + 0.005),
            pos=(0, 0, 0),
            parent=self.frame
        )
        border_frame.setTransparency(TransparencyAttrib.MAlpha)
        
        # Title
        self.title_label = DirectLabel(
            text="Tactical Overlay",
            text_scale=0.03,
            text_fg=(0.9, 0.95, 1.0, 1.0),
            text_align=TextNode.ACenter,
            frameColor=(0, 0, 0, 0),
            pos=(0, 0, self.size/2 + 0.02),
            parent=self.frame
        )
        
        # Range indicator
        self.range_label = DirectLabel(
            text=f"{self.max_range:.0f} km",
            text_scale=0.025,
            text_fg=(0.7, 0.75, 0.8, 1.0),
            text_align=TextNode.ACenter,
            frameColor=(0, 0, 0, 0),
            pos=(0, 0, -self.size/2 - 0.02),
            parent=self.frame
        )
        
        # Range rings (circles at 25%, 50%, 75%, 100% range)
        self._create_range_rings()
        
        # Center crosshair (player position)
        self._create_center_crosshair()
        
        # Create node for entity markers
        self.entity_markers_node = self.frame.attachNewNode("entity_markers")
    
    def _create_range_rings(self):
        """Create range indicator rings"""
        for ring_percent in [0.25, 0.5, 0.75, 1.0]:
            ring_radius = (self.size / 2) * ring_percent * 0.95  # 0.95 to fit inside frame
            self._create_circle(ring_radius, (0.2, 0.3, 0.4, 0.3))
    
    def _create_circle(self, radius: float, color: Vec4, segments: int = 32):
        """Create a circle on the minimap"""
        # Create circle geometry
        format = GeomVertexFormat.getV3()
        vdata = GeomVertexData('circle', format, Geom.UHStatic)
        vertex = GeomVertexWriter(vdata, 'vertex')
        
        # Create circle vertices
        for i in range(segments + 1):
            angle = (i / segments) * 2 * math.pi
            x = radius * math.cos(angle)
            y = radius * math.sin(angle)
            vertex.addData3(x, 0, y)
        
        # Create line strip
        lines = GeomLines(Geom.UHStatic)
        for i in range(segments):
            lines.addVertex(i)
            lines.addVertex(i + 1)
        
        # Create geom
        geom = Geom(vdata)
        geom.addPrimitive(lines)
        
        # Create node
        node = GeomNode('circle_node')
        node.addGeom(geom)
        
        # Attach to frame
        circle_np = self.frame.attachNewNode(node)
        circle_np.setColor(color)
        circle_np.setTransparency(TransparencyAttrib.MAlpha)
        
        return circle_np
    
    def _create_center_crosshair(self):
        """Create crosshair at center (player position)"""
        # Create crosshair lines
        format = GeomVertexFormat.getV3()
        vdata = GeomVertexData('crosshair', format, Geom.UHStatic)
        vertex = GeomVertexWriter(vdata, 'vertex')
        
        crosshair_size = 0.02
        # Horizontal line
        vertex.addData3(-crosshair_size, 0, 0)
        vertex.addData3(crosshair_size, 0, 0)
        # Vertical line
        vertex.addData3(0, 0, -crosshair_size)
        vertex.addData3(0, 0, crosshair_size)
        
        # Create lines
        lines = GeomLines(Geom.UHStatic)
        lines.addVertex(0)
        lines.addVertex(1)
        lines.addVertex(2)
        lines.addVertex(3)
        
        # Create geom
        geom = Geom(vdata)
        geom.addPrimitive(lines)
        
        # Create node
        node = GeomNode('crosshair_node')
        node.addGeom(geom)
        
        # Attach to frame
        crosshair_np = self.frame.attachNewNode(node)
        crosshair_np.setColor(0.9, 0.95, 1.0, 1.0)
        
        return crosshair_np
    
    def update_player_position(self, pos: Vec3, rotation: float = 0.0):
        """Update player position (center of radar)"""
        self.player_pos = pos
        self.player_rotation = rotation
    
    def add_entity(self, entity_id: str, pos: Vec3, entity_type: str = "ship", faction: str = "neutral"):
        """
        Add or update entity on radar
        
        Args:
            entity_id: Unique entity identifier
            pos: Entity position in world space
            entity_type: Type of entity (ship, npc, structure, etc.)
            faction: Faction (friendly, hostile, neutral)
        """
        self.entities[entity_id] = {
            'pos': pos,
            'type': entity_type,
            'faction': faction
        }
    
    def remove_entity(self, entity_id: str):
        """Remove entity from radar"""
        if entity_id in self.entities:
            del self.entities[entity_id]
    
    def clear_entities(self):
        """Clear all entities from radar"""
        self.entities.clear()
    
    def update_display(self):
        """Update radar display with current entities"""
        # Clear previous markers
        self.entity_markers_node.removeNode()
        self.entity_markers_node = self.frame.attachNewNode("entity_markers")
        
        # Create markers for each entity
        for entity_id, data in self.entities.items():
            entity_pos = data['pos']
            entity_type = data['type']
            faction = data['faction']
            
            # Calculate relative position
            rel_x = entity_pos.x - self.player_pos.x
            rel_y = entity_pos.y - self.player_pos.y
            
            # Calculate distance
            distance = math.sqrt(rel_x**2 + rel_y**2)
            
            # Skip if out of range
            if distance > self.max_range:
                continue
            
            # Scale to minimap size
            scale = (self.size / 2) * 0.95 / self.max_range
            map_x = rel_x * scale
            map_y = rel_y * scale
            
            # Get marker color based on faction
            color = self._get_faction_color(faction)
            
            # Create marker
            self._create_entity_marker(map_x, map_y, color, entity_type)
    
    def _get_faction_color(self, faction: str) -> Vec4:
        """Get color for faction"""
        colors = {
            'friendly': Vec4(0.2, 0.8, 0.4, 1.0),  # Green
            'hostile': Vec4(1.0, 0.2, 0.2, 1.0),   # Red
            'neutral': Vec4(0.7, 0.7, 0.7, 1.0),   # Gray
            'ally': Vec4(0.2, 0.6, 1.0, 1.0),      # Blue
        }
        return colors.get(faction, colors['neutral'])
    
    def _create_entity_marker(self, x: float, y: float, color: Vec4, entity_type: str):
        """Create a marker for an entity"""
        # Create simple square marker
        cm = CardMaker('marker')
        marker_size = 0.01
        cm.setFrame(-marker_size, marker_size, -marker_size, marker_size)
        
        marker_np = self.entity_markers_node.attachNewNode(cm.generate())
        marker_np.setPos(x, 0, y)
        marker_np.setColor(color)
        marker_np.setTransparency(TransparencyAttrib.MAlpha)
        
        return marker_np
    
    def set_range(self, new_range: float):
        """Set new maximum range"""
        self.max_range = new_range
        self.range_label['text'] = f"{self.max_range:.0f} km"
    
    def show(self):
        """Show minimap"""
        self.frame.show()
        self.visible = True
    
    def hide(self):
        """Hide minimap"""
        self.frame.hide()
        self.visible = False
    
    def toggle(self):
        """Toggle minimap visibility"""
        if self.visible:
            self.hide()
        else:
            self.show()
    
    def destroy(self):
        """Destroy minimap and clean up"""
        if hasattr(self, 'frame'):
            self.frame.destroy()
