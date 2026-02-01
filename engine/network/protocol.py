"""
Network Protocol
Messages for client-server communication
"""

from dataclasses import dataclass, asdict
from typing import Dict, List, Any, Optional
from enum import Enum
import json


class MessageType(Enum):
    """Message types for network communication"""
    # Connection
    CONNECT = "connect"
    DISCONNECT = "disconnect"
    CONNECT_ACK = "connect_ack"
    
    # Game state
    STATE_UPDATE = "state_update"
    SPAWN_ENTITY = "spawn_entity"
    DESTROY_ENTITY = "destroy_entity"
    
    # Player input
    INPUT_MOVE = "input_move"
    INPUT_TARGET = "input_target"
    INPUT_FIRE = "input_fire"
    INPUT_DOCK = "input_dock"
    INPUT_UNDOCK = "input_undock"
    
    # Combat
    DAMAGE = "damage"
    TARGET_LOCK = "target_lock"
    TARGET_UNLOCK = "target_unlock"
    
    # Chat
    CHAT = "chat"
    
    # Missions
    MISSION_ACCEPT = "mission_accept"
    MISSION_COMPLETE = "mission_complete"
    MISSION_UPDATE = "mission_update"


@dataclass
class NetworkMessage:
    """Base class for network messages"""
    message_type: str
    timestamp: float
    data: Dict[str, Any]
    
    def to_json(self) -> str:
        """Convert message to JSON"""
        return json.dumps({
            'type': self.message_type,
            'timestamp': self.timestamp,
            'data': self.data
        })
    
    @classmethod
    def from_json(cls, json_str: str) -> 'NetworkMessage':
        """Create message from JSON"""
        data = json.loads(json_str)
        return cls(
            message_type=data['type'],
            timestamp=data['timestamp'],
            data=data['data']
        )


@dataclass
class ConnectMessage:
    """Player connection request"""
    player_id: str
    character_name: str
    version: str = "0.1.0"


@dataclass
class StateUpdateMessage:
    """World state update from server to client"""
    entities: List[Dict[str, Any]]
    tick: int


@dataclass
class InputMessage:
    """Player input from client to server"""
    player_id: str
    input_type: str  # move, target, fire, etc.
    data: Dict[str, Any]


@dataclass
class ChatMessage:
    """Chat message"""
    sender: str
    message: str
    channel: str = "local"


def create_message(msg_type: MessageType, data: Dict[str, Any], timestamp: float = None) -> NetworkMessage:
    """Helper to create a network message"""
    import time
    if timestamp is None:
        timestamp = time.time()
    return NetworkMessage(msg_type.value, timestamp, data)
