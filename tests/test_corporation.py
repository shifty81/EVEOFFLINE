"""
Tests for Corporation System
Tests corporation management, member roles, hangars, and wallets
"""

import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import unittest
from engine.core.ecs import World
from engine.components.game_components import Player, CorporationMembership, Inventory
from engine.systems.corporation_system import CorporationSystem, CorporationRole


class TestCorporationSystem(unittest.TestCase):
    """Test corporation system functionality"""
    
    def setUp(self):
        """Set up test fixtures"""
        self.world = World()
        self.corp_system = CorporationSystem(self.world)
    
    def test_init_npc_corps(self):
        """Test that NPC corporations are initialized"""
        self.assertIn("npc_corp_caldari", self.corp_system.corporations)
        self.assertIn("npc_corp_gallente", self.corp_system.corporations)
        self.assertIn("npc_corp_amarr", self.corp_system.corporations)
        self.assertIn("npc_corp_minmatar", self.corp_system.corporations)
        
        caldari_corp = self.corp_system.corporations["npc_corp_caldari"]
        self.assertEqual(caldari_corp.corporation_name, "Caldari Navy")
        self.assertEqual(caldari_corp.ticker, "CLDN")
        self.assertEqual(caldari_corp.tax_rate, 0.11)
    
    def test_create_corporation(self):
        """Test creating a new corporation"""
        # Create CEO entity
        ceo = self.world.create_entity()
        player = Player(character_name="Test CEO", isk=2000000.0)
        ceo.add_component(player)
        
        # Create corporation
        corp_id = self.corp_system.create_corporation(
            ceo,
            "Test Corporation",
            "TEST",
            "A test corporation",
            creation_cost=1000000.0
        )
        
        self.assertIsNotNone(corp_id)
        self.assertIn(corp_id, self.corp_system.corporations)
        
        # Check corp details
        corp = self.corp_system.corporations[corp_id]
        self.assertEqual(corp.corporation_name, "Test Corporation")
        self.assertEqual(corp.ticker, "TEST")
        self.assertEqual(corp.ceo_id, ceo.id)
        self.assertEqual(len(corp.members), 1)
        self.assertIn(ceo.id, corp.members)
        
        # Check CEO is a member
        member = corp.members[ceo.id]
        self.assertEqual(member.role, CorporationRole.CEO)
        
        # Check ISK was charged
        self.assertEqual(player.isk, 1000000.0)
        
        # Check CEO has membership component
        membership = ceo.get_component(CorporationMembership)
        self.assertIsNotNone(membership)
        self.assertEqual(membership.corporation_id, corp_id)
        self.assertEqual(membership.role, "ceo")
    
    def test_create_corporation_insufficient_isk(self):
        """Test that corporation creation fails with insufficient ISK"""
        ceo = self.world.create_entity()
        player = Player(character_name="Poor CEO", isk=500000.0)
        ceo.add_component(player)
        
        corp_id = self.corp_system.create_corporation(
            ceo,
            "Test Corp",
            "TEST",
            "",
            creation_cost=1000000.0
        )
        
        self.assertIsNone(corp_id)
        self.assertEqual(player.isk, 500000.0)
    
    def test_create_corporation_invalid_ticker(self):
        """Test that corporation creation fails with invalid ticker"""
        ceo = self.world.create_entity()
        player = Player(character_name="Test CEO", isk=2000000.0)
        ceo.add_component(player)
        
        # Too short
        corp_id = self.corp_system.create_corporation(ceo, "Test Corp", "AB", "")
        self.assertIsNone(corp_id)
        
        # Too long
        corp_id = self.corp_system.create_corporation(ceo, "Test Corp", "ABCDEF", "")
        self.assertIsNone(corp_id)
    
    def test_add_member(self):
        """Test adding members to a corporation"""
        # Create corporation
        ceo = self.world.create_entity()
        player_ceo = Player(character_name="CEO", isk=2000000.0)
        ceo.add_component(player_ceo)
        
        corp_id = self.corp_system.create_corporation(ceo, "Test Corp", "TEST", "")
        
        # Create new member
        member = self.world.create_entity()
        player_member = Player(character_name="New Member", isk=100000.0)
        member.add_component(player_member)
        
        # Add member
        result = self.corp_system.add_member(corp_id, member)
        self.assertTrue(result)
        
        # Check member was added
        corp = self.corp_system.corporations[corp_id]
        self.assertEqual(len(corp.members), 2)
        self.assertIn(member.id, corp.members)
        
        member_data = corp.members[member.id]
        self.assertEqual(member_data.role, CorporationRole.MEMBER)
        self.assertEqual(member_data.character_name, "New Member")
        
        # Check membership component
        membership = member.get_component(CorporationMembership)
        self.assertIsNotNone(membership)
        self.assertEqual(membership.corporation_id, corp_id)
    
    def test_remove_member(self):
        """Test removing members from a corporation"""
        # Create corporation with members
        ceo = self.world.create_entity()
        player_ceo = Player(character_name="CEO", isk=2000000.0)
        ceo.add_component(player_ceo)
        
        corp_id = self.corp_system.create_corporation(ceo, "Test Corp", "TEST", "")
        
        member = self.world.create_entity()
        player_member = Player(character_name="Member", isk=100000.0)
        member.add_component(player_member)
        self.corp_system.add_member(corp_id, member)
        
        # Remove member
        result = self.corp_system.remove_member(corp_id, member.id)
        self.assertTrue(result)
        
        # Check member was removed
        corp = self.corp_system.corporations[corp_id]
        self.assertNotIn(member.id, corp.members)
        
        # Check membership updated to NPC corp
        membership = member.get_component(CorporationMembership)
        self.assertEqual(membership.corporation_id, "npc_corp_caldari")
    
    def test_cannot_remove_ceo(self):
        """Test that CEO cannot be removed"""
        ceo = self.world.create_entity()
        player_ceo = Player(character_name="CEO", isk=2000000.0)
        ceo.add_component(player_ceo)
        
        corp_id = self.corp_system.create_corporation(ceo, "Test Corp", "TEST", "")
        
        result = self.corp_system.remove_member(corp_id, ceo.id)
        self.assertFalse(result)
    
    def test_set_member_role(self):
        """Test changing member roles"""
        # Create corporation with members
        ceo = self.world.create_entity()
        player_ceo = Player(character_name="CEO", isk=2000000.0)
        ceo.add_component(player_ceo)
        
        corp_id = self.corp_system.create_corporation(ceo, "Test Corp", "TEST", "")
        
        member = self.world.create_entity()
        player_member = Player(character_name="Member", isk=100000.0)
        member.add_component(player_member)
        self.corp_system.add_member(corp_id, member)
        
        # Set member to director
        result = self.corp_system.set_member_role(
            corp_id,
            member.id,
            CorporationRole.DIRECTOR,
            ceo
        )
        self.assertTrue(result)
        
        # Check role was changed
        corp = self.corp_system.corporations[corp_id]
        member_data = corp.members[member.id]
        self.assertEqual(member_data.role, CorporationRole.DIRECTOR)
        
        # Check membership component
        membership = member.get_component(CorporationMembership)
        self.assertEqual(membership.role, "director")
    
    def test_only_ceo_can_set_roles(self):
        """Test that only CEO can change roles"""
        ceo = self.world.create_entity()
        player_ceo = Player(character_name="CEO", isk=2000000.0)
        ceo.add_component(player_ceo)
        
        corp_id = self.corp_system.create_corporation(ceo, "Test Corp", "TEST", "")
        
        member1 = self.world.create_entity()
        player_member1 = Player(character_name="Member1", isk=100000.0)
        member1.add_component(player_member1)
        self.corp_system.add_member(corp_id, member1)
        
        member2 = self.world.create_entity()
        player_member2 = Player(character_name="Member2", isk=100000.0)
        member2.add_component(player_member2)
        self.corp_system.add_member(corp_id, member2)
        
        # Member1 tries to change Member2's role
        result = self.corp_system.set_member_role(
            corp_id,
            member2.id,
            CorporationRole.DIRECTOR,
            member1
        )
        self.assertFalse(result)
    
    def test_wallet_deposit(self):
        """Test depositing ISK to corporation wallet"""
        ceo = self.world.create_entity()
        player_ceo = Player(character_name="CEO", isk=2000000.0)
        ceo.add_component(player_ceo)
        
        corp_id = self.corp_system.create_corporation(ceo, "Test Corp", "TEST", "")
        
        # Deposit ISK
        result = self.corp_system.deposit_to_wallet(corp_id, ceo, 500000.0)
        self.assertTrue(result)
        
        # Check balances
        corp = self.corp_system.corporations[corp_id]
        self.assertEqual(corp.wallet_balance, 500000.0)
        self.assertEqual(player_ceo.isk, 500000.0)  # 2M - 1M creation - 500k deposit
    
    def test_wallet_withdraw(self):
        """Test withdrawing ISK from corporation wallet"""
        ceo = self.world.create_entity()
        player_ceo = Player(character_name="CEO", isk=2000000.0)
        ceo.add_component(player_ceo)
        
        corp_id = self.corp_system.create_corporation(ceo, "Test Corp", "TEST", "")
        
        # Deposit then withdraw
        self.corp_system.deposit_to_wallet(corp_id, ceo, 500000.0)
        result = self.corp_system.withdraw_from_wallet(corp_id, ceo, 200000.0)
        self.assertTrue(result)
        
        # Check balances
        corp = self.corp_system.corporations[corp_id]
        self.assertEqual(corp.wallet_balance, 300000.0)
        self.assertEqual(player_ceo.isk, 700000.0)  # 2M - 1M creation - 500k + 200k
    
    def test_wallet_permissions(self):
        """Test that only authorized roles can withdraw from wallet"""
        ceo = self.world.create_entity()
        player_ceo = Player(character_name="CEO", isk=2000000.0)
        ceo.add_component(player_ceo)
        
        corp_id = self.corp_system.create_corporation(ceo, "Test Corp", "TEST", "")
        self.corp_system.deposit_to_wallet(corp_id, ceo, 500000.0)
        
        # Add regular member
        member = self.world.create_entity()
        player_member = Player(character_name="Member", isk=100000.0)
        member.add_component(player_member)
        self.corp_system.add_member(corp_id, member)
        
        # Member tries to withdraw
        result = self.corp_system.withdraw_from_wallet(corp_id, member, 100000.0)
        self.assertFalse(result)
    
    def test_hangar_add_remove(self):
        """Test adding and removing items from corporation hangars"""
        ceo = self.world.create_entity()
        player_ceo = Player(character_name="CEO", isk=2000000.0)
        ceo.add_component(player_ceo)
        
        corp_id = self.corp_system.create_corporation(ceo, "Test Corp", "TEST", "")
        
        # Add items to hangar
        result = self.corp_system.add_to_hangar(corp_id, "Division 1", "item_rifter", 5)
        self.assertTrue(result)
        
        corp = self.corp_system.corporations[corp_id]
        self.assertEqual(corp.hangars["Division 1"]["item_rifter"], 5)
        
        # Remove items from hangar
        result = self.corp_system.remove_from_hangar(
            corp_id,
            "Division 1",
            ceo,
            "item_rifter",
            3
        )
        self.assertTrue(result)
        self.assertEqual(corp.hangars["Division 1"]["item_rifter"], 2)
    
    def test_hangar_permissions(self):
        """Test hangar access permissions"""
        ceo = self.world.create_entity()
        player_ceo = Player(character_name="CEO", isk=2000000.0)
        ceo.add_component(player_ceo)
        
        corp_id = self.corp_system.create_corporation(ceo, "Test Corp", "TEST", "")
        self.corp_system.add_to_hangar(corp_id, "Division 3", "item_rifter", 5)
        
        # Add regular member
        member = self.world.create_entity()
        player_member = Player(character_name="Member", isk=100000.0)
        member.add_component(player_member)
        self.corp_system.add_member(corp_id, member)
        
        # Member tries to access CEO-only division
        result = self.corp_system.remove_from_hangar(
            corp_id,
            "Division 3",
            member,
            "item_rifter",
            1
        )
        self.assertFalse(result)
    
    def test_apply_tax(self):
        """Test applying corporation tax to earnings"""
        ceo = self.world.create_entity()
        player_ceo = Player(character_name="CEO", isk=2000000.0)
        ceo.add_component(player_ceo)
        
        corp_id = self.corp_system.create_corporation(ceo, "Test Corp", "TEST", "")
        corp = self.corp_system.corporations[corp_id]
        corp.tax_rate = 0.10  # 10% tax
        
        # Apply tax to earnings
        earnings = 100000.0
        after_tax = self.corp_system.apply_tax(ceo, earnings)
        
        self.assertEqual(after_tax, 90000.0)  # 90% of earnings
        self.assertEqual(corp.wallet_balance, 10000.0)  # 10% goes to corp


if __name__ == '__main__':
    unittest.main()
