#!/usr/bin/env python3
"""
EVE OFFLINE - Gameplay Showcase
Demonstrates all features in a scripted format for screenshots
"""

import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from interactive_demo import InteractiveDemo
import time


def main():
    """Run gameplay showcase"""
    demo = InteractiveDemo()
    
    # Print banner
    demo.print_banner()
    
    # Show initial status
    print("=" * 80)
    print("  INITIAL SHIP STATUS")
    print("=" * 80)
    demo.print_status()
    
    # Show scanning
    print("\n" + "=" * 80)
    print("  SCANNING SECTOR")
    print("=" * 80)
    demo.cmd_scan()
    
    # Show skills
    print("\n" + "=" * 80)
    print("  CHARACTER SKILLS")
    print("=" * 80)
    demo.cmd_skills()
    
    # Launch drones
    print("\n" + "=" * 80)
    print("  DRONE OPERATIONS")
    print("=" * 80)
    
    from engine.components.game_components import DroneBay, Ship, Health, Position, Fitting, Target, WarpDrive, Celestial
    drones = demo.player.get_component(DroneBay)
    print(f"\nDrone Bay: {len(drones.drones)} drones available")
    print(f"Bandwidth: {drones.drone_bandwidth} Mbit/s")
    
    # Launch all drones
    count = 0
    for i in range(len(drones.drones)):
        if demo.drone_sys.launch_drone(demo.player, i):
            count += 1
    print(f"\n✅ Launched {count} drones")
    
    drones = demo.player.get_component(DroneBay)
    print(f"Active drones: {len(drones.active_drones)}")
    print(f"Bandwidth used: {len(drones.active_drones)*10}/{drones.drone_bandwidth} Mbit/s")
    
    # Engage target
    if demo.enemies:
        target = demo.enemies[0]
        demo.drone_sys.engage_target(demo.player, target.id)
        ship = target.get_component(Ship)
        print(f"\n🎯 Drones engaging {ship.ship_name}!")
    
    # Show combat
    print("\n" + "=" * 80)
    print("  COMBAT ENGAGEMENT")
    print("=" * 80)
    
    print(f"\n⚔️  Enemies detected:")
    for i, enemy in enumerate(demo.enemies):
        ship = enemy.get_component(Ship)
        health = enemy.get_component(Health)
        pos = enemy.get_component(Position)
        
        player_pos = demo.player.get_component(Position)
        distance = ((pos.x - player_pos.x)**2 + (pos.y - player_pos.y)**2)**0.5
        
        print(f"\n  {i+1}. {ship.ship_name}")
        print(f"     Shield: {health.shield_hp:.0f}/{health.shield_max:.0f} HP")
        print(f"     Armor:  {health.armor_hp:.0f}/{health.armor_max:.0f} HP")
        print(f"     Hull:   {health.hull_hp:.0f}/{health.hull_max:.0f} HP")
        print(f"     Distance: {distance:.0f}m")
    
    # Attack first enemy
    target = demo.enemies[0]
    target_comp = demo.player.get_component(Target)
    if target.id not in target_comp.locked_targets:
        target_comp.locked_targets.append(target.id)
    
    print(f"\n🎯 Target locked: {target.get_component(Ship).ship_name}")
    print("\n🔫 Opening fire with drones...")
    
    # Simulate damage
    health = target.get_component(Health)
    damage = 35 * (1 + demo.skill_sys.get_skill_bonus(demo.player, 'Gunnery', 'damage') / 100)
    health.shield_hp = max(0, health.shield_hp - damage)
    
    print(f"💥 HIT! Damage dealt: {damage:.1f}")
    print(f"   Enemy shield remaining: {health.shield_hp:.0f}/{health.shield_max:.0f} HP")
    
    # Show fitting
    print("\n" + "=" * 80)
    print("  SHIP FITTING")
    print("=" * 80)
    
    fitting = demo.player.get_component(Fitting)
    ship = demo.player.get_component(Ship)
    
    print(f"\n📋 Current Fitting:")
    print(f"  High slots: {fitting.high_slots}")
    print(f"  Mid slots:  {fitting.mid_slots}")
    print(f"  Low slots:  {fitting.low_slots}")
    print(f"  Rig slots:  {fitting.rig_slots}")
    
    print(f"\n⚡ Resources:")
    print(f"  CPU:        {ship.cpu:.0f}/{ship.cpu_max:.0f} tf ({ship.cpu/ship.cpu_max*100:.1f}% used)")
    print(f"  PowerGrid:  {ship.powergrid:.0f}/{ship.powergrid_max:.0f} MW ({ship.powergrid/ship.powergrid_max*100:.1f}% used)")
    
    # Navigation info
    print("\n" + "=" * 80)
    print("  NAVIGATION")
    print("=" * 80)
    
    pos = demo.player.get_component(Position)
    station_pos = demo.station.get_component(Position)
    distance = ((station_pos.x - pos.x)**2 + (station_pos.y - pos.y)**2)**0.5
    
    print(f"\n📍 Current Position: ({pos.x:.0f}, {pos.y:.0f}, {pos.z:.0f})")
    print(f"\n🏢 Nearest Station: {demo.station.get_component(Celestial).celestial_name}")
    print(f"   Distance: {distance/149597870700:.2f} AU ({distance:.0f} meters)")
    
    warp = demo.player.get_component(WarpDrive)
    print(f"\n🚀 Warp Capability:")
    print(f"   Warp Speed: {warp.warp_speed:.1f} AU/s")
    print(f"   Align Time: {warp.align_time:.1f} seconds")
    
    # Final status
    print("\n" + "=" * 80)
    print("  FINAL SHIP STATUS")
    print("=" * 80)
    demo.print_status()
    
    print("\n" + "=" * 80)
    print("  GAMEPLAY FEATURES DEMONSTRATED:")
    print("=" * 80)
    print("""
  ✅ Character Skills & Training
  ✅ Ship Fitting & Resource Management (CPU/PowerGrid)
  ✅ Drone Bay Operations
  ✅ Combat & Targeting System
  ✅ Directional Scanner
  ✅ Navigation & Warp Drive
  ✅ Multiple Ship Classes (Frigates, Cruisers)
  ✅ Shield/Armor/Hull Defense System
  ✅ NPC Enemies with AI
  ✅ Space Stations & Celestial Objects
  
  🎮 EVE OFFLINE - Full EVE Online Experience!
""")
    
    print("=" * 80)
    print("  For interactive gameplay, run: python3 interactive_demo.py")
    print("=" * 80)
    

if __name__ == "__main__":
    main()
