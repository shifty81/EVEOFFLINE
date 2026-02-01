"""
Dedicated Game Server
Handles multiplayer game state and client connections
"""

import asyncio
import time
from typing import Dict, Set, Optional
from engine.core.engine import Engine
from engine.core.ecs import World
from engine.network.protocol import NetworkMessage, MessageType, create_message
from engine.utils.data_loader import DataLoader
from engine.systems.game_systems import (
    MovementSystem, CapacitorSystem, ShieldRechargeSystem,
    WeaponSystem, AISystem, TargetingSystem
)


class GameServer:
    """
    Authoritative game server
    Manages world state and client connections
    """
    
    def __init__(self, host: str = "localhost", port: int = 8765):
        self.host = host
        self.port = port
        self.engine = Engine(target_fps=30)  # Server tick rate
        self.data_loader = DataLoader()
        self.clients: Dict[str, asyncio.StreamWriter] = {}  # player_id -> writer
        self.running = False
        self.tick = 0
        
    async def initialize(self):
        """Initialize server"""
        print(f"[Server] Initializing EVE OFFLINE Server")
        print(f"[Server] Host: {self.host}:{self.port}")
        
        # Load game data
        self.data_loader.load_all()
        
        # Initialize engine
        self.engine.initialize()
        
        # Add game systems
        self.engine.world.add_system(MovementSystem(self.engine.world))
        self.engine.world.add_system(CapacitorSystem(self.engine.world))
        self.engine.world.add_system(ShieldRechargeSystem(self.engine.world))
        self.engine.world.add_system(WeaponSystem(self.engine.world))
        self.engine.world.add_system(AISystem(self.engine.world))
        self.engine.world.add_system(TargetingSystem(self.engine.world))
        
        print(f"[Server] Server initialized successfully")
        
    async def handle_client(self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter):
        """Handle a client connection"""
        addr = writer.get_extra_info('peername')
        print(f"[Server] New connection from {addr}")
        
        player_id = None
        
        try:
            while self.running:
                # Read message
                data = await reader.read(4096)
                if not data:
                    break
                    
                message_str = data.decode()
                message = NetworkMessage.from_json(message_str)
                
                # Handle message
                if message.message_type == MessageType.CONNECT.value:
                    player_id = message.data.get('player_id')
                    character_name = message.data.get('character_name')
                    
                    # Register client
                    self.clients[player_id] = writer
                    print(f"[Server] Player {character_name} ({player_id}) connected")
                    
                    # Send acknowledgment
                    ack = create_message(MessageType.CONNECT_ACK, {
                        'success': True,
                        'message': 'Welcome to EVE OFFLINE!'
                    })
                    writer.write(ack.to_json().encode())
                    await writer.drain()
                    
                    # Spawn player entity
                    await self.spawn_player(player_id, character_name)
                    
                elif message.message_type == MessageType.INPUT_MOVE.value:
                    await self.handle_player_input(player_id, message)
                    
                elif message.message_type == MessageType.CHAT.value:
                    await self.broadcast_chat(message.data)
                    
        except Exception as e:
            print(f"[Server] Error handling client {addr}: {e}")
        finally:
            # Cleanup
            if player_id and player_id in self.clients:
                del self.clients[player_id]
                print(f"[Server] Player {player_id} disconnected")
            writer.close()
            await writer.wait_closed()
            
    async def spawn_player(self, player_id: str, character_name: str):
        """Spawn a player's ship in the world"""
        from engine.components.game_components import (
            Position, Velocity, Health, Ship, Player, Fitting, Skills, Target, Capacitor
        )
        
        # Create player entity
        entity = self.engine.world.create_entity(player_id)
        
        # Add components
        entity.add_component(Position(x=0, y=0, z=0))
        entity.add_component(Velocity(max_speed=325))
        entity.add_component(Health(
            hull_hp=350, hull_max=350,
            armor_hp=400, armor_max=400,
            shield_hp=450, shield_max=450,
            shield_recharge_rate=5.0
        ))
        entity.add_component(Capacitor(capacitor=280, capacitor_max=280, recharge_rate=3.0))
        entity.add_component(Ship(
            ship_type="Frigate",
            ship_name="Rifter",
            race="Minmatar",
            cpu_max=140,
            powergrid_max=38
        ))
        entity.add_component(Player(player_id=player_id, character_name=character_name))
        entity.add_component(Fitting())
        entity.add_component(Skills())
        entity.add_component(Target())
        
        # Broadcast spawn
        await self.broadcast_spawn(entity)
        
    async def handle_player_input(self, player_id: str, message: NetworkMessage):
        """Handle player input"""
        from engine.components.game_components import Velocity
        
        entity = self.engine.world.get_entity(player_id)
        if not entity:
            return
            
        velocity = entity.get_component(Velocity)
        if not velocity:
            return
            
        # Update velocity based on input
        input_data = message.data
        velocity.vx = input_data.get('vx', 0)
        velocity.vy = input_data.get('vy', 0)
        velocity.vz = input_data.get('vz', 0)
        
    async def broadcast_spawn(self, entity):
        """Broadcast entity spawn to all clients"""
        from engine.components.game_components import Position, Health
        
        pos = entity.get_component(Position)
        health = entity.get_component(Health)
        
        spawn_msg = create_message(MessageType.SPAWN_ENTITY, {
            'entity_id': entity.id,
            'position': {'x': pos.x, 'y': pos.y, 'z': pos.z} if pos else None,
            'health': {
                'hull': health.hull_hp,
                'armor': health.armor_hp,
                'shield': health.shield_hp
            } if health else None
        })
        
        await self.broadcast(spawn_msg)
        
    async def broadcast(self, message: NetworkMessage):
        """Broadcast message to all connected clients"""
        msg_bytes = message.to_json().encode()
        for writer in self.clients.values():
            try:
                writer.write(msg_bytes)
                await writer.drain()
            except Exception as e:
                print(f"[Server] Error broadcasting: {e}")
                
    async def broadcast_chat(self, chat_data: Dict):
        """Broadcast chat message"""
        msg = create_message(MessageType.CHAT, chat_data)
        await self.broadcast(msg)
        
    async def update_loop(self):
        """Main server update loop"""
        last_time = time.time()
        
        while self.running:
            current_time = time.time()
            delta_time = current_time - last_time
            last_time = current_time
            
            # Update world
            self.engine.world.update(delta_time)
            self.tick += 1
            
            # Send state updates to clients
            if self.tick % 3 == 0:  # Send updates every 3 ticks (~10 Hz)
                await self.broadcast_state()
                
            # Sleep to maintain tick rate
            await asyncio.sleep(1.0 / 30)  # 30 Hz server tick
            
    async def broadcast_state(self):
        """Broadcast world state to all clients"""
        from engine.components.game_components import Position, Health, Velocity
        
        entities_data = []
        for entity in self.engine.world.entities.values():
            pos = entity.get_component(Position)
            health = entity.get_component(Health)
            vel = entity.get_component(Velocity)
            
            entity_data = {'id': entity.id}
            if pos:
                entity_data['pos'] = {'x': pos.x, 'y': pos.y, 'z': pos.z, 'rot': pos.rotation}
            if health:
                entity_data['health'] = {'h': health.hull_hp, 'a': health.armor_hp, 's': health.shield_hp}
            if vel:
                entity_data['vel'] = {'vx': vel.vx, 'vy': vel.vy, 'vz': vel.vz}
                
            entities_data.append(entity_data)
            
        state_msg = create_message(MessageType.STATE_UPDATE, {
            'entities': entities_data,
            'tick': self.tick
        })
        
        await self.broadcast(state_msg)
        
    async def start(self):
        """Start the server"""
        await self.initialize()
        
        self.running = True
        
        # Start TCP server
        server = await asyncio.start_server(
            self.handle_client,
            self.host,
            self.port
        )
        
        addr = server.sockets[0].getsockname()
        print(f"[Server] Server running on {addr}")
        
        # Run update loop
        try:
            await asyncio.gather(
                server.serve_forever(),
                self.update_loop()
            )
        except KeyboardInterrupt:
            print("[Server] Shutting down...")
        finally:
            self.running = False
            server.close()
            await server.wait_closed()
            self.engine.shutdown()


def run_server(host: str = "localhost", port: int = 8765):
    """Run the game server"""
    server = GameServer(host, port)
    asyncio.run(server.start())


if __name__ == "__main__":
    run_server()
