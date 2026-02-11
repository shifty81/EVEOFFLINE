#!/usr/bin/env python3
"""
EVE OFFLINE - Interactive Ship Creator

Guides you through creating a new ship JSON definition with all required fields.

Usage:
    python tools/create_ship.py
"""

import json
import sys
from pathlib import Path


def prompt(message, default=None, type_=str, choices=None):
    """Prompt user for input with optional default and validation"""
    if default is not None:
        message = f"{message} [{default}]"
    if choices:
        message = f"{message} ({'/'.join(choices)})"
    message += ": "
    
    while True:
        value = input(message).strip()
        if not value and default is not None:
            return default
        if not value:
            print("This field is required. Please enter a value.")
            continue
        
        if choices and value not in choices:
            print(f"Invalid choice. Please choose from: {', '.join(choices)}")
            continue
        
        if type_ == int:
            try:
                return int(value)
            except ValueError:
                print("Please enter a valid integer.")
                continue
        elif type_ == float:
            try:
                return float(value)
            except ValueError:
                print("Please enter a valid number.")
                continue
        
        return value


def main():
    print("=" * 60)
    print(" EVE OFFLINE - Interactive Ship Creator")
    print("=" * 60)
    print()
    print("This tool will guide you through creating a new ship definition.")
    print("Press Ctrl+C at any time to cancel.")
    print()
    
    try:
        # Basic information
        print("=== Basic Information ===")
        ship_id = prompt("Ship ID (e.g., 'custom_frigate')")
        name = prompt("Display Name (e.g., 'Custom Frigate')")
        
        # Ship class
        print("\n=== Ship Class ===")
        class_choices = [
            'Frigate', 'Destroyer', 'Cruiser', 'Battlecruiser', 'Battleship',
            'Carrier', 'Dreadnought', 'Titan',
            'Mining Barge', 'Exhumer', 'Industrial',
            'Assault Frigate', 'Interceptor', 'Covert Ops',
            'Heavy Assault Cruiser', 'Heavy Interdictor', 'Recon Ship',
            'Logistics Cruiser', 'Command Ship', 'Interdictor',
            'Stealth Bomber', 'Marauder'
        ]
        print("Available classes:")
        for i, cls in enumerate(class_choices, 1):
            print(f"  {i}. {cls}")
        class_idx = prompt("Select class number", type_=int)
        if class_idx < 1 or class_idx > len(class_choices):
            print(f"Invalid class number. Using 'Frigate'.")
            ship_class = 'Frigate'
        else:
            ship_class = class_choices[class_idx - 1]
        
        # Race
        print("\n=== Race ===")
        race = prompt("Race", choices=['Solari', 'Veyren', 'Aurelian', 'Keldari'])
        
        # Description
        print("\n=== Description ===")
        description = prompt("Ship description")
        
        # Hull stats
        print("\n=== Hull Stats ===")
        hull_hp = prompt("Hull HP", default=350, type_=int)
        armor_hp = prompt("Armor HP", default=400, type_=int)
        shield_hp = prompt("Shield HP", default=450, type_=int)
        capacitor = prompt("Capacitor (GJ)", default=280, type_=int)
        
        # Fitting
        print("\n=== Fitting ===")
        cpu = prompt("CPU (tf)", default=140, type_=int)
        powergrid = prompt("PowerGrid (MW)", default=38, type_=int)
        
        # Slots
        print("\n=== Module Slots ===")
        high_slots = prompt("High Slots", default=3, type_=int)
        mid_slots = prompt("Mid Slots", default=3, type_=int)
        low_slots = prompt("Low Slots", default=3, type_=int)
        rig_slots = prompt("Rig Slots", default=3, type_=int)
        
        # Mobility
        print("\n=== Mobility ===")
        cargo_capacity = prompt("Cargo Capacity (m³)", default=120, type_=int)
        max_velocity = prompt("Max Velocity (m/s)", default=325, type_=int)
        inertia = prompt("Inertia Modifier", default=2.8, type_=float)
        
        # Targeting
        print("\n=== Targeting ===")
        signature_radius = prompt("Signature Radius (m)", default=37, type_=int)
        scan_resolution = prompt("Scan Resolution", default=620, type_=int)
        max_locked_targets = prompt("Max Locked Targets", default=4, type_=int)
        max_targeting_range = prompt("Max Targeting Range (m)", default=18000, type_=int)
        
        # Recharge rates
        print("\n=== Recharge Rates ===")
        shield_recharge_time = prompt("Shield Recharge Time (s)", default=625, type_=int)
        capacitor_recharge_time = prompt("Capacitor Recharge Time (s)", default=157, type_=int)
        
        # Bonuses (optional)
        print("\n=== Bonuses (Optional) ===")
        print("Enter bonuses as 'bonus_name: value' pairs.")
        print("Examples: small_projectile_damage: 5, shield_boost_amount: 10")
        print("Press Enter with empty input when done.")
        bonuses = {}
        while True:
            bonus = input("Bonus (or press Enter to finish): ").strip()
            if not bonus:
                break
            if ':' not in bonus:
                print("Invalid format. Use 'bonus_name: value'")
                continue
            key, value = bonus.split(':', 1)
            try:
                bonuses[key.strip()] = float(value.strip())
            except ValueError:
                print("Invalid value. Please enter a number.")
        
        # Build ship dictionary
        ship_data = {
            ship_id: {
                "id": ship_id,
                "name": name,
                "class": ship_class,
                "race": race,
                "description": description,
                "hull_hp": hull_hp,
                "armor_hp": armor_hp,
                "shield_hp": shield_hp,
                "capacitor": capacitor,
                "cpu": cpu,
                "powergrid": powergrid,
                "high_slots": high_slots,
                "mid_slots": mid_slots,
                "low_slots": low_slots,
                "rig_slots": rig_slots,
                "cargo_capacity": cargo_capacity,
                "max_velocity": max_velocity,
                "inertia_modifier": inertia,
                "signature_radius": signature_radius,
                "scan_resolution": scan_resolution,
                "max_locked_targets": max_locked_targets,
                "max_targeting_range": max_targeting_range,
                "shield_recharge_time": shield_recharge_time,
                "capacitor_recharge_time": capacitor_recharge_time
            }
        }
        
        if bonuses:
            ship_data[ship_id]["bonuses"] = bonuses
        
        # Display result
        print("\n" + "=" * 60)
        print(" Ship Definition Generated")
        print("=" * 60)
        print()
        print(json.dumps(ship_data, indent=2))
        print()
        
        # Save option
        save = prompt("Save to file?", choices=['y', 'n'], default='y')
        if save == 'y':
            filename = prompt("Output filename", default=f"data/ships/custom/{ship_id}.json")
            filepath = Path(filename)
            filepath.parent.mkdir(parents=True, exist_ok=True)
            
            # Check if file exists
            if filepath.exists():
                overwrite = prompt(f"{filename} exists. Overwrite?", choices=['y', 'n'], default='n')
                if overwrite != 'y':
                    print("Cancelled. Ship definition not saved.")
                    return 0
            
            with open(filepath, 'w') as f:
                json.dump(ship_data, f, indent=2)
                f.write('\n')  # Add trailing newline
            
            print(f"\n✓ Ship saved to {filename}")
            print(f"\nTo use this ship, add it to your ship database loader or")
            print(f"merge it into an existing ship JSON file in data/ships/")
        
    except KeyboardInterrupt:
        print("\n\nCancelled.")
        return 1
    except Exception as e:
        print(f"\nError: {e}")
        return 1
    
    return 0


if __name__ == '__main__':
    sys.exit(main())
