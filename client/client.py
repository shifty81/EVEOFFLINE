"""
Game Client
Connects to server and renders game state
"""

import asyncio
import time
import sys
import os
from typing import Optional, Dict

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from engine.network.protocol import NetworkMessage, MessageType, create_message
from engine.utils.name_generator import NameGenerator

# Import client config from same directory
import importlib.util
spec = importlib.util.spec_from_file_location("client_config", 
    os.path.join(os.path.dirname(__file__), "client_config.py"))
client_config_module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(client_config_module)
ClientConfig = client_config_module.ClientConfig


class GameClient:
    """
    Game client
    Connects to server and handles rendering/input
    """
    
    def __init__(self, player_id: str, character_name: str, config: Optional[ClientConfig] = None):
        self.player_id = player_id
        self.character_name = character_name
        self.reader: Optional[asyncio.StreamReader] = None
        self.writer: Optional[asyncio.StreamWriter] = None
        self.running = False
        self.entities: Dict[str, Dict] = {}
        self.config = config or ClientConfig()
        self.reconnect_attempts = 0
        self.max_reconnect_attempts = 5
        
    async def connect(self, host: Optional[str] = None, port: Optional[int] = None):
        """Connect to game server"""
        # Use config values if not specified
        if host is None:
            host = self.config.get_host()
        if port is None:
            port = self.config.get_port()
        
        print(f"[Client] Connecting to {host}:{port}...")
        
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
                print(f"[Client] Connected! {message.data.get('message')}")
                self.running = True
                self.reconnect_attempts = 0
                return True
            else:
                print(f"[Client] Connection failed: Invalid response")
                return False
                
        except asyncio.TimeoutError:
            print(f"[Client] Connection timeout after {timeout}s")
            return False
        except Exception as e:
            print(f"[Client] Connection error: {e}")
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
        print("[Client] Disconnected")
        
    async def send_input(self, vx: float, vy: float, vz: float):
        """Send movement input to server"""
        if not self.writer:
            return
            
        input_msg = create_message(MessageType.INPUT_MOVE, {
            'player_id': self.player_id,
            'vx': vx,
            'vy': vy,
            'vz': vz
        })
        
        self.writer.write(input_msg.to_json().encode())
        await self.writer.drain()
        
    async def send_chat(self, message: str):
        """Send chat message"""
        if not self.writer:
            return
            
        chat_msg = create_message(MessageType.CHAT, {
            'sender': self.character_name,
            'message': message,
            'channel': 'local'
        })
        
        self.writer.write(chat_msg.to_json().encode())
        await self.writer.drain()
        
    async def receive_loop(self):
        """Receive messages from server"""
        buffer_size = self.config.get_buffer_size()
        
        while self.running:
            try:
                data = await self.reader.read(buffer_size)
                if not data:
                    print("[Client] Connection closed by server")
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
                print(f"[Client] Error receiving: {e}")
                self.running = False
                break
        
        # Attempt reconnection if enabled
        if self.config.get_auto_reconnect() and self.reconnect_attempts < self.max_reconnect_attempts:
            self.reconnect_attempts += 1
            delay = self.config.get_reconnect_delay()
            print(f"[Client] Reconnecting in {delay}s (attempt {self.reconnect_attempts}/{self.max_reconnect_attempts})...")
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
        print(f"[Client] Entity spawned: {entity_id}")
        
    def handle_destroy(self, message: NetworkMessage):
        """Handle entity destruction"""
        entity_id = message.data['entity_id']
        if entity_id in self.entities:
            del self.entities[entity_id]
            print(f"[Client] Entity destroyed: {entity_id}")
            
    def handle_chat(self, message: NetworkMessage):
        """Handle chat message"""
        sender = message.data.get('sender', 'Unknown')
        text = message.data.get('message', '')
        print(f"[Chat] {sender}: {text}")
    
    def handle_error(self, message: NetworkMessage):
        """Handle error message from server"""
        error = message.data.get('error', 'Unknown error')
        print(f"[Error] Server error: {error}")
        
    def render(self):
        """Simple text-based rendering for now"""
        print(f"\n--- Game State (Tick) ---")
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
            # Render
            self.render()
            
            # Sleep
            await asyncio.sleep(0.5)
            
    async def run(self, host: str = "localhost", port: int = 8765):
        """Run the client"""
        if await self.connect(host, port):
            try:
                await asyncio.gather(
                    self.receive_loop(),
                    self.game_loop()
                )
            except KeyboardInterrupt:
                print("\n[Client] Shutting down...")
            finally:
                await self.disconnect()


async def run_client(player_id: str, character_name: str, host: Optional[str] = None, port: Optional[int] = None, config: Optional[ClientConfig] = None):
    """Run a game client"""
    client = GameClient(player_id, character_name, config)
    await client.run(host, port)


if __name__ == "__main__":
    import uuid
    import argparse
    
    # Parse arguments
    parser = argparse.ArgumentParser(description='EVE OFFLINE Text Client')
    parser.add_argument('character_name', nargs='?', help='Character name')
    parser.add_argument('--host', help='Server host (overrides config)')
    parser.add_argument('--port', type=int, help='Server port (overrides config)')
    parser.add_argument('--config', help='Path to config file')
    parser.add_argument('--generate-name', action='store_true', help='Generate random character name')
    parser.add_argument('--name-style', choices=['random', 'male', 'female'], default='random',
                       help='Name generation style')
    args = parser.parse_args()
    
    # Load configuration
    config = ClientConfig(args.config) if args.config else ClientConfig()
    
    # Generate or use character name
    if args.generate_name or (not args.character_name and config.get_auto_generate_name()):
        style = args.name_style if args.name_style != 'random' else None
        character_name = NameGenerator.generate_character_name(style)
        print(f"[Client] Generated character name: {character_name}")
    else:
        character_name = args.character_name or "TestPilot"
    
    # Generate player ID
    player_id = str(uuid.uuid4())
    
    # Get connection settings
    host = args.host or config.get_host()
    port = args.port or config.get_port()
    
    print(f"[Client] Starting EVE OFFLINE Text Client")
    print(f"[Client] Character: {character_name}")
    print(f"[Client] Player ID: {player_id}")
    print(f"[Client] Server: {host}:{port}")
    print()
    
    # Run client
    asyncio.run(run_client(player_id, character_name, host, port, config))
