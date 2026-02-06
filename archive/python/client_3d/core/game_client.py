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
from client_3d.rendering.healthbars import HealthBarManager
from client_3d.ui.hud import create_hud
from client_3d.ui.selection import SelectionSystem
from client_3d.ui.context_menu import ContextMenu


class GameClient3D(ShowBase):
    """
    Main 3D game client
    Uses Panda3D for rendering and asyncio for networking
    """
    
    # UI Constants
    CLICK_THRESHOLD = 0.02  # Threshold to distinguish click from drag
    
    # Entity collision radius by ship type (meters)
    COLLISION_RADII = {
        'default': 5.0,
        'frigate': 5.0,
        'destroyer': 6.0,
        'cruiser': 8.0,
        'battlecruiser': 10.0,
        'battleship': 12.0,
        'barge': 10.0,
    }
    
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
        
        # Initialize health bars
        self.health_bars = HealthBarManager(self.render)
        
        # Initialize star field
        self.star_field = StarField(self.render, self.camera)
        self.star_field.create(num_stars=1500)
        
        # Initialize camera
        self.camera_system = CameraSystem(self.camera, self.render)
        
        # Initialize HUD (EVE-styled by default)
        self.hud = create_hud(self.aspect2d, self.render2d, style='eve')
        
        # Initialize selection system
        self.selection = SelectionSystem(self.camera, self.render, self.mouseWatcherNode)
        
        # Initialize context menu
        self.context_menu = ContextMenu(self.aspect2d)
        
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
        self.accept("h", self.toggle_hud)
        self.accept("b", self.toggle_health_bars)
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
        print("\nEntity Interaction:")
        print("  Left Click       - Select entity")
        print("  Right Click      - Context menu (Approach, Orbit, etc.)")
        print("  Click in Space   - Deselect / Space menu")
        print("\nTest Commands:")
        print("  Space            - Test weapon fire effect")
        print("\nUtility:")
        print("  H                - Toggle HUD visibility")
        print("  B                - Toggle health bars visibility")
        print("  ESC              - Quit")
        print("="*60 + "\n")
        
    def on_mouse_down(self, button):
        """Handle mouse button press"""
        self.mouse_down[button] = True
        if self.mouseWatcherNode.hasMouse():
            self.last_mouse_x = self.mouseWatcherNode.getMouseX()
            self.last_mouse_y = self.mouseWatcherNode.getMouseY()
            
            # Store mouse position for click detection (not drag)
            if button == 0:  # Left click
                self.left_click_start_x = self.last_mouse_x
                self.left_click_start_y = self.last_mouse_y
            elif button == 1:  # Right click
                self.right_click_start_x = self.last_mouse_x
                self.right_click_start_y = self.last_mouse_y
    
    def on_mouse_up(self, button):
        """Handle mouse button release"""
        self.mouse_down[button] = False
        
        # Handle click (not drag)
        if self.mouseWatcherNode.hasMouse():
            mouse_x = self.mouseWatcherNode.getMouseX()
            mouse_y = self.mouseWatcherNode.getMouseY()
            
            # Check if mouse moved significantly (drag vs click)
            if button == 0:  # Left click
                dx = abs(mouse_x - getattr(self, 'left_click_start_x', mouse_x))
                dy = abs(mouse_y - getattr(self, 'left_click_start_y', mouse_y))
                if dx < self.CLICK_THRESHOLD and dy < self.CLICK_THRESHOLD:
                    self._handle_left_click(mouse_x, mouse_y)
                    
            elif button == 1:  # Right click
                dx = abs(mouse_x - getattr(self, 'right_click_start_x', mouse_x))
                dy = abs(mouse_y - getattr(self, 'right_click_start_y', mouse_y))
                if dx < self.CLICK_THRESHOLD and dy < self.CLICK_THRESHOLD:
                    self._handle_right_click(mouse_x, mouse_y)
    
    def on_mouse_wheel(self, direction):
        """Handle mouse wheel"""
        self.camera_system.zoom(direction)
    
    def _handle_left_click(self, mouse_x, mouse_y):
        """Handle left click - entity selection"""
        entity_id = self.selection.pick_entity(mouse_x, mouse_y)
        if entity_id:
            print(f"[GameClient3D] Selected entity: {entity_id}")
            # Update HUD with selected entity info
            entity = self.entities.get_entity(entity_id)
            if entity:
                target_data = {
                    'name': getattr(entity, 'ship_type', 'Unknown'),
                    'distance': 0,  # Will be calculated
                    'shield_current': getattr(entity, 'shield', 0),
                    'shield_max': getattr(entity, 'max_shield', 1),
                    'armor_current': getattr(entity, 'armor', 0),
                    'armor_max': getattr(entity, 'max_armor', 1),
                    'hull_current': getattr(entity, 'hull', 0),
                    'hull_max': getattr(entity, 'max_hull', 1),
                }
                # Calculate distance to player
                player_entity = self.entities.get_player_entity()
                if player_entity:
                    player_pos = Vec3(*player_entity.get_position())
                    target_pos = Vec3(*entity.get_position())
                    distance = (target_pos - player_pos).length()
                    target_data['distance'] = distance
                
                self.hud.update_target_info(target_data)
        else:
            # Clicked on empty space - deselect
            self.selection.deselect()
            print(f"[GameClient3D] Deselected entity")
    
    def _handle_right_click(self, mouse_x, mouse_y):
        """Handle right click - context menu"""
        entity_id = self.selection.pick_entity(mouse_x, mouse_y)
        
        # Convert mouse coords to screen coords for menu positioning
        # Panda3D mouse coords are -1 to 1, need to convert to aspect2d coords
        menu_x = mouse_x
        menu_y = mouse_y
        
        if entity_id:
            # Show entity context menu
            entity = self.entities.get_entity(entity_id)
            entity_name = getattr(entity, 'ship_type', 'Unknown')
            self.context_menu.show_entity_menu(entity_id, entity_name, menu_x, menu_y, 
                                               self._on_menu_action)
            print(f"[GameClient3D] Showing context menu for: {entity_id}")
        else:
            # Show space context menu
            self.context_menu.show_space_menu(menu_x, menu_y, self._on_menu_action)
            print(f"[GameClient3D] Showing space context menu")
    
    def _on_menu_action(self, action, entity_id=None, **kwargs):
        """Handle context menu action"""
        print(f"[GameClient3D] Menu action: {action} on entity: {entity_id}, kwargs: {kwargs}")
        
        # Handle different actions
        if action == "approach":
            self._send_command("approach", target=entity_id)
        elif action == "orbit":
            distance = kwargs.get('distance', 5000)
            self._send_command("orbit", target=entity_id, distance=distance)
        elif action == "keep_at_range":
            distance = kwargs.get('distance', 10000)
            self._send_command("keep_at_range", target=entity_id, distance=distance)
        elif action == "lock":
            self._send_command("lock_target", target=entity_id)
        elif action == "warp_to":
            self._send_command("warp_to", target=entity_id)
        elif action == "look_at":
            # Local camera action
            entity = self.entities.get_entity(entity_id)
            if entity:
                self.camera_system.look_at(Vec3(*entity.get_position()))
        elif action == "navigate_to":
            # Navigate to clicked position in space
            # For now, just log it
            print(f"[GameClient3D] Navigate to position (not implemented)")
        
        # Add combat log message
        self.hud.add_combat_message(f"Command: {action}", Vec4(0.7, 0.7, 1, 1))
    
    def _send_command(self, command, **kwargs):
        """Send command to server"""
        if self.network.connected:
            message = {
                'type': command,
                'data': kwargs
            }
            asyncio.create_task(self.network.send_message(message))
            print(f"[GameClient3D] Sent command: {command} with data: {kwargs}")
        else:
            print(f"[GameClient3D] Not connected to server, cannot send command: {command}")
    
    def on_key_press(self, key):
        """Handle key press"""
        print(f"[Input] Key pressed: {key}")
        
        # Only handle test commands for now
        # Real EVE-style commands will come from UI (right-click menus, buttons, etc.)
        if key == "fire":
            # Test weapon fire effect (not a real game command)
            self._create_test_weapon_effect()
        
    def toggle_hud(self):
        """Toggle HUD visibility"""
        self.hud.toggle_visibility()
        print("[GameClient3D] HUD visibility toggled")
    
    def toggle_health_bars(self):
        """Toggle health bars visibility"""
        # Check if any bars are visible
        if self.health_bars.health_bars:
            first_bar = next(iter(self.health_bars.health_bars.values()))
            if first_bar.isHidden():
                self.health_bars.show_all()
                print("[GameClient3D] Health bars shown")
            else:
                self.health_bars.hide_all()
                print("[GameClient3D] Health bars hidden")
        else:
            print("[GameClient3D] No health bars to toggle")
        
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
        
        # Update HUD with player entity data
        self._update_hud_from_entities()
        
    def on_spawn_entity(self, message):
        """Handle entity spawn"""
        print(f"[GameClient3D] Entity spawned: {message['data']}")
        
    def on_destroy_entity(self, message):
        """Handle entity destruction"""
        entity_id = message['data'].get('entity_id')
        if entity_id:
            # Get entity position before destroying for explosion effect
            entity = self.entities.get_entity(entity_id)
            if entity:
                pos = Vec3(*entity.get_position())
                # Create explosion effect at entity position
                self.effects.create_explosion_effect(pos, size=5.0)
                print(f"[GameClient3D] Entity destroyed with explosion: {entity_id}")
            
            # Remove entity visuals
            self.entity_renderer.remove_entity(entity_id)
            self.health_bars.remove_health_bar(entity_id)
    
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
            
            # Check if shield hit or armor/hull hit for appropriate effect
            target_shield = getattr(target, 'shield', 0)
            if target_shield > 0:
                # Shield hit - create shield effect
                self.effects.create_shield_hit_effect(target_pos)
            
            # Add combat message to HUD
            damage = data.get('damage', 0)
            if target_id == self.player_id:
                self.hud.add_combat_message(f"Taking {damage:.0f} damage!", Vec4(1, 0.3, 0.3, 1))
            else:
                self.hud.add_combat_message(f"Hit for {damage:.0f} damage", Vec4(0.3, 1, 0.3, 1))
            
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
            self.hud.add_combat_message("Test weapon fired!", Vec4(1, 0.8, 0.3, 1))
            print(f"[GameClient3D] Test weapon effect created")
    
    def _update_hud_from_entities(self):
        """Update HUD with current entity data"""
        # Get player entity
        player_entity = self.entities.get_player_entity()
        if player_entity:
            # Extract ship data
            ship_data = {
                'name': getattr(player_entity, 'ship_type', 'Unknown'),
                'shield_current': getattr(player_entity, 'shield', 0),
                'shield_max': getattr(player_entity, 'max_shield', 1),
                'armor_current': getattr(player_entity, 'armor', 0),
                'armor_max': getattr(player_entity, 'max_armor', 1),
                'hull_current': getattr(player_entity, 'hull', 0),
                'hull_max': getattr(player_entity, 'max_hull', 1),
            }
            self.hud.update_ship_status(ship_data)
            
            # Update speed (calculate from velocity if available)
            velocity = getattr(player_entity, 'velocity', (0, 0, 0))
            speed = (velocity[0]**2 + velocity[1]**2 + velocity[2]**2) ** 0.5
            position = Vec3(*player_entity.get_position())
            self.hud.update_speed(speed, position)
    
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
        for entity_id, entity in self.entities.get_all_entities().items():
            node = self.entity_renderer.render_entity(entity)
            # Register entity with selection system
            if node:
                # Determine collision radius based on ship type
                ship_type = getattr(entity, 'ship_type', '').lower()
                radius = self.COLLISION_RADII['default']
                
                # Check for ship class keywords
                for ship_class, ship_radius in self.COLLISION_RADII.items():
                    if ship_class in ship_type:
                        radius = ship_radius
                        break
                
                # Add to selection system (it handles duplicate checking)
                self.selection.add_pickable_entity(entity_id, node, radius)
        
        # Update health bars
        self.health_bars.update_all_health_bars(self.entities.get_all_entities())
        
        # Update HUD
        self._update_hud_from_entities()
        
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
        
        # Hide context menu
        self.context_menu.hide()
        
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
