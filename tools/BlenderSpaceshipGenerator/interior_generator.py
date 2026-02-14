"""
Interior generation module
Generates ship interiors with rooms, corridors, and access points
Designed for FPV exploration
"""

import bpy
import random
import math


# Standard human scale for FPV (in Blender units, approximately meters)
HUMAN_HEIGHT = 1.8
DOOR_HEIGHT = 2.0
DOOR_WIDTH = 1.0
CORRIDOR_WIDTH = 1.5
CORRIDOR_HEIGHT = 2.5
ROOM_HEIGHT = 3.0


def _prefixed_name(prefix, name):
    """Return name with project prefix applied if prefix is non-empty."""
    if prefix:
        return f"{prefix}_{name}"
    return name


def generate_interior(ship_class='FIGHTER', scale=1.0, crew_capacity=1, naming_prefix=''):
    """
    Generate complete interior for a ship
    
    Args:
        ship_class: Type of ship
        scale: Ship scale factor
        crew_capacity: Number of crew members
        naming_prefix: Project naming prefix
    
    Returns:
        List of interior objects
    """
    interior_objects = []
    
    # Create interior collection
    collection_name = _prefixed_name(naming_prefix, f"Interior_{ship_class}")
    
    # Determine interior layout based on ship class
    if ship_class in ['SHUTTLE', 'FIGHTER']:
        # Small ships: Simple cockpit area
        interior_objects.extend(generate_cockpit_interior(scale, naming_prefix=naming_prefix))
    elif ship_class in ['CORVETTE', 'FRIGATE']:
        # Medium ships: Cockpit + small crew area
        interior_objects.extend(generate_cockpit_interior(scale, naming_prefix=naming_prefix))
        interior_objects.extend(generate_corridor(scale, length=scale * 0.5, naming_prefix=naming_prefix))
        interior_objects.extend(generate_crew_quarters(scale, bunks=crew_capacity, naming_prefix=naming_prefix))
    else:
        # Large ships: Full interior with multiple rooms
        interior_objects.extend(generate_bridge(scale, naming_prefix=naming_prefix))
        interior_objects.extend(generate_corridor_network(scale, crew_capacity, naming_prefix=naming_prefix))
        interior_objects.extend(generate_crew_quarters(scale, bunks=crew_capacity, naming_prefix=naming_prefix))
        interior_objects.extend(generate_cargo_bay(scale, naming_prefix=naming_prefix))
        interior_objects.extend(generate_engine_room(scale, naming_prefix=naming_prefix))
    
    return interior_objects


def generate_cockpit_interior(scale=1.0, naming_prefix=''):
    """
    Generate cockpit/pilot area interior
    
    Args:
        scale: Ship scale factor
        naming_prefix: Project naming prefix
    """
    objects = []
    
    # Create cockpit floor
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(0, scale * 0.7, -scale * 0.15)
    )
    floor = bpy.context.active_object
    floor.name = _prefixed_name(naming_prefix, "Cockpit_Floor")
    floor.scale = (scale * 0.4, scale * 0.3, 0.05)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(floor)
    
    # Create pilot seat
    bpy.ops.mesh.primitive_cube_add(
        size=0.5,
        location=(0, scale * 0.65, -scale * 0.1)
    )
    seat = bpy.context.active_object
    seat.name = _prefixed_name(naming_prefix, "Pilot_Seat")
    objects.append(seat)
    
    # Create control panel
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(0, scale * 0.8, -scale * 0.05)
    )
    panel = bpy.context.active_object
    panel.name = _prefixed_name(naming_prefix, "Control_Panel")
    panel.scale = (scale * 0.3, 0.1, scale * 0.2)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(panel)
    
    return objects


def generate_bridge(scale=1.0, naming_prefix=''):
    """
    Generate bridge for large ships
    
    Args:
        scale: Ship scale factor
        naming_prefix: Project naming prefix
    """
    objects = []
    
    # Bridge floor
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(0, scale * 0.7, -scale * 0.15)
    )
    floor = bpy.context.active_object
    floor.name = _prefixed_name(naming_prefix, "Bridge_Floor")
    floor.scale = (scale * 0.6, scale * 0.4, 0.05)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(floor)
    
    # Bridge walls
    wall_height = ROOM_HEIGHT
    
    # Front wall
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(0, scale * 0.9, -scale * 0.15 + wall_height / 2)
    )
    front_wall = bpy.context.active_object
    front_wall.name = _prefixed_name(naming_prefix, "Bridge_Wall_Front")
    front_wall.scale = (scale * 0.6, 0.1, wall_height)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(front_wall)
    
    # Command chair
    bpy.ops.mesh.primitive_cube_add(
        size=0.7,
        location=(0, scale * 0.7, -scale * 0.1)
    )
    command_chair = bpy.context.active_object
    command_chair.name = _prefixed_name(naming_prefix, "Command_Chair")
    objects.append(command_chair)
    
    # Navigation console
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(scale * 0.2, scale * 0.75, -scale * 0.05)
    )
    nav_console = bpy.context.active_object
    nav_console.name = _prefixed_name(naming_prefix, "Nav_Console")
    nav_console.scale = (0.4, 0.4, 0.6)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(nav_console)
    
    return objects


def generate_corridor(scale=1.0, length=5.0, start_pos=(0, 0, 0), naming_prefix=''):
    """
    Generate a corridor segment
    
    Args:
        scale: Ship scale factor
        length: Length of corridor
        start_pos: Starting position
        naming_prefix: Project naming prefix
    """
    objects = []
    
    # Corridor floor
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(start_pos[0], start_pos[1] + length / 2, start_pos[2])
    )
    floor = bpy.context.active_object
    floor.name = _prefixed_name(naming_prefix, "Corridor_Floor")
    floor.scale = (CORRIDOR_WIDTH, length, 0.05)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(floor)
    
    # Corridor ceiling
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(start_pos[0], start_pos[1] + length / 2, start_pos[2] + CORRIDOR_HEIGHT)
    )
    ceiling = bpy.context.active_object
    ceiling.name = _prefixed_name(naming_prefix, "Corridor_Ceiling")
    ceiling.scale = (CORRIDOR_WIDTH, length, 0.05)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(ceiling)
    
    # Corridor walls
    # Left wall
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(start_pos[0] - CORRIDOR_WIDTH / 2, start_pos[1] + length / 2, start_pos[2] + CORRIDOR_HEIGHT / 2)
    )
    left_wall = bpy.context.active_object
    left_wall.name = _prefixed_name(naming_prefix, "Corridor_Wall_Left")
    left_wall.scale = (0.1, length, CORRIDOR_HEIGHT)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(left_wall)
    
    # Right wall
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(start_pos[0] + CORRIDOR_WIDTH / 2, start_pos[1] + length / 2, start_pos[2] + CORRIDOR_HEIGHT / 2)
    )
    right_wall = bpy.context.active_object
    right_wall.name = _prefixed_name(naming_prefix, "Corridor_Wall_Right")
    right_wall.scale = (0.1, length, CORRIDOR_HEIGHT)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(right_wall)
    
    return objects


def generate_corridor_network(scale=1.0, crew_capacity=10, naming_prefix=''):
    """
    Generate network of corridors connecting ship areas
    
    Args:
        scale: Ship scale factor
        crew_capacity: Number of crew (affects corridor count)
        naming_prefix: Project naming prefix
    """
    objects = []
    
    # Main corridor running along ship center
    main_length = scale * 0.6
    objects.extend(generate_corridor(scale, main_length, (0, 0, -scale * 0.15), naming_prefix=naming_prefix))
    
    # Add side corridors for larger ships
    if crew_capacity > 20:
        # Side corridors
        side_length = scale * 0.3
        objects.extend(generate_corridor(scale, side_length, (CORRIDOR_WIDTH, scale * 0.2, -scale * 0.15), naming_prefix=naming_prefix))
        objects.extend(generate_corridor(scale, side_length, (-CORRIDOR_WIDTH, scale * 0.2, -scale * 0.15), naming_prefix=naming_prefix))
    
    return objects


def generate_crew_quarters(scale=1.0, bunks=4, naming_prefix=''):
    """
    Generate crew quarters with bunks
    
    Args:
        scale: Ship scale factor
        bunks: Number of bunks to create
        naming_prefix: Project naming prefix
    """
    objects = []
    
    # Room floor
    room_width = max(3.0, bunks * 0.8)
    room_depth = 4.0
    
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(scale * 0.3, -scale * 0.3, -scale * 0.15)
    )
    floor = bpy.context.active_object
    floor.name = _prefixed_name(naming_prefix, "Quarters_Floor")
    floor.scale = (room_width, room_depth, 0.05)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(floor)
    
    # Create bunks
    bunk_spacing = room_width / bunks if bunks > 0 else 1.0
    for i in range(bunks):
        x_pos = scale * 0.3 - room_width / 2 + (i + 0.5) * bunk_spacing
        bpy.ops.mesh.primitive_cube_add(
            size=1,
            location=(x_pos, -scale * 0.3 + room_depth / 2 - 0.5, -scale * 0.1)
        )
        bunk = bpy.context.active_object
        bunk.name = _prefixed_name(naming_prefix, f"Bunk_{i+1}")
        bunk.scale = (0.8, 2.0, 0.3)
        bpy.ops.object.transform_apply(scale=True)
        objects.append(bunk)
    
    return objects


def generate_cargo_bay(scale=1.0, naming_prefix=''):
    """
    Generate cargo bay area
    
    Args:
        scale: Ship scale factor
        naming_prefix: Project naming prefix
    """
    objects = []
    
    # Large open area for cargo
    bay_width = scale * 0.5
    bay_depth = scale * 0.4
    bay_height = ROOM_HEIGHT * 1.5
    
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(0, -scale * 0.5, -scale * 0.15)
    )
    floor = bpy.context.active_object
    floor.name = _prefixed_name(naming_prefix, "Cargo_Bay_Floor")
    floor.scale = (bay_width, bay_depth, 0.1)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(floor)
    
    # Add cargo containers
    for i in range(3):
        for j in range(2):
            x_pos = -bay_width / 3 + i * bay_width / 3
            y_pos = -scale * 0.5 - bay_depth / 4 + j * bay_depth / 2
            bpy.ops.mesh.primitive_cube_add(
                size=1,
                location=(x_pos, y_pos, -scale * 0.1)
            )
            container = bpy.context.active_object
            container.name = _prefixed_name(naming_prefix, f"Cargo_Container_{i}_{j}")
            container.scale = (0.8, 0.8, 1.0)
            bpy.ops.object.transform_apply(scale=True)
            objects.append(container)
    
    return objects


def generate_engine_room(scale=1.0, naming_prefix=''):
    """
    Generate engine room with machinery
    
    Args:
        scale: Ship scale factor
        naming_prefix: Project naming prefix
    """
    objects = []
    
    # Engine room floor
    room_width = scale * 0.4
    room_depth = scale * 0.3
    
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=(0, -scale * 0.8, -scale * 0.15)
    )
    floor = bpy.context.active_object
    floor.name = _prefixed_name(naming_prefix, "Engine_Room_Floor")
    floor.scale = (room_width, room_depth, 0.05)
    bpy.ops.object.transform_apply(scale=True)
    objects.append(floor)
    
    # Reactor core (central)
    bpy.ops.mesh.primitive_cylinder_add(
        radius=scale * 0.1,
        depth=ROOM_HEIGHT,
        location=(0, -scale * 0.8, -scale * 0.15 + ROOM_HEIGHT / 2)
    )
    core = bpy.context.active_object
    core.name = _prefixed_name(naming_prefix, "Reactor_Core")
    objects.append(core)
    
    # Add glowing material to core
    mat = bpy.data.materials.new(name="Reactor_Glow")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    emission = nodes.new(type='ShaderNodeEmission')
    emission.inputs['Color'].default_value = (0.8, 0.3, 0.1, 1.0)  # Orange glow
    emission.inputs['Strength'].default_value = 3.0
    output = nodes.get('Material Output')
    mat.node_tree.links.new(emission.outputs['Emission'], output.inputs['Surface'])
    core.data.materials.append(mat)
    
    return objects


def generate_doorway(position=(0, 0, 0), rotation=(0, 0, 0), naming_prefix=''):
    """
    Generate a doorway/access point
    
    Args:
        position: Doorway position
        rotation: Doorway rotation
        naming_prefix: Project naming prefix
    """
    bpy.ops.mesh.primitive_cube_add(
        size=1,
        location=position
    )
    doorway = bpy.context.active_object
    doorway.name = _prefixed_name(naming_prefix, "Doorway")
    doorway.scale = (DOOR_WIDTH, 0.1, DOOR_HEIGHT)
    doorway.rotation_euler = rotation
    bpy.ops.object.transform_apply(scale=True, rotation=True)
    
    return doorway


def register():
    """Register this module"""
    pass


def unregister():
    """Unregister this module"""
    pass
