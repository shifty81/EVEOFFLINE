"""
Network Client for 3D Client
Handles connection to Python game server
"""

import asyncio
import json
import time
from typing import Optional, Dict, Any, Callable


class NetworkClient:
    """
    Handles network communication with game server
    Uses TCP sockets with JSON message protocol
    """
    
    def __init__(self, host: str = "localhost", port: int = 8765):
        self.host = host
        self.port = port
        self.reader: Optional[asyncio.StreamReader] = None
        self.writer: Optional[asyncio.StreamWriter] = None
        self.connected = False
        self.message_handlers: Dict[str, Callable] = {}
        
    async def connect(self, player_id: str, character_name: str) -> bool:
        """
        Connect to game server
        
        Args:
            player_id: Unique player identifier
            character_name: Display name for character
            
        Returns:
            True if connection successful
        """
        try:
            print(f"[Network] Connecting to {self.host}:{self.port}...")
            self.reader, self.writer = await asyncio.open_connection(
                self.host, self.port
            )
            
            # Send connection message
            connect_msg = {
                'type': 'connect',
                'timestamp': time.time(),
                'data': {
                    'player_id': player_id,
                    'character_name': character_name,
                    'version': '0.1.0'
                }
            }
            
            await self.send_message(connect_msg)
            
            # Wait for acknowledgment
            response = await self.receive_message()
            if response and response.get('type') == 'connect_ack':
                if response['data'].get('success'):
                    self.connected = True
                    print(f"[Network] Connected successfully: {response['data'].get('message')}")
                    return True
                    
            print("[Network] Connection failed: No acknowledgment received")
            return False
            
        except Exception as e:
            print(f"[Network] Connection error: {e}")
            return False
    
    async def disconnect(self):
        """Disconnect from server"""
        if self.connected:
            try:
                disconnect_msg = {
                    'type': 'disconnect',
                    'timestamp': time.time(),
                    'data': {}
                }
                await self.send_message(disconnect_msg)
                self.writer.close()
                await self.writer.wait_closed()
            except Exception as e:
                print(f"[Network] Disconnect error: {e}")
            finally:
                self.connected = False
                print("[Network] Disconnected")
    
    async def send_message(self, message: Dict[str, Any]):
        """
        Send message to server
        
        Args:
            message: Dictionary containing message data
        """
        if not self.writer:
            return
            
        try:
            json_str = json.dumps(message)
            self.writer.write(json_str.encode())
            await self.writer.drain()
        except Exception as e:
            print(f"[Network] Send error: {e}")
            self.connected = False
    
    async def receive_message(self) -> Optional[Dict[str, Any]]:
        """
        Receive message from server
        
        Returns:
            Dictionary containing message data, or None if error
        """
        if not self.reader:
            return None
            
        try:
            data = await self.reader.read(4096)
            if not data:
                self.connected = False
                return None
                
            json_str = data.decode()
            return json.loads(json_str)
        except Exception as e:
            print(f"[Network] Receive error: {e}")
            self.connected = False
            return None
    
    def register_handler(self, message_type: str, handler: Callable):
        """
        Register a handler for a specific message type
        
        Args:
            message_type: Type of message to handle
            handler: Function to call when message is received
        """
        self.message_handlers[message_type] = handler
    
    async def receive_loop(self):
        """
        Main receive loop - processes incoming messages
        Should be run as an async task
        """
        while self.connected:
            message = await self.receive_message()
            if message:
                msg_type = message.get('type')
                if msg_type in self.message_handlers:
                    try:
                        self.message_handlers[msg_type](message)
                    except Exception as e:
                        print(f"[Network] Handler error for {msg_type}: {e}")
            else:
                # Connection lost
                break
        
        print("[Network] Receive loop ended")
    
    # Convenience methods for common messages
    
    async def send_move_input(self, direction: Dict[str, float]):
        """Send movement input to server"""
        await self.send_message({
            'type': 'input_move',
            'timestamp': time.time(),
            'data': {'direction': direction}
        })
    
    async def send_target_input(self, target_id: str):
        """Send targeting input to server"""
        await self.send_message({
            'type': 'input_target',
            'timestamp': time.time(),
            'data': {'target_id': target_id}
        })
    
    async def send_fire_input(self):
        """Send fire weapons input to server"""
        await self.send_message({
            'type': 'input_fire',
            'timestamp': time.time(),
            'data': {}
        })
    
    async def send_chat(self, message: str, channel: str = "local"):
        """Send chat message to server"""
        await self.send_message({
            'type': 'chat',
            'timestamp': time.time(),
            'data': {
                'message': message,
                'channel': channel
            }
        })
