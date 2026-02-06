"""
Test for Name Generator
"""

import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from engine.utils.name_generator import NameGenerator


def test_character_names():
    print("Testing Character Name Generation...")
    for _ in range(10):
        male_name = NameGenerator.generate_character_name('male')
        female_name = NameGenerator.generate_character_name('female')
        random_name = NameGenerator.generate_character_name()
        
        assert len(male_name.split()) == 2, "Male name should have two parts"
        assert len(female_name.split()) == 2, "Female name should have two parts"
        assert len(random_name.split()) == 2, "Random name should have two parts"
    
    print("✓ Character names test passed")


def test_ship_names():
    print("Testing Ship Name Generation...")
    styles = ['heroic', 'celestial', 'mythic', 'descriptive']
    
    for style in styles:
        name = NameGenerator.generate_ship_name(style)
        parts = name.split()
        assert len(parts) >= 2, f"Ship name '{name}' should have at least 2 parts"
        assert parts[0] in NameGenerator.SHIP_PREFIXES, f"Invalid ship prefix: {parts[0]}"
    
    # Test random style
    for _ in range(5):
        name = NameGenerator.generate_ship_name()
        assert len(name.split()) >= 2, "Ship name should have at least 2 parts"
    
    print("✓ Ship names test passed")


def test_corporation_names():
    print("Testing Corporation Name Generation...")
    for _ in range(10):
        name = NameGenerator.generate_corporation_name()
        parts = name.split()
        assert len(parts) >= 2, "Corporation name should have at least 2 parts"
    
    print("✓ Corporation names test passed")


def test_system_names():
    print("Testing System Name Generation...")
    for _ in range(10):
        name = NameGenerator.generate_system_name()
        parts = name.split()
        assert len(parts) >= 2, "System name should have at least 2 parts"
        assert len(parts) <= 3, "System name should have at most 3 parts"
    
    print("✓ System names test passed")


def test_station_names():
    print("Testing Station Name Generation...")
    for _ in range(10):
        name = NameGenerator.generate_station_name()
        assert len(name) > 0, "Station name should not be empty"
        
        # Test with system name
        system = "Alpha Centauri"
        station = NameGenerator.generate_station_name(system)
        assert len(station) > 0, "Station name should not be empty"
    
    print("✓ Station names test passed")


def test_mission_names():
    print("Testing Mission Name Generation...")
    for _ in range(10):
        name = NameGenerator.generate_mission_name()
        parts = name.split()
        assert len(parts) == 2, "Mission name should have exactly 2 parts"
    
    print("✓ Mission names test passed")


def test_exploration_sites():
    print("Testing Exploration Site Name Generation...")
    for _ in range(10):
        name = NameGenerator.generate_exploration_site_name()
        parts = name.split()
        assert len(parts) == 2, "Exploration site name should have exactly 2 parts"
    
    print("✓ Exploration site names test passed")


def test_pirate_names():
    print("Testing Pirate Name Generation...")
    for _ in range(10):
        name = NameGenerator.generate_pirate_name()
        parts = name.split()
        assert len(parts) == 2, "Pirate name should have exactly 2 parts"
    
    print("✓ Pirate names test passed")


def test_callsigns():
    print("Testing Pilot Callsign Generation...")
    for _ in range(10):
        callsign = NameGenerator.generate_pilot_callsign()
        assert '-' in callsign, "Callsign should contain a hyphen"
        parts = callsign.split('-')
        assert len(parts) == 2, "Callsign should have name and number"
        assert parts[1].isdigit(), "Callsign number should be digits"
    
    print("✓ Pilot callsigns test passed")


def test_asteroid_designations():
    print("Testing Asteroid Designation Generation...")
    for _ in range(10):
        designation = NameGenerator.generate_asteroid_designation()
        assert '-' in designation, "Asteroid designation should contain a hyphen"
        parts = designation.split('-')
        assert len(parts) == 2, "Asteroid designation should have ore type and number"
        assert parts[1].isdigit(), "Asteroid number should be digits"
    
    print("✓ Asteroid designations test passed")


def test_uniqueness():
    print("Testing Name Uniqueness...")
    
    # Generate many names and check for reasonable variety
    names = set()
    for _ in range(100):
        names.add(NameGenerator.generate_character_name())
    
    # Should have at least 50 unique names out of 100
    assert len(names) > 50, f"Expected > 50 unique names, got {len(names)}"
    
    print(f"✓ Uniqueness test passed (generated {len(names)} unique names out of 100)")


def print_examples():
    print("\n" + "=" * 60)
    print("Example Generated Names")
    print("=" * 60)
    
    print("\nCharacter Names:")
    for i in range(5):
        print(f"  {i+1}. {NameGenerator.generate_character_name()}")
    
    print("\nShip Names:")
    for style in ['heroic', 'celestial', 'mythic', 'descriptive']:
        print(f"  {style.title()}: {NameGenerator.generate_ship_name(style)}")
    
    print("\nCorporation Names:")
    for i in range(3):
        print(f"  {i+1}. {NameGenerator.generate_corporation_name()}")
    
    print("\nSystem & Station Names:")
    for i in range(3):
        system = NameGenerator.generate_system_name()
        station = NameGenerator.generate_station_name(system)
        print(f"  {i+1}. System: {system}")
        print(f"      Station: {station}")
    
    print("\nMission Names:")
    for i in range(5):
        print(f"  {i+1}. {NameGenerator.generate_mission_name()}")
    
    print("\nExploration Sites:")
    for i in range(3):
        print(f"  {i+1}. {NameGenerator.generate_exploration_site_name()}")
    
    print("\nPilot Callsigns:")
    for i in range(5):
        print(f"  {i+1}. {NameGenerator.generate_pilot_callsign()}")
    
    print("\nPirate Names:")
    for i in range(3):
        print(f"  {i+1}. {NameGenerator.generate_pirate_name()}")
    
    print("\n" + "=" * 60)


if __name__ == "__main__":
    print("=" * 60)
    print("EVE OFFLINE Name Generator Test Suite")
    print("=" * 60)
    print()
    
    try:
        test_character_names()
        test_ship_names()
        test_corporation_names()
        test_system_names()
        test_station_names()
        test_mission_names()
        test_exploration_sites()
        test_pirate_names()
        test_callsigns()
        test_asteroid_designations()
        test_uniqueness()
        
        print()
        print("=" * 60)
        print("✓ All tests passed!")
        print("=" * 60)
        
        print_examples()
        
    except AssertionError as e:
        print(f"\n✗ Test failed: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"\n✗ Unexpected error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
