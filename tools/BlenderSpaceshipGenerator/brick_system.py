"""
Brick taxonomy and hardpoint grid system for EVE-style LEGO ship building.

Defines a small, well-defined set of brick types that players can snap
together on a 3D grid.  Each brick declares its size, role, and available
hardpoints so the generator (and later the player builder) can validate
placement rules.

Ship DNA
--------
The :func:`generate_ship_dna` helper serialises a generated ship into a
compact JSON structure that can be saved, loaded, and used to reproduce the
exact same ship later.
"""

import json
import math
import random

# ---------------------------------------------------------------------------
# Brick taxonomy
# ---------------------------------------------------------------------------

BRICK_CATEGORIES = {
    'CORE': ['REACTOR_CORE', 'POWER_BUS', 'STRUCTURAL_SPINE'],
    'HULL': ['HULL_PLATE', 'HULL_WEDGE', 'HULL_CORNER', 'ARMOR_BLOCK'],
    'FUNCTION': ['ENGINE_BLOCK', 'THRUSTER', 'CAPACITOR', 'SHIELD_EMITTER'],
    'UTILITY': ['HARDPOINT_MOUNT', 'DOCKING_CLAMP', 'SENSOR_MAST', 'ANTENNA_DISH'],
    'DETAIL': ['PANEL', 'VENT', 'PIPE'],
}

# Every brick type with its default properties
BRICK_TYPES = {
    # --- CORE ---
    'REACTOR_CORE': {
        'category': 'CORE',
        'size': (2, 2, 2),
        'shape': 'box',
        'scale_band': 'primary',
        'hardpoints': [
            {'role': 'power', 'direction': (1, 0, 0)},
            {'role': 'power', 'direction': (-1, 0, 0)},
            {'role': 'power', 'direction': (0, 1, 0)},
            {'role': 'power', 'direction': (0, -1, 0)},
        ],
    },
    'POWER_BUS': {
        'category': 'CORE',
        'size': (1, 3, 1),
        'shape': 'box',
        'scale_band': 'structural',
        'hardpoints': [
            {'role': 'power', 'direction': (0, 1, 0)},
            {'role': 'power', 'direction': (0, -1, 0)},
        ],
    },
    'STRUCTURAL_SPINE': {
        'category': 'CORE',
        'size': (1, 4, 1),
        'shape': 'box',
        'scale_band': 'primary',
        'hardpoints': [
            {'role': 'attach', 'direction': (1, 0, 0)},
            {'role': 'attach', 'direction': (-1, 0, 0)},
            {'role': 'attach', 'direction': (0, 0, 1)},
            {'role': 'attach', 'direction': (0, 0, -1)},
            {'role': 'attach', 'direction': (0, 1, 0)},
            {'role': 'attach', 'direction': (0, -1, 0)},
        ],
    },

    # --- HULL ---
    'HULL_PLATE': {
        'category': 'HULL',
        'size': (2, 2, 1),
        'shape': 'box',
        'scale_band': 'structural',
        'hardpoints': [
            {'role': 'attach', 'direction': (0, 0, -1)},
        ],
    },
    'HULL_WEDGE': {
        'category': 'HULL',
        'size': (2, 2, 1),
        'shape': 'wedge',
        'scale_band': 'structural',
        'hardpoints': [
            {'role': 'attach', 'direction': (0, 0, -1)},
        ],
    },
    'HULL_CORNER': {
        'category': 'HULL',
        'size': (1, 1, 1),
        'shape': 'box',
        'scale_band': 'structural',
        'hardpoints': [
            {'role': 'attach', 'direction': (0, 0, -1)},
        ],
    },
    'ARMOR_BLOCK': {
        'category': 'HULL',
        'size': (2, 2, 2),
        'shape': 'box',
        'scale_band': 'structural',
        'hardpoints': [
            {'role': 'attach', 'direction': (0, 0, -1)},
        ],
    },

    # --- FUNCTION ---
    'ENGINE_BLOCK': {
        'category': 'FUNCTION',
        'size': (2, 3, 2),
        'shape': 'cylinder',
        'scale_band': 'primary',
        'hardpoints': [
            {'role': 'exhaust', 'direction': (0, -1, 0)},
            {'role': 'power', 'direction': (0, 1, 0)},
        ],
    },
    'THRUSTER': {
        'category': 'FUNCTION',
        'size': (1, 1, 1),
        'shape': 'cylinder',
        'scale_band': 'detail',
        'hardpoints': [
            {'role': 'exhaust', 'direction': (0, -1, 0)},
        ],
    },
    'CAPACITOR': {
        'category': 'FUNCTION',
        'size': (1, 2, 1),
        'shape': 'cylinder',
        'scale_band': 'structural',
        'hardpoints': [
            {'role': 'power', 'direction': (0, 1, 0)},
            {'role': 'power', 'direction': (0, -1, 0)},
        ],
    },
    'SHIELD_EMITTER': {
        'category': 'FUNCTION',
        'size': (1, 1, 1),
        'shape': 'sphere',
        'scale_band': 'detail',
        'hardpoints': [
            {'role': 'power', 'direction': (0, 0, -1)},
        ],
    },

    # --- UTILITY ---
    'HARDPOINT_MOUNT': {
        'category': 'UTILITY',
        'size': (1, 1, 1),
        'shape': 'cylinder',
        'scale_band': 'detail',
        'hardpoints': [
            {'role': 'weapon', 'direction': (0, 0, 1)},
            {'role': 'attach', 'direction': (0, 0, -1)},
        ],
    },
    'DOCKING_CLAMP': {
        'category': 'UTILITY',
        'size': (2, 1, 1),
        'shape': 'box',
        'scale_band': 'structural',
        'hardpoints': [
            {'role': 'dock', 'direction': (0, 1, 0)},
            {'role': 'attach', 'direction': (0, -1, 0)},
        ],
    },
    'SENSOR_MAST': {
        'category': 'UTILITY',
        'size': (1, 2, 1),
        'shape': 'cone',
        'scale_band': 'detail',
        'hardpoints': [
            {'role': 'attach', 'direction': (0, 0, -1)},
        ],
    },
    'ANTENNA_DISH': {
        'category': 'UTILITY',
        'size': (1, 1, 1),
        'shape': 'cone',
        'scale_band': 'detail',
        'hardpoints': [
            {'role': 'attach', 'direction': (0, 0, -1)},
        ],
    },

    # --- DETAIL ---
    'PANEL': {
        'category': 'DETAIL',
        'size': (1, 1, 1),
        'shape': 'box',
        'scale_band': 'detail',
        'hardpoints': [
            {'role': 'attach', 'direction': (0, 0, -1)},
        ],
    },
    'VENT': {
        'category': 'DETAIL',
        'size': (1, 1, 1),
        'shape': 'box',
        'scale_band': 'detail',
        'hardpoints': [
            {'role': 'attach', 'direction': (0, 0, -1)},
        ],
    },
    'PIPE': {
        'category': 'DETAIL',
        'size': (1, 2, 1),
        'shape': 'cylinder',
        'scale_band': 'detail',
        'hardpoints': [
            {'role': 'attach', 'direction': (0, 1, 0)},
            {'role': 'attach', 'direction': (0, -1, 0)},
        ],
    },
}

# ---------------------------------------------------------------------------
# Scale hierarchy bands
# ---------------------------------------------------------------------------

SCALE_BANDS = {
    'primary': 1.0,
    'structural': 0.7,
    'detail': 0.22,
}

# ---------------------------------------------------------------------------
# Grid sizes per ship class
# ---------------------------------------------------------------------------

GRID_SIZES = {
    'SHUTTLE': 0.5,
    'FIGHTER': 1.0,
    'CORVETTE': 1.0,
    'FRIGATE': 1.0,
    'DESTROYER': 1.5,
    'CRUISER': 2.0,
    'BATTLECRUISER': 2.0,
    'BATTLESHIP': 3.0,
    'CARRIER': 4.0,
    'DREADNOUGHT': 4.0,
    'CAPITAL': 6.0,
    'TITAN': 8.0,
    'INDUSTRIAL': 1.5,
    'MINING_BARGE': 1.0,
    'EXHUMER': 1.5,
    'EXPLORER': 1.0,
    'HAULER': 1.5,
    'EXOTIC': 1.0,
}

# ---------------------------------------------------------------------------
# Engine archetypes
# ---------------------------------------------------------------------------

ENGINE_ARCHETYPES = {
    'MAIN_THRUST': {
        'description': 'Primary propulsion - big, recessed',
        'depth_range': (1.0, 1.4),
        'radius_factor': 1.0,
        'has_nozzle_flare': True,
        'inner_cone': True,
        'exhaust_rings': 2,
        'glow_strength': 5.0,
    },
    'MANEUVERING': {
        'description': 'Small angled thrusters for maneuvering',
        'depth_range': (0.3, 0.6),
        'radius_factor': 0.4,
        'has_nozzle_flare': False,
        'inner_cone': False,
        'exhaust_rings': 0,
        'glow_strength': 2.0,
    },
    'UTILITY_EXHAUST': {
        'description': 'Flat vents for auxiliary systems',
        'depth_range': (0.2, 0.4),
        'radius_factor': 0.6,
        'has_nozzle_flare': False,
        'inner_cone': False,
        'exhaust_rings': 1,
        'glow_strength': 1.0,
    },
}

# ---------------------------------------------------------------------------
# Validation helpers
# ---------------------------------------------------------------------------


def get_brick_type(brick_type_name):
    """Return the brick definition dict for *brick_type_name*, or ``None``."""
    return BRICK_TYPES.get(brick_type_name)


def get_scale_factor(scale_band):
    """Return the multiplier for a given scale band name."""
    return SCALE_BANDS.get(scale_band, 1.0)


def get_grid_size(ship_class):
    """Return the snap grid size for *ship_class*."""
    return GRID_SIZES.get(ship_class, 1.0)


def snap_to_grid(position, grid_size):
    """Snap a 3-tuple *position* to the nearest *grid_size* increment."""
    return tuple(round(v / grid_size) * grid_size for v in position)


def validate_snap(brick_type_name, position, grid_size):
    """Return ``True`` if *position* is aligned to the grid."""
    snapped = snap_to_grid(position, grid_size)
    return snapped == tuple(round(v, 6) for v in position)


def get_engine_archetype(name):
    """Return the engine archetype dict for *name*, or ``None``."""
    return ENGINE_ARCHETYPES.get(name)


# Engine archetype selection thresholds (fraction of total engine count)
_MAIN_THRUST_RATIO = 0.6
_MANEUVERING_RATIO = 0.85


def select_engine_archetype(index, total_engines):
    """Choose an engine archetype based on the engine's index and total count.

    The first ~60 % of engines are main thrust, then ~25 % maneuvering,
    and the rest are utility exhaust.
    """
    ratio = index / max(total_engines, 1)
    if ratio < _MAIN_THRUST_RATIO:
        return 'MAIN_THRUST'
    elif ratio < _MANEUVERING_RATIO:
        return 'MANEUVERING'
    return 'UTILITY_EXHAUST'


# ---------------------------------------------------------------------------
# Ship DNA serialisation
# ---------------------------------------------------------------------------


def generate_ship_dna(ship_class, seed, bricks, style='MIXED',
                      naming_prefix=''):
    """Create a JSON-serialisable Ship DNA dict.

    Args:
        ship_class: Ship class string (e.g. ``'CRUISER'``).
        seed: The random seed used for generation.
        bricks: List of placed brick dicts, each with at minimum
                 ``type`` and ``pos`` keys.
        style: Design style string.
        naming_prefix: Naming prefix applied during generation.

    Returns:
        A dictionary that can be written to JSON with :func:`json.dumps`.
    """
    return {
        'seed': seed,
        'class': ship_class,
        'style': style,
        'naming_prefix': naming_prefix,
        'grid_size': get_grid_size(ship_class),
        'bricks': bricks,
    }


def ship_dna_to_json(dna, indent=2):
    """Serialise a Ship DNA dict to a JSON string."""
    return json.dumps(dna, indent=indent)


def ship_dna_from_json(json_string):
    """Deserialise a JSON string into a Ship DNA dict."""
    return json.loads(json_string)


# ---------------------------------------------------------------------------
# Blender registration stubs
# ---------------------------------------------------------------------------


def register():
    """Register this module."""
    pass


def unregister():
    """Unregister this module."""
    pass
