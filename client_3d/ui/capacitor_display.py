"""
EVE-Style Capacitor Display
Circular energy gauge mimicking EVE Online's capacitor ring
"""

from direct.gui.DirectGui import DirectFrame
from panda3d.core import (
    GeomNode, GeomVertexFormat, GeomVertexData, GeomVertexWriter,
    Geom, GeomTriangles, GeomLines, NodePath, TransparencyAttrib
)
from panda3d.core import Vec3, Vec4
import math
from typing import Optional

from .eve_style import EVEColorScheme, get_capacitor_color


class CapacitorDisplay:
    """
    EVE-style circular capacitor gauge
    Shows energy level as a circular ring around the ship
    """
    
    def __init__(self, parent, pos=(0, 0, -0.6), radius=0.15, thickness=0.03, segments=60):
        """
        Initialize capacitor display
        
        Args:
            parent: Parent node (usually aspect2d)
            pos: Position tuple (x, y, z) in normalized coordinates
            radius: Radius of the capacitor ring
            thickness: Thickness of the ring
            segments: Number of segments for smooth circle
        """
        self.parent = parent
        self.pos = pos
        self.radius = radius
        self.thickness = thickness
        self.segments = segments
        
        # Current values
        self.current_cap = 100.0
        self.max_cap = 100.0
        self.percentage = 1.0
        
        # Create the visual elements
        self.cap_node = None
        self.background_ring = None
        self.foreground_ring = None
        
        self._create_capacitor_ring()
        
        print("[Capacitor] EVE-style capacitor display created")
    
    def _create_capacitor_ring(self):
        """Create the capacitor ring geometry"""
        # Create a node to hold everything
        self.cap_node = self.parent.attachNewNode("capacitor_display")
        self.cap_node.setPos(self.pos[0], self.pos[1], self.pos[2])
        
        # Create background ring (darker, shows full circle)
        self.background_ring = self._create_ring_geometry(
            "cap_background",
            self.radius,
            self.thickness,
            self.segments,
            Vec4(0.15, 0.15, 0.15, 0.5),  # Dark gray
            0, 360  # Full circle
        )
        self.background_ring.reparentTo(self.cap_node)
        
        # Create foreground ring (colored, shows current capacitor)
        self.foreground_ring = self._create_ring_geometry(
            "cap_foreground",
            self.radius,
            self.thickness,
            self.segments,
            EVEColorScheme.CAPACITOR_FULL,
            0, 360  # Will be updated based on percentage
        )
        self.foreground_ring.reparentTo(self.cap_node)
        
        # Add transparency
        self.cap_node.setTransparency(TransparencyAttrib.MAlpha)
    
    def _create_ring_geometry(self, name: str, radius: float, thickness: float,
                             segments: int, color: Vec4, start_angle: float,
                             end_angle: float) -> NodePath:
        """
        Create a circular ring segment
        
        Args:
            name: Name for the geometry node
            radius: Outer radius
            thickness: Ring thickness
            segments: Number of segments
            color: Ring color
            start_angle: Start angle in degrees (0 = top, clockwise)
            end_angle: End angle in degrees
            
        Returns:
            NodePath containing the ring geometry
        """
        # Create vertex data
        vformat = GeomVertexFormat.getV3c4()
        vdata = GeomVertexData(name, vformat, Geom.UHDynamic)
        
        vertex = GeomVertexWriter(vdata, 'vertex')
        color_writer = GeomVertexWriter(vdata, 'color')
        
        # Calculate inner and outer radius
        outer_r = radius
        inner_r = radius - thickness
        
        # Convert angles to radians and adjust
        # EVE capacitor starts at top (90°) and goes clockwise
        angle_start = math.radians(90 - start_angle)
        angle_end = math.radians(90 - end_angle)
        angle_range = angle_end - angle_start
        
        # Create vertices for ring segments
        for i in range(segments + 1):
            t = i / segments
            angle = angle_start + angle_range * t
            
            # Outer vertex
            x_outer = outer_r * math.cos(angle)
            z_outer = outer_r * math.sin(angle)
            vertex.addData3(x_outer, 0, z_outer)
            color_writer.addData4(color)
            
            # Inner vertex
            x_inner = inner_r * math.cos(angle)
            z_inner = inner_r * math.sin(angle)
            vertex.addData3(x_inner, 0, z_inner)
            color_writer.addData4(color)
        
        # Create triangles
        tris = GeomTriangles(Geom.UHDynamic)
        for i in range(segments):
            # First triangle
            tris.addVertex(i * 2)
            tris.addVertex(i * 2 + 1)
            tris.addVertex(i * 2 + 2)
            
            # Second triangle
            tris.addVertex(i * 2 + 1)
            tris.addVertex(i * 2 + 3)
            tris.addVertex(i * 2 + 2)
        
        # Create geom and node
        geom = Geom(vdata)
        geom.addPrimitive(tris)
        
        node = GeomNode(name)
        node.addGeom(geom)
        
        return NodePath(node)
    
    def update(self, current: float, maximum: float):
        """
        Update capacitor display
        
        Args:
            current: Current capacitor value
            maximum: Maximum capacitor value
        """
        self.current_cap = current
        self.max_cap = maximum
        self.percentage = current / maximum if maximum > 0 else 0
        
        # Update the visual
        self._update_ring()
    
    def _update_ring(self):
        """Update the foreground ring to show current percentage"""
        # Remove old foreground ring
        if self.foreground_ring:
            self.foreground_ring.removeNode()
        
        # Get color based on percentage
        color = get_capacitor_color(self.current_cap, self.max_cap)
        
        # Create new ring with updated angle
        # EVE capacitor depletes clockwise from top
        end_angle = 360 * self.percentage
        
        self.foreground_ring = self._create_ring_geometry(
            "cap_foreground",
            self.radius,
            self.thickness,
            self.segments,
            color,
            0,
            end_angle
        )
        self.foreground_ring.reparentTo(self.cap_node)
        self.foreground_ring.setTransparency(TransparencyAttrib.MAlpha)
    
    def show(self):
        """Show the capacitor display"""
        if self.cap_node:
            self.cap_node.show()
    
    def hide(self):
        """Hide the capacitor display"""
        if self.cap_node:
            self.cap_node.hide()
    
    def destroy(self):
        """Clean up the capacitor display"""
        if self.cap_node:
            self.cap_node.removeNode()
            self.cap_node = None


class ShipHealthRings:
    """
    EVE-style concentric rings showing shield/armor/hull
    Displayed around the ship in the center of the screen
    """
    
    def __init__(self, parent, pos=(0, 0, -0.6), base_radius=0.12, ring_thickness=0.015, spacing=0.005):
        """
        Initialize health rings
        
        Args:
            parent: Parent node
            pos: Position tuple
            base_radius: Radius of innermost ring (hull)
            ring_thickness: Thickness of each ring
            spacing: Space between rings
        """
        self.parent = parent
        self.pos = pos
        self.base_radius = base_radius
        self.ring_thickness = ring_thickness
        self.spacing = spacing
        
        # Health values
        self.shield_current = 100.0
        self.shield_max = 100.0
        self.armor_current = 100.0
        self.armor_max = 100.0
        self.hull_current = 100.0
        self.hull_max = 100.0
        
        # Visual elements
        self.health_node = None
        self.shield_ring = None
        self.armor_ring = None
        self.hull_ring = None
        
        self._create_health_rings()
        
        print("[HealthRings] EVE-style health rings created")
    
    def _create_health_rings(self):
        """Create the three concentric health rings"""
        self.health_node = self.parent.attachNewNode("health_rings")
        self.health_node.setPos(self.pos[0], self.pos[1], self.pos[2])
        
        # Calculate radii for each ring (from inside out: hull, armor, shield)
        hull_radius = self.base_radius
        armor_radius = hull_radius + self.ring_thickness + self.spacing
        shield_radius = armor_radius + self.ring_thickness + self.spacing
        
        # Create background rings first (darker)
        self._create_ring_geometry("hull_bg", hull_radius, self.ring_thickness,
                                   40, Vec4(0.2, 0.1, 0.1, 0.3), 0, 360)
        self._create_ring_geometry("armor_bg", armor_radius, self.ring_thickness,
                                   40, Vec4(0.2, 0.2, 0.1, 0.3), 0, 360)
        self._create_ring_geometry("shield_bg", shield_radius, self.ring_thickness,
                                   40, Vec4(0.1, 0.1, 0.2, 0.3), 0, 360)
        
        # Create foreground rings (will update based on health)
        self.hull_ring = self._create_ring_geometry(
            "hull", hull_radius, self.ring_thickness, 40,
            EVEColorScheme.HULL_COLOR, 0, 360
        )
        self.armor_ring = self._create_ring_geometry(
            "armor", armor_radius, self.ring_thickness, 40,
            EVEColorScheme.ARMOR_COLOR, 0, 360
        )
        self.shield_ring = self._create_ring_geometry(
            "shield", shield_radius, self.ring_thickness, 40,
            EVEColorScheme.SHIELD_COLOR, 0, 360
        )
        
        self.health_node.setTransparency(TransparencyAttrib.MAlpha)
    
    def _create_ring_geometry(self, name: str, radius: float, thickness: float,
                             segments: int, color: Vec4, start_angle: float,
                             end_angle: float) -> NodePath:
        """Create a ring segment (same as capacitor)"""
        # Create vertex data
        vformat = GeomVertexFormat.getV3c4()
        vdata = GeomVertexData(name, vformat, Geom.UHDynamic)
        
        vertex = GeomVertexWriter(vdata, 'vertex')
        color_writer = GeomVertexWriter(vdata, 'color')
        
        outer_r = radius
        inner_r = radius - thickness
        
        angle_start = math.radians(90 - start_angle)
        angle_end = math.radians(90 - end_angle)
        angle_range = angle_end - angle_start
        
        for i in range(segments + 1):
            t = i / segments
            angle = angle_start + angle_range * t
            
            x_outer = outer_r * math.cos(angle)
            z_outer = outer_r * math.sin(angle)
            vertex.addData3(x_outer, 0, z_outer)
            color_writer.addData4(color)
            
            x_inner = inner_r * math.cos(angle)
            z_inner = inner_r * math.sin(angle)
            vertex.addData3(x_inner, 0, z_inner)
            color_writer.addData4(color)
        
        tris = GeomTriangles(Geom.UHDynamic)
        for i in range(segments):
            tris.addVertex(i * 2)
            tris.addVertex(i * 2 + 1)
            tris.addVertex(i * 2 + 2)
            
            tris.addVertex(i * 2 + 1)
            tris.addVertex(i * 2 + 3)
            tris.addVertex(i * 2 + 2)
        
        geom = Geom(vdata)
        geom.addPrimitive(tris)
        
        node = GeomNode(name)
        node.addGeom(geom)
        
        np = NodePath(node)
        np.reparentTo(self.health_node)
        np.setTransparency(TransparencyAttrib.MAlpha)
        
        return np
    
    def update(self, shield_cur: float, shield_max: float,
               armor_cur: float, armor_max: float,
               hull_cur: float, hull_max: float):
        """
        Update health rings
        
        Args:
            shield_cur, shield_max: Shield values
            armor_cur, armor_max: Armor values
            hull_cur, hull_max: Hull values
        """
        self.shield_current = shield_cur
        self.shield_max = shield_max
        self.armor_current = armor_cur
        self.armor_max = armor_max
        self.hull_current = hull_cur
        self.hull_max = hull_max
        
        # Update visual representation
        self._update_rings()
    
    def _update_rings(self):
        """Update all three rings based on current health"""
        # Calculate percentages
        shield_pct = self.shield_current / self.shield_max if self.shield_max > 0 else 0
        armor_pct = self.armor_current / self.armor_max if self.armor_max > 0 else 0
        hull_pct = self.hull_current / self.hull_max if self.hull_max > 0 else 0
        
        # Update shield ring
        if self.shield_ring:
            self.shield_ring.removeNode()
        hull_radius = self.base_radius
        armor_radius = hull_radius + self.ring_thickness + self.spacing
        shield_radius = armor_radius + self.ring_thickness + self.spacing
        from .eve_style import get_health_color
        shield_color = get_health_color(self.shield_current, self.shield_max, 'shield')
        self.shield_ring = self._create_ring_geometry(
            "shield", shield_radius, self.ring_thickness, 40,
            shield_color, 0, 360 * shield_pct
        )
        
        # Update armor ring
        if self.armor_ring:
            self.armor_ring.removeNode()
        armor_color = get_health_color(self.armor_current, self.armor_max, 'armor')
        self.armor_ring = self._create_ring_geometry(
            "armor", armor_radius, self.ring_thickness, 40,
            armor_color, 0, 360 * armor_pct
        )
        
        # Update hull ring
        if self.hull_ring:
            self.hull_ring.removeNode()
        hull_color = get_health_color(self.hull_current, self.hull_max, 'hull')
        self.hull_ring = self._create_ring_geometry(
            "hull", hull_radius, self.ring_thickness, 40,
            hull_color, 0, 360 * hull_pct
        )
    
    def show(self):
        """Show health rings"""
        if self.health_node:
            self.health_node.show()
    
    def hide(self):
        """Hide health rings"""
        if self.health_node:
            self.health_node.hide()
    
    def destroy(self):
        """Clean up health rings"""
        if self.health_node:
            self.health_node.removeNode()
            self.health_node = None


__all__ = ['CapacitorDisplay', 'ShipHealthRings']
