"""
Procedural Ship Model Generator
Generates 3D ship models for different ship classes and factions
"""

from panda3d.core import (
    Vec3, Vec4, Point3, NodePath,
    GeomVertexFormat, GeomVertexData, Geom, GeomNode,
    GeomTriangles, GeomVertexWriter, GeomVertexReader
)
import math


class ShipModelGenerator:
    """
    Generates procedural 3D ship models
    Creates distinct models for different ship classes and factions
    """
    
    # Faction color schemes (RGB + metallic factor)
    FACTION_COLORS = {
        'Minmatar': {
            'primary': Vec4(0.5, 0.35, 0.25, 1.0),  # Rust brown
            'secondary': Vec4(0.3, 0.2, 0.15, 1.0),  # Dark brown
            'accent': Vec4(0.8, 0.6, 0.3, 1.0)  # Light rust
        },
        'Caldari': {
            'primary': Vec4(0.35, 0.45, 0.55, 1.0),  # Steel blue
            'secondary': Vec4(0.2, 0.25, 0.35, 1.0),  # Dark blue
            'accent': Vec4(0.5, 0.7, 0.9, 1.0)  # Light blue
        },
        'Gallente': {
            'primary': Vec4(0.3, 0.4, 0.35, 1.0),  # Dark green-gray
            'secondary': Vec4(0.2, 0.3, 0.25, 1.0),  # Darker green
            'accent': Vec4(0.4, 0.7, 0.5, 1.0)  # Light green
        },
        'Amarr': {
            'primary': Vec4(0.6, 0.55, 0.45, 1.0),  # Gold-brass
            'secondary': Vec4(0.4, 0.35, 0.25, 1.0),  # Dark gold
            'accent': Vec4(0.9, 0.8, 0.5, 1.0)  # Bright gold
        },
        'Serpentis': {
            'primary': Vec4(0.4, 0.25, 0.45, 1.0),  # Purple
            'secondary': Vec4(0.2, 0.15, 0.25, 1.0),  # Dark purple
            'accent': Vec4(0.7, 0.3, 0.7, 1.0)  # Bright purple
        },
        'Guristas': {
            'primary': Vec4(0.5, 0.2, 0.2, 1.0),  # Dark red
            'secondary': Vec4(0.3, 0.1, 0.1, 1.0),  # Very dark red
            'accent': Vec4(0.9, 0.3, 0.3, 1.0)  # Bright red
        },
        'Blood Raiders': {
            'primary': Vec4(0.4, 0.15, 0.15, 1.0),  # Blood red
            'secondary': Vec4(0.2, 0.05, 0.05, 1.0),  # Almost black
            'accent': Vec4(0.8, 0.2, 0.2, 1.0)  # Crimson
        },
        'default': {
            'primary': Vec4(0.5, 0.5, 0.5, 1.0),  # Gray
            'secondary': Vec4(0.3, 0.3, 0.3, 1.0),  # Dark gray
            'accent': Vec4(0.7, 0.7, 0.7, 1.0)  # Light gray
        }
    }
    
    def __init__(self):
        self.model_cache = {}
    
    def generate_ship_model(self, ship_type: str, faction: str) -> NodePath:
        """
        Generate a procedural 3D model for a ship
        
        Args:
            ship_type: Type of ship (e.g., "Frigate", "Cruiser")
            faction: Faction name (e.g., "Caldari", "Serpentis")
        
        Returns:
            NodePath containing the ship model
        """
        # Check cache
        cache_key = f"{faction}_{ship_type}"
        if cache_key in self.model_cache:
            return self.model_cache[cache_key].copyTo(NodePath())
        
        # Determine ship class and generate appropriate model
        if self._is_frigate(ship_type):
            model = self._create_frigate_model(faction)
        elif self._is_destroyer(ship_type):
            model = self._create_destroyer_model(faction)
        elif self._is_cruiser(ship_type):
            model = self._create_cruiser_model(faction)
        else:
            model = self._create_generic_model(faction)
        
        # Cache the model
        self.model_cache[cache_key] = model
        
        # Return a copy
        return model.copyTo(NodePath())
    
    def _is_frigate(self, ship_type: str) -> bool:
        """Check if ship is a frigate class"""
        frigate_names = ['Frigate', 'Rifter', 'Merlin', 'Tristan', 'Punisher']
        return any(name in ship_type for name in frigate_names)
    
    def _is_destroyer(self, ship_type: str) -> bool:
        """Check if ship is a destroyer class"""
        destroyer_names = ['Destroyer', 'Thrasher', 'Cormorant', 'Catalyst', 'Coercer']
        return any(name in ship_type for name in destroyer_names)
    
    def _is_cruiser(self, ship_type: str) -> bool:
        """Check if ship is a cruiser class"""
        cruiser_names = ['Cruiser', 'Stabber', 'Caracal', 'Vexor', 'Maller', 'Rupture', 'Moa']
        return any(name in ship_type for name in cruiser_names)
    
    def _get_faction_colors(self, faction: str) -> dict:
        """Get color scheme for a faction"""
        return self.FACTION_COLORS.get(faction, self.FACTION_COLORS['default'])
    
    def _create_frigate_model(self, faction: str) -> NodePath:
        """
        Create a frigate-class ship model
        Small, agile ships with compact design
        """
        colors = self._get_faction_colors(faction)
        root = NodePath(f"frigate_{faction}")
        
        # Main hull - elongated wedge shape
        hull = self._create_wedge_shape(
            length=6.0, width=2.0, height=1.5,
            color=colors['primary']
        )
        hull.reparentTo(root)
        
        # Engine pods (2x rear)
        for side in [-1, 1]:
            engine = self._create_cylinder(
                radius=0.4, length=1.5,
                color=colors['secondary']
            )
            engine.setPos(side * 0.8, -2.5, 0)
            engine.setHpr(0, 0, 90)
            engine.reparentTo(root)
            
            # Engine glow
            glow = self._create_cylinder(
                radius=0.3, length=0.3,
                color=Vec4(0.3, 0.5, 1.0, 1.0)  # Blue glow
            )
            glow.setPos(side * 0.8, -3.5, 0)
            glow.setHpr(0, 0, 90)
            glow.reparentTo(root)
        
        # Cockpit/bridge
        cockpit = self._create_box(
            width=1.0, length=1.5, height=0.8,
            color=colors['accent']
        )
        cockpit.setPos(0, 1.5, 0.5)
        cockpit.reparentTo(root)
        
        return root
    
    def _create_destroyer_model(self, faction: str) -> NodePath:
        """
        Create a destroyer-class ship model
        Longer, more angular ships focused on weaponry
        """
        colors = self._get_faction_colors(faction)
        root = NodePath(f"destroyer_{faction}")
        
        # Main hull - long, angular design
        hull = self._create_wedge_shape(
            length=10.0, width=2.5, height=1.8,
            color=colors['primary']
        )
        hull.reparentTo(root)
        
        # Weapon hardpoints (multiple turrets)
        for x_pos in [-1.2, 0, 1.2]:
            turret = self._create_box(
                width=0.6, length=0.6, height=0.8,
                color=colors['secondary']
            )
            turret.setPos(x_pos, 2.0, 0.9)
            turret.reparentTo(root)
        
        # Engine section - dual engines
        for side in [-1, 1]:
            engine = self._create_cylinder(
                radius=0.6, length=2.5,
                color=colors['secondary']
            )
            engine.setPos(side * 1.0, -3.5, 0)
            engine.setHpr(0, 0, 90)
            engine.reparentTo(root)
            
            # Engine glow
            glow = self._create_cylinder(
                radius=0.5, length=0.4,
                color=Vec4(0.3, 0.5, 1.0, 1.0)
            )
            glow.setPos(side * 1.0, -5.5, 0)
            glow.setHpr(0, 0, 90)
            glow.reparentTo(root)
        
        # Spine/keel
        spine = self._create_box(
            width=0.4, length=8.0, height=0.5,
            color=colors['accent']
        )
        spine.setPos(0, 0, -0.7)
        spine.reparentTo(root)
        
        return root
    
    def _create_cruiser_model(self, faction: str) -> NodePath:
        """
        Create a cruiser-class ship model
        Larger, more imposing ships with complex geometry
        """
        colors = self._get_faction_colors(faction)
        root = NodePath(f"cruiser_{faction}")
        
        # Main hull - large central body
        hull = self._create_ellipsoid(
            length=8.0, width=4.0, height=3.0,
            color=colors['primary']
        )
        hull.reparentTo(root)
        
        # Wing structures
        for side in [-1, 1]:
            wing = self._create_wedge_shape(
                length=6.0, width=2.0, height=1.0,
                color=colors['secondary']
            )
            wing.setPos(side * 2.5, -1.0, 0)
            wing.setHpr(side * 30, 0, side * 10)
            wing.reparentTo(root)
        
        # Bridge/command section
        bridge = self._create_box(
            width=2.5, length=3.0, height=2.0,
            color=colors['accent']
        )
        bridge.setPos(0, 2.0, 1.5)
        bridge.reparentTo(root)
        
        # Engine nacelles (4x for cruiser)
        engine_positions = [(-1.5, -3.5), (-0.5, -3.5), (0.5, -3.5), (1.5, -3.5)]
        for x, y in engine_positions:
            engine = self._create_cylinder(
                radius=0.5, length=2.0,
                color=colors['secondary']
            )
            engine.setPos(x, y, 0)
            engine.setHpr(0, 0, 90)
            engine.reparentTo(root)
            
            # Engine glow
            glow = self._create_cylinder(
                radius=0.4, length=0.5,
                color=Vec4(0.3, 0.5, 1.0, 1.0)
            )
            glow.setPos(x, y - 1.5, 0)
            glow.setHpr(0, 0, 90)
            glow.reparentTo(root)
        
        return root
    
    def _create_generic_model(self, faction: str) -> NodePath:
        """Create a generic ship model as fallback"""
        return self._create_frigate_model(faction)
    
    # Primitive shape generators
    
    def _create_box(self, width: float, length: float, height: float, color: Vec4) -> NodePath:
        """Create a box-shaped geometry"""
        format = GeomVertexFormat.getV3c4()
        vdata = GeomVertexData('box', format, Geom.UHStatic)
        
        vertex = GeomVertexWriter(vdata, 'vertex')
        color_writer = GeomVertexWriter(vdata, 'color')
        
        # Define 8 vertices of a box
        hw, hl, hh = width / 2, length / 2, height / 2
        vertices = [
            (-hw, -hl, -hh), (hw, -hl, -hh), (hw, hl, -hh), (-hw, hl, -hh),  # Bottom
            (-hw, -hl, hh), (hw, -hl, hh), (hw, hl, hh), (-hw, hl, hh)       # Top
        ]
        
        for v in vertices:
            vertex.addData3(*v)
            color_writer.addData4(color)
        
        # Create triangles for all 6 faces
        tris = GeomTriangles(Geom.UHStatic)
        
        # Define faces (as triangle pairs)
        faces = [
            (0, 1, 2), (0, 2, 3),  # Bottom
            (4, 7, 6), (4, 6, 5),  # Top
            (0, 4, 5), (0, 5, 1),  # Front
            (2, 6, 7), (2, 7, 3),  # Back
            (0, 3, 7), (0, 7, 4),  # Left
            (1, 5, 6), (1, 6, 2)   # Right
        ]
        
        for face in faces:
            tris.addVertices(*face)
        
        geom = Geom(vdata)
        geom.addPrimitive(tris)
        
        node = GeomNode('box')
        node.addGeom(geom)
        
        return NodePath(node)
    
    def _create_wedge_shape(self, length: float, width: float, height: float, color: Vec4) -> NodePath:
        """Create a wedge-shaped geometry (like an arrow)"""
        format = GeomVertexFormat.getV3c4()
        vdata = GeomVertexData('wedge', format, Geom.UHStatic)
        
        vertex = GeomVertexWriter(vdata, 'vertex')
        color_writer = GeomVertexWriter(vdata, 'color')
        
        hl, hw, hh = length / 2, width / 2, height / 2
        
        # Wedge vertices
        vertices = [
            (0, hl, 0),           # Tip (front)
            (-hw, -hl, -hh),      # Back bottom left
            (hw, -hl, -hh),       # Back bottom right
            (-hw, -hl, hh),       # Back top left
            (hw, -hl, hh),        # Back top right
        ]
        
        for v in vertices:
            vertex.addData3(*v)
            color_writer.addData4(color)
        
        tris = GeomTriangles(Geom.UHStatic)
        
        # Define triangular faces
        faces = [
            (0, 2, 1),  # Bottom
            (0, 4, 2),  # Right side
            (0, 1, 3),  # Left side
            (0, 3, 4),  # Top
            (1, 2, 4),  # Back face 1
            (1, 4, 3),  # Back face 2
        ]
        
        for face in faces:
            tris.addVertices(*face)
        
        geom = Geom(vdata)
        geom.addPrimitive(tris)
        
        node = GeomNode('wedge')
        node.addGeom(geom)
        
        return NodePath(node)
    
    def _create_cylinder(self, radius: float, length: float, color: Vec4, segments: int = 12) -> NodePath:
        """Create a cylindrical geometry"""
        format = GeomVertexFormat.getV3c4()
        vdata = GeomVertexData('cylinder', format, Geom.UHStatic)
        
        vertex = GeomVertexWriter(vdata, 'vertex')
        color_writer = GeomVertexWriter(vdata, 'color')
        
        # Generate cylinder vertices (along Y axis)
        hl = length / 2
        
        # Cap centers
        vertex.addData3(0, -hl, 0)
        color_writer.addData4(color)
        vertex.addData3(0, hl, 0)
        color_writer.addData4(color)
        
        # Ring vertices
        for i in range(segments):
            angle = (i / segments) * 2 * math.pi
            x = radius * math.cos(angle)
            z = radius * math.sin(angle)
            
            vertex.addData3(x, -hl, z)
            color_writer.addData4(color)
            vertex.addData3(x, hl, z)
            color_writer.addData4(color)
        
        tris = GeomTriangles(Geom.UHStatic)
        
        # Bottom cap
        for i in range(segments):
            next_i = (i + 1) % segments
            tris.addVertices(0, 2 + i * 2, 2 + next_i * 2)
        
        # Top cap
        for i in range(segments):
            next_i = (i + 1) % segments
            tris.addVertices(1, 3 + next_i * 2, 3 + i * 2)
        
        # Side faces
        for i in range(segments):
            next_i = (i + 1) % segments
            base_1 = 2 + i * 2
            base_2 = 2 + next_i * 2
            
            tris.addVertices(base_1, base_2, base_1 + 1)
            tris.addVertices(base_2, base_2 + 1, base_1 + 1)
        
        geom = Geom(vdata)
        geom.addPrimitive(tris)
        
        node = GeomNode('cylinder')
        node.addGeom(geom)
        
        return NodePath(node)
    
    def _create_ellipsoid(self, length: float, width: float, height: float, color: Vec4, segments: int = 16) -> NodePath:
        """Create an ellipsoid (stretched sphere) geometry"""
        format = GeomVertexFormat.getV3c4()
        vdata = GeomVertexData('ellipsoid', format, Geom.UHStatic)
        
        vertex = GeomVertexWriter(vdata, 'vertex')
        color_writer = GeomVertexWriter(vdata, 'color')
        
        # Generate sphere vertices with scaling
        for lat in range(segments // 2 + 1):
            lat_angle = (lat / (segments // 2)) * math.pi - math.pi / 2
            y = length / 2 * math.sin(lat_angle)
            ring_radius = math.cos(lat_angle)
            
            for lon in range(segments):
                lon_angle = (lon / segments) * 2 * math.pi
                x = width / 2 * ring_radius * math.cos(lon_angle)
                z = height / 2 * ring_radius * math.sin(lon_angle)
                
                vertex.addData3(x, y, z)
                color_writer.addData4(color)
        
        tris = GeomTriangles(Geom.UHStatic)
        
        # Create triangles
        for lat in range(segments // 2):
            for lon in range(segments):
                next_lon = (lon + 1) % segments
                
                v1 = lat * segments + lon
                v2 = lat * segments + next_lon
                v3 = (lat + 1) * segments + lon
                v4 = (lat + 1) * segments + next_lon
                
                tris.addVertices(v1, v2, v3)
                tris.addVertices(v2, v4, v3)
        
        geom = Geom(vdata)
        geom.addPrimitive(tris)
        
        node = GeomNode('ellipsoid')
        node.addGeom(geom)
        
        return NodePath(node)
