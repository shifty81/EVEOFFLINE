"""
Advanced Systems Test
Tests mission, navigation, and other advanced systems
"""

import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from engine.core.ecs import World
from engine.components.game_components import (
    Position, Velocity, Player, Mission, WarpDrive, Celestial, Docking
)
from engine.systems.mission_system import MissionSystem
from engine.systems.navigation_system import NavigationSystem
from engine.utils.data_loader import DataLoader


def test_mission_system():
    """Test mission system"""
    print("\n=== Testing Mission System ===")
    
    world = World()
    loader = DataLoader()
    loader.load_all()
    
    # Create player
    player = world.create_entity("player")
    player.add_component(Player(
        player_id="player1",
        character_name="Test Pilot",
        isk=1000000
    ))
    player.add_component(Position(x=0, y=0, z=0))
    
    # Create mission system
    mission_sys = MissionSystem(world, loader)
    
    # Accept a mission
    success = mission_sys.accept_mission(player, "destroy_serpentis_rats")
    assert success
    print("✓ Mission accepted")
    
    # Check mission component was added
    mission = player.get_component(Mission)
    assert mission is not None
    assert mission.status == "active"
    print(f"✓ Mission is active: {mission.mission_id}")
    
    # Get mission progress
    progress = mission_sys.get_mission_progress(player)
    assert progress['has_mission']
    print(f"✓ Mission progress: {progress['progress']}")
    
    print("✅ Mission system tests passed!\n")


def test_navigation_system():
    """Test warp and navigation"""
    print("=== Testing Navigation System ===")
    
    world = World()
    
    # Create ship
    ship = world.create_entity("ship")
    ship.add_component(Position(x=0, y=0, z=0))
    ship.add_component(Velocity(max_speed=300))
    ship.add_component(WarpDrive(
        warp_speed=3.0,
        align_time=5.0
    ))
    
    # Create station
    station = world.create_entity("station")
    station.add_component(Position(x=100000, y=50000, z=0))
    station.add_component(Celestial(
        celestial_type="station",
        celestial_name="Jita 4-4",
        dockable=True
    ))
    
    # Create navigation system
    nav_sys = NavigationSystem(world)
    
    # Test warp initiation
    warp_before = ship.get_component(WarpDrive)
    print(f"  WarpDrive component exists: {warp_before is not None}")
    if warp_before:
        print(f"  Is warping: {warp_before.is_warping}, Is aligning: {warp_before.is_aligning}")
    
    success = nav_sys.initiate_warp(ship, 100000, 50000, 0)
    print(f"  Initiate warp returned: {success}")
    assert success
    print("✓ Warp initiated")
    
    warp = ship.get_component(WarpDrive)
    assert warp.is_aligning
    print("✓ Ship is aligning")
    
    # Simulate alignment completion
    nav_sys.update(10.0)  # More than align time
    assert not warp.is_aligning
    assert warp.is_warping
    print("✓ Ship entered warp")
    
    # Test approach
    ship2 = world.create_entity("ship2")
    ship2.add_component(Position(x=0, y=0, z=0))
    ship2.add_component(Velocity(max_speed=300))
    
    target = world.create_entity("target")
    target.add_component(Position(x=5000, y=0, z=0))
    
    success = nav_sys.approach(ship2, target, 1000)
    assert success
    vel = ship2.get_component(Velocity)
    assert vel.vx > 0  # Moving towards target
    print("✓ Ship approaching target")
    
    # Test orbit
    success = nav_sys.orbit(ship2, target, 5000)
    assert success
    print("✓ Ship orbiting target")
    
    print("✅ Navigation system tests passed!\n")


def test_docking():
    """Test docking system"""
    print("=== Testing Docking System ===")
    
    world = World()
    
    # Create ship near station
    ship = world.create_entity("ship")
    ship.add_component(Position(x=1000, y=0, z=0))
    ship.add_component(Velocity(max_speed=300))
    
    # Create station
    station = world.create_entity("station")
    station.add_component(Position(x=0, y=0, z=0))
    station.add_component(Celestial(
        celestial_type="station",
        celestial_name="Test Station",
        dockable=True,
        services=['repair', 'market', 'fitting']
    ))
    
    nav_sys = NavigationSystem(world)
    
    # Test docking
    success = nav_sys.dock(ship, station)
    assert success
    print("✓ Ship docked successfully")
    
    docking = ship.get_component(Docking)
    assert docking is not None
    assert docking.is_docked
    assert docking.docked_at == station.id
    print(f"✓ Ship is docked at {docking.docked_at}")
    
    # Test undocking
    success = nav_sys.undock(ship)
    assert success
    assert not docking.is_docked
    print("✓ Ship undocked successfully")
    
    print("✅ Docking tests passed!\n")


def run_all_tests():
    """Run all advanced tests"""
    print("\n" + "="*50)
    print("EVE OFFLINE - Advanced Systems Tests")
    print("="*50)
    
    try:
        test_mission_system()
        test_navigation_system()
        test_docking()
        
        print("="*50)
        print("✅ ALL ADVANCED TESTS PASSED!")
        print("="*50 + "\n")
        
    except Exception as e:
        print(f"\n❌ TEST FAILED: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    run_all_tests()
