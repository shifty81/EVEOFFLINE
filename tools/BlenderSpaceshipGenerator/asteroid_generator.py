"""
Asteroid and asteroid belt generation module
Generates procedural asteroids and belt layouts matching EVEOFFLINE ore types
and belt configurations.

Ore types: Dustite, Ferrite, Ignaite, Crystite, Shadite, Corite, Lumine,
           Sangite, Glacite, Densite, Voidite, Spodumain, Pyranite,
           Stellite, Cosmite, Nexorite
Belt shapes: Semicircle, Sphere, Cluster, Ring

EVEOFFLINE project: https://github.com/shifty81/EVEOFFLINE
"""

import bpy
import random
import math


# Ore visual properties (color, roughness, metallic) from EVEOFFLINE data
ORE_TYPES = {
    'DUSTITE':   {'color': (0.6, 0.4, 0.2, 1.0), 'roughness': 0.8, 'metallic': 0.3},
    'FERRITE':   {'color': (0.5, 0.5, 0.55, 1.0), 'roughness': 0.7, 'metallic': 0.6},
    'IGNAITE':   {'color': (0.7, 0.3, 0.2, 1.0), 'roughness': 0.75, 'metallic': 0.4},
    'CRYSTITE':  {'color': (0.3, 0.5, 0.4, 1.0), 'roughness': 0.65, 'metallic': 0.5},
    'SHADITE':   {'color': (0.8, 0.6, 0.3, 1.0), 'roughness': 0.7, 'metallic': 0.45},
    'CORITE':    {'color': (0.3, 0.6, 0.7, 1.0), 'roughness': 0.6, 'metallic': 0.55},
    'LUMINE':    {'color': (0.6, 0.2, 0.3, 1.0), 'roughness': 0.8, 'metallic': 0.35},
    'SANGITE':   {'color': (0.9, 0.3, 0.2, 1.0), 'roughness': 0.65, 'metallic': 0.7},
    'GLACITE':   {'color': (0.9, 0.7, 0.4, 1.0), 'roughness': 0.6, 'metallic': 0.65},
    'DENSITE':   {'color': (0.7, 0.7, 0.8, 1.0), 'roughness': 0.7, 'metallic': 0.5},
    'VOIDITE':   {'color': (0.4, 0.3, 0.25, 1.0), 'roughness': 0.85, 'metallic': 0.4},
    'SPODUMAIN': {'color': (0.8, 0.8, 0.9, 1.0), 'roughness': 0.5, 'metallic': 0.8},
    'PYRANITE':  {'color': (0.6, 0.4, 0.6, 1.0), 'roughness': 0.7, 'metallic': 0.6},
    'STELLITE':  {'color': (0.3, 0.8, 0.6, 1.0), 'roughness': 0.6, 'metallic': 0.7},
    'COSMITE':   {'color': (0.9, 0.5, 0.3, 1.0), 'roughness': 0.65, 'metallic': 0.75},
    'NEXORITE':  {'color': (0.2, 0.9, 0.9, 1.0), 'roughness': 0.4, 'metallic': 0.9},
}

# Asteroid size tiers
ASTEROID_SIZES = {
    'SMALL':  {'min_radius': 0.5, 'max_radius': 2.0, 'probability': 0.6},
    'MEDIUM': {'min_radius': 2.0, 'max_radius': 5.0, 'probability': 0.3},
    'LARGE':  {'min_radius': 5.0, 'max_radius': 10.0, 'probability': 0.09},
    'HUGE':   {'min_radius': 10.0, 'max_radius': 20.0, 'probability': 0.01},
}

# Belt layout presets
BELT_LAYOUTS = {
    'SEMICIRCLE': {'shape': 'semicircle', 'default_count': 30, 'radius': 100.0},
    'SPHERE':     {'shape': 'sphere',     'default_count': 40, 'radius': 120.0},
    'CLUSTER':    {'shape': 'cluster',    'default_count': 50, 'radius': 60.0},
    'RING':       {'shape': 'ring',       'default_count': 25, 'radius': 150.0},
}


def generate_asteroid(ore_type='DUSTITE', size_class='MEDIUM', seed=1,
                      location=(0, 0, 0)):
    """
    Generate a single asteroid.

    Args:
        ore_type: Ore type key from ORE_TYPES
        size_class: Size tier (SMALL, MEDIUM, LARGE, HUGE)
        seed: Random seed
        location: World-space position

    Returns:
        The asteroid Blender object
    """
    random.seed(seed)

    ore = ORE_TYPES.get(ore_type, ORE_TYPES['DUSTITE'])
    size = ASTEROID_SIZES.get(size_class, ASTEROID_SIZES['MEDIUM'])
    radius = random.uniform(size['min_radius'], size['max_radius'])

    # Start from an icosphere for natural rocky look
    bpy.ops.mesh.primitive_ico_sphere_add(
        subdivisions=3,
        radius=radius,
        location=location,
    )
    asteroid = bpy.context.active_object
    asteroid.name = f"Asteroid_{ore_type}_{seed}"

    # Deform for irregular shape
    bpy.context.view_layer.objects.active = asteroid
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.transform.resize(value=(
        random.uniform(0.7, 1.3),
        random.uniform(0.7, 1.3),
        random.uniform(0.6, 1.0),
    ))
    bpy.ops.object.mode_set(mode='OBJECT')

    # Displace for rough surface
    displace = asteroid.modifiers.new(name="Displace", type='DISPLACE')
    tex = bpy.data.textures.new(f"AsteroidTex_{seed}", type='CLOUDS')
    tex.noise_scale = radius * 0.5
    displace.texture = tex
    displace.strength = radius * 0.25

    # Random rotation
    asteroid.rotation_euler = (
        random.uniform(0, math.pi * 2),
        random.uniform(0, math.pi * 2),
        random.uniform(0, math.pi * 2),
    )

    # Apply material
    _apply_ore_material(asteroid, ore_type, ore)

    bpy.ops.object.shade_smooth()

    return asteroid


def generate_asteroid_belt(layout='SEMICIRCLE', ore_types=None, count=None,
                           seed=1):
    """
    Generate an asteroid belt.

    Args:
        layout: Belt layout key from BELT_LAYOUTS
        ore_types: List of ore type keys to include (random selection per rock).
                   Defaults to common highsec ores.
        count: Number of asteroids (overrides layout default)
        seed: Random seed

    Returns:
        The parent empty that holds all asteroids
    """
    random.seed(seed)

    belt_cfg = BELT_LAYOUTS.get(layout, BELT_LAYOUTS['SEMICIRCLE'])
    num = count if count is not None else belt_cfg['default_count']
    radius = belt_cfg['radius']

    if ore_types is None:
        ore_types = ['DUSTITE', 'FERRITE', 'IGNAITE', 'CRYSTITE']

    # Create collection
    collection_name = f"Belt_{layout}_{seed}"
    collection = bpy.data.collections.new(collection_name)
    bpy.context.scene.collection.children.link(collection)

    # Parent empty
    bpy.ops.object.empty_add(type='PLAIN_AXES', location=bpy.context.scene.cursor.location)
    belt_root = bpy.context.active_object
    belt_root.name = collection_name
    collection.objects.link(belt_root)

    # Pick a size class per asteroid weighted by probability
    size_keys = list(ASTEROID_SIZES.keys())
    size_weights = [ASTEROID_SIZES[k]['probability'] for k in size_keys]

    for i in range(num):
        pos = _belt_position(belt_cfg['shape'], radius, i, num)
        ore = random.choice(ore_types)
        size_class = random.choices(size_keys, weights=size_weights, k=1)[0]

        ast = generate_asteroid(
            ore_type=ore,
            size_class=size_class,
            seed=seed + i + 1,
            location=pos,
        )
        collection.objects.link(ast)
        ast.parent = belt_root

    return belt_root


def _belt_position(shape, radius, index, total):
    """Calculate position for an asteroid within a belt layout."""
    if shape == 'semicircle':
        angle = (index / max(total, 1)) * math.pi
        r = radius + random.uniform(-radius * 0.15, radius * 0.15)
        return (
            math.cos(angle) * r,
            math.sin(angle) * r,
            random.uniform(-radius * 0.05, radius * 0.05),
        )

    elif shape == 'sphere':
        theta = random.uniform(0, math.pi * 2)
        phi = random.uniform(0, math.pi)
        r = radius + random.uniform(-radius * 0.2, radius * 0.2)
        return (
            r * math.sin(phi) * math.cos(theta),
            r * math.sin(phi) * math.sin(theta),
            r * math.cos(phi),
        )

    elif shape == 'cluster':
        return (
            random.gauss(0, radius * 0.3),
            random.gauss(0, radius * 0.3),
            random.gauss(0, radius * 0.15),
        )

    elif shape == 'ring':
        angle = (index / max(total, 1)) * 2 * math.pi
        r = radius + random.uniform(-radius * 0.1, radius * 0.1)
        return (
            math.cos(angle) * r,
            math.sin(angle) * r,
            random.uniform(-radius * 0.03, radius * 0.03),
        )

    # Fallback
    return (
        random.uniform(-radius, radius),
        random.uniform(-radius, radius),
        random.uniform(-radius * 0.1, radius * 0.1),
    )


def _apply_ore_material(obj, ore_name, ore):
    """Apply a PBR material matching the ore type."""
    mat = bpy.data.materials.new(name=f"Ore_{ore_name}")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    bsdf = nodes.get('Principled BSDF')
    if bsdf:
        bsdf.inputs['Base Color'].default_value = ore['color']
        bsdf.inputs['Metallic'].default_value = ore['metallic']
        bsdf.inputs['Roughness'].default_value = ore['roughness']
    obj.data.materials.append(mat)


def register():
    """Register this module"""
    pass


def unregister():
    """Unregister this module"""
    pass
