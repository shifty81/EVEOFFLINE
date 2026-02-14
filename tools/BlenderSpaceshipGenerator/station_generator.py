"""
Station generation module
Generates procedural space stations matching EVEOFFLINE station types and faction styles.

Station types: Industrial, Military, Commercial, Research, Mining
Upwell structures: Astrahus, Fortizar, Keepstar
Faction styles: Solari (cathedral), Veyren (industrial blocks), Aurelian (organic domes),
                Keldari (rusted patchwork)

EVEOFFLINE project: https://github.com/shifty81/EVEOFFLINE
"""

import bpy
import random
import math


# Station type configurations
STATION_CONFIGS = {
    'INDUSTRIAL': {
        'name': 'Industrial Station',
        'base_scale': 80.0,
        'sections': 4,
        'docking_bays': 2,
        'has_hangars': True,
    },
    'MILITARY': {
        'name': 'Military Station',
        'base_scale': 100.0,
        'sections': 5,
        'docking_bays': 3,
        'has_hangars': True,
    },
    'COMMERCIAL': {
        'name': 'Commercial Hub',
        'base_scale': 90.0,
        'sections': 4,
        'docking_bays': 4,
        'has_hangars': False,
    },
    'RESEARCH': {
        'name': 'Research Facility',
        'base_scale': 60.0,
        'sections': 3,
        'docking_bays': 1,
        'has_hangars': False,
    },
    'MINING': {
        'name': 'Mining Station',
        'base_scale': 70.0,
        'sections': 3,
        'docking_bays': 2,
        'has_hangars': True,
    },
    'ASTRAHUS': {
        'name': 'Astrahus Citadel',
        'base_scale': 50.0,
        'sections': 3,
        'docking_bays': 2,
        'has_hangars': False,
    },
    'FORTIZAR': {
        'name': 'Fortizar Citadel',
        'base_scale': 100.0,
        'sections': 5,
        'docking_bays': 4,
        'has_hangars': True,
    },
    'KEEPSTAR': {
        'name': 'Keepstar Citadel',
        'base_scale': 200.0,
        'sections': 8,
        'docking_bays': 8,
        'has_hangars': True,
    },
}

# Faction visual parameters
FACTION_STATION_STYLES = {
    'SOLARI': {
        'spires': True,
        'domes': True,
        'symmetry': 'radial',
        'color': (0.8, 0.6, 0.2, 1.0),
        'emissive': (1.0, 0.8, 0.3),
    },
    'VEYREN': {
        'spires': False,
        'domes': False,
        'symmetry': 'cubic',
        'color': (0.4, 0.45, 0.5, 1.0),
        'emissive': (0.3, 0.5, 0.8),
    },
    'AURELIAN': {
        'spires': False,
        'domes': True,
        'symmetry': 'organic',
        'color': (0.2, 0.4, 0.3, 1.0),
        'emissive': (0.3, 0.8, 0.5),
    },
    'KELDARI': {
        'spires': False,
        'domes': False,
        'symmetry': 'asymmetric',
        'color': (0.4, 0.3, 0.25, 1.0),
        'emissive': (0.8, 0.4, 0.2),
    },
}


def _prefixed_name(prefix, name):
    """Return name with project prefix applied if prefix is non-empty."""
    if prefix:
        return f"{prefix}_{name}"
    return name


def generate_station(station_type='INDUSTRIAL', faction='SOLARI', seed=1, naming_prefix=''):
    """
    Generate a complete space station.

    Args:
        station_type: Type of station to generate
        faction: Faction style (SOLARI, VEYREN, AURELIAN, KELDARI)
        seed: Random seed for variation
        naming_prefix: Project naming prefix

    Returns:
        The root object of the station
    """
    random.seed(seed)

    config = STATION_CONFIGS.get(station_type, STATION_CONFIGS['INDUSTRIAL'])
    style = FACTION_STATION_STYLES.get(faction, FACTION_STATION_STYLES['SOLARI'])
    scale = config['base_scale']

    # Create collection
    collection_name = _prefixed_name(naming_prefix, f"Station_{station_type}_{faction}_{seed}")
    collection = bpy.data.collections.new(collection_name)
    bpy.context.scene.collection.children.link(collection)

    # Generate central hub
    hub = _generate_hub(scale, style)
    hub.name = _prefixed_name(naming_prefix, "Station_Hub")
    collection.objects.link(hub)

    # Generate radiating sections
    sections = _generate_sections(config['sections'], scale, style)
    for s in sections:
        collection.objects.link(s)

    # Generate docking bays
    bays = _generate_docking_bays(config['docking_bays'], scale, style)
    for b in bays:
        collection.objects.link(b)

    # Faction-specific additions
    if style['spires']:
        spires = _generate_spires(scale, style)
        for sp in spires:
            collection.objects.link(sp)

    if style['domes']:
        domes = _generate_domes(scale, style)
        for d in domes:
            collection.objects.link(d)

    if config['has_hangars']:
        hangars = _generate_hangars(scale, style)
        for h in hangars:
            collection.objects.link(h)

    # Parent everything to the hub
    for obj in collection.objects:
        if obj != hub:
            obj.parent = hub

    hub.location = bpy.context.scene.cursor.location
    return hub


def _generate_hub(scale, style):
    """Generate the central hub of the station."""
    if style['symmetry'] == 'radial':
        bpy.ops.mesh.primitive_cylinder_add(radius=scale * 0.3, depth=scale * 0.2)
    elif style['symmetry'] == 'cubic':
        bpy.ops.mesh.primitive_cube_add(size=scale * 0.5)
    elif style['symmetry'] == 'organic':
        bpy.ops.mesh.primitive_uv_sphere_add(radius=scale * 0.3)
    else:
        bpy.ops.mesh.primitive_cube_add(size=scale * 0.4)
        # Asymmetric: scale unevenly
        bpy.context.active_object.scale = (1.0, 1.3, 0.8)
        bpy.ops.object.transform_apply(scale=True)

    hub = bpy.context.active_object

    # Apply material
    _apply_station_material(hub, "Station_Hull", style['color'], style['emissive'])

    # Smooth shading
    bpy.ops.object.shade_smooth()

    modifier = hub.modifiers.new(name="Subdivision", type='SUBSURF')
    modifier.levels = 1
    modifier.render_levels = 2

    return hub


def _generate_sections(count, scale, style):
    """Generate radiating arm/section structures."""
    sections = []
    for i in range(count):
        angle = (i / count) * 2 * math.pi
        length = scale * 0.5 + random.uniform(-scale * 0.1, scale * 0.1)
        x = math.cos(angle) * scale * 0.4
        y = math.sin(angle) * scale * 0.4

        bpy.ops.mesh.primitive_cube_add(size=1, location=(x, y, 0))
        section = bpy.context.active_object
        section.name = f"Section_{i+1}"
        section.scale = (scale * 0.08, length, scale * 0.06)
        section.rotation_euler = (0, 0, angle)
        bpy.ops.object.transform_apply(scale=True, rotation=True)

        _apply_station_material(section, f"Section_Mat_{i}", style['color'],
                                style['emissive'])
        sections.append(section)

    return sections


def _generate_docking_bays(count, scale, style):
    """Generate docking bay structures."""
    bays = []
    for i in range(count):
        angle = (i / count) * 2 * math.pi + math.pi / count
        dist = scale * 0.55
        x = math.cos(angle) * dist
        y = math.sin(angle) * dist

        bpy.ops.mesh.primitive_cube_add(size=1, location=(x, y, 0))
        bay = bpy.context.active_object
        bay.name = f"Docking_Bay_{i+1}"
        bay.scale = (scale * 0.12, scale * 0.12, scale * 0.08)
        bpy.ops.object.transform_apply(scale=True)

        # Add emissive strip to mark docking entrance
        bpy.ops.mesh.primitive_cube_add(
            size=1,
            location=(x, y, -scale * 0.04)
        )
        strip = bpy.context.active_object
        strip.name = f"Docking_Light_{i+1}"
        strip.scale = (scale * 0.1, scale * 0.01, scale * 0.01)
        bpy.ops.object.transform_apply(scale=True)
        strip.parent = bay

        mat = bpy.data.materials.new(name=f"Dock_Emissive_{i}")
        mat.use_nodes = True
        nodes = mat.node_tree.nodes
        emission = nodes.new(type='ShaderNodeEmission')
        emission.inputs['Color'].default_value = (*style['emissive'], 1.0)
        emission.inputs['Strength'].default_value = 3.0
        output = nodes.get('Material Output')
        mat.node_tree.links.new(emission.outputs['Emission'], output.inputs['Surface'])
        strip.data.materials.append(mat)

        bays.append(bay)

    return bays


def _generate_spires(scale, style):
    """Generate vertical spires (Solari style)."""
    spires = []
    for i in range(4):
        angle = (i / 4) * 2 * math.pi
        x = math.cos(angle) * scale * 0.15
        y = math.sin(angle) * scale * 0.15

        bpy.ops.mesh.primitive_cone_add(
            radius1=scale * 0.02,
            depth=scale * 0.4,
            location=(x, y, scale * 0.2)
        )
        spire = bpy.context.active_object
        spire.name = f"Spire_{i+1}"
        _apply_station_material(spire, f"Spire_Mat_{i}", style['color'],
                                style['emissive'])
        spires.append(spire)

    return spires


def _generate_domes(scale, style):
    """Generate dome structures (Solari/Aurelian style)."""
    domes = []
    positions = [(0, 0, scale * 0.12), (0, 0, -scale * 0.12)]
    for i, pos in enumerate(positions):
        bpy.ops.mesh.primitive_uv_sphere_add(radius=scale * 0.15, location=pos)
        dome = bpy.context.active_object
        dome.name = f"Dome_{i+1}"
        dome.scale = (1.0, 1.0, 0.5)
        bpy.ops.object.transform_apply(scale=True)
        bpy.ops.object.shade_smooth()
        _apply_station_material(dome, f"Dome_Mat_{i}", style['color'],
                                style['emissive'])
        domes.append(dome)

    return domes


def _generate_hangars(scale, style):
    """Generate hangar bay structures."""
    hangars = []
    bpy.ops.mesh.primitive_cube_add(size=1, location=(0, -scale * 0.45, 0))
    hangar = bpy.context.active_object
    hangar.name = "Hangar_Bay"
    hangar.scale = (scale * 0.2, scale * 0.15, scale * 0.1)
    bpy.ops.object.transform_apply(scale=True)

    _apply_station_material(hangar, "Hangar_Mat", style['color'], style['emissive'])
    hangars.append(hangar)

    return hangars


def _apply_station_material(obj, name, color, emissive):
    """Apply a basic station material with color and emissive accent."""
    mat = bpy.data.materials.new(name=name)
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    bsdf = nodes.get('Principled BSDF')
    if bsdf:
        bsdf.inputs['Base Color'].default_value = color
        bsdf.inputs['Metallic'].default_value = 0.7
        bsdf.inputs['Roughness'].default_value = 0.4
    obj.data.materials.append(mat)


def register():
    """Register this module"""
    pass


def unregister():
    """Unregister this module"""
    pass
