"""
Atlas / EVEOFFLINE integration module
Reads ship definitions from EVEOFFLINE JSON data files and exports
generated ships as OBJ models for the Atlas engine pipeline.

EVEOFFLINE project: https://github.com/shifty81/EVEOFFLINE
"""

import json
import os

# Faction-to-style mapping for EVEOFFLINE factions
FACTION_STYLE_MAP = {
    'Solari': 'SOLARI',
    'Veyren': 'VEYREN',
    'Aurelian': 'AURELIAN',
    'Keldari': 'KELDARI',
}

# EVEOFFLINE ship class mapping to generator ship classes
EVEOFFLINE_CLASS_MAP = {
    'Frigate': 'FRIGATE',
    'Destroyer': 'DESTROYER',
    'Cruiser': 'CRUISER',
    'Battlecruiser': 'BATTLECRUISER',
    'Battleship': 'BATTLESHIP',
    'Carrier': 'CARRIER',
    'Dreadnought': 'DREADNOUGHT',
    'Titan': 'TITAN',
    'Industrial': 'INDUSTRIAL',
    'Mining Barge': 'MINING_BARGE',
    'Exhumer': 'EXHUMER',
}


def load_ship_data(json_path):
    """
    Load ship definitions from an EVEOFFLINE JSON data file.

    Args:
        json_path: Path to a ship JSON file (e.g. frigates.json)

    Returns:
        Dictionary of ship definitions keyed by ship id
    """
    with open(json_path, 'r') as f:
        return json.load(f)


def parse_ship_config(ship_data):
    """
    Convert an EVEOFFLINE ship definition to generator parameters.

    Args:
        ship_data: Single ship dict from EVEOFFLINE JSON

    Returns:
        Dictionary of parameters for generate_spaceship()
    """
    model = ship_data.get('model_data', {})
    ship_class_name = ship_data.get('class', 'Frigate')
    race = ship_data.get('race', '')

    ship_class = EVEOFFLINE_CLASS_MAP.get(ship_class_name, 'FRIGATE')
    style = FACTION_STYLE_MAP.get(race, 'MIXED')
    seed = model.get('generation_seed', 1)

    turrets = model.get('turret_hardpoints', 0)
    launchers = model.get('launcher_hardpoints', 0)
    total_weapons = turrets + launchers

    engines = model.get('engine_count', 2)

    high_slots = ship_data.get('high_slots', 0)
    mid_slots = ship_data.get('mid_slots', 0)
    low_slots = ship_data.get('low_slots', 0)
    module_slots = min((high_slots + mid_slots + low_slots) // 3, 10)

    return {
        'ship_class': ship_class,
        'seed': seed,
        'generate_interior': True,
        'module_slots': module_slots,
        'hull_complexity': 1.0,
        'symmetry': True,
        'style': style,
        'ship_name': ship_data.get('name', ''),
        'weapons_override': total_weapons,
        'engines_override': engines,
        'turret_hardpoints': min(turrets, 10),
    }


def load_all_ships(data_dir):
    """
    Load all ship definitions from an EVEOFFLINE data/ships directory.

    Args:
        data_dir: Path to the data/ships directory

    Returns:
        Dictionary of all ship definitions keyed by ship id
    """
    all_ships = {}
    if not os.path.isdir(data_dir):
        return all_ships

    for filename in sorted(os.listdir(data_dir)):
        if filename.endswith('.json'):
            filepath = os.path.join(data_dir, filename)
            try:
                ships = load_ship_data(filepath)
                all_ships.update(ships)
            except (json.JSONDecodeError, IOError):
                continue

    return all_ships


def export_obj(filepath):
    """
    Export the active ship as OBJ for the EVEOFFLINE asset pipeline.
    Must be called inside Blender with an active object.

    Args:
        filepath: Output .obj file path
    """
    import bpy

    bpy.ops.wm.obj_export(
        filepath=filepath,
        export_selected_objects=True,
        apply_modifiers=True,
        export_uv=True,
        export_normals=True,
        export_materials=True,
        forward_axis='NEGATIVE_Z',
        up_axis='Y',
    )


def register():
    """Register this module"""
    pass


def unregister():
    """Unregister this module"""
    pass
