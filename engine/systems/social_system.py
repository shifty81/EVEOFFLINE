"""
Social System
Handles contacts, standings, mail, and chat channels
Based on EVE Online's social features
"""

from typing import Optional, Dict, List
from dataclasses import dataclass, field
from engine.core.ecs import World, Entity
import time


@dataclass
class MailMessage:
    """Individual mail message"""
    message_id: str
    sender_id: str
    sender_name: str
    recipient_ids: List[str]  # Can send to multiple recipients
    subject: str
    body: str
    timestamp: float
    is_read: bool = False
    labels: List[str] = field(default_factory=list)  # Custom labels


@dataclass
class ChatMessage:
    """Individual chat message"""
    message_id: str
    sender_id: str
    sender_name: str
    channel_id: str
    message: str
    timestamp: float


@dataclass
class ChatChannel:
    """Chat channel data"""
    channel_id: str
    channel_name: str
    channel_type: str  # local, corporation, fleet, private, system
    members: List[str] = field(default_factory=list)  # Entity IDs
    operators: List[str] = field(default_factory=list)  # Channel operators
    password: Optional[str] = None
    is_moderated: bool = False
    message_history: List[ChatMessage] = field(default_factory=list)
    max_history: int = 100  # Keep last 100 messages


class SocialSystem:
    """
    Handles social features: contacts, standings, mail, and chat
    """
    
    def __init__(self, world: World):
        self.world = world
        self.mail_counter = 0
        self.message_counter = 0
        self.channel_counter = 0
        
        # Chat channels: {channel_id: ChatChannel}
        self.channels: Dict[str, ChatChannel] = {}
        
        # Initialize system chat channels
        self._init_system_channels()
    
    def _init_system_channels(self):
        """Initialize default system channels"""
        # Local channel (everyone in same solar system)
        self.channels["local"] = ChatChannel(
            channel_id="local",
            channel_name="Local",
            channel_type="local"
        )
        
        # Help channel
        self.channels["help"] = ChatChannel(
            channel_id="help",
            channel_name="Help",
            channel_type="system"
        )
        
        # Rookie help
        self.channels["rookie"] = ChatChannel(
            channel_id="rookie",
            channel_name="Rookie Help",
            channel_type="system"
        )
    
    # ===== CONTACTS & STANDINGS =====
    
    def add_contact(
        self,
        entity: Entity,
        contact_id: str,
        standing: float = 0.0
    ) -> bool:
        """
        Add a contact with a standing
        
        Args:
            entity: Entity adding the contact
            contact_id: Entity ID of the contact
            standing: Standing value from -10 (terrible) to +10 (excellent)
            
        Returns:
            True if added successfully
        """
        from engine.components.game_components import Contacts
        
        contacts_comp = entity.get_component(Contacts)
        if not contacts_comp:
            contacts_comp = Contacts()
            entity.add_component(contacts_comp)
        
        # Clamp standing to valid range
        standing = max(-10.0, min(10.0, standing))
        
        contacts_comp.contacts[contact_id] = standing
        return True
    
    def remove_contact(
        self,
        entity: Entity,
        contact_id: str
    ) -> bool:
        """
        Remove a contact
        
        Args:
            entity: Entity removing the contact
            contact_id: Entity ID of the contact to remove
            
        Returns:
            True if removed successfully
        """
        from engine.components.game_components import Contacts
        
        contacts_comp = entity.get_component(Contacts)
        if not contacts_comp or contact_id not in contacts_comp.contacts:
            return False
        
        del contacts_comp.contacts[contact_id]
        return True
    
    def set_standing(
        self,
        entity: Entity,
        contact_id: str,
        standing: float
    ) -> bool:
        """
        Set or update standing for a contact
        
        Args:
            entity: Entity setting the standing
            contact_id: Entity ID of the contact
            standing: Standing value from -10 to +10
            
        Returns:
            True if set successfully
        """
        from engine.components.game_components import Contacts
        
        contacts_comp = entity.get_component(Contacts)
        if not contacts_comp:
            contacts_comp = Contacts()
            entity.add_component(contacts_comp)
        
        # Clamp standing to valid range
        standing = max(-10.0, min(10.0, standing))
        
        contacts_comp.contacts[contact_id] = standing
        return True
    
    def get_standing(
        self,
        entity: Entity,
        contact_id: str
    ) -> Optional[float]:
        """
        Get standing for a contact
        
        Args:
            entity: Entity checking the standing
            contact_id: Entity ID of the contact
            
        Returns:
            Standing value or None if not set
        """
        from engine.components.game_components import Contacts
        
        contacts_comp = entity.get_component(Contacts)
        if not contacts_comp:
            return None
        
        return contacts_comp.contacts.get(contact_id)
    
    def block_contact(
        self,
        entity: Entity,
        contact_id: str
    ) -> bool:
        """
        Block a contact (prevents mail and chat)
        
        Args:
            entity: Entity blocking
            contact_id: Entity ID to block
            
        Returns:
            True if blocked successfully
        """
        from engine.components.game_components import Contacts
        
        contacts_comp = entity.get_component(Contacts)
        if not contacts_comp:
            contacts_comp = Contacts()
            entity.add_component(contacts_comp)
        
        if contact_id not in contacts_comp.blocked:
            contacts_comp.blocked.append(contact_id)
        
        return True
    
    def unblock_contact(
        self,
        entity: Entity,
        contact_id: str
    ) -> bool:
        """
        Unblock a contact
        
        Args:
            entity: Entity unblocking
            contact_id: Entity ID to unblock
            
        Returns:
            True if unblocked successfully
        """
        from engine.components.game_components import Contacts
        
        contacts_comp = entity.get_component(Contacts)
        if not contacts_comp or contact_id not in contacts_comp.blocked:
            return False
        
        contacts_comp.blocked.remove(contact_id)
        return True
    
    def is_blocked(
        self,
        entity: Entity,
        contact_id: str
    ) -> bool:
        """
        Check if a contact is blocked
        
        Args:
            entity: Entity to check
            contact_id: Contact to check
            
        Returns:
            True if blocked
        """
        from engine.components.game_components import Contacts
        
        contacts_comp = entity.get_component(Contacts)
        if not contacts_comp:
            return False
        
        return contact_id in contacts_comp.blocked
    
    # ===== MAIL SYSTEM =====
    
    def send_mail(
        self,
        sender_entity: Entity,
        recipient_ids: List[str],
        subject: str,
        body: str
    ) -> Optional[str]:
        """
        Send a mail message
        
        Args:
            sender_entity: Entity sending the mail
            recipient_ids: List of recipient entity IDs
            subject: Mail subject
            body: Mail body text
            
        Returns:
            message_id if sent successfully
        """
        from engine.components.game_components import Mail, Player
        
        # Get sender info
        player_comp = sender_entity.get_component(Player)
        if not player_comp:
            return None
        
        sender_name = player_comp.character_name
        
        # Generate message ID
        self.mail_counter += 1
        message_id = f"mail_{self.mail_counter}"
        
        # Create mail message
        mail_msg = MailMessage(
            message_id=message_id,
            sender_id=sender_entity.id,
            sender_name=sender_name,
            recipient_ids=recipient_ids,
            subject=subject,
            body=body,
            timestamp=time.time()
        )
        
        # Add to sender's sent folder
        mail_comp = sender_entity.get_component(Mail)
        if not mail_comp:
            mail_comp = Mail()
            sender_entity.add_component(mail_comp)
        
        mail_comp.sent.append(mail_msg.__dict__)
        
        # Add to recipients' inboxes
        for recipient_id in recipient_ids:
            # Check if blocked
            if self.is_blocked(self.world.get_entity(recipient_id), sender_entity.id):
                continue
            
            recipient_entity = self.world.get_entity(recipient_id)
            if not recipient_entity:
                continue
            
            recipient_mail = recipient_entity.get_component(Mail)
            if not recipient_mail:
                recipient_mail = Mail()
                recipient_entity.add_component(recipient_mail)
            
            recipient_mail.inbox.append(mail_msg.__dict__)
            recipient_mail.unread_count += 1
        
        return message_id
    
    def read_mail(
        self,
        entity: Entity,
        message_id: str
    ) -> Optional[Dict]:
        """
        Read a mail message (marks as read)
        
        Args:
            entity: Entity reading the mail
            message_id: Message ID to read
            
        Returns:
            Message dict if found
        """
        from engine.components.game_components import Mail
        
        mail_comp = entity.get_component(Mail)
        if not mail_comp:
            return None
        
        # Find message in inbox
        for msg in mail_comp.inbox:
            if msg.get('message_id') == message_id:
                if not msg.get('is_read', False):
                    msg['is_read'] = True
                    mail_comp.unread_count = max(0, mail_comp.unread_count - 1)
                return msg
        
        # Check sent folder
        for msg in mail_comp.sent:
            if msg.get('message_id') == message_id:
                return msg
        
        return None
    
    def delete_mail(
        self,
        entity: Entity,
        message_id: str
    ) -> bool:
        """
        Delete a mail message
        
        Args:
            entity: Entity deleting the mail
            message_id: Message ID to delete
            
        Returns:
            True if deleted successfully
        """
        from engine.components.game_components import Mail
        
        mail_comp = entity.get_component(Mail)
        if not mail_comp:
            return False
        
        # Check inbox
        for i, msg in enumerate(mail_comp.inbox):
            if msg.get('message_id') == message_id:
                if not msg.get('is_read', False):
                    mail_comp.unread_count = max(0, mail_comp.unread_count - 1)
                del mail_comp.inbox[i]
                return True
        
        # Check sent
        for i, msg in enumerate(mail_comp.sent):
            if msg.get('message_id') == message_id:
                del mail_comp.sent[i]
                return True
        
        return False
    
    def get_inbox(
        self,
        entity: Entity,
        limit: int = 50
    ) -> List[Dict]:
        """
        Get entity's inbox messages
        
        Args:
            entity: Entity to get inbox for
            limit: Maximum number of messages to return
            
        Returns:
            List of mail messages
        """
        from engine.components.game_components import Mail
        
        mail_comp = entity.get_component(Mail)
        if not mail_comp:
            return []
        
        # Return most recent messages first
        return list(reversed(mail_comp.inbox[-limit:]))
    
    # ===== CHAT SYSTEM =====
    
    def create_channel(
        self,
        creator_entity: Entity,
        channel_name: str,
        channel_type: str = "private",
        password: Optional[str] = None
    ) -> Optional[str]:
        """
        Create a new chat channel
        
        Args:
            creator_entity: Entity creating the channel
            channel_name: Name of the channel
            channel_type: Type (private, fleet, etc.)
            password: Optional password
            
        Returns:
            channel_id if created successfully
        """
        self.channel_counter += 1
        channel_id = f"channel_{self.channel_counter}"
        
        channel = ChatChannel(
            channel_id=channel_id,
            channel_name=channel_name,
            channel_type=channel_type,
            password=password,
            operators=[creator_entity.id]
        )
        
        self.channels[channel_id] = channel
        
        # Auto-join creator
        self.join_channel(creator_entity, channel_id, password)
        
        return channel_id
    
    def join_channel(
        self,
        entity: Entity,
        channel_id: str,
        password: Optional[str] = None
    ) -> bool:
        """
        Join a chat channel
        
        Args:
            entity: Entity joining
            channel_id: Channel ID to join
            password: Password if required
            
        Returns:
            True if joined successfully
        """
        from engine.components.game_components import Chat
        
        channel = self.channels.get(channel_id)
        if not channel:
            return False
        
        # Check password
        if channel.password and channel.password != password:
            return False
        
        # Add to channel members
        if entity.id not in channel.members:
            channel.members.append(entity.id)
        
        # Update entity's chat component
        chat_comp = entity.get_component(Chat)
        if not chat_comp:
            chat_comp = Chat()
            entity.add_component(chat_comp)
        
        if channel_id not in chat_comp.active_channels:
            chat_comp.active_channels.append(channel_id)
        
        return True
    
    def leave_channel(
        self,
        entity: Entity,
        channel_id: str
    ) -> bool:
        """
        Leave a chat channel
        
        Args:
            entity: Entity leaving
            channel_id: Channel ID to leave
            
        Returns:
            True if left successfully
        """
        from engine.components.game_components import Chat
        
        channel = self.channels.get(channel_id)
        if not channel:
            return False
        
        # Remove from channel members
        if entity.id in channel.members:
            channel.members.remove(entity.id)
        
        # Update entity's chat component
        chat_comp = entity.get_component(Chat)
        if chat_comp and channel_id in chat_comp.active_channels:
            chat_comp.active_channels.remove(channel_id)
        
        return True
    
    def send_chat_message(
        self,
        sender_entity: Entity,
        channel_id: str,
        message: str
    ) -> Optional[str]:
        """
        Send a chat message to a channel
        
        Args:
            sender_entity: Entity sending the message
            channel_id: Channel ID
            message: Message text
            
        Returns:
            message_id if sent successfully
        """
        from engine.components.game_components import Chat, Player
        
        channel = self.channels.get(channel_id)
        if not channel:
            return None
        
        # Check if member
        if sender_entity.id not in channel.members:
            return None
        
        # Get sender name
        player_comp = sender_entity.get_component(Player)
        sender_name = player_comp.character_name if player_comp else "Unknown"
        
        # Create message
        self.message_counter += 1
        message_id = f"msg_{self.message_counter}"
        
        chat_msg = ChatMessage(
            message_id=message_id,
            sender_id=sender_entity.id,
            sender_name=sender_name,
            channel_id=channel_id,
            message=message,
            timestamp=time.time()
        )
        
        # Add to channel history
        channel.message_history.append(chat_msg)
        
        # Keep history limited
        if len(channel.message_history) > channel.max_history:
            channel.message_history = channel.message_history[-channel.max_history:]
        
        # Update chat component for all members
        for member_id in channel.members:
            if member_id == sender_entity.id:
                continue  # Skip sender
            
            member_entity = self.world.get_entity(member_id)
            if not member_entity:
                continue
            
            # Check if sender is muted by this member
            chat_comp = member_entity.get_component(Chat)
            if chat_comp and sender_entity.id in chat_comp.muted_characters:
                continue
            
            # Add message to member's chat history
            if not chat_comp:
                chat_comp = Chat()
                member_entity.add_component(chat_comp)
            
            if channel_id not in chat_comp.channels:
                chat_comp.channels[channel_id] = []
            
            chat_comp.channels[channel_id].append(chat_msg.__dict__)
            
            # Keep history limited per member
            if len(chat_comp.channels[channel_id]) > channel.max_history:
                chat_comp.channels[channel_id] = chat_comp.channels[channel_id][-channel.max_history:]
        
        return message_id
    
    def get_channel_history(
        self,
        entity: Entity,
        channel_id: str,
        limit: int = 50
    ) -> List[Dict]:
        """
        Get channel message history
        
        Args:
            entity: Entity requesting history
            channel_id: Channel ID
            limit: Maximum messages to return
            
        Returns:
            List of chat messages
        """
        channel = self.channels.get(channel_id)
        if not channel or entity.id not in channel.members:
            return []
        
        # Return most recent messages
        messages = [msg.__dict__ for msg in channel.message_history[-limit:]]
        return messages
    
    def mute_character(
        self,
        entity: Entity,
        character_id: str
    ) -> bool:
        """
        Mute a character in chat
        
        Args:
            entity: Entity muting
            character_id: Character to mute
            
        Returns:
            True if muted successfully
        """
        from engine.components.game_components import Chat
        
        chat_comp = entity.get_component(Chat)
        if not chat_comp:
            chat_comp = Chat()
            entity.add_component(chat_comp)
        
        if character_id not in chat_comp.muted_characters:
            chat_comp.muted_characters.append(character_id)
        
        return True
    
    def unmute_character(
        self,
        entity: Entity,
        character_id: str
    ) -> bool:
        """
        Unmute a character in chat
        
        Args:
            entity: Entity unmuting
            character_id: Character to unmute
            
        Returns:
            True if unmuted successfully
        """
        from engine.components.game_components import Chat
        
        chat_comp = entity.get_component(Chat)
        if not chat_comp or character_id not in chat_comp.muted_characters:
            return False
        
        chat_comp.muted_characters.remove(character_id)
        return True
    
    def get_channel(self, channel_id: str) -> Optional[ChatChannel]:
        """Get channel by ID"""
        return self.channels.get(channel_id)
    
    def list_channels(self, entity: Entity) -> List[ChatChannel]:
        """List all channels the entity is a member of"""
        from engine.components.game_components import Chat
        
        chat_comp = entity.get_component(Chat)
        if not chat_comp:
            return []
        
        channels = []
        for channel_id in chat_comp.active_channels:
            channel = self.channels.get(channel_id)
            if channel:
                channels.append(channel)
        
        return channels
