"""
Game Client
Connects to server and renders game state
"""

import asyncio
import time
from typing import Optional, Dict
from engine.network.protocol import NetworkMessage, MessageType, create_message


class GameClient:
    """
    Game client
    Connects to server and handles rendering/input
    """
    
    def __init__(self, player_id: str, character_name: str):
        self.player_id = player_id
        self.character_name = character_name
        self.reader: Optional[asyncio.StreamReader] = None
        self.writer: Optional[asyncio.StreamWriter] = None
        self.running = False
        self.entities: Dict[str, Dict] = {}
        
    async def connect(self, host: str = "localhost", port: int = 8765):
        """Connect to game server"""
        print(f"[Client] Connecting to {host}:{port}...")
        
        self.reader, self.writer = await asyncio.open_connection(host, port)
        
        # Send connection message
        connect_msg = create_message(MessageType.CONNECT, {
            'player_id': self.player_id,
            'character_name': self.character_name,
            'version': '0.1.0'
        })
        
        self.writer.write(connect_msg.to_json().encode())
        await self.writer.drain()
        
        # Wait for acknowledgment
        data = await self.reader.read(4096)
        message = NetworkMessage.from_json(data.decode())
        
        if message.message_type == MessageType.CONNECT_ACK.value:
            print(f"[Client] Connected! {message.data.get('message')}")
            self.running = True
            return True
        else:
            print(f"[Client] Connection failed")
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
        while self.running:
            try:
                data = await self.reader.read(4096)
                if not data:
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
                    
            except Exception as e:
                print(f"[Client] Error receiving: {e}")
                break
                
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
        sender = message.data['sender']
        text = message.data['message']
        print(f"[Chat] {sender}: {text}")
        
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


async def run_client(player_id: str, character_name: str, host: str = "localhost", port: int = 8765):
    """Run a game client"""
    client = GameClient(player_id, character_name)
    await client.run(host, port)


if __name__ == "__main__":
    import sys
    import uuid
    
    player_id = str(uuid.uuid4())
    character_name = sys.argv[1] if len(sys.argv) > 1 else "TestPilot"
    
    asyncio.run(run_client(player_id, character_name))
