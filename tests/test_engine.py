"""
Basic Engine Test
Tests core functionality without networking
"""

import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from engine.core.ecs import World, Entity
from engine.components.game_components import (
    Position, Velocity, Health, Ship, Weapon, AI, Player,
    Fitting, Skills, DroneBay, Drone, SkillTraining
)
from engine.systems.game_systems import (
    MovementSystem, WeaponSystem, AISystem, FittingSystem, 
    DroneSystem, SkillSystem
)
from engine.utils.data_loader import DataLoader


def test_ecs():
    """Test Entity Component System"""
    print("\n=== Testing ECS ===")
    
    world = World()
    
    # Create entity
    entity = world.create_entity("test_ship")
    print(f"✓ Created entity: {entity.id}")
    
    # Add components
    entity.add_component(Position(x=100, y=200, z=0))
    entity.add_component(Velocity(vx=10, vy=5, max_speed=50))
    entity.add_component(Health(hull_hp=500, armor_hp=400, shield_hp=600))
    print(f"✓ Added components")
    
    # Check components
    pos = entity.get_component(Position)
    assert pos.x == 100
    assert pos.y == 200
    print(f"✓ Component retrieval works: pos=({pos.x}, {pos.y})")
    
    # Query entities
    entities = world.get_entities(Position, Velocity)
    assert len(entities) == 1
    print(f"✓ Entity query works: found {len(entities)} entities")
    
    print("✅ ECS tests passed!\n")


def test_systems():
    """Test game systems"""
    print("=== Testing Systems ===")
    
    world = World()
    
    # Create entity with position and velocity
    entity = world.create_entity()
    entity.add_component(Position(x=0, y=0, z=0))
    entity.add_component(Velocity(vx=10, vy=20, vz=0))
    
    # Add movement system
    movement_sys = MovementSystem(world)
    world.add_system(movement_sys)
    
    # Update once
    print(f"Initial position: (0, 0)")
    world.update(1.0)  # 1 second
    
    pos = entity.get_component(Position)
    print(f"After 1 second: ({pos.x}, {pos.y})")
    assert pos.x == 10
    assert pos.y == 20
    print(f"✓ Movement system works")
    
    print("✅ System tests passed!\n")


def test_data_loader():
    """Test data loading"""
    print("=== Testing Data Loader ===")
    
    loader = DataLoader()
    loader.load_all()
    
    # Check ships
    rifter = loader.get_ship("rifter")
    assert rifter is not None
    print(f"✓ Loaded ship: {rifter['name']} ({rifter['class']})")
    
    # Check modules
    autocannon = loader.get_module("200mm_autocannon")
    assert autocannon is not None
    print(f"✓ Loaded module: {autocannon['name']}")
    
    # Check skills
    gunnery = loader.get_skill("gunnery")
    assert gunnery is not None
    print(f"✓ Loaded skill: {gunnery['name']}")
    
    # Check NPCs
    serpentis = loader.get_npc("serpentis_scout")
    assert serpentis is not None
    print(f"✓ Loaded NPC: {serpentis['name']}")
    
    # Check missions
    mission = loader.get_mission("destroy_serpentis_rats")
    assert mission is not None
    print(f"✓ Loaded mission: {mission['name']}")
    
    print(f"✅ Data loader tests passed!\n")


def test_combat():
    """Test combat system"""
    print("=== Testing Combat System ===")
    
    world = World()
    
    # Create attacker
    attacker = world.create_entity("attacker")
    attacker.add_component(Position(x=0, y=0, z=0))
    attacker.add_component(Weapon(
        damage=50,
        damage_type="kinetic",
        optimal_range=5000,
        rate_of_fire=3.0,
        ammo_count=100
    ))
    
    # Create target
    target = world.create_entity("target")
    target.add_component(Position(x=1000, y=0, z=0))  # 1000m away
    target.add_component(Health(
        hull_hp=500, hull_max=500,
        armor_hp=500, armor_max=500,
        shield_hp=500, shield_max=500,
        shield_kinetic_resist=0.5  # 50% resist
    ))
    
    # Create weapon system
    weapon_sys = WeaponSystem(world)
    
    # Fire weapon
    initial_shield = target.get_component(Health).shield_hp
    print(f"Initial shield: {initial_shield}")
    
    success = weapon_sys.fire_weapon(attacker, target.id)
    assert success
    print(f"✓ Weapon fired successfully")
    
    # Check damage
    final_shield = target.get_component(Health).shield_hp
    print(f"Final shield: {final_shield}")
    assert final_shield < initial_shield
    print(f"✓ Damage applied: {initial_shield - final_shield}")
    
    print("✅ Combat tests passed!\n")


def test_fitting_system():
    """Test module fitting system"""
    print("=== Testing Fitting System ===")
    
    world = World()
    
    # Create ship entity
    ship_entity = world.create_entity("ship")
    ship_entity.add_component(Ship(
        ship_name="Rifter",
        cpu_max=100,
        powergrid_max=50
    ))
    ship_entity.add_component(Fitting(
        high_slots=[None, None, None],
        mid_slots=[None, None, None],
        low_slots=[None, None]
    ))
    
    # Create fitting system
    fitting_sys = FittingSystem(world)
    
    # Test fitting a module
    success = fitting_sys.fit_module(ship_entity, "autocannon_1", "high", 0)
    assert success
    print("✓ Module fitted successfully")
    
    fitting = ship_entity.get_component(Fitting)
    assert fitting.high_slots[0] == "autocannon_1"
    print("✓ Module appears in fitting")
    
    # Test unfitting
    module_id = fitting_sys.unfit_module(ship_entity, "high", 0)
    assert module_id == "autocannon_1"
    assert fitting.high_slots[0] is None
    print("✓ Module unfitted successfully")
    
    print("✅ Fitting system tests passed!\n")


def test_drone_system():
    """Test drone system"""
    print("=== Testing Drone System ===")
    
    world = World()
    
    # Create ship
    ship = world.create_entity("ship")
    ship.add_component(Position(x=0, y=0, z=0))
    ship.add_component(DroneBay(
        drone_bandwidth=25.0,
        drone_bay_capacity=50.0
    ))
    
    # Create drone
    drone = world.create_entity("drone_1")
    drone.add_component(Drone(
        drone_type="Light Scout Drone",
        bandwidth=5.0,
        damage=10.0,
        state="in_bay"
    ))
    drone.add_component(Position(x=0, y=0, z=0))
    drone.add_component(Velocity())
    
    # Add drone to ship's bay
    drone_bay = ship.get_component(DroneBay)
    drone_bay.drones.append(drone.id)
    
    # Create drone system
    drone_sys = DroneSystem(world)
    
    # Test launching drone
    success = drone_sys.launch_drone(ship, 0)
    assert success
    print("✓ Drone launched successfully")
    
    assert drone.get_component(Drone).state == "active"
    assert drone.id in drone_bay.active_drones
    print("✓ Drone is active")
    
    # Test recalling drone
    success = drone_sys.recall_drone(ship, drone.id)
    assert success
    assert drone.get_component(Drone).state == "returning"
    print("✓ Drone recalled successfully")
    
    print("✅ Drone system tests passed!\n")


def test_skill_system():
    """Test skill training system"""
    print("=== Testing Skill System ===")
    
    world = World()
    
    # Create character entity
    char = world.create_entity("character")
    char.add_component(Skills(
        skills={'Gunnery': 2},
        skill_points={'Gunnery': 1414}
    ))
    char.add_component(SkillTraining(
        sp_per_minute=30.0
    ))
    
    # Create skill system
    skill_sys = SkillSystem(world)
    
    # Start training
    success = skill_sys.start_training(char, 'Gunnery', 3)
    assert success
    print("✓ Skill training started")
    
    training = char.get_component(SkillTraining)
    assert training.current_skill == 'Gunnery'
    assert training.current_target_level == 3
    print(f"✓ Training Gunnery to level 3")
    
    # Test skill bonus calculation
    bonus = skill_sys.get_skill_bonus(char, 'Gunnery', 'damage')
    assert bonus == 0.04  # 2% per level * 2 levels
    print(f"✓ Skill bonus calculated: {bonus*100}%")
    
    print("✅ Skill system tests passed!\n")


def run_all_tests():
    """Run all tests"""
    print("\n" + "="*50)
    print("EVE OFFLINE - Engine Tests")
    print("="*50)
    
    try:
        test_ecs()
        test_systems()
        test_data_loader()
        test_combat()
        test_fitting_system()
        test_drone_system()
        test_skill_system()
        
        print("="*50)
        print("✅ ALL TESTS PASSED!")
        print("="*50)
        print("\nThe engine is working correctly!")
        print("Next steps:")
        print("1. Run the server: python server/server.py")
        print("2. Run a client: python client/client.py YourName")
        print("="*50 + "\n")
        
    except Exception as e:
        print(f"\n❌ TEST FAILED: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    run_all_tests()
