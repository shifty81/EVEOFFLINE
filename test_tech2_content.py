"""
Test for Tech II ships and modules loading
"""

import sys
import os

# Add the parent directory to the path
sys.path.insert(0, os.path.abspath('.'))

from engine.utils.data_loader import DataLoader

def test_tech2_content():
    """Test that Tech II frigates and modules load correctly"""
    print("Testing Tech II Content Loading")
    print("=" * 60)
    
    # Create data loader
    loader = DataLoader(data_dir="data")
    
    # Load all data
    loader.load_all()
    
    print("\n" + "=" * 60)
    print("Tech II Frigates")
    print("=" * 60)
    
    # Check for Tech II frigates
    tech2_frigates = [
        "jaguar", "hawk", "enyo", "retribution", "wolf", "harpy"
    ]
    
    found_frigates = []
    for frigate_id in tech2_frigates:
        if frigate_id in loader.ships:
            ship = loader.ships[frigate_id]
            found_frigates.append(frigate_id)
            print(f"✅ {ship['name']} ({ship['race']}) - {ship['class']}")
            print(f"   Hull: {ship['hull_hp']} | Armor: {ship['armor_hp']} | Shield: {ship['shield_hp']}")
            print(f"   Slots: {ship['high_slots']}H / {ship['mid_slots']}M / {ship['low_slots']}L")
            if 'tech_level' in ship:
                print(f"   Tech Level: {ship['tech_level']}")
        else:
            print(f"❌ {frigate_id} - NOT FOUND")
    
    print(f"\nFound {len(found_frigates)}/{len(tech2_frigates)} Tech II frigates")
    
    print("\n" + "=" * 60)
    print("Tech II Modules")
    print("=" * 60)
    
    # Check for key Tech II modules
    tech2_modules = [
        "assault_damage_control",
        "200mm_autocannon_ii",
        "small_pulse_laser_ii",
        "light_neutron_blaster_ii",
        "rocket_launcher_ii",
        "damage_control_ii",
        "magnetic_field_stabilizer_ii",
        "gyrostabilizer_ii"
    ]
    
    found_modules = []
    for module_id in tech2_modules:
        if module_id in loader.modules:
            module = loader.modules[module_id]
            found_modules.append(module_id)
            print(f"✅ {module['name']}")
            print(f"   Type: {module['type']} | Slot: {module['slot']}")
            print(f"   CPU: {module['cpu']} | PG: {module['powergrid']}")
            if 'tech_level' in module:
                print(f"   Tech Level: {module['tech_level']}")
        else:
            print(f"❌ {module_id} - NOT FOUND")
    
    print(f"\nFound {len(found_modules)}/{len(tech2_modules)} Tech II modules")
    
    print("\n" + "=" * 60)
    print("Summary")
    print("=" * 60)
    print(f"Total Ships: {len(loader.ships)}")
    print(f"Total Modules: {len(loader.modules)}")
    print(f"Total Skills: {len(loader.skills)}")
    print(f"Total NPCs: {len(loader.npcs)}")
    print(f"Total Missions: {len(loader.missions)}")
    
    # Check if all expected content was found
    if len(found_frigates) == len(tech2_frigates) and len(found_modules) == len(tech2_modules):
        print("\n✅ ALL TECH II CONTENT LOADED SUCCESSFULLY!")
        return True
    else:
        print(f"\n❌ Missing content:")
        print(f"   Frigates: {len(tech2_frigates) - len(found_frigates)} missing")
        print(f"   Modules: {len(tech2_modules) - len(found_modules)} missing")
        return False

if __name__ == "__main__":
    success = test_tech2_content()
    sys.exit(0 if success else 1)
