"""
Star Field Background for 3D Client
Creates a beautiful space background with stars
"""

from panda3d.core import Vec3, Vec4, GeomNode, Geom, GeomVertexFormat
from panda3d.core import GeomVertexData, GeomVertexWriter, GeomPoints
from panda3d.core import NodePath
import random


class StarField:
    """
    Creates a star field background for space scenes
    """
    
    def __init__(self, render, camera):
        self.render = render
        self.camera = camera
        self.star_node = None
        self.num_stars = 1000
        self.star_distance = 5000  # Far away from camera
        
    def create(self, num_stars: int = 1000):
        """
        Create star field
        
        Args:
            num_stars: Number of stars to create
        """
        self.num_stars = num_stars
        
        # Create vertex data for points
        format = GeomVertexFormat.getV3c4()
        vdata = GeomVertexData('stars', format, Geom.UHStatic)
        vdata.setNumRows(num_stars)
        
        vertex = GeomVertexWriter(vdata, 'vertex')
        color = GeomVertexWriter(vdata, 'color')
        
        # Generate random star positions in a sphere around origin
        for i in range(num_stars):
            # Random position on sphere surface
            theta = random.uniform(0, 2 * 3.14159)  # 0 to 2*pi
            phi = random.uniform(-1.5708, 1.5708)   # -pi/2 to pi/2
            
            x = self.star_distance * random.uniform(0.8, 1.0) * (
                (1.0 - abs(phi) / 1.5708) ** 0.5 * 
                (1.0 if random.random() > 0.5 else -1.0) * 
                abs(random.gauss(0, 1))
            )
            y = self.star_distance * random.uniform(0.8, 1.0) * (
                (1.0 - abs(phi) / 1.5708) ** 0.5 * 
                (1.0 if random.random() > 0.5 else -1.0) * 
                abs(random.gauss(0, 1))
            )
            z = self.star_distance * random.uniform(-0.3, 0.3)
            
            vertex.addData3(x, y, z)
            
            # Random star brightness and slight color variation
            brightness = random.uniform(0.3, 1.0)
            
            # Vary color slightly (most white, some blue, very few yellow)
            rand = random.random()
            if rand < 0.85:  # 85% white stars
                r = g = b = brightness
            elif rand < 0.95:  # 10% blue stars
                r = brightness * 0.8
                g = brightness * 0.9
                b = brightness
            else:  # 5% yellow stars
                r = brightness
                g = brightness * 0.9
                b = brightness * 0.7
            
            color.addData4(r, g, b, 1.0)
        
        # Create geometry
        points = GeomPoints(Geom.UHStatic)
        points.addNextVertices(num_stars)
        
        geom = Geom(vdata)
        geom.addPrimitive(points)
        
        node = GeomNode('stars')
        node.addGeom(geom)
        
        # Create NodePath and attach to render
        self.star_node = NodePath(node)
        self.star_node.reparentTo(self.render)
        
        # Set render state for points
        self.star_node.setRenderModeThickness(2)  # Point size
        
        # Stars should not be affected by lighting
        self.star_node.setLightOff()
        
        print(f"[StarField] Created {num_stars} stars")
        
    def remove(self):
        """Remove star field from scene"""
        if self.star_node:
            self.star_node.removeNode()
            self.star_node = None
            print("[StarField] Removed star field")
    
    def update(self, camera_pos: Vec3):
        """
        Update star field position to follow camera
        (So stars always appear at infinite distance)
        
        Args:
            camera_pos: Current camera position
        """
        if self.star_node:
            # Keep stars centered on camera
            self.star_node.setPos(camera_pos)
