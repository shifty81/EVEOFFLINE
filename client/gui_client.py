"""
EVE OFFLINE - GUI Client with Pygame
Provides a graphical interface for the game
"""

import asyncio
import sys
import os
import time
from typing import Optional, Dict, List, Tuple

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

try:
    import pygame
    PYGAME_AVAILABLE = True
except ImportError:
    PYGAME_AVAILABLE = False
    print("Warning: pygame not installed. Run: pip install pygame")
    print("Falling back to text mode...")

from engine.network.protocol import NetworkMessage, MessageType, create_message
from engine.utils.name_generator import NameGenerator

# Import client config from same directory
import importlib.util
spec = importlib.util.spec_from_file_location("client_config", 
    os.path.join(os.path.dirname(__file__), "client_config.py"))
client_config_module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(client_config_module)
ClientConfig = client_config_module.ClientConfig


class GUIClient:
    """
    Graphical game client using Pygame
    Provides visual representation of game state
    """
    
    # Camera constants
    BASE_CAMERA_SPEED = 10
    
    def __init__(self, player_id: str, character_name: str, config: Optional[ClientConfig] = None):
        self.player_id = player_id
        self.character_name = character_name
        self.reader: Optional[asyncio.StreamReader] = None
        self.writer: Optional[asyncio.StreamWriter] = None
        self.running = False
        self.entities: Dict[str, Dict] = {}
        self.config = config or ClientConfig()
        
        # Pygame settings from config
        width, height = self.config.get_display_size()
        self.width = width
        self.height = height
        self.screen = None
        self.clock = None
        self.font = None
        self.font_small = None
        self.fullscreen = self.config.get_fullscreen()
        self.show_fps = self.config.get_show_fps()
        self.fps_limit = self.config.get_fps_limit()
        
        # Connection state
        self.reconnect_attempts = 0
        self.max_reconnect_attempts = 5
        self.connection_status = "Disconnected"
        
        # Colors
        self.COLOR_BG = (0, 0, 20)  # Dark blue space
        self.COLOR_STARS = (255, 255, 255)
        self.COLOR_PLAYER = (0, 255, 0)  # Green
        self.COLOR_ENEMY = (255, 0, 0)  # Red
        self.COLOR_FRIENDLY = (0, 150, 255)  # Blue
        self.COLOR_UI_BG = (20, 20, 40, 180)  # Semi-transparent dark blue
        self.COLOR_TEXT = (200, 200, 255)
        self.COLOR_TEXT_BRIGHT = (255, 255, 255)
        self.COLOR_SHIELD = (0, 150, 255)
        self.COLOR_ARMOR = (255, 200, 0)
        self.COLOR_HULL = (180, 180, 180)
        
        # Camera
        self.camera_x = 0.0
        self.camera_y = 0.0
        self.zoom = 1.0
        
        # Star field for background
        self.stars: List[Tuple[int, int, int]] = []
        
        # UI state
        self.show_help = False
        
    def initialize_pygame(self):
        """Initialize Pygame"""
        if not PYGAME_AVAILABLE:
            return False
        
        pygame.init()
        
        # Use fullscreen if configured
        flags = pygame.FULLSCREEN if self.fullscreen else 0
        if self.config.get('display', 'vsync', default=True):
            flags |= pygame.SCALED
        
        self.screen = pygame.display.set_mode((self.width, self.height), flags)
        pygame.display.set_caption(f"EVE OFFLINE - {self.character_name}")
        self.clock = pygame.time.Clock()
        
        # Load fonts
        try:
            self.font = pygame.font.Font(None, 24)
            self.font_small = pygame.font.Font(None, 18)
        except (pygame.error, OSError) as e:
            print(f"Warning: Could not load default font, using system font: {e}")
            self.font = pygame.font.SysFont('monospace', 24)
            self.font_small = pygame.font.SysFont('monospace', 18)
        
        # Generate star field
        import random
        for _ in range(200):
            x = random.randint(0, self.width)
            y = random.randint(0, self.height)
            brightness = random.randint(100, 255)
            self.stars.append((x, y, brightness))
        
        return True
    
    async def connect(self, host: Optional[str] = None, port: Optional[int] = None):
        """Connect to game server"""
        # Use config values if not specified
        if host is None:
            host = self.config.get_host()
        if port is None:
            port = self.config.get_port()
        
        print(f"[GUI Client] Connecting to {host}:{port}...")
        self.connection_status = "Connecting..."
        
        try:
            # Use connection timeout from config
            timeout = self.config.get_connection_timeout()
            self.reader, self.writer = await asyncio.wait_for(
                asyncio.open_connection(host, port),
                timeout=timeout
            )
            
            # Send connection message
            connect_msg = create_message(MessageType.CONNECT, {
                'player_id': self.player_id,
                'character_name': self.character_name,
                'version': '0.1.0'
            })
            
            self.writer.write(connect_msg.to_json().encode())
            await self.writer.drain()
            
            # Wait for acknowledgment
            buffer_size = self.config.get_buffer_size()
            data = await asyncio.wait_for(
                self.reader.read(buffer_size),
                timeout=timeout
            )
            message = NetworkMessage.from_json(data.decode())
            
            if message.message_type == MessageType.CONNECT_ACK.value:
                print(f"[GUI Client] Connected! {message.data.get('message')}")
                self.running = True
                self.reconnect_attempts = 0
                self.connection_status = "Connected"
                return True
            else:
                print(f"[GUI Client] Connection failed: Invalid response")
                self.connection_status = "Failed"
                return False
                
        except asyncio.TimeoutError:
            print(f"[GUI Client] Connection timeout after {timeout}s")
            self.connection_status = "Timeout"
            return False
        except Exception as e:
            print(f"[GUI Client] Connection error: {e}")
            self.connection_status = f"Error: {e}"
            return False
    
    async def disconnect(self):
        """Disconnect from server"""
        if self.writer:
            disconnect_msg = create_message(MessageType.DISCONNECT, {
                'player_id': self.player_id
            })
            self.writer.write(disconnect_msg.to_json().encode())
            await self.writer.drain()
            self.writer.close()
            await self.writer.wait_closed()
        self.running = False
        print("[GUI Client] Disconnected")
    
    async def receive_loop(self):
        """Receive messages from server"""
        buffer_size = self.config.get_buffer_size()
        
        while self.running:
            try:
                data = await self.reader.read(buffer_size)
                if not data:
                    print("[GUI Client] Connection closed by server")
                    self.connection_status = "Disconnected"
                    break
                
                message = NetworkMessage.from_json(data.decode())
                
                if message.message_type == MessageType.STATE_UPDATE.value:
                    self.handle_state_update(message)
                elif message.message_type == MessageType.CHAT.value:
                    self.handle_chat(message)
                elif message.message_type == MessageType.SPAWN_ENTITY.value:
                    self.handle_spawn(message)
                elif message.message_type == MessageType.DESTROY_ENTITY.value:
                    self.handle_destroy(message)
                elif message.message_type == MessageType.ERROR.value:
                    self.handle_error(message)
                    
            except Exception as e:
                print(f"[GUI Client] Error receiving: {e}")
                self.running = False
                self.connection_status = "Error"
                break
        
        # Attempt reconnection if enabled
        if self.config.get_auto_reconnect() and self.reconnect_attempts < self.max_reconnect_attempts:
            self.reconnect_attempts += 1
            delay = self.config.get_reconnect_delay()
            print(f"[GUI Client] Reconnecting in {delay}s (attempt {self.reconnect_attempts}/{self.max_reconnect_attempts})...")
            self.connection_status = f"Reconnecting ({self.reconnect_attempts}/{self.max_reconnect_attempts})"
            await asyncio.sleep(delay)
            if await self.connect():
                # Restart receive loop
                await self.receive_loop()
    
    def handle_state_update(self, message: NetworkMessage):
        """Handle state update from server"""
        entities_data = message.data.get('entities', [])
        for entity_data in entities_data:
            entity_id = entity_data['id']
            self.entities[entity_id] = entity_data
    
    def handle_spawn(self, message: NetworkMessage):
        """Handle entity spawn"""
        entity_id = message.data['entity_id']
        self.entities[entity_id] = message.data
        print(f"[GUI Client] Entity spawned: {entity_id}")
    
    def handle_destroy(self, message: NetworkMessage):
        """Handle entity destruction"""
        entity_id = message.data['entity_id']
        if entity_id in self.entities:
            del self.entities[entity_id]
            print(f"[GUI Client] Entity destroyed: {entity_id}")
    
    def handle_chat(self, message: NetworkMessage):
        """Handle chat message"""
        sender = message.data.get('sender', 'Unknown')
        text = message.data.get('message', '')
        print(f"[Chat] {sender}: {text}")
    
    def handle_error(self, message: NetworkMessage):
        """Handle error message from server"""
        error = message.data.get('error', 'Unknown error')
        print(f"[Error] Server error: {error}")
        self.connection_status = f"Error: {error}"
    
    def world_to_screen(self, x: float, y: float) -> Tuple[int, int]:
        """Convert world coordinates to screen coordinates"""
        screen_x = int((x - self.camera_x) * self.zoom + self.width / 2)
        screen_y = int((y - self.camera_y) * self.zoom + self.height / 2)
        return screen_x, screen_y
    
    def draw_star_field(self):
        """Draw background star field"""
        for x, y, brightness in self.stars:
            color = (brightness, brightness, brightness)
            pygame.draw.circle(self.screen, color, (x, y), 1)
    
    def draw_entity(self, entity_id: str, entity_data: Dict):
        """Draw an entity on screen"""
        pos = entity_data.get('pos')
        if not pos:
            return
        
        # Convert to screen coordinates
        screen_x, screen_y = self.world_to_screen(pos['x'], pos['y'])
        
        # Skip if off-screen
        if screen_x < -50 or screen_x > self.width + 50:
            return
        if screen_y < -50 or screen_y > self.height + 50:
            return
        
        # Determine color based on entity type
        is_player = entity_data.get('player', False)
        if is_player:
            color = self.COLOR_PLAYER
            size = 8
        else:
            color = self.COLOR_ENEMY
            size = 6
        
        # Draw ship (triangle)
        points = [
            (screen_x, screen_y - size),
            (screen_x - size, screen_y + size),
            (screen_x + size, screen_y + size)
        ]
        pygame.draw.polygon(self.screen, color, points)
        
        # Draw selection circle if this is the player
        if is_player:
            pygame.draw.circle(self.screen, color, (screen_x, screen_y), size * 2, 1)
        
        # Draw health bar
        health = entity_data.get('health')
        if health:
            self.draw_health_bar(screen_x, screen_y - size - 5, 
                               health['s'], health['sm'],
                               health['a'], health['am'],
                               health['h'], health['hm'])
    
    def draw_health_bar(self, x: int, y: int, 
                       shield: float, shield_max: float,
                       armor: float, armor_max: float,
                       hull: float, hull_max: float):
        """Draw health bars above entity"""
        bar_width = 40
        bar_height = 3
        spacing = 1
        
        # Shield bar (blue)
        if shield_max > 0:
            shield_percent = shield / shield_max
            pygame.draw.rect(self.screen, (50, 50, 80), 
                           (x - bar_width//2, y, bar_width, bar_height))
            pygame.draw.rect(self.screen, self.COLOR_SHIELD,
                           (x - bar_width//2, y, int(bar_width * shield_percent), bar_height))
            y += bar_height + spacing
        
        # Armor bar (yellow)
        if armor_max > 0:
            armor_percent = armor / armor_max
            pygame.draw.rect(self.screen, (80, 80, 50),
                           (x - bar_width//2, y, bar_width, bar_height))
            pygame.draw.rect(self.screen, self.COLOR_ARMOR,
                           (x - bar_width//2, y, int(bar_width * armor_percent), bar_height))
            y += bar_height + spacing
        
        # Hull bar (gray)
        if hull_max > 0:
            hull_percent = hull / hull_max
            pygame.draw.rect(self.screen, (50, 50, 50),
                           (x - bar_width//2, y, bar_width, bar_height))
            pygame.draw.rect(self.screen, self.COLOR_HULL,
                           (x - bar_width//2, y, int(bar_width * hull_percent), bar_height))
    
    def draw_ui(self):
        """Draw UI overlay"""
        # Semi-transparent background for UI
        ui_surface = pygame.Surface((self.width, 120), pygame.SRCALPHA)
        ui_surface.fill(self.COLOR_UI_BG)
        self.screen.blit(ui_surface, (0, self.height - 120))
        
        # Draw status info
        y = self.height - 110
        
        # Title with connection status
        status_color = (0, 255, 0) if self.connection_status == "Connected" else (255, 100, 100)
        text = self.font.render(f"EVE OFFLINE - {self.character_name}", True, self.COLOR_TEXT_BRIGHT)
        self.screen.blit(text, (10, y))
        
        # Connection status indicator
        status_text = self.font_small.render(f"[{self.connection_status}]", True, status_color)
        self.screen.blit(status_text, (10 + text.get_width() + 10, y + 5))
        y += 30
        
        # Entity count
        text = self.font_small.render(f"Entities: {len(self.entities)}", True, self.COLOR_TEXT)
        self.screen.blit(text, (10, y))
        y += 20
        
        # Camera info
        text = self.font_small.render(f"Camera: ({self.camera_x:.0f}, {self.camera_y:.0f}) Zoom: {self.zoom:.2f}x", 
                                     True, self.COLOR_TEXT)
        self.screen.blit(text, (10, y))
        y += 20
        
        # Controls hint
        text = self.font_small.render("Controls: Arrow Keys=Move Camera | +/-=Zoom | H=Help | ESC=Quit", 
                                     True, self.COLOR_TEXT)
        self.screen.blit(text, (10, y))
        
        # FPS counter if enabled
        if self.show_fps:
            fps = self.clock.get_fps()
            fps_text = self.font_small.render(f"FPS: {fps:.1f}", True, self.COLOR_TEXT_BRIGHT)
            self.screen.blit(fps_text, (self.width - 80, 10))
        
        # Draw help overlay if enabled
        if self.show_help:
            self.draw_help()
    
    def draw_help(self):
        """Draw help overlay"""
        help_width = 500
        help_height = 400
        help_x = (self.width - help_width) // 2
        help_y = (self.height - help_height) // 2
        
        # Background
        help_surface = pygame.Surface((help_width, help_height), pygame.SRCALPHA)
        help_surface.fill((0, 0, 0, 220))
        self.screen.blit(help_surface, (help_x, help_y))
        
        # Border
        pygame.draw.rect(self.screen, self.COLOR_TEXT_BRIGHT, 
                        (help_x, help_y, help_width, help_height), 2)
        
        # Title
        text = self.font.render("EVE OFFLINE - Controls", True, self.COLOR_TEXT_BRIGHT)
        self.screen.blit(text, (help_x + 20, help_y + 20))
        
        # Help text
        help_lines = [
            "",
            "CAMERA CONTROLS:",
            "  Arrow Keys - Pan camera",
            "  + / -      - Zoom in/out",
            "  Home       - Reset camera to origin",
            "",
            "GAME CONTROLS:",
            "  H          - Toggle this help",
            "  ESC        - Quit game",
            "",
            "CURRENT FEATURES:",
            "  • Visual representation of ships",
            "  • Real-time entity tracking",
            "  • Health bars (Shield/Armor/Hull)",
            "  • Star field background",
            "",
            "NOTE: This is a basic GUI demo. Full gameplay",
            "features are accessible via the text-based client",
            "and interactive demo.",
        ]
        
        y = help_y + 60
        for line in help_lines:
            text = self.font_small.render(line, True, self.COLOR_TEXT)
            self.screen.blit(text, (help_x + 20, y))
            y += 20
    
    def handle_input(self):
        """Handle keyboard/mouse input"""
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    self.running = False
                elif event.key == pygame.K_h:
                    self.show_help = not self.show_help
                elif event.key == pygame.K_HOME:
                    self.camera_x = 0
                    self.camera_y = 0
                    self.zoom = 1.0
        
        # Camera controls
        keys = pygame.key.get_pressed()
        camera_speed = self.BASE_CAMERA_SPEED / self.zoom
        
        if keys[pygame.K_LEFT]:
            self.camera_x -= camera_speed
        if keys[pygame.K_RIGHT]:
            self.camera_x += camera_speed
        if keys[pygame.K_UP]:
            self.camera_y -= camera_speed
        if keys[pygame.K_DOWN]:
            self.camera_y += camera_speed
        if keys[pygame.K_EQUALS] or keys[pygame.K_PLUS]:
            self.zoom *= 1.02
        if keys[pygame.K_MINUS]:
            self.zoom /= 1.02
        
        # Clamp zoom
        self.zoom = max(0.1, min(5.0, self.zoom))
    
    def render(self):
        """Render the game"""
        if not PYGAME_AVAILABLE or not self.screen:
            # Fallback to text rendering
            self.render_text()
            return
        
        # Clear screen
        self.screen.fill(self.COLOR_BG)
        
        # Draw star field
        self.draw_star_field()
        
        # Draw entities
        for entity_id, entity_data in self.entities.items():
            self.draw_entity(entity_id, entity_data)
        
        # Draw UI overlay
        self.draw_ui()
        
        # Update display
        pygame.display.flip()
        self.clock.tick(self.fps_limit)
    
    def render_text(self):
        """Fallback text rendering"""
        print(f"\n--- Game State ---")
        print(f"Player: {self.character_name}")
        print(f"Entities: {len(self.entities)}")
        for entity_id, entity_data in self.entities.items():
            pos = entity_data.get('pos')
            health = entity_data.get('health')
            if pos:
                print(f"  {entity_id[:8]}: pos({pos['x']:.1f}, {pos['y']:.1f}) ", end="")
            if health:
                print(f"HP[H:{health['h']:.0f} A:{health['a']:.0f} S:{health['s']:.0f}]")
            else:
                print()
    
    async def game_loop(self):
        """Main client game loop"""
        while self.running:
            if PYGAME_AVAILABLE:
                self.handle_input()
                self.render()
                await asyncio.sleep(0.016)  # ~60 FPS
            else:
                self.render()
                await asyncio.sleep(0.5)
    
    async def run(self, host: str = "localhost", port: int = 8765):
        """Run the GUI client"""
        if PYGAME_AVAILABLE:
            if not self.initialize_pygame():
                print("Failed to initialize Pygame")
                return
        
        if await self.connect(host, port):
            try:
                await asyncio.gather(
                    self.receive_loop(),
                    self.game_loop()
                )
            except KeyboardInterrupt:
                print("\n[GUI Client] Shutting down...")
            finally:
                await self.disconnect()
                if PYGAME_AVAILABLE:
                    pygame.quit()


async def run_gui_client(player_id: str, character_name: str, 
                        host: Optional[str] = None, port: Optional[int] = None, config: Optional[ClientConfig] = None):
    """Run a GUI game client"""
    client = GUIClient(player_id, character_name, config)
    await client.run(host, port)


if __name__ == "__main__":
    import uuid
    import argparse
    
    # Parse arguments
    parser = argparse.ArgumentParser(description='EVE OFFLINE GUI Client')
    parser.add_argument('character_name', nargs='?', help='Character name')
    parser.add_argument('--host', help='Server host (overrides config)')
    parser.add_argument('--port', type=int, help='Server port (overrides config)')
    parser.add_argument('--config', help='Path to config file')
    parser.add_argument('--generate-name', action='store_true', help='Generate random character name')
    parser.add_argument('--name-style', choices=['random', 'male', 'female'], default='random',
                       help='Name generation style')
    parser.add_argument('--fullscreen', action='store_true', help='Run in fullscreen mode')
    args = parser.parse_args()
    
    # Load configuration
    config = ClientConfig(args.config) if args.config else ClientConfig()
    
    # Override fullscreen if specified
    if args.fullscreen:
        config.set('display', 'fullscreen', value=True)
    
    # Generate or use character name
    if args.generate_name or (not args.character_name and config.get_auto_generate_name()):
        style = args.name_style if args.name_style != 'random' else None
        character_name = NameGenerator.generate_character_name(style)
        print(f"[GUI Client] Generated character name: {character_name}")
    else:
        character_name = args.character_name or "TestPilot"
    
    # Generate player ID
    player_id = str(uuid.uuid4())
    
    # Get connection settings
    host = args.host or config.get_host()
    port = args.port or config.get_port()
    
    print(f"[GUI Client] Starting EVE OFFLINE GUI Client")
    print(f"[GUI Client] Character: {character_name}")
    print(f"[GUI Client] Player ID: {player_id}")
    print(f"[GUI Client] Server: {host}:{port}")
    print()
    
    # Run client
    asyncio.run(run_gui_client(player_id, character_name, host, port, config))
