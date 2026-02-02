"""
Main Game Client for 3D Client
Integrates networking, rendering, and game logic
"""

import asyncio
from direct.showbase.ShowBase import ShowBase
from panda3d.core import Vec3, WindowProperties
from panda3d.core import loadPrcFileData
import sys

from client_3d.core import NetworkClient, EntityManager
from client_3d.rendering.camera import CameraSystem
from client_3d.rendering.renderer import EntityRenderer
from client_3d.rendering.starfield import StarField
from client_3d.rendering.effects import EffectsManager


class GameClient3D(ShowBase):
    """
    Main 3D game client
    Uses Panda3D for rendering and asyncio for networking
    """
    
    def __init__(self, player_id: str, character_name: str, server_host: str = "localhost", server_port: int = 8765):
        # Configure Panda3D before initialization
        loadPrcFileData("", "window-title EVE OFFLINE - " + character_name)
        loadPrcFileData("", "win-size 1280 720")
        loadPrcFileData("", "sync-video #t")  # V-sync on
        loadPrcFileData("", "show-frame-rate-meter #t")  # Show FPS
        
        # Initialize Panda3D
        ShowBase.__init__(self)
        
        # Game state
        self.player_id = player_id
        self.character_name = character_name
        self.server_host = server_host
        self.server_port = server_port
        self.running = True
        
        # Setup background color (dark space)
        self.setBackgroundColor(0.0, 0.0, 0.02, 1)  # Very dark blue-black
        
        # Initialize systems
        self.network = NetworkClient(server_host, server_port)
        self.entities = EntityManager()
        self.entities.set_player_id(player_id)
        
        # Initialize rendering
        self.entity_renderer = EntityRenderer(self.render, self.loader)
        
        # Initialize effects manager
        self.effects = EffectsManager(self.render, self.loader)
        
        # Initialize star field
        self.star_field = StarField(self.render, self.camera)
        self.star_field.create(num_stars=1500)
        
        # Initialize camera
        self.camera_system = CameraSystem(self.camera, self.render)
        
        # Network task
        self.network_task = None
        
        # Setup input
        self._setup_input()
        
        # Setup tasks
        self.taskMgr.add(self.update_task, "update_task")
        
        # Print controls
        self._print_controls()
        
        print(f"[GameClient3D] Initialized for {character_name}")
        print(f"[GameClient3D] Server: {server_host}:{server_port}")
        
    def _setup_input(self):
        """Setup keyboard and mouse input"""
        # Keyboard controls
        self.accept("escape", self.quit)
        self.accept("h", self.toggle_help)
        self.accept("f", self.toggle_follow_camera)
        self.accept("r", self.camera_system.reset)
        
        # Mouse controls for camera (EVE-style)
        self.accept("mouse1", self.on_mouse_down, [0])  # Left click
        self.accept("mouse1-up", self.on_mouse_up, [0])
        self.accept("mouse2", self.on_mouse_down, [1])  # Right click (future: context menu)
        self.accept("mouse2-up", self.on_mouse_up, [1])
        self.accept("mouse3", self.on_mouse_down, [2])  # Middle click
        self.accept("mouse3-up", self.on_mouse_up, [2])
        self.accept("wheel_up", self.on_mouse_wheel, [1])
        self.accept("wheel_down", self.on_mouse_wheel, [-1])
        
        # Tactical keys (for testing effects)
        self.accept("space", self.on_key_press, ["fire"])  # Test weapon fire effect
        
        # Mouse state
        self.mouse_down = [False, False, False]
        self.last_mouse_x = 0
        self.last_mouse_y = 0
        
    def _print_controls(self):
        """Print control instructions"""
        print("\n" + "="*60)
        print("EVE OFFLINE - 3D Client Controls (EVE-Style)")
        print("="*60)
        print("Camera (EVE-Style):")
        print("  Left Mouse Drag  - Rotate camera around target")
        print("  Mouse Wheel      - Zoom in/out")
        print("  Middle Mouse     - Pan camera")
        print("  F                - Toggle camera follow mode")
        print("  R                - Reset camera")
        print("\nTactical Commands (Future - via UI):")
        print("  Right Click      - Context menu (planned)")
        print("  Click in Space   - Navigate to (planned)")
        print("  Target Entity    - Approach/Orbit/Keep at Range (planned)")
        print("\nTest Commands:")
        print("  Space            - Test weapon fire effect")
        print("\nUtility:")
        print("  H                - Toggle help")
        print("  ESC              - Quit")
        print("="*60 + "\n")
        
    def on_mouse_down(self, button):
        """Handle mouse button press"""
        self.mouse_down[button] = True
        if self.mouseWatcherNode.hasMouse():
            self.last_mouse_x = self.mouseWatcherNode.getMouseX()
            self.last_mouse_y = self.mouseWatcherNode.getMouseY()
    
    def on_mouse_up(self, button):
        """Handle mouse button release"""
        self.mouse_down[button] = False
    
    def on_mouse_wheel(self, direction):
        """Handle mouse wheel"""
        self.camera_system.zoom(direction)
    
    def on_key_press(self, key):
        """Handle key press"""
        print(f"[Input] Key pressed: {key}")
        
        # Only handle test commands for now
        # Real EVE-style commands will come from UI (right-click menus, buttons, etc.)
        if key == "fire":
            # Test weapon fire effect (not a real game command)
            self._create_test_weapon_effect()
        
    def toggle_help(self):
        """Toggle help display"""
        print("[GameClient3D] Help toggle (UI not implemented yet)")
        
    def toggle_follow_camera(self):
        """Toggle camera follow mode"""
        self.camera_system.toggle_mode()
        
    def quit(self):
        """Quit the game"""
        print("[GameClient3D] Shutting down...")
        self.running = False
        self.userExit()
        
    async def connect_to_server(self):
        """Connect to game server"""
        print(f"[GameClient3D] Connecting to server...")
        success = await self.network.connect(self.player_id, self.character_name)
        
        if success:
            print(f"[GameClient3D] Connected successfully!")
            
            # Register message handlers
            self.network.register_handler('state_update', self.on_state_update)
            self.network.register_handler('spawn_entity', self.on_spawn_entity)
            self.network.register_handler('destroy_entity', self.on_destroy_entity)
            self.network.register_handler('damage', self.on_damage)
            self.network.register_handler('input_fire', self.on_weapon_fire)
            
            # Start receive loop
            self.network_task = asyncio.create_task(self.network.receive_loop())
            
            return True
        else:
            print(f"[GameClient3D] Connection failed!")
            return False
    
    def on_state_update(self, message):
        """Handle state update from server"""
        self.entities.update_from_state(message['data'])
        
    def on_spawn_entity(self, message):
        """Handle entity spawn"""
        print(f"[GameClient3D] Entity spawned: {message['data']}")
        
    def on_destroy_entity(self, message):
        """Handle entity destruction"""
        entity_id = message['data'].get('entity_id')
        if entity_id:
            self.entity_renderer.remove_entity(entity_id)
            print(f"[GameClient3D] Entity destroyed: {entity_id}")
    
    def on_damage(self, message):
        """Handle damage event - create visual effects"""
        data = message['data']
        shooter_id = data.get('shooter_id')
        target_id = data.get('target_id')
        
        # Get entity positions
        shooter = self.entities.get_entity(shooter_id)
        target = self.entities.get_entity(target_id)
        
        if shooter and target:
            shooter_pos = Vec3(*shooter.get_position())
            target_pos = Vec3(*target.get_position())
            
            # Create weapon fire effect
            weapon_type = data.get('weapon_type', 'laser')
            self.effects.create_weapon_fire_effect(shooter_pos, target_pos, weapon_type)
            print(f"[GameClient3D] Damage effect: {shooter_id} -> {target_id}")
    
    def on_weapon_fire(self, message):
        """Handle weapon fire event"""
        data = message['data']
        shooter_id = data.get('shooter_id')
        target_id = data.get('target_id')
        
        if shooter_id and target_id:
            self.on_damage(message)
    
    def _create_test_weapon_effect(self):
        """Create a test weapon effect for immediate visual feedback"""
        # Get player entity
        player_entity = self.entities.get_player_entity()
        if player_entity:
            player_pos = Vec3(*player_entity.get_position())
            
            # Create effect shooting forward
            target_pos = player_pos + Vec3(0, 50, 0)
            self.effects.create_weapon_fire_effect(player_pos, target_pos, "laser")
            print(f"[GameClient3D] Test weapon effect created")
    
    def update_task(self, task):
        """Main update task (called every frame)"""
        if not self.running:
            return task.done
        
        # Get delta time
        dt = globalClock.getDt()
        
        # Update entity interpolation
        self.entities.update_interpolation(dt)
        
        # Update camera
        # Set camera target to player entity if in follow mode
        if self.camera_system.mode == CameraSystem.MODE_FOLLOW:
            player_entity = self.entities.get_player_entity()
            if player_entity:
                self.camera_system.set_target_entity(player_entity)
        
        self.camera_system.update(dt)
        
        # Update star field to follow camera
        self.star_field.update(self.camera.getPos())
        
        # Handle mouse input for camera
        if self.mouseWatcherNode.hasMouse():
            mouse_x = self.mouseWatcherNode.getMouseX()
            mouse_y = self.mouseWatcherNode.getMouseY()
            
            if self.mouse_down[0]:  # Left mouse - rotate
                dx = mouse_x - self.last_mouse_x
                dy = mouse_y - self.last_mouse_y
                self.camera_system.rotate(dx * 100, dy * 100)
                
            elif self.mouse_down[2]:  # Middle mouse - pan
                dx = mouse_x - self.last_mouse_x
                dy = mouse_y - self.last_mouse_y
                self.camera_system.pan(dx * 100, dy * 100)
                
            self.last_mouse_x = mouse_x
            self.last_mouse_y = mouse_y
        
        # Update renderer
        self.entity_renderer.update_entities(self.entities.get_all_entities())
        
        return task.cont
    
    async def run_async(self):
        """Async run method"""
        # Connect to server
        connected = await self.connect_to_server()
        if not connected:
            print("[GameClient3D] Failed to connect to server. Exiting.")
            self.quit()
            return
        
        # Panda3D main loop will handle the rest
        print("[GameClient3D] Running...")
        
    def cleanup(self):
        """Cleanup on exit"""
        print("[GameClient3D] Cleaning up...")
        
        # Disconnect from server
        if self.network.connected:
            # Run disconnect in event loop
            try:
                loop = asyncio.get_event_loop()
                if loop.is_running():
                    asyncio.create_task(self.network.disconnect())
                else:
                    loop.run_until_complete(self.network.disconnect())
            except Exception as e:
                print(f"[GameClient3D] Error during disconnect: {e}")
        
        # Clear entities
        self.entity_renderer.clear()
        
        print("[GameClient3D] Cleanup complete")
