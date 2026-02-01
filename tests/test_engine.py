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
    Position, Velocity, Health, Ship, Weapon, AI, Player
)
from engine.systems.game_systems import (
    MovementSystem, WeaponSystem, AISystem
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
