"""
Demo: Name Generator in Game Scenarios
Shows how the name generator integrates with actual gameplay
"""

import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from engine.utils.name_generator import NameGenerator


def demo_character_creation():
    """Simulate character creation"""
    print("=" * 70)
    print("SCENARIO 1: Character Creation")
    print("=" * 70)
    print("\nNew players joining the server:\n")
    
    for i in range(5):
        char_name = NameGenerator.generate_character_name()
        ship_name = NameGenerator.generate_ship_name('heroic')
        print(f"  Player {i+1}: {char_name}")
        print(f"    Starting ship: {ship_name}")
        print()


def demo_pirate_encounter():
    """Simulate a pirate encounter"""
    print("=" * 70)
    print("SCENARIO 2: Pirate Encounter")
    print("=" * 70)
    print("\nYou've entered a dangerous asteroid belt...\n")
    
    fleet_leader = NameGenerator.generate_pirate_name()
    print(f"⚠ WARNING: {fleet_leader} and their fleet detected!")
    print(f"\nEnemy Fleet:")
    print(f"  Commander: {fleet_leader}")
    
    for i in range(4):
        callsign = NameGenerator.generate_pilot_callsign()
        print(f"  Wingman {i+1}: Serpentis {callsign}")
    
    print()


def demo_mission_board():
    """Simulate mission selection"""
    print("=" * 70)
    print("SCENARIO 3: Mission Board")
    print("=" * 70)
    print("\nAvailable missions at station:\n")
    
    levels = [1, 2, 2, 3, 4]
    rewards = [50000, 150000, 200000, 500000, 1000000]
    
    for i, (level, reward) in enumerate(zip(levels, rewards)):
        mission_name = NameGenerator.generate_mission_name()
        print(f"  [{i+1}] {mission_name}")
        print(f"      Level: {level} | Reward: {reward:,} ISK")
    
    print()


def demo_exploration():
    """Simulate exploration discoveries"""
    print("=" * 70)
    print("SCENARIO 4: Exploration Discovery")
    print("=" * 70)
    print("\nScanning for cosmic signatures...\n")
    
    system = NameGenerator.generate_system_name()
    print(f"Current System: {system}\n")
    print(f"Discovered signatures:")
    
    for i in range(5):
        site = NameGenerator.generate_exploration_site_name()
        sig_id = f"ABC-{i+100}"
        site_types = ["Combat", "Data", "Relic", "Gas", "Wormhole"]
        print(f"  {sig_id}: {site} ({site_types[i]})")
    
    print()


def demo_mining_operation():
    """Simulate mining operation"""
    print("=" * 70)
    print("SCENARIO 5: Mining Operation")
    print("=" * 70)
    
    system = NameGenerator.generate_system_name()
    print(f"\nMining in {system} - Belt Alpha-3\n")
    print(f"Asteroid Field:")
    
    for i in range(8):
        asteroid = NameGenerator.generate_asteroid_designation()
        size = ["Small", "Medium", "Large", "Huge"][i % 4]
        print(f"  [{i+1}] {asteroid} ({size})")
    
    print()


def demo_corporation_founding():
    """Simulate corporation creation"""
    print("=" * 70)
    print("SCENARIO 6: Corporation Founding")
    print("=" * 70)
    print("\nNew corporations established this week:\n")
    
    for i in range(5):
        corp = NameGenerator.generate_corporation_name()
        ceo = NameGenerator.generate_character_name()
        members = i * 3 + 5
        print(f"  {corp}")
        print(f"    CEO: {ceo} | Members: {members}")
    
    print()


def demo_universe_generation():
    """Simulate universe/system generation"""
    print("=" * 70)
    print("SCENARIO 7: Universe Generation")
    print("=" * 70)
    print("\nGenerating new solar systems:\n")
    
    for i in range(3):
        system = NameGenerator.generate_system_name()
        sec_status = round(0.1 + i * 0.3, 1)
        print(f"  {system} (Security: {sec_status})")
        
        # Generate stations
        num_stations = 2 + i
        for j in range(num_stations):
            station = NameGenerator.generate_station_name(system if j == 0 else None)
            print(f"    └─ {station}")
        print()


def demo_fleet_composition():
    """Simulate fleet naming"""
    print("=" * 70)
    print("SCENARIO 8: Fleet Composition")
    print("=" * 70)
    print("\nAssembling fleet for incursion:\n")
    
    print("  Command Ship:")
    fc_name = NameGenerator.generate_character_name()
    fc_ship = NameGenerator.generate_ship_name('mythic')
    print(f"    FC: {fc_name} in {fc_ship}")
    
    print("\n  DPS Squadron:")
    for i in range(3):
        pilot = NameGenerator.generate_pilot_callsign()
        ship = NameGenerator.generate_ship_name('heroic')
        print(f"    {pilot} - {ship}")
    
    print("\n  Logistics Wing:")
    for i in range(2):
        pilot = NameGenerator.generate_character_name()
        ship = NameGenerator.generate_ship_name('celestial')
        print(f"    {pilot} - {ship}")
    
    print()


def main():
    """Run all demo scenarios"""
    print("\n")
    print("╔" + "═" * 68 + "╗")
    print("║" + " " * 10 + "EVE OFFLINE - Name Generator Game Demo" + " " * 18 + "║")
    print("╚" + "═" * 68 + "╝")
    print()
    
    demo_character_creation()
    demo_pirate_encounter()
    demo_mission_board()
    demo_exploration()
    demo_mining_operation()
    demo_corporation_founding()
    demo_universe_generation()
    demo_fleet_composition()
    
    print("=" * 70)
    print("Demo Complete!")
    print("=" * 70)
    print("\nThe name generator creates unique, EVE-like names for all")
    print("game elements, enhancing immersion and reducing repetitive content.")
    print()


if __name__ == "__main__":
    main()
