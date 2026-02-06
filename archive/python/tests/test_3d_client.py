#!/usr/bin/env python3
"""
Standalone 3D Client Test
Tests the 3D client without requiring server connection

This creates a simple demo scene with a few mock entities to verify:
- Panda3D initialization
- Camera controls
- Star field rendering  
- Entity rendering (placeholders)
"""

import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

try:
    from direct.showbase.ShowBase import ShowBase
    from panda3d.core import Vec3, loadPrcFileData
    PANDA3D_AVAILABLE = True
except ImportError:
    print("ERROR: Panda3D not installed. Run: pip install panda3d")
    sys.exit(1)

from client_3d.rendering.camera import CameraSystem
from client_3d.rendering.starfield import StarField
from client_3d.rendering.renderer import EntityRenderer
from client_3d.core.entity_manager import Entity


class TestClient3D(ShowBase):
    """Standalone test client"""
    
    def __init__(self):
        # Configure Panda3D
        loadPrcFileData("", "window-title EVE OFFLINE - 3D Test")
        loadPrcFileData("", "win-size 1280 720")
        loadPrcFileData("", "show-frame-rate-meter #t")
        
        ShowBase.__init__(self)
        
        # Background
        self.setBackgroundColor(0.0, 0.0, 0.02, 1)
        
        # Create systems
        self.camera_system = CameraSystem(self.camera, self.render)
        self.star_field = StarField(self.render, self.camera)
        self.entity_renderer = EntityRenderer(self.render, self.loader)
        
        # Create star field
        self.star_field.create(num_stars=1500)
        
        # Create test entities
        self._create_test_entities()
        
        # Setup input
        self.accept("escape", sys.exit)
        self.accept("r", self.camera_system.reset)
        self.accept("f", self.camera_system.toggle_mode)
        self.accept("wheel_up", self.on_zoom, [1])
        self.accept("wheel_down", self.on_zoom, [-1])
        
        # Mouse
        self.mouse_down = False
        self.last_mouse_x = 0
        self.last_mouse_y = 0
        self.accept("mouse1", self.on_mouse_down)
        self.accept("mouse1-up", self.on_mouse_up)
        
        # Update task
        self.taskMgr.add(self.update, "update")
        
        # Print instructions
        print("\n" + "="*60)
        print("EVE OFFLINE - 3D Client Test")
        print("="*60)
        print("Controls:")
        print("  Left Mouse Drag  - Rotate camera")
        print("  Mouse Wheel      - Zoom in/out")
        print("  F                - Toggle camera mode")
        print("  R                - Reset camera")
        print("  ESC              - Exit")
        print("="*60)
        print("\nTest entities:")
        print("  - Rifter (Minmatar Frigate) at (0, 0, 0)")
        print("  - Merlin (Caldari Frigate) at (50, 50, 0)")
        print("  - Catalyst (Gallente Destroyer) at (-50, 50, 0)")
        print("  - Serpentis Scout (NPC) at (0, 100, 0)")
        print("="*60 + "\n")
        
    def _create_test_entities(self):
        """Create test entities for visualization"""
        # Create mock entity data
        entities = [
            {
                'id': 'test_1',
                'position': {'x': 0, 'y': 0, 'z': 0},
                'velocity': {'x': 0, 'y': 0, 'z': 0},
                'health': {'shield': 500, 'armor': 400, 'hull': 300},
                'ship': {'type': 'Rifter', 'faction': 'Minmatar'},
            },
            {
                'id': 'test_2',
                'position': {'x': 50, 'y': 50, 'z': 0},
                'velocity': {'x': 0, 'y': 0, 'z': 0},
                'health': {'shield': 600, 'armor': 500, 'hull': 400},
                'ship': {'type': 'Merlin', 'faction': 'Caldari'},
            },
            {
                'id': 'test_3',
                'position': {'x': -50, 'y': 50, 'z': 0},
                'velocity': {'x': 0, 'y': 0, 'z': 0},
                'health': {'shield': 700, 'armor': 600, 'hull': 500},
                'ship': {'type': 'Catalyst', 'faction': 'Gallente'},
            },
            {
                'id': 'test_4',
                'position': {'x': 0, 'y': 100, 'z': 0},
                'velocity': {'x': 0, 'y': 0, 'z': 0},
                'health': {'shield': 300, 'armor': 200, 'hull': 150},
                'ship': {'type': 'Serpentis Scout', 'faction': 'Serpentis'},
            },
        ]
        
        self.entities = {}
        for entity_data in entities:
            entity = Entity(entity_data['id'], entity_data)
            self.entities[entity['id']] = entity
            self.entity_renderer.render_entity(entity)
        
        # Set camera to look at center
        self.camera_system.set_target(Vec3(0, 0, 0))
        
    def on_mouse_down(self):
        """Handle mouse press"""
        self.mouse_down = True
        if self.mouseWatcherNode.hasMouse():
            self.last_mouse_x = self.mouseWatcherNode.getMouseX()
            self.last_mouse_y = self.mouseWatcherNode.getMouseY()
    
    def on_mouse_up(self):
        """Handle mouse release"""
        self.mouse_down = False
    
    def on_zoom(self, direction):
        """Handle zoom"""
        self.camera_system.zoom(direction)
    
    def update(self, task):
        """Update loop"""
        dt = globalClock.getDt()
        
        # Update camera
        self.camera_system.update(dt)
        
        # Update star field
        self.star_field.update(self.camera.getPos())
        
        # Handle mouse drag
        if self.mouse_down and self.mouseWatcherNode.hasMouse():
            mouse_x = self.mouseWatcherNode.getMouseX()
            mouse_y = self.mouseWatcherNode.getMouseY()
            
            dx = mouse_x - self.last_mouse_x
            dy = mouse_y - self.last_mouse_y
            
            self.camera_system.rotate(dx * 100, dy * 100)
            
            self.last_mouse_x = mouse_x
            self.last_mouse_y = mouse_y
        
        return task.cont


if __name__ == "__main__":
    print("Starting 3D Client Test...")
    app = TestClient3D()
    app.run()
