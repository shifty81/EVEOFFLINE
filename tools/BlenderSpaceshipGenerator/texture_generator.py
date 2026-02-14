"""
Procedural texture generation module
Creates PBR materials using Blender's shader node system
Supports hull plating, glow effects, weathering, and faction-specific color palettes
"""

import bpy
import random


# Color palettes for different styles
COLOR_PALETTES = {
    'MIXED': {
        'primary': (0.4, 0.4, 0.45, 1.0),
        'secondary': (0.3, 0.3, 0.35, 1.0),
        'accent': (0.2, 0.5, 1.0, 1.0),
    },
    'X4': {
        'primary': (0.35, 0.38, 0.4, 1.0),
        'secondary': (0.25, 0.28, 0.3, 1.0),
        'accent': (0.9, 0.6, 0.1, 1.0),
    },
    'ELITE': {
        'primary': (0.5, 0.5, 0.52, 1.0),
        'secondary': (0.3, 0.3, 0.32, 1.0),
        'accent': (0.1, 0.7, 0.9, 1.0),
    },
    'EVE': {
        'primary': (0.45, 0.42, 0.4, 1.0),
        'secondary': (0.35, 0.32, 0.3, 1.0),
        'accent': (0.6, 0.2, 0.2, 1.0),
    },
    'SOLARI': {
        'primary': (0.8, 0.65, 0.2, 1.0),
        'secondary': (0.6, 0.45, 0.15, 1.0),
        'accent': (1.0, 0.9, 0.5, 1.0),
    },
    'VEYREN': {
        'primary': (0.4, 0.45, 0.5, 1.0),
        'secondary': (0.3, 0.35, 0.4, 1.0),
        'accent': (0.3, 0.5, 0.9, 1.0),
    },
    'AURELIAN': {
        'primary': (0.2, 0.45, 0.35, 1.0),
        'secondary': (0.15, 0.35, 0.25, 1.0),
        'accent': (0.0, 0.8, 0.7, 1.0),
    },
    'KELDARI': {
        'primary': (0.45, 0.35, 0.25, 1.0),
        'secondary': (0.35, 0.25, 0.15, 1.0),
        'accent': (0.9, 0.5, 0.1, 1.0),
    },
    'NMS': {
        'primary': (0.7, 0.15, 0.1, 1.0),
        'secondary': (0.9, 0.85, 0.75, 1.0),
        'accent': (0.1, 0.8, 0.6, 1.0),
    },
}

# NMS has a pool of vibrant color sets chosen per-seed
NMS_COLOR_SETS = [
    {'primary': (0.7, 0.15, 0.1, 1.0), 'secondary': (0.9, 0.85, 0.75, 1.0), 'accent': (0.1, 0.8, 0.6, 1.0)},
    {'primary': (0.1, 0.35, 0.7, 1.0), 'secondary': (0.85, 0.85, 0.9, 1.0), 'accent': (0.9, 0.7, 0.1, 1.0)},
    {'primary': (0.15, 0.6, 0.3, 1.0), 'secondary': (0.2, 0.2, 0.25, 1.0), 'accent': (0.9, 0.2, 0.4, 1.0)},
    {'primary': (0.85, 0.6, 0.1, 1.0), 'secondary': (0.3, 0.3, 0.35, 1.0), 'accent': (0.2, 0.6, 0.9, 1.0)},
    {'primary': (0.6, 0.1, 0.5, 1.0), 'secondary': (0.9, 0.9, 0.85, 1.0), 'accent': (0.2, 0.9, 0.3, 1.0)},
    {'primary': (0.9, 0.9, 0.9, 1.0), 'secondary': (0.15, 0.15, 0.2, 1.0), 'accent': (0.9, 0.3, 0.1, 1.0)},
]


def get_palette(style, seed=0):
    """Return the color palette for the given style.

    For the NMS style a random set is chosen from *NMS_COLOR_SETS* based on
    the seed so that each generated ship gets a unique but consistent scheme.
    """
    if style == 'NMS':
        random.seed(seed)
        return random.choice(NMS_COLOR_SETS)
    return COLOR_PALETTES.get(style, COLOR_PALETTES['MIXED'])


def generate_hull_material(style='MIXED', seed=0, weathering=0.0):
    """
    Generate a procedural hull material using Blender's shader nodes.

    Args:
        style: Design style for color palette selection
        seed: Seed used to vary the NMS palette
        weathering: Amount of surface weathering (0.0 - 1.0)

    Returns:
        bpy.types.Material
    """
    palette = get_palette(style, seed)

    mat = bpy.data.materials.new(name=f"Hull_{style}")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links

    # Clear default nodes
    nodes.clear()

    # Create output node
    output = nodes.new(type='ShaderNodeOutputMaterial')
    output.location = (800, 0)

    # Create Principled BSDF
    principled = nodes.new(type='ShaderNodeBsdfPrincipled')
    principled.location = (400, 0)
    links.new(principled.outputs['BSDF'], output.inputs['Surface'])

    # Set base color from palette
    principled.inputs['Base Color'].default_value = palette['primary']
    principled.inputs['Metallic'].default_value = 0.7
    principled.inputs['Roughness'].default_value = 0.4

    # Add procedural noise for hull panel variation
    tex_coord = nodes.new(type='ShaderNodeTexCoord')
    tex_coord.location = (-600, 0)

    noise = nodes.new(type='ShaderNodeTexNoise')
    noise.location = (-400, 100)
    noise.inputs['Scale'].default_value = 15.0
    noise.inputs['Detail'].default_value = 8.0
    links.new(tex_coord.outputs['Object'], noise.inputs['Vector'])

    # Mix primary and secondary colors using noise
    color_mix = nodes.new(type='ShaderNodeMixRGB')
    color_mix.location = (0, 100)
    color_mix.blend_type = 'MIX'
    color_mix.inputs['Color1'].default_value = palette['primary']
    color_mix.inputs['Color2'].default_value = palette['secondary']
    links.new(noise.outputs['Fac'], color_mix.inputs['Fac'])
    links.new(color_mix.outputs['Color'], principled.inputs['Base Color'])

    # Add Voronoi for hull plating pattern
    voronoi = nodes.new(type='ShaderNodeTexVoronoi')
    voronoi.location = (-400, -150)
    voronoi.inputs['Scale'].default_value = 8.0
    links.new(tex_coord.outputs['Object'], voronoi.inputs['Vector'])

    # Use Voronoi distance to drive roughness variation
    roughness_mix = nodes.new(type='ShaderNodeMath')
    roughness_mix.location = (100, -150)
    roughness_mix.operation = 'MULTIPLY_ADD'
    roughness_mix.inputs[1].default_value = 0.3
    roughness_mix.inputs[2].default_value = 0.3
    links.new(voronoi.outputs['Distance'], roughness_mix.inputs[0])
    links.new(roughness_mix.outputs['Value'], principled.inputs['Roughness'])

    # Apply weathering if requested
    if weathering > 0.0:
        _apply_weathering(mat, weathering)

    return mat


def generate_accent_material(style='MIXED', seed=0):
    """
    Generate an accent/trim material with emissive glow for details.

    Args:
        style: Design style for color palette selection
        seed: Seed used to vary the NMS palette

    Returns:
        bpy.types.Material
    """
    palette = get_palette(style, seed)

    mat = bpy.data.materials.new(name=f"Accent_{style}")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links

    nodes.clear()

    output = nodes.new(type='ShaderNodeOutputMaterial')
    output.location = (600, 0)

    principled = nodes.new(type='ShaderNodeBsdfPrincipled')
    principled.location = (300, 0)
    principled.inputs['Base Color'].default_value = palette['accent']
    principled.inputs['Metallic'].default_value = 0.9
    principled.inputs['Roughness'].default_value = 0.2

    # Add subtle emission for accent glow
    principled.inputs['Emission Color'].default_value = palette['accent']
    principled.inputs['Emission Strength'].default_value = 0.5

    links.new(principled.outputs['BSDF'], output.inputs['Surface'])

    return mat


def generate_engine_material(style='MIXED', seed=0):
    """
    Generate an engine glow material.

    Args:
        style: Design style for color palette selection
        seed: Seed used to vary the NMS palette

    Returns:
        bpy.types.Material
    """
    palette = get_palette(style, seed)

    mat = bpy.data.materials.new(name=f"Engine_{style}")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links

    nodes.clear()

    output = nodes.new(type='ShaderNodeOutputMaterial')
    output.location = (400, 0)

    emission = nodes.new(type='ShaderNodeEmission')
    emission.location = (200, 0)
    emission.inputs['Color'].default_value = palette['accent']
    emission.inputs['Strength'].default_value = 5.0
    links.new(emission.outputs['Emission'], output.inputs['Surface'])

    return mat


def _apply_weathering(material, amount):
    """
    Add weathering/wear effects to an existing material.

    Uses a Musgrave-like noise texture mixed with darker colour to simulate
    dirt, scratches and wear on the hull surface.

    Args:
        material: The bpy.types.Material to modify
        amount: Intensity of weathering (0.0 - 1.0), clamped internally
    """
    amount = max(0.0, min(1.0, amount))
    nodes = material.node_tree.nodes
    links = material.node_tree.links

    # Find the principled BSDF
    principled = None
    for node in nodes:
        if node.type == 'BSDF_PRINCIPLED':
            principled = node
            break
    if principled is None:
        return

    # Weathering noise texture
    weather_noise = nodes.new(type='ShaderNodeTexNoise')
    weather_noise.location = (-200, -300)
    weather_noise.inputs['Scale'].default_value = 4.0
    weather_noise.inputs['Detail'].default_value = 12.0
    weather_noise.inputs['Roughness'].default_value = 0.8

    # Color ramp to sharpen weathering mask
    ramp = nodes.new(type='ShaderNodeValToRGB')
    ramp.location = (0, -300)
    ramp.color_ramp.elements[0].position = 0.4
    ramp.color_ramp.elements[1].position = 0.6
    links.new(weather_noise.outputs['Fac'], ramp.inputs['Fac'])

    # Darken roughness in weathered areas
    rough_add = nodes.new(type='ShaderNodeMath')
    rough_add.location = (200, -300)
    rough_add.operation = 'MULTIPLY_ADD'
    rough_add.inputs[1].default_value = amount * 0.4
    rough_add.inputs[2].default_value = 0.4
    links.new(ramp.outputs['Color'], rough_add.inputs[0])
    links.new(rough_add.outputs['Value'], principled.inputs['Roughness'])


def apply_textures_to_ship(ship_object, style='MIXED', seed=0,
                           weathering=0.0):
    """
    Apply procedural textures to all parts of a ship hierarchy.

    Walks the object hierarchy starting from *ship_object* and assigns
    appropriate materials based on part names.

    Args:
        ship_object: Root hull object of the ship
        style: Design style for palette selection
        seed: Random seed for NMS palette variation
        weathering: Weathering intensity (0.0 - 1.0)
    """
    hull_mat = generate_hull_material(style=style, seed=seed,
                                      weathering=weathering)
    accent_mat = generate_accent_material(style=style, seed=seed)
    engine_mat = generate_engine_material(style=style, seed=seed)

    def _assign(obj):
        if obj.type != 'MESH':
            return
        name_lower = obj.name.lower()
        if 'engine' in name_lower:
            if obj.data.materials:
                obj.data.materials[0] = engine_mat
            else:
                obj.data.materials.append(engine_mat)
        elif any(k in name_lower for k in ('weapon', 'hardpoint', 'cockpit',
                                            'wing', 'turret')):
            if obj.data.materials:
                obj.data.materials[0] = accent_mat
            else:
                obj.data.materials.append(accent_mat)
        else:
            if obj.data.materials:
                obj.data.materials[0] = hull_mat
            else:
                obj.data.materials.append(hull_mat)

    _assign(ship_object)
    for child in ship_object.children_recursive:
        _assign(child)


def register():
    """Register this module"""
    pass


def unregister():
    """Unregister this module"""
    pass
