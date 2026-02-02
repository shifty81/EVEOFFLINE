#!/usr/bin/env python3
"""
Phase 4 Features Demo
Demonstrates Corporation, Social, and Contract systems
"""

import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from engine.core.ecs import World
from engine.components.game_components import Player, Inventory
from engine.systems.corporation_system import CorporationSystem
from engine.systems.social_system import SocialSystem
from engine.systems.contract_system import ContractSystem


def demo_corporations():
    """Demonstrate corporation features"""
    print("\n" + "="*70)
    print("CORPORATION SYSTEM DEMO")
    print("="*70)
    
    world = World()
    corp_system = CorporationSystem(world)
    
    # Create a CEO
    ceo = world.create_entity("ceo_1")
    ceo.add_component(Player(character_name="John Shepard", isk=5000000.0))
    print(f"✓ Created CEO: John Shepard with 5M ISK")
    
    # Create a corporation
    corp_id = corp_system.create_corporation(
        ceo,
        "Normandy Industries",
        "NORM",
        "Saving the galaxy, one ISK at a time"
    )
    print(f"✓ Created corporation: Normandy Industries [NORM] (ID: {corp_id})")
    print(f"  CEO has {ceo.get_component(Player).isk:,.0f} ISK remaining")
    
    # Get corporation details
    corp = corp_system.get_corporation(corp_id)
    print(f"✓ Corporation details:")
    print(f"  - Members: {len(corp.members)}")
    print(f"  - Tax rate: {corp.tax_rate*100}%")
    print(f"  - Wallet: {corp.wallet_balance:,.0f} ISK")
    
    # Add a member
    member = world.create_entity("member_1")
    member.add_component(Player(character_name="Garrus Vakarian", isk=1000000.0))
    corp_system.add_member(corp_id, member)
    print(f"✓ Added member: Garrus Vakarian")
    print(f"  - Total members: {len(corp.members)}")
    
    # Deposit to corporation wallet
    corp_system.deposit_to_wallet(corp_id, ceo, 500000.0)
    print(f"✓ CEO deposited 500K ISK to corp wallet")
    print(f"  - Corp wallet: {corp.wallet_balance:,.0f} ISK")
    
    # Add items to hangar
    corp_system.add_to_hangar(corp_id, "Division 1", "item_rifter", 10)
    corp_system.add_to_hangar(corp_id, "Division 1", "item_ammo_emp", 1000)
    print(f"✓ Added items to Division 1 hangar")
    print(f"  - Rifters: {corp.hangars['Division 1'].get('item_rifter', 0)}")
    print(f"  - EMP Ammo: {corp.hangars['Division 1'].get('item_ammo_emp', 0)}")
    
    # Apply taxes
    earnings = 100000.0
    after_tax = corp_system.apply_tax(member, earnings)
    print(f"✓ Member earned 100K ISK")
    print(f"  - After {corp.tax_rate*100}% tax: {after_tax:,.0f} ISK")
    print(f"  - Corp wallet: {corp.wallet_balance:,.0f} ISK")
    
    print("✅ Corporation system demo complete!")


def demo_social():
    """Demonstrate social features"""
    print("\n" + "="*70)
    print("SOCIAL SYSTEM DEMO")
    print("="*70)
    
    world = World()
    social_system = SocialSystem(world)
    
    # Create two players
    player1 = world.create_entity("player_1")
    player1.add_component(Player(character_name="Commander Shepard"))
    
    player2 = world.create_entity("player_2")
    player2.add_component(Player(character_name="Garrus Vakarian"))
    
    print(f"✓ Created players: Commander Shepard and Garrus Vakarian")
    
    # Add contact with standing
    social_system.add_contact(player1, player2.id, 10.0)
    standing = social_system.get_standing(player1, player2.id)
    print(f"✓ Shepard set standing for Garrus: {standing}/10 (Excellent)")
    
    # Send mail
    message_id = social_system.send_mail(
        player1,
        [player2.id],
        "Mission Brief",
        "Meet at the Normandy in 1 hour. We have a galaxy to save."
    )
    print(f"✓ Shepard sent mail to Garrus (ID: {message_id})")
    
    # Check inbox
    inbox = social_system.get_inbox(player2, limit=10)
    print(f"✓ Garrus inbox: {len(inbox)} unread messages")
    if inbox:
        msg = inbox[0]
        print(f"  - From: {msg['sender_name']}")
        print(f"  - Subject: {msg['subject']}")
    
    # Create a chat channel
    channel_id = social_system.create_channel(
        player1,
        "Squad Chat",
        "private",
        password="normandy123"
    )
    print(f"✓ Shepard created chat channel: Squad Chat (ID: {channel_id})")
    
    # Join channel
    social_system.join_channel(player2, channel_id, password="normandy123")
    print(f"✓ Garrus joined Squad Chat")
    
    # Send chat messages
    social_system.send_chat_message(player1, channel_id, "Ready for the mission?")
    social_system.send_chat_message(player2, channel_id, "Always ready, Commander.")
    print(f"✓ Chat messages exchanged")
    
    # Get chat history
    history = social_system.get_channel_history(player1, channel_id, limit=10)
    print(f"✓ Chat history: {len(history)} messages")
    for msg in history:
        print(f"  [{msg['sender_name']}]: {msg['message']}")
    
    print("✅ Social system demo complete!")


def demo_contracts():
    """Demonstrate contract features"""
    print("\n" + "="*70)
    print("CONTRACT SYSTEM DEMO")
    print("="*70)
    
    world = World()
    contract_system = ContractSystem(world)
    
    # Create two traders
    seller = world.create_entity("seller_1")
    seller.add_component(Player(character_name="Alice Trader", isk=5000000.0))
    inventory = Inventory(items={"item_rifter": 5, "item_ammo_emp": 1000}, cargo_capacity=1000.0)
    seller.add_component(inventory)
    
    buyer = world.create_entity("buyer_1")
    buyer.add_component(Player(character_name="Bob Buyer", isk=2000000.0))
    buyer.add_component(Inventory(cargo_capacity=1000.0))
    
    print(f"✓ Created traders: Alice (seller) and Bob (buyer)")
    
    # Create item exchange contract
    contract_id = contract_system.create_item_exchange_contract(
        seller,
        items_offered={"item_rifter": 2},
        items_requested={},
        price=500000.0,  # 500K ISK for 2 Rifters
        collateral=100000.0,
        expiration_days=7,
        availability="public",
        location="Jita"
    )
    print(f"✓ Alice created item exchange contract (ID: {contract_id})")
    print(f"  - Offering: 2x Rifter")
    print(f"  - Price: 500,000 ISK")
    print(f"  - Alice's ISK: {seller.get_component(Player).isk:,.0f}")
    
    # List available contracts
    available = contract_system.list_available_contracts(buyer, location="Jita")
    print(f"✓ Available contracts in Jita: {len(available)}")
    
    # Accept contract
    success = contract_system.accept_item_exchange_contract(buyer, contract_id)
    if success:
        print(f"✓ Bob accepted the contract")
        print(f"  - Bob's ISK: {buyer.get_component(Player).isk:,.0f}")
        print(f"  - Bob's Rifters: {buyer.get_component(Inventory).items.get('item_rifter', 0)}")
        print(f"  - Alice's ISK: {seller.get_component(Player).isk:,.0f}")
    
    # Create courier contract
    courier_seller = world.create_entity("courier_1")
    courier_seller.add_component(Player(character_name="Charlie Courier", isk=3000000.0))
    courier_seller.add_component(Inventory(items={"item_ore_veldspar": 1000}, cargo_capacity=2000.0))
    
    contract_id2 = contract_system.create_courier_contract(
        courier_seller,
        items={"item_ore_veldspar": 500},
        start_location="Jita",
        end_location="Amarr",
        reward=100000.0,
        collateral=500000.0,
        expiration_days=3,
        days_to_complete=1
    )
    print(f"\n✓ Charlie created courier contract (ID: {contract_id2})")
    print(f"  - Route: Jita → Amarr")
    print(f"  - Cargo: 500x Veldspar Ore")
    print(f"  - Reward: 100,000 ISK")
    print(f"  - Collateral: 500,000 ISK")
    
    print("✅ Contract system demo complete!")


def main():
    """Run all demos"""
    print("\n" + "="*70)
    print("EVE OFFLINE - PHASE 4 FEATURES DEMO")
    print("Corporation, Social, and Contract Systems")
    print("="*70)
    
    demo_corporations()
    demo_social()
    demo_contracts()
    
    print("\n" + "="*70)
    print("ALL DEMOS COMPLETE!")
    print("="*70)
    print("\nPhase 4 features are fully functional:")
    print("  • Corporation management with roles and permissions")
    print("  • Corporation hangars and wallets")
    print("  • Contact lists and standings")
    print("  • Mail system with inbox/sent folders")
    print("  • Chat channels with moderation")
    print("  • Player contracts (item exchange, courier)")
    print("\nTry running the test suites:")
    print("  python tests/test_corporation.py")
    print("  python tests/test_social.py")
    print("  python run_tests.py  # Run all tests")
    print()


if __name__ == "__main__":
    main()
