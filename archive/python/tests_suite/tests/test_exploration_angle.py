"""
Test directional scanning angle checking in exploration system
"""
import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import math
from engine.core.ecs import World, Entity
from engine.components.game_components import Position
from engine.systems.exploration_system import ExplorationSystem, ShipScanner


def test_directional_scan_full_sphere():
    """Test that 360-degree scan detects all entities in range"""
    world = World()
    exp_system = ExplorationSystem(world)
    
    # Create scanner ship at origin
    scanner = world.create_entity()
    scanner.add_component(Position(x=0.0, y=0.0, z=0.0, rotation=0.0))
    scanner.add_component(ShipScanner(
        max_range=10.0,
        scan_angle=360.0,  # Full sphere
        scan_cooldown=0.0
    ))
    
    # Create targets around the scanner
    target1 = world.create_entity()
    target1.add_component(Position(x=5.0, y=0.0, z=0.0))  # In front
    
    target2 = world.create_entity()
    target2.add_component(Position(x=-5.0, y=0.0, z=0.0))  # Behind
    
    target3 = world.create_entity()
    target3.add_component(Position(x=0.0, y=5.0, z=0.0))  # To the side
    
    # Perform scan
    detected = exp_system.directional_scan(scanner, current_time=0.0)
    
    # All targets should be detected with 360-degree scan
    assert len(detected) == 3, f"Expected 3 targets, got {len(detected)}"
    print("✓ Full sphere (360°) scan detects all targets")


def test_directional_scan_cone():
    """Test that narrow cone only detects entities in front"""
    world = World()
    exp_system = ExplorationSystem(world)
    
    # Create scanner ship at origin, facing right (rotation=0)
    scanner = world.create_entity()
    scanner.add_component(Position(x=0.0, y=0.0, z=0.0, rotation=0.0))
    scanner.add_component(ShipScanner(
        max_range=10.0,
        scan_angle=90.0,  # 90-degree cone (45° on each side)
        scan_cooldown=0.0
    ))
    
    # Create targets
    target1 = world.create_entity()
    target1.add_component(Position(x=5.0, y=0.0, z=0.0))  # Directly in front (0°)
    
    target2 = world.create_entity()
    target2.add_component(Position(x=5.0, y=2.0, z=0.0))  # Slightly to side (~22°)
    
    target3 = world.create_entity()
    target3.add_component(Position(x=-5.0, y=0.0, z=0.0))  # Behind (180°)
    
    target4 = world.create_entity()
    target4.add_component(Position(x=0.0, y=5.0, z=0.0))  # 90° to side
    
    # Perform scan
    detected = exp_system.directional_scan(scanner, current_time=0.0)
    
    # Only targets 1 and 2 should be detected (within 45° of forward)
    assert len(detected) == 2, f"Expected 2 targets in 90° cone, got {len(detected)}"
    assert target1 in detected, "Target directly in front should be detected"
    assert target2 in detected, "Target slightly to side should be detected"
    assert target3 not in detected, "Target behind should NOT be detected"
    print("✓ Cone scan (90°) detects only targets in front")


def test_directional_scan_narrow_cone():
    """Test narrow 30-degree cone"""
    world = World()
    exp_system = ExplorationSystem(world)
    
    # Create scanner facing at 45 degrees (northeast)
    scanner = world.create_entity()
    scanner.add_component(Position(x=0.0, y=0.0, z=0.0, rotation=math.radians(45)))
    scanner.add_component(ShipScanner(
        max_range=10.0,
        scan_angle=30.0,  # Narrow 30-degree cone
        scan_cooldown=0.0
    ))
    
    # Create targets
    # At 45 degrees, the cone covers 30° to 60°
    target1 = world.create_entity()
    # Place at 45 degrees (within cone)
    dist = 5.0
    target1.add_component(Position(
        x=dist * math.cos(math.radians(45)),
        y=dist * math.sin(math.radians(45)),
        z=0.0
    ))
    
    target2 = world.create_entity()
    # Place at 0 degrees (outside cone - too far right)
    target2.add_component(Position(x=5.0, y=0.0, z=0.0))
    
    target3 = world.create_entity()
    # Place at 90 degrees (outside cone - too far left)
    target3.add_component(Position(x=0.0, y=5.0, z=0.0))
    
    # Perform scan
    detected = exp_system.directional_scan(scanner, current_time=0.0)
    
    # Only target1 should be detected
    assert len(detected) == 1, f"Expected 1 target in 30° cone, got {len(detected)}"
    assert target1 in detected, "Target in cone direction should be detected"
    print("✓ Narrow cone (30°) detects only targets in narrow angle")


def test_directional_scan_range():
    """Test that scan respects max range"""
    world = World()
    exp_system = ExplorationSystem(world)
    
    # Create scanner with limited range
    scanner = world.create_entity()
    scanner.add_component(Position(x=0.0, y=0.0, z=0.0, rotation=0.0))
    scanner.add_component(ShipScanner(
        max_range=5.0,  # Limited range
        scan_angle=360.0,
        scan_cooldown=0.0
    ))
    
    # Create targets at different ranges
    near_target = world.create_entity()
    near_target.add_component(Position(x=3.0, y=0.0, z=0.0))  # Within range
    
    far_target = world.create_entity()
    far_target.add_component(Position(x=10.0, y=0.0, z=0.0))  # Out of range
    
    # Perform scan
    detected = exp_system.directional_scan(scanner, current_time=0.0)
    
    # Only near target should be detected
    assert len(detected) == 1, f"Expected 1 target in range, got {len(detected)}"
    assert near_target in detected, "Near target should be detected"
    assert far_target not in detected, "Far target should NOT be detected"
    print("✓ Scan respects max range")


def test_directional_scan_cooldown():
    """Test that scan respects cooldown"""
    world = World()
    exp_system = ExplorationSystem(world)
    
    # Create scanner with cooldown
    scanner = world.create_entity()
    scanner.add_component(Position(x=0.0, y=0.0, z=0.0, rotation=0.0))
    scanner.add_component(ShipScanner(
        max_range=10.0,
        scan_angle=360.0,
        scan_cooldown=5.0  # 5 second cooldown
    ))
    
    # Create a target
    target = world.create_entity()
    target.add_component(Position(x=5.0, y=0.0, z=0.0))
    
    # First scan at t=0
    detected1 = exp_system.directional_scan(scanner, current_time=0.0)
    assert len(detected1) == 1, "First scan should detect target"
    
    # Second scan at t=2 (within cooldown)
    detected2 = exp_system.directional_scan(scanner, current_time=2.0)
    assert detected2 == detected1, "Scan within cooldown should return cached results"
    
    # Third scan at t=6 (after cooldown)
    detected3 = exp_system.directional_scan(scanner, current_time=6.0)
    assert len(detected3) == 1, "Scan after cooldown should perform new scan"
    print("✓ Scan cooldown works correctly")


if __name__ == "__main__":
    print("=== Testing Directional Scan Angle Checking ===\n")
    
    test_directional_scan_full_sphere()
    test_directional_scan_cone()
    test_directional_scan_narrow_cone()
    test_directional_scan_range()
    test_directional_scan_cooldown()
    
    print("\n" + "=" * 50)
    print("✅ ALL EXPLORATION ANGLE TESTS PASSED!")
    print("=" * 50)
