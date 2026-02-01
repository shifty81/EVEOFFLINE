#!/usr/bin/env python3
"""
New Features Demo
Demonstrates manufacturing, market, and exploration systems
"""

import sys
import time
from engine.core.ecs import World
from engine.components.game_components import Position, Health, Ship
from engine.systems.industry_system import (
    IndustrySystem, Blueprint, BlueprintInventory, Inventory, 
    ManufacturingQueue
)
from engine.systems.market_system import (
    MarketSystem, Wallet, MarketAccess, OrderType
)
from engine.systems.exploration_system import (
    ExplorationSystem, ProbeScanner, ShipScanner, 
    ExplorationData, SignatureType
)


def print_section(title: str):
    """Print a formatted section header"""
    print(f"\n{'='*70}")
    print(f"  {title}")
    print(f"{'='*70}\n")


def demo_manufacturing():
    """Demonstrate the manufacturing system"""
    print_section("MANUFACTURING SYSTEM DEMO")
    
    world = World()
    industry = IndustrySystem(world)
    
    # Create a player character with manufacturing capability
    player = world.create_entity()
    
    # Add components
    bp_inv = BlueprintInventory()
    inventory = Inventory(capacity=10000.0)
    mfg_queue = ManufacturingQueue()
    
    # Create a blueprint for a frigate
    blueprint = Blueprint(
        blueprint_id="rifter_bp",
        item_type="ship",
        item_id="rifter",
        material_efficiency=5,  # 5% material reduction
        time_efficiency=10,  # 10% time reduction
        runs=-1,  # BPO (infinite runs)
        materials={
            "tritanium": 30000,
            "pyerite": 8000,
            "mexallon": 2000
        },
        manufacturing_time=3600.0  # 1 hour
    )
    
    bp_inv.blueprints["rifter_bp"] = blueprint
    
    # Add materials to inventory
    print("📦 Starting inventory:")
    inventory.add_item("tritanium", 100000, 0.01)
    inventory.add_item("pyerite", 25000, 0.01)
    inventory.add_item("mexallon", 10000, 0.01)
    
    for item_id, qty in inventory.items.items():
        print(f"  - {item_id}: {qty:,}")
    print(f"  Capacity used: {inventory.used_capacity:.1f}/{inventory.capacity:.1f} m³")
    
    # Attach components
    player.add_component(bp_inv)
    player.add_component(inventory)
    player.add_component(mfg_queue)
    
    # Start manufacturing
    print("\n🏭 Starting manufacturing job: Rifter (2 units)")
    job_id = industry.start_manufacturing(player, "rifter_bp", quantity=2, current_time=0.0)
    
    if job_id:
        print(f"✅ Manufacturing job started: {job_id}")
        
        # Show job details
        jobs = industry.get_active_jobs(player)
        for job in jobs:
            print(f"\n  Job: {job.job_id}")
            print(f"    Item: {job.item_id} x{job.quantity}")
            print(f"    Start: {job.start_time:.0f}s")
            print(f"    End: {job.end_time:.0f}s")
            print(f"    Duration: {job.end_time - job.start_time:.0f}s")
            print(f"    Status: {job.status}")
        
        # Simulate time passing
        print("\n⏳ Simulating manufacturing time...")
        current_time = 0.0
        while current_time < 4000.0:
            current_time += 100.0
            industry.update(1.0, current_time)
            
            active = industry.get_active_jobs(player)
            if not active:
                print(f"✅ Manufacturing completed at {current_time:.0f}s")
                break
        
        # Show final inventory
        print("\n📦 Final inventory:")
        for item_id, qty in inventory.items.items():
            print(f"  - {item_id}: {qty:,}")
    else:
        print("❌ Failed to start manufacturing job")
    
    # Demonstrate blueprint research
    print("\n🔬 Researching Material Efficiency...")
    if industry.research_material_efficiency(player, "rifter_bp"):
        print(f"✅ ME increased to {blueprint.material_efficiency}")
    
    # Demonstrate blueprint copying
    print("\n📋 Copying blueprint (10 runs)...")
    if industry.copy_blueprint(player, "rifter_bp", runs=10):
        print("✅ Blueprint copy created")
        print(f"  Total blueprints: {len(bp_inv.blueprints)}")


def demo_market():
    """Demonstrate the market system"""
    print_section("MARKET SYSTEM DEMO")
    
    world = World()
    market = MarketSystem(world)
    
    # Initialize NPC prices
    npc_prices = {
        "tritanium": 5.0,
        "rifter": 350000.0,
        "autocannon": 25000.0
    }
    market.initialize_npc_prices(npc_prices)
    
    # Create two players (buyer and seller)
    seller = world.create_entity()
    buyer = world.create_entity()
    
    # Setup seller
    seller_wallet = Wallet(isk=5000000.0)
    seller_market = MarketAccess(location_id="jita_4_4")
    seller_inv = Inventory(capacity=1000.0)
    seller_inv.add_item("tritanium", 100000, 0.01)
    
    seller.add_component(seller_wallet)
    seller.add_component(seller_market)
    seller.add_component(seller_inv)
    
    # Setup buyer
    buyer_wallet = Wallet(isk=10000000.0)
    buyer_market = MarketAccess(location_id="jita_4_4")
    buyer_inv = Inventory(capacity=1000.0)
    
    buyer.add_component(buyer_wallet)
    buyer.add_component(buyer_market)
    buyer.add_component(buyer_inv)
    
    print("💰 Initial state:")
    print(f"  Seller ISK: {seller_wallet.isk:,.0f}")
    print(f"  Seller tritanium: {seller_inv.items.get('tritanium', 0):,}")
    print(f"  Buyer ISK: {buyer_wallet.isk:,.0f}")
    print(f"  Buyer tritanium: {buyer_inv.items.get('tritanium', 0):,}")
    
    # Seller places a sell order
    print("\n📊 Seller places sell order:")
    print("  Item: Tritanium x50,000")
    print("  Price: 6.0 ISK each")
    
    order_id = market.place_order(
        seller,
        "tritanium",
        OrderType.SELL,
        price=6.0,
        quantity=50000
    )
    
    if order_id:
        print(f"  ✅ Order placed: {order_id}")
        
        # Check market price
        prices = market.get_market_price("tritanium", "jita_4_4")
        print(f"\n💱 Current market prices:")
        print(f"  Best sell: {prices['sell']:.2f} ISK")
        print(f"  Best buy: {prices['buy']:.2f} ISK")
        print(f"  Average: {prices['average']:.2f} ISK")
        
        # Buyer makes instant purchase
        print("\n🛒 Buyer makes instant purchase:")
        print("  Quantity: 10,000 units")
        
        if market.instant_buy(buyer, "tritanium", 10000):
            print("  ✅ Purchase successful!")
            
            print(f"\n💰 After transaction:")
            print(f"  Seller ISK: {seller_wallet.isk:,.0f} (+{300000 - 6000:.0f})")
            print(f"  Seller tritanium: {seller_inv.items.get('tritanium', 0):,}")
            print(f"  Buyer ISK: {buyer_wallet.isk:,.0f} (-{61200:.0f})")
            print(f"  Buyer tritanium: {buyer_inv.items.get('tritanium', 0):,}")
            
            print(f"\n📈 Transaction history:")
            for tx in market.transaction_history[-3:]:
                print(f"  - {tx['item_id']} x{tx['quantity']} @ {tx['price']:.2f} ISK")
        else:
            print("  ❌ Purchase failed")
    else:
        print("  ❌ Failed to place order")


def demo_exploration():
    """Demonstrate the exploration system"""
    print_section("EXPLORATION SYSTEM DEMO")
    
    world = World()
    exploration = ExplorationSystem(world)
    
    # Generate signatures in a system
    system_id = "outer_rim"
    print(f"🌌 Generating cosmic signatures in {system_id}...")
    exploration.generate_signatures(system_id, count=5)
    
    signatures = exploration.get_signatures(system_id)
    print(f"  Found {len(signatures)} signatures:")
    
    for sig in signatures:
        print(f"\n  📡 {sig.signature_id}")
        print(f"    Type: {sig.signature_type.value}")
        print(f"    Name: {sig.name}")
        print(f"    Position: ({sig.position[0]:.1f}, {sig.position[1]:.1f}, {sig.position[2]:.1f}) AU")
        print(f"    Signal: {sig.signal_strength:.0f}%")
        print(f"    Difficulty: {'⭐' * sig.difficulty}")
    
    # Create explorer ship
    explorer = world.create_entity()
    position = Position(x=0.0, y=0.0, z=0.0)
    scanner = ProbeScanner(max_probes=8)
    ship_scanner = ShipScanner()
    exp_data = ExplorationData()
    
    explorer.add_component(position)
    explorer.add_component(scanner)
    explorer.add_component(ship_scanner)
    explorer.add_component(exp_data)
    
    # Launch probes
    print("\n🛰️ Launching scanner probes...")
    if exploration.launch_probes(explorer, count=8):
        print(f"  ✅ Launched {len(scanner.probes)} probes")
        
        # Position probes in a formation
        print("\n📍 Positioning probes in cubic formation...")
        positions = [
            (-2.0, -2.0, -2.0), (2.0, -2.0, -2.0),
            (-2.0, 2.0, -2.0), (2.0, 2.0, -2.0),
            (-2.0, -2.0, 2.0), (2.0, -2.0, 2.0),
            (-2.0, 2.0, 2.0), (2.0, 2.0, 2.0)
        ]
        
        for i, probe in enumerate(scanner.probes):
            if i < len(positions):
                exploration.move_probe(explorer, probe.probe_id, positions[i])
                print(f"  Probe {i+1}: {positions[i]}")
        
        # Perform scans
        print("\n🔍 Scanning for signatures...")
        scan_count = 0
        max_scans = 10
        
        while scan_count < max_scans:
            scan_count += 1
            detected = exploration.scan(explorer, system_id)
            
            if detected:
                print(f"\n  Scan #{scan_count}:")
                for sig_id in detected:
                    sig = exploration.signatures[system_id][sig_id]
                    print(f"    {sig_id}: {sig.scan_progress:.1f}% {'✅ SCANNED' if sig.is_scanned else ''}")
            
            # Check if all are scanned
            all_scanned = all(
                sig.is_scanned 
                for sig in exploration.signatures[system_id].values()
            )
            
            if all_scanned:
                print(f"\n  ✅ All signatures fully scanned after {scan_count} scans")
                break
        
        # Show scanned signatures
        scanned_sigs = exploration.get_signatures(system_id, scanned_only=True)
        print(f"\n📋 Scanned signatures: {len(scanned_sigs)}")
        
        for sig in scanned_sigs:
            print(f"\n  🎯 {sig.name} ({sig.signature_id})")
            print(f"    Type: {sig.signature_type.value}")
            print(f"    Difficulty: {'⭐' * sig.difficulty}")
            
            if sig.signature_type == SignatureType.COMBAT:
                print(f"    NPCs: {', '.join(sig.npc_spawns)}")
            elif sig.signature_type in [SignatureType.RELIC, SignatureType.DATA]:
                print(f"    Loot: {', '.join(f'{k} x{v}' for k, v in sig.loot_table.items())}")
        
        # Warp to and complete a site
        if scanned_sigs:
            target_sig = scanned_sigs[0]
            print(f"\n🚀 Warping to {target_sig.name}...")
            
            if exploration.warp_to_signature(explorer, system_id, target_sig.signature_id):
                print(f"  ✅ Arrived at signature")
                print(f"  Position: ({position.x:.1f}, {position.y:.1f}, {position.z:.1f}) AU")
                
                print(f"\n🎁 Completing site...")
                rewards = exploration.complete_site(explorer, system_id, target_sig.signature_id)
                
                if rewards:
                    print("  ✅ Site completed!")
                    print(f"  Rewards:")
                    for item_id, qty in rewards['loot'].items():
                        print(f"    - {item_id}: {qty}")
        
        # Recall probes
        print("\n📥 Recalling probes...")
        exploration.recall_probes(explorer)
        print(f"  ✅ Probes recalled")
    else:
        print("  ❌ Failed to launch probes")


def main():
    """Run all demos"""
    print("\n" + "="*70)
    print("  EVE OFFLINE - NEW FEATURES DEMONSTRATION")
    print("  Manufacturing, Market, and Exploration Systems")
    print("="*70)
    
    try:
        # Run each demo
        demo_manufacturing()
        time.sleep(0.5)
        
        demo_market()
        time.sleep(0.5)
        
        demo_exploration()
        
        # Summary
        print_section("DEMO COMPLETE")
        print("✅ All new systems demonstrated successfully!")
        print("\nNew systems available:")
        print("  1. 🏭 Manufacturing System - Blueprint research, manufacturing, copying")
        print("  2. 💰 Market System - Buy/sell orders, instant trading, ISK wallet")
        print("  3. 🔍 Exploration System - Probe scanning, cosmic signatures, sites")
        print("\nSee LANGUAGE_AND_3D_OPTIONS.md for information about:")
        print("  - Python vs C++20 implementation")
        print("  - 3D graphics support options")
        print("  - EVE Online and Astrox Imperium visual style")
        
    except KeyboardInterrupt:
        print("\n\nDemo interrupted by user")
        sys.exit(0)
    except Exception as e:
        print(f"\n\n❌ Error during demo: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()
