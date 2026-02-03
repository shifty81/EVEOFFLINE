#!/usr/bin/env python3
"""
Test script for ShipModelGenerator
Validates the procedural ship model generation without requiring graphics display
"""

from client_3d.rendering.ship_models import ShipModelGenerator


def test_ship_generator():
    """Test the ship model generator"""
    print("="*70)
    print("Testing ShipModelGenerator")
    print("="*70)
    
    generator = ShipModelGenerator()
    
    # Test ship types
    ship_types = [
        # Tech I Frigates
        "Rifter",
        "Merlin",
        "Tristan",
        "Punisher",
        # Tech II Assault Frigates
        "Jaguar",
        "Hawk",
        "Enyo",
        "Retribution",
        "Wolf",
        "Harpy",
        # Destroyers
        "Thrasher",
        "Cormorant",
        "Catalyst",
        "Coercer",
        # Cruisers
        "Stabber",
        "Caracal",
        "Vexor",
        "Maller",
        # Tech II Heavy Assault Cruisers
        "Vagabond",
        "Cerberus",
        "Ishtar",
        "Zealot",
        # Tech II Heavy Interdiction Cruisers
        "Broadsword",
        "Onyx",
        "Phobos",
        "Devoter",
        # Tech II Force Recon Ships
        "Huginn",
        "Rapier",
        "Falcon",
        "Arazu",
        "Pilgrim",
        # Tech II Combat Recon Ships
        "Rook",
        "Lachesis",
        "Curse",
        # Tech II Logistics Cruisers
        "Scimitar",
        "Basilisk",
        "Oneiros",
        "Guardian",
        # Battlecruisers
        "Cyclone",
        "Ferox",
        "Brutix",
        "Harbinger",
        # Battleships
        "Tempest",
        "Raven",
        "Dominix",
        "Apocalypse"
    ]
    
    # Test factions
    factions = [
        "Minmatar",
        "Caldari",
        "Gallente",
        "Amarr",
        "Serpentis",
        "Guristas",
        "Blood Raiders"
    ]
    
    print("\nTesting ship model generation...")
    success_count = 0
    total_tests = 0
    
    for ship_type in ship_types:
        for faction in factions:
            total_tests += 1
            try:
                model = generator.generate_ship_model(ship_type, faction)
                if model is not None:
                    # Check that model has geometry
                    if model.getName():
                        success_count += 1
                        print(f"✓ {faction:15s} {ship_type:15s} - Generated successfully")
                    else:
                        print(f"✗ {faction:15s} {ship_type:15s} - No name")
                else:
                    print(f"✗ {faction:15s} {ship_type:15s} - Model is None")
            except Exception as e:
                print(f"✗ {faction:15s} {ship_type:15s} - Error: {e}")
    
    print("\n" + "="*70)
    print(f"Results: {success_count}/{total_tests} models generated successfully")
    print("="*70)
    
    # Test caching
    print("\nTesting model caching...")
    cache_size_before = len(generator.model_cache)
    model1 = generator.generate_ship_model("Rifter", "Minmatar")
    cache_size_after = len(generator.model_cache)
    
    if cache_size_after > cache_size_before:
        print("✓ Model added to cache on first generation")
    
    model2 = generator.generate_ship_model("Rifter", "Minmatar")
    cache_size_final = len(generator.model_cache)
    
    if cache_size_final == cache_size_after:
        print("✓ Model retrieved from cache on second generation")
    else:
        print("✗ Cache not working properly")
    
    print("\n" + "="*70)
    print("Ship Model Generator Test Complete!")
    print("="*70)
    
    return success_count == total_tests


if __name__ == "__main__":
    success = test_ship_generator()
    exit(0 if success else 1)
