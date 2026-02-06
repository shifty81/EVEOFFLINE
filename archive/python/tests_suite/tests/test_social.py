"""
Tests for Social System
Tests contacts, standings, mail, and chat functionality
"""

import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import unittest
from engine.core.ecs import World
from engine.components.game_components import Player, Contacts, Mail, Chat
from engine.systems.social_system import SocialSystem


class TestSocialSystem(unittest.TestCase):
    """Test social system functionality"""
    
    def setUp(self):
        """Set up test fixtures"""
        self.world = World()
        self.social_system = SocialSystem(self.world)
    
    def test_init_system_channels(self):
        """Test that system channels are initialized"""
        self.assertIn("local", self.social_system.channels)
        self.assertIn("help", self.social_system.channels)
        self.assertIn("rookie", self.social_system.channels)
        
        local_channel = self.social_system.channels["local"]
        self.assertEqual(local_channel.channel_name, "Local")
        self.assertEqual(local_channel.channel_type, "local")
    
    # ===== CONTACTS & STANDINGS TESTS =====
    
    def test_add_contact(self):
        """Test adding a contact"""
        entity = self.world.create_entity()
        player = Player(character_name="Test Player")
        entity.add_component(player)
        
        result = self.social_system.add_contact(entity, "contact_1", 5.0)
        self.assertTrue(result)
        
        contacts_comp = entity.get_component(Contacts)
        self.assertIsNotNone(contacts_comp)
        self.assertEqual(contacts_comp.contacts["contact_1"], 5.0)
    
    def test_add_contact_clamp_standing(self):
        """Test that standings are clamped to valid range"""
        entity = self.world.create_entity()
        player = Player(character_name="Test Player")
        entity.add_component(player)
        
        # Too high
        self.social_system.add_contact(entity, "contact_1", 15.0)
        contacts_comp = entity.get_component(Contacts)
        self.assertEqual(contacts_comp.contacts["contact_1"], 10.0)
        
        # Too low
        self.social_system.add_contact(entity, "contact_2", -15.0)
        self.assertEqual(contacts_comp.contacts["contact_2"], -10.0)
    
    def test_remove_contact(self):
        """Test removing a contact"""
        entity = self.world.create_entity()
        player = Player(character_name="Test Player")
        entity.add_component(player)
        
        self.social_system.add_contact(entity, "contact_1", 5.0)
        result = self.social_system.remove_contact(entity, "contact_1")
        self.assertTrue(result)
        
        contacts_comp = entity.get_component(Contacts)
        self.assertNotIn("contact_1", contacts_comp.contacts)
    
    def test_set_standing(self):
        """Test setting/updating standings"""
        entity = self.world.create_entity()
        player = Player(character_name="Test Player")
        entity.add_component(player)
        
        self.social_system.add_contact(entity, "contact_1", 5.0)
        result = self.social_system.set_standing(entity, "contact_1", -3.0)
        self.assertTrue(result)
        
        contacts_comp = entity.get_component(Contacts)
        self.assertEqual(contacts_comp.contacts["contact_1"], -3.0)
    
    def test_get_standing(self):
        """Test getting standing for a contact"""
        entity = self.world.create_entity()
        player = Player(character_name="Test Player")
        entity.add_component(player)
        
        self.social_system.add_contact(entity, "contact_1", 7.5)
        standing = self.social_system.get_standing(entity, "contact_1")
        self.assertEqual(standing, 7.5)
        
        # Non-existent contact
        standing = self.social_system.get_standing(entity, "contact_2")
        self.assertIsNone(standing)
    
    def test_block_contact(self):
        """Test blocking a contact"""
        entity = self.world.create_entity()
        player = Player(character_name="Test Player")
        entity.add_component(player)
        
        result = self.social_system.block_contact(entity, "contact_1")
        self.assertTrue(result)
        
        contacts_comp = entity.get_component(Contacts)
        self.assertIn("contact_1", contacts_comp.blocked)
    
    def test_unblock_contact(self):
        """Test unblocking a contact"""
        entity = self.world.create_entity()
        player = Player(character_name="Test Player")
        entity.add_component(player)
        
        self.social_system.block_contact(entity, "contact_1")
        result = self.social_system.unblock_contact(entity, "contact_1")
        self.assertTrue(result)
        
        contacts_comp = entity.get_component(Contacts)
        self.assertNotIn("contact_1", contacts_comp.blocked)
    
    def test_is_blocked(self):
        """Test checking if a contact is blocked"""
        entity = self.world.create_entity()
        player = Player(character_name="Test Player")
        entity.add_component(player)
        
        self.assertFalse(self.social_system.is_blocked(entity, "contact_1"))
        
        self.social_system.block_contact(entity, "contact_1")
        self.assertTrue(self.social_system.is_blocked(entity, "contact_1"))
    
    # ===== MAIL SYSTEM TESTS =====
    
    def test_send_mail(self):
        """Test sending mail"""
        sender = self.world.create_entity()
        player_sender = Player(character_name="Sender")
        sender.add_component(player_sender)
        
        recipient = self.world.create_entity()
        player_recipient = Player(character_name="Recipient")
        recipient.add_component(player_recipient)
        
        message_id = self.social_system.send_mail(
            sender,
            [recipient.id],
            "Test Subject",
            "Test mail body"
        )
        
        self.assertIsNotNone(message_id)
        
        # Check sender's sent folder
        mail_comp_sender = sender.get_component(Mail)
        self.assertEqual(len(mail_comp_sender.sent), 1)
        self.assertEqual(mail_comp_sender.sent[0]['subject'], "Test Subject")
        
        # Check recipient's inbox
        mail_comp_recipient = recipient.get_component(Mail)
        self.assertEqual(len(mail_comp_recipient.inbox), 1)
        self.assertEqual(mail_comp_recipient.unread_count, 1)
        self.assertEqual(mail_comp_recipient.inbox[0]['subject'], "Test Subject")
    
    def test_send_mail_blocked(self):
        """Test that blocked senders cannot send mail"""
        sender = self.world.create_entity()
        player_sender = Player(character_name="Sender")
        sender.add_component(player_sender)
        
        recipient = self.world.create_entity()
        player_recipient = Player(character_name="Recipient")
        recipient.add_component(player_recipient)
        
        # Block sender
        self.social_system.block_contact(recipient, sender.id)
        
        # Try to send mail
        message_id = self.social_system.send_mail(
            sender,
            [recipient.id],
            "Test Subject",
            "Test body"
        )
        
        # Mail sent but not delivered to blocked recipient
        self.assertIsNotNone(message_id)
        mail_comp_recipient = recipient.get_component(Mail)
        # No mail component created if no mail was delivered
        if mail_comp_recipient:
            self.assertEqual(len(mail_comp_recipient.inbox), 0)
        else:
            # This is also valid - no component means no mail
            pass
    
    def test_send_mail_multiple_recipients(self):
        """Test sending mail to multiple recipients"""
        sender = self.world.create_entity()
        player_sender = Player(character_name="Sender")
        sender.add_component(player_sender)
        
        recipient1 = self.world.create_entity()
        player_r1 = Player(character_name="Recipient1")
        recipient1.add_component(player_r1)
        
        recipient2 = self.world.create_entity()
        player_r2 = Player(character_name="Recipient2")
        recipient2.add_component(player_r2)
        
        message_id = self.social_system.send_mail(
            sender,
            [recipient1.id, recipient2.id],
            "Test Subject",
            "Test body"
        )
        
        # Check both recipients received mail
        mail_comp_r1 = recipient1.get_component(Mail)
        mail_comp_r2 = recipient2.get_component(Mail)
        self.assertEqual(len(mail_comp_r1.inbox), 1)
        self.assertEqual(len(mail_comp_r2.inbox), 1)
    
    def test_read_mail(self):
        """Test reading mail"""
        sender = self.world.create_entity()
        player_sender = Player(character_name="Sender")
        sender.add_component(player_sender)
        
        recipient = self.world.create_entity()
        player_recipient = Player(character_name="Recipient")
        recipient.add_component(player_recipient)
        
        message_id = self.social_system.send_mail(
            sender,
            [recipient.id],
            "Test Subject",
            "Test body"
        )
        
        # Read mail
        message = self.social_system.read_mail(recipient, message_id)
        self.assertIsNotNone(message)
        self.assertTrue(message['is_read'])
        
        # Check unread count decreased
        mail_comp = recipient.get_component(Mail)
        self.assertEqual(mail_comp.unread_count, 0)
    
    def test_delete_mail(self):
        """Test deleting mail"""
        sender = self.world.create_entity()
        player_sender = Player(character_name="Sender")
        sender.add_component(player_sender)
        
        recipient = self.world.create_entity()
        player_recipient = Player(character_name="Recipient")
        recipient.add_component(player_recipient)
        
        message_id = self.social_system.send_mail(
            sender,
            [recipient.id],
            "Test Subject",
            "Test body"
        )
        
        result = self.social_system.delete_mail(recipient, message_id)
        self.assertTrue(result)
        
        mail_comp = recipient.get_component(Mail)
        self.assertEqual(len(mail_comp.inbox), 0)
    
    def test_get_inbox(self):
        """Test getting inbox messages"""
        sender = self.world.create_entity()
        player_sender = Player(character_name="Sender")
        sender.add_component(player_sender)
        
        recipient = self.world.create_entity()
        player_recipient = Player(character_name="Recipient")
        recipient.add_component(player_recipient)
        
        # Send multiple mails
        for i in range(3):
            self.social_system.send_mail(
                sender,
                [recipient.id],
                f"Subject {i}",
                f"Body {i}"
            )
        
        inbox = self.social_system.get_inbox(recipient)
        self.assertEqual(len(inbox), 3)
        # Most recent first
        self.assertEqual(inbox[0]['subject'], "Subject 2")
    
    # ===== CHAT SYSTEM TESTS =====
    
    def test_create_channel(self):
        """Test creating a chat channel"""
        creator = self.world.create_entity()
        player = Player(character_name="Creator")
        creator.add_component(player)
        
        channel_id = self.social_system.create_channel(
            creator,
            "Test Channel",
            "private"
        )
        
        self.assertIsNotNone(channel_id)
        self.assertIn(channel_id, self.social_system.channels)
        
        channel = self.social_system.channels[channel_id]
        self.assertEqual(channel.channel_name, "Test Channel")
        self.assertIn(creator.id, channel.members)
        self.assertIn(creator.id, channel.operators)
    
    def test_join_channel(self):
        """Test joining a chat channel"""
        creator = self.world.create_entity()
        player_creator = Player(character_name="Creator")
        creator.add_component(player_creator)
        
        joiner = self.world.create_entity()
        player_joiner = Player(character_name="Joiner")
        joiner.add_component(player_joiner)
        
        channel_id = self.social_system.create_channel(creator, "Test Channel", "private")
        
        result = self.social_system.join_channel(joiner, channel_id)
        self.assertTrue(result)
        
        channel = self.social_system.channels[channel_id]
        self.assertIn(joiner.id, channel.members)
        
        chat_comp = joiner.get_component(Chat)
        self.assertIn(channel_id, chat_comp.active_channels)
    
    def test_join_channel_with_password(self):
        """Test joining a password-protected channel"""
        creator = self.world.create_entity()
        player_creator = Player(character_name="Creator")
        creator.add_component(player_creator)
        
        joiner = self.world.create_entity()
        player_joiner = Player(character_name="Joiner")
        joiner.add_component(player_joiner)
        
        channel_id = self.social_system.create_channel(
            creator,
            "Secret Channel",
            "private",
            password="secret123"
        )
        
        # Try without password
        result = self.social_system.join_channel(joiner, channel_id)
        self.assertFalse(result)
        
        # Try with correct password
        result = self.social_system.join_channel(joiner, channel_id, password="secret123")
        self.assertTrue(result)
    
    def test_leave_channel(self):
        """Test leaving a chat channel"""
        entity = self.world.create_entity()
        player = Player(character_name="Test Player")
        entity.add_component(player)
        
        channel_id = self.social_system.create_channel(entity, "Test Channel", "private")
        
        result = self.social_system.leave_channel(entity, channel_id)
        self.assertTrue(result)
        
        channel = self.social_system.channels[channel_id]
        self.assertNotIn(entity.id, channel.members)
    
    def test_send_chat_message(self):
        """Test sending a chat message"""
        sender = self.world.create_entity()
        player_sender = Player(character_name="Sender")
        sender.add_component(player_sender)
        
        recipient = self.world.create_entity()
        player_recipient = Player(character_name="Recipient")
        recipient.add_component(player_recipient)
        
        channel_id = self.social_system.create_channel(sender, "Test Channel", "private")
        self.social_system.join_channel(recipient, channel_id)
        
        message_id = self.social_system.send_chat_message(
            sender,
            channel_id,
            "Hello, world!"
        )
        
        self.assertIsNotNone(message_id)
        
        # Check channel history
        channel = self.social_system.channels[channel_id]
        self.assertEqual(len(channel.message_history), 1)
        self.assertEqual(channel.message_history[0].message, "Hello, world!")
    
    def test_chat_message_muted(self):
        """Test that muted users' messages are not delivered"""
        sender = self.world.create_entity()
        player_sender = Player(character_name="Sender")
        sender.add_component(player_sender)
        
        recipient = self.world.create_entity()
        player_recipient = Player(character_name="Recipient")
        recipient.add_component(player_recipient)
        
        channel_id = self.social_system.create_channel(sender, "Test Channel", "private")
        self.social_system.join_channel(recipient, channel_id)
        
        # Mute sender
        self.social_system.mute_character(recipient, sender.id)
        
        # Send message
        self.social_system.send_chat_message(sender, channel_id, "You won't see this")
        
        # Check recipient's chat history (should be empty)
        chat_comp = recipient.get_component(Chat)
        self.assertEqual(len(chat_comp.channels.get(channel_id, [])), 0)
    
    def test_get_channel_history(self):
        """Test getting channel message history"""
        sender = self.world.create_entity()
        player_sender = Player(character_name="Sender")
        sender.add_component(player_sender)
        
        channel_id = self.social_system.create_channel(sender, "Test Channel", "private")
        
        # Send messages
        for i in range(3):
            self.social_system.send_chat_message(sender, channel_id, f"Message {i}")
        
        history = self.social_system.get_channel_history(sender, channel_id)
        self.assertEqual(len(history), 3)
        self.assertEqual(history[0]['message'], "Message 0")
    
    def test_mute_unmute_character(self):
        """Test muting and unmuting characters"""
        entity = self.world.create_entity()
        player = Player(character_name="Test Player")
        entity.add_component(player)
        
        result = self.social_system.mute_character(entity, "annoying_player")
        self.assertTrue(result)
        
        chat_comp = entity.get_component(Chat)
        self.assertIn("annoying_player", chat_comp.muted_characters)
        
        result = self.social_system.unmute_character(entity, "annoying_player")
        self.assertTrue(result)
        self.assertNotIn("annoying_player", chat_comp.muted_characters)
    
    def test_list_channels(self):
        """Test listing channels for an entity"""
        entity = self.world.create_entity()
        player = Player(character_name="Test Player")
        entity.add_component(player)
        
        # Join multiple channels
        channel_id1 = self.social_system.create_channel(entity, "Channel 1", "private")
        channel_id2 = self.social_system.create_channel(entity, "Channel 2", "private")
        
        channels = self.social_system.list_channels(entity)
        self.assertEqual(len(channels), 2)
        self.assertEqual(channels[0].channel_name, "Channel 1")
        self.assertEqual(channels[1].channel_name, "Channel 2")


if __name__ == '__main__':
    unittest.main()
