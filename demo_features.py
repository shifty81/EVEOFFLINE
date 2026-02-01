#!/usr/bin/env python3
"""
EVE OFFLINE - Full Feature Demonstration
Showcases all newly implemented features
"""

import sys
import os

# Add project root to path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from engine.core.ecs import World
from engine.core.engine import Engine
from engine.components.game_components import (
    Position, Velocity, Health, Ship, Weapon, Player,
    Fitting, Skills, DroneBay, Drone, SkillTraining,
    Mission, WarpDrive, Celestial, Docking, Target
)
from engine.systems.game_systems import (
    MovementSystem, CapacitorSystem, ShieldRechargeSystem,
    WeaponSystem, AISystem, TargetingSystem, FittingSystem,
    DroneSystem, SkillSystem
)
from engine.systems.mission_system import MissionSystem
from engine.systems.navigation_system import NavigationSystem
from engine.utils.data_loader import DataLoader


def print_header(text):
    """Print a formatted header"""
    print(f"\n{'='*60}")
    print(f"  {text}")
    print(f"{'='*60}\n")


def demo_ship_fitting():
    """Demonstrate ship fitting system"""
    print_header("SHIP FITTING DEMONSTRATION")
    
    world = World()
    
    # Create a ship
    ship = world.create_entity("my_rifter")
    ship.add_component(Ship(
        ship_name="Rifter",
        ship_class="Frigate",
        race="Minmatar",
        cpu=0,
        cpu_max=130,
        powergrid=0,
        powergrid_max=40
    ))
    ship.add_component(Fitting(
        high_slots=[None, None, None],
        mid_slots=[None, None, None],
        low_slots=[None, None]
    ))
    
    fitting_sys = FittingSystem(world)
    
    print("📦 Rifter fitting started...")
    print(f"   Available CPU: 130 tf")
    print(f"   Available PowerGrid: 40 MW\n")
    
    # Fit some modules
    modules = [
        ("200mm_autocannon", "high", 0),
        ("200mm_autocannon", "high", 1),
        ("small_shield_booster", "mid", 0),
        ("1mn_afterburner", "mid", 1),
        ("damage_control", "low", 0)
    ]
    
    for module_id, slot_type, slot_idx in modules:
        success = fitting_sys.fit_module(ship, module_id, slot_type, slot_idx)
        if success:
            print(f"✅ Fitted {module_id} to {slot_type} slot {slot_idx}")
        else:
            print(f"❌ Failed to fit {module_id}")
    
    fitting = ship.get_component(Fitting)
    print(f"\n📋 Final fitting:")
    print(f"   High slots: {fitting.high_slots}")
    print(f"   Mid slots: {fitting.mid_slots}")
    print(f"   Low slots: {fitting.low_slots}")


def demo_drone_operations():
    """Demonstrate drone system"""
    print_header("DRONE OPERATIONS DEMONSTRATION")
    
    world = World()
    
    # Create ship with drone bay
    ship = world.create_entity("vexor")
    ship.add_component(Position(x=0, y=0, z=0))
    ship.add_component(Ship(ship_name="Vexor", ship_class="Cruiser"))
    ship.add_component(DroneBay(
        drone_bandwidth=75.0,
        drone_bay_capacity=75.0
    ))
    
    # Create some drones
    drones = []
    for i in range(5):
        drone = world.create_entity(f"drone_{i}")
        drone.add_component(Drone(
            drone_type="Hammerhead I",
            size="medium",
            bandwidth=10.0,
            damage=15.0,
            state="in_bay"
        ))
        drone.add_component(Position(x=0, y=0, z=0))
        drone.add_component(Velocity())
        drones.append(drone)
    
    # Add drones to ship's bay
    drone_bay = ship.get_component(DroneBay)
    for drone in drones:
        drone_bay.drones.append(drone.id)
    
    print("🤖 Vexor drone operations...")
    print(f"   Drone bay capacity: 75 m³")
    print(f"   Drone bandwidth: 75 Mbit/s")
    print(f"   Available drones: 5x Hammerhead I (10 Mbit/s each)\n")
    
    drone_sys = DroneSystem(world)
    
    # Launch drones
    print("🚀 Launching drones...")
    for i in range(5):
        success = drone_sys.launch_drone(ship, i)
        if success:
            print(f"   ✅ Launched drone {i+1}/5")
    
    print(f"\n📊 Bandwidth used: {drone_bay.bandwidth_used}/{drone_bay.drone_bandwidth} Mbit/s")
    print(f"   Active drones: {len(drone_bay.active_drones)}")
    
    # Create target
    target = world.create_entity("serpentis_rat")
    target.add_component(Position(x=10000, y=0, z=0))
    target.add_component(Health(
        hull_hp=500, armor_hp=400, shield_hp=600
    ))
    
    print(f"\n🎯 Engaging target...")
    drone_sys.engage_target(ship, target.id)
    print(f"   All drones attacking Serpentis Rat!")
    
    # Recall drones
    print(f"\n↩️  Recalling drones...")
    drone_sys.recall_all_drones(ship)
    print(f"   All drones returning to bay")


def demo_skill_training():
    """Demonstrate skill training"""
    print_header("SKILL TRAINING DEMONSTRATION")
    
    world = World()
    
    # Create character
    char = world.create_entity("pilot")
    char.add_component(Player(
        player_id="pilot1",
        character_name="Commander Shepard"
    ))
    char.add_component(Skills(
        skills={'Gunnery': 3, 'Spaceship Command': 4},
        skill_points={'Gunnery': 8000, 'Spaceship Command': 45000}
    ))
    char.add_component(SkillTraining(
        sp_per_minute=30.0
    ))
    
    skill_sys = SkillSystem(world)
    
    print("👨‍🚀 Commander Shepard's skills:")
    skills = char.get_component(Skills)
    for skill_name, level in skills.skills.items():
        print(f"   {skill_name}: Level {level}")
    
    print(f"\n📚 Starting skill training...")
    skill_sys.start_training(char, 'Gunnery', 4)
    print(f"   Training Gunnery IV")
    print(f"   SP rate: 30 SP/minute")
    
    training = char.get_component(SkillTraining)
    print(f"   Current skill: {training.current_skill}")
    print(f"   Target level: {training.current_target_level}")
    
    # Calculate bonus
    bonus = skill_sys.get_skill_bonus(char, 'Gunnery', 'damage')
    print(f"\n💪 Current Gunnery bonus: +{bonus*100}% damage")


def demo_mission_system():
    """Demonstrate mission system"""
    print_header("MISSION SYSTEM DEMONSTRATION")
    
    world = World()
    loader = DataLoader()
    loader.load_all()
    
    # Create player
    player = world.create_entity("pilot")
    player.add_component(Player(
        player_id="pilot1",
        character_name="Captain Kirk",
        isk=5000000
    ))
    player.add_component(Position(x=0, y=0, z=0))
    
    mission_sys = MissionSystem(world, loader)
    
    print("🎯 Approaching mission agent...")
    print("   Agent: Security Chief")
    print("   Location: Jita IV - Moon 4\n")
    
    print("💼 Available missions:")
    print("   1. Destroy Serpentis Rats (Combat)")
    print("   2. Reconnaissance Flight (Exploration)")
    print("   3. Courier Delivery (Transport)\n")
    
    print("📋 Accepting mission: Destroy Serpentis Rats")
    success = mission_sys.accept_mission(player, "destroy_serpentis_rats")
    
    if success:
        print("   ✅ Mission accepted!")
        
        progress = mission_sys.get_mission_progress(player)
        print(f"\n📊 Mission Status:")
        print(f"   Type: {progress['type']}")
        print(f"   Status: {progress['status']}")
        print(f"   Progress: {progress['progress']}")
    
    print("\n💰 Mission Rewards:")
    print("   ISK: 150,000")
    print("   Loyalty Points: 500")


def demo_navigation():
    """Demonstrate navigation and warp"""
    print_header("NAVIGATION & WARP DEMONSTRATION")
    
    world = World()
    
    # Create ship
    ship = world.create_entity("ship")
    ship.add_component(Position(x=0, y=0, z=0))
    ship.add_component(Velocity(max_speed=300))
    ship.add_component(Ship(ship_name="Stabber", ship_class="Cruiser"))
    ship.add_component(WarpDrive(
        warp_speed=3.0,
        align_time=6.0
    ))
    
    # Create station
    station = world.create_entity("jita_4_4")
    station.add_component(Position(x=150000000, y=50000000, z=0))  # 1 AU away
    station.add_component(Celestial(
        celestial_type="station",
        celestial_name="Jita IV - Moon 4 - Caldari Navy Assembly Plant",
        dockable=True,
        services=['market', 'repair', 'fitting', 'clone']
    ))
    
    nav_sys = NavigationSystem(world)
    
    print("🚀 Stabber preparing for warp...")
    print(f"   Current position: (0, 0, 0)")
    print(f"   Destination: Jita 4-4 (150,000,000, 50,000,000, 0)")
    print(f"   Distance: ~1 AU\n")
    
    print("⏱️  Initiating warp...")
    success = nav_sys.warp_to_celestial(ship, station)
    
    if success:
        warp = ship.get_component(WarpDrive)
        print(f"   Aligning to destination...")
        print(f"   Align time: {warp.align_time} seconds")
        print(f"   Warp speed: {warp.warp_speed} AU/s\n")
        
        # Simulate alignment
        nav_sys.update(10.0)
        
        if warp.is_warping:
            print("   ✅ Entering warp!")
            print("   🌟 Traveling at FTL speeds...")
    
    # Now demonstrate docking
    print(f"\n🏢 Approaching station for docking...")
    
    # Move ship close to station for demo
    ship_pos = ship.get_component(Position)
    station_pos = station.get_component(Position)
    ship_pos.x = station_pos.x + 2000
    ship_pos.y = station_pos.y
    ship_pos.z = station_pos.z
    
    print(f"   Distance to station: 2 km")
    print(f"   Requesting docking permission...")
    
    success = nav_sys.dock(ship, station)
    if success:
        print(f"   ✅ Docking request granted!")
        print(f"   🏢 Docked at {station.get_component(Celestial).celestial_name}")
        
        docking = ship.get_component(Docking)
        celestial = station.get_component(Celestial)
        print(f"\n   Available services:")
        for service in celestial.services:
            print(f"      • {service.title()}")


def demo_full_combat_scenario():
    """Demonstrate complete combat scenario"""
    print_header("FULL COMBAT SCENARIO")
    
    world = World()
    
    print("⚔️  Combat Situation:")
    print("   Location: Perimeter asteroid belt")
    print("   Hostiles detected: 2x Serpentis Frigate\n")
    
    # Create player ship
    player_ship = world.create_entity("player_ship")
    player_ship.add_component(Position(x=0, y=0, z=0))
    player_ship.add_component(Velocity(max_speed=300))
    player_ship.add_component(Health(
        hull_hp=400, hull_max=400,
        armor_hp=450, armor_max=450,
        shield_hp=600, shield_max=600,
        shield_kinetic_resist=0.3
    ))
    player_ship.add_component(Ship(
        ship_name="Tristan",
        ship_class="Frigate",
        max_locked_targets=3
    ))
    player_ship.add_component(Weapon(
        damage=20,
        damage_type="kinetic",
        optimal_range=8000,
        ammo_count=100
    ))
    player_ship.add_component(Target())
    
    # Create enemy
    enemy = world.create_entity("serpentis_scout")
    enemy.add_component(Position(x=5000, y=0, z=0))
    enemy.add_component(Velocity(max_speed=350))
    enemy.add_component(Health(
        hull_hp=200, hull_max=200,
        armor_hp=150, armor_max=150,
        shield_hp=300, shield_max=300
    ))
    
    print("📊 Player Ship Status:")
    health = player_ship.get_component(Health)
    print(f"   Shield: {health.shield_hp}/{health.shield_max} HP")
    print(f"   Armor: {health.armor_hp}/{health.armor_max} HP")
    print(f"   Hull: {health.hull_hp}/{health.hull_max} HP")
    
    # Systems
    targeting_sys = TargetingSystem(world)
    weapon_sys = WeaponSystem(world)
    
    print(f"\n🎯 Locking target...")
    targeting_sys.start_lock(player_ship, enemy.id)
    
    # Simulate lock time
    for _ in range(3):
        targeting_sys.update(1.0)
    
    target_comp = player_ship.get_component(Target)
    if enemy.id in target_comp.locked_targets:
        print(f"   ✅ Target locked: Serpentis Scout")
        print(f"   Distance: 5,000 m")
        print(f"   Optimal range: 8,000 m\n")
        
        print(f"🔫 Opening fire...")
        enemy_health = enemy.get_component(Health)
        initial_shield = enemy_health.shield_hp
        
        success = weapon_sys.fire_weapon(player_ship, enemy.id)
        if success:
            damage = initial_shield - enemy_health.shield_hp
            print(f"   ✅ Hit! Damage: {damage:.1f}")
            print(f"   Enemy shield: {enemy_health.shield_hp:.1f}/{enemy_health.shield_max} HP")


def main():
    """Run all demonstrations"""
    print("\n" + "="*60)
    print("  EVE OFFLINE - Full Feature Demonstration")
    print("  Showcasing all newly implemented systems")
    print("="*60)
    
    demos = [
        ("Ship Fitting", demo_ship_fitting),
        ("Drone Operations", demo_drone_operations),
        ("Skill Training", demo_skill_training),
        ("Mission System", demo_mission_system),
        ("Navigation & Warp", demo_navigation),
        ("Combat Scenario", demo_full_combat_scenario)
    ]
    
    for i, (name, demo_func) in enumerate(demos, 1):
        try:
            demo_func()
        except Exception as e:
            print(f"\n❌ Error in {name}: {e}")
            import traceback
            traceback.print_exc()
    
    print_header("DEMONSTRATION COMPLETE")
    print("✅ All core EVE Online features demonstrated!")
    print("\n📚 For more information:")
    print("   • See NEW_FEATURES.md for complete feature documentation")
    print("   • Run tests/test_engine.py for core system tests")
    print("   • Run tests/test_advanced_systems.py for advanced tests")
    print("\n🚀 EVE OFFLINE is ready for the full experience!")
    print("="*60 + "\n")


if __name__ == "__main__":
    main()
