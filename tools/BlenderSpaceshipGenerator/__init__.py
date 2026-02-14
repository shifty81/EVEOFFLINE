"""
Blender Spaceship Generator Addon
Generates procedural spaceships with modular parts and interiors
Inspired by X4, Elite Dangerous, Eve Online, and the EVEOFFLINE project
"""

bl_info = {
    "name": "Spaceship Generator",
    "author": "BlenderSpaceshipGenerator",
    "version": (2, 0, 0),
    "blender": (2, 80, 0),
    "location": "View3D > Sidebar > Spaceship",
    "description": "Generate procedural spaceships with EVEOFFLINE/Atlas integration",
    "category": "Add Mesh",
}

import bpy
from bpy.props import (
    EnumProperty,
    BoolProperty,
    IntProperty,
    FloatProperty,
    StringProperty,
)

from . import ship_generator
from . import ship_parts
from . import interior_generator
from . import module_system
from . import atlas_exporter
from . import station_generator
from . import asteroid_generator
from . import texture_generator
from . import brick_system


class SpaceshipGeneratorProperties(bpy.types.PropertyGroup):
    """Properties for the spaceship generator"""
    
    ship_class: EnumProperty(
        name="Ship Class",
        description="Type of ship to generate",
        items=[
            ('SHUTTLE', "Shuttle", "Small transport vessel"),
            ('FIGHTER', "Fighter", "Single-seat combat ship"),
            ('CORVETTE', "Corvette", "Small multi-crew combat ship"),
            ('FRIGATE', "Frigate", "Medium combat/utility ship"),
            ('DESTROYER', "Destroyer", "Heavy combat ship"),
            ('CRUISER', "Cruiser", "Large multi-role ship"),
            ('BATTLECRUISER', "Battlecruiser", "Heavy attack cruiser"),
            ('BATTLESHIP', "Battleship", "Heavy capital ship"),
            ('CARRIER', "Carrier", "Fleet carrier ship"),
            ('DREADNOUGHT', "Dreadnought", "Siege capital ship"),
            ('CAPITAL', "Capital", "Largest class capital ship"),
            ('TITAN', "Titan", "Supercapital flagship"),
            ('INDUSTRIAL', "Industrial", "Cargo hauler"),
            ('MINING_BARGE', "Mining Barge", "Mining vessel"),
            ('EXHUMER', "Exhumer", "Advanced mining vessel"),
            ('EXPLORER', "Explorer", "NMS-style long range exploration ship"),
            ('HAULER', "Hauler", "NMS-style heavy cargo transport"),
            ('EXOTIC', "Exotic", "NMS-style rare experimental ship"),
        ],
        default='FIGHTER'
    )
    
    generate_interior: BoolProperty(
        name="Generate Interior",
        description="Generate ship interior with rooms and corridors",
        default=True
    )
    
    module_slots: IntProperty(
        name="Module Slots",
        description="Number of additional module slots",
        default=2,
        min=0,
        max=10
    )
    
    seed: IntProperty(
        name="Random Seed",
        description="Seed for procedural generation",
        default=1,
        min=1
    )
    
    hull_complexity: FloatProperty(
        name="Hull Complexity",
        description="Complexity of hull geometry",
        default=1.0,
        min=0.1,
        max=3.0
    )
    
    symmetry: BoolProperty(
        name="Symmetry",
        description="Generate symmetric ship design",
        default=True
    )
    
    style: EnumProperty(
        name="Style",
        description="Ship design style",
        items=[
            ('MIXED', "Mixed", "Mixed style from all inspirations"),
            ('X4', "X4", "X4 Foundations style"),
            ('ELITE', "Elite Dangerous", "Elite Dangerous style"),
            ('EVE', "Eve Online", "Eve Online style"),
            ('SOLARI', "Solari", "EVEOFFLINE Solari faction - golden, elegant"),
            ('VEYREN', "Veyren", "EVEOFFLINE Veyren faction - angular, utilitarian"),
            ('AURELIAN', "Aurelian", "EVEOFFLINE Aurelian faction - sleek, organic"),
            ('KELDARI', "Keldari", "EVEOFFLINE Keldari faction - rugged, industrial"),
            ('NMS', "No Man's Sky", "No Man's Sky style - colorful, varied, organic"),
        ],
        default='MIXED'
    )

    eveoffline_json_path: StringProperty(
        name="Ship JSON",
        description="Path to an EVEOFFLINE ship JSON file to import",
        subtype='FILE_PATH',
        default=""
    )

    eveoffline_export_path: StringProperty(
        name="Export Path",
        description="Directory to export OBJ files for the EVEOFFLINE asset pipeline",
        subtype='DIR_PATH',
        default=""
    )

    station_type: EnumProperty(
        name="Station Type",
        description="Type of station to generate",
        items=[
            ('INDUSTRIAL', "Industrial", "Manufacturing and production"),
            ('MILITARY', "Military", "Naval and defense installation"),
            ('COMMERCIAL', "Commercial", "Trade and commerce hub"),
            ('RESEARCH', "Research", "Scientific research facility"),
            ('MINING', "Mining", "Ore refinement and mining support"),
            ('ASTRAHUS', "Astrahus", "Medium Upwell citadel"),
            ('FORTIZAR', "Fortizar", "Large Upwell citadel"),
            ('KEEPSTAR', "Keepstar", "Extra-large Upwell citadel"),
        ],
        default='INDUSTRIAL'
    )

    station_faction: EnumProperty(
        name="Station Faction",
        description="Faction style for the station",
        items=[
            ('SOLARI', "Solari", "Golden cathedral style"),
            ('VEYREN', "Veyren", "Industrial block style"),
            ('AURELIAN', "Aurelian", "Organic dome style"),
            ('KELDARI', "Keldari", "Rusted patchwork style"),
        ],
        default='SOLARI'
    )

    belt_layout: EnumProperty(
        name="Belt Layout",
        description="Asteroid belt shape",
        items=[
            ('SEMICIRCLE', "Semicircle", "Standard semicircular belt"),
            ('SPHERE', "Sphere", "Spherical distribution"),
            ('CLUSTER', "Cluster", "Dense anomaly cluster"),
            ('RING', "Ring", "Sparse outer ring"),
        ],
        default='SEMICIRCLE'
    )

    belt_ore_type: EnumProperty(
        name="Primary Ore",
        description="Primary ore type for the belt",
        items=[
            ('DUSTITE', "Dustite", "Brown-orange, common ore"),
            ('FERRITE', "Ferrite", "Gray metallic ore"),
            ('IGNAITE', "Ignaite", "Red-brown volcanic ore"),
            ('CRYSTITE', "Crystite", "Green crystalline ore"),
            ('SHADITE', "Shadite", "Golden-brown ore"),
            ('CORITE', "Corite", "Blue-cyan icy ore"),
            ('LUMINE', "Lumine", "Dark red dense ore"),
            ('SANGITE', "Sangite", "Bright red metallic ore"),
            ('GLACITE', "Glacite", "Golden valuable ore"),
            ('DENSITE', "Densite", "Light gray banded ore"),
            ('VOIDITE', "Voidite", "Dark nullsec ore"),
            ('SPODUMAIN', "Spodumain", "Silvery reflective ore"),
            ('PYRANITE', "Pyranite", "Purple rare ore"),
            ('STELLITE', "Stellite", "Green luminescent ore"),
            ('COSMITE', "Cosmite", "Orange-gold most valuable ore"),
            ('NEXORITE', "Nexorite", "Cyan crystalline radioactive ore"),
        ],
        default='DUSTITE'
    )

    belt_count: IntProperty(
        name="Asteroid Count",
        description="Number of asteroids in the belt",
        default=30,
        min=5,
        max=200
    )

    generate_textures: BoolProperty(
        name="Generate Textures",
        description="Apply procedural PBR textures to the generated ship",
        default=True
    )

    weathering: FloatProperty(
        name="Weathering",
        description="Amount of surface weathering, dirt, and wear",
        default=0.0,
        min=0.0,
        max=1.0
    )

    naming_prefix: StringProperty(
        name="Naming Prefix",
        description="Project naming prefix applied to all generated elements (e.g. 'EVEOFFLINE')",
        default=""
    )

    turret_hardpoints: IntProperty(
        name="Turret Hardpoints",
        description="Number of turret hardpoints (visual turret fittings with base, ring and barrel)",
        default=0,
        min=0,
        max=10
    )

    hull_taper: FloatProperty(
        name="Hull Taper",
        description="Silhouette taper factor (lower = more tapered, 1.0 = no taper)",
        default=0.85,
        min=0.5,
        max=1.0
    )

    ship_dna_export_path: StringProperty(
        name="Ship DNA Path",
        description="File path to export Ship DNA JSON for reproducible ships",
        subtype='FILE_PATH',
        default=""
    )


class SPACESHIP_OT_generate(bpy.types.Operator):
    """Generate a procedural spaceship"""
    bl_idname = "mesh.generate_spaceship"
    bl_label = "Generate Spaceship"
    bl_options = {'REGISTER', 'UNDO'}
    
    def execute(self, context):
        props = context.scene.spaceship_props
        
        # Generate the spaceship
        hull = ship_generator.generate_spaceship(
            ship_class=props.ship_class,
            seed=props.seed,
            generate_interior=props.generate_interior,
            module_slots=props.module_slots,
            hull_complexity=props.hull_complexity,
            symmetry=props.symmetry,
            style=props.style,
            naming_prefix=props.naming_prefix,
            turret_hardpoints=props.turret_hardpoints,
            hull_taper=props.hull_taper,
        )

        # Apply procedural textures if requested
        if props.generate_textures:
            texture_generator.apply_textures_to_ship(
                hull,
                style=props.style,
                seed=props.seed,
                weathering=props.weathering,
            )

        self.report({'INFO'}, f"Generated {props.ship_class} class spaceship")
        return {'FINISHED'}


class SPACESHIP_OT_import_eveoffline(bpy.types.Operator):
    """Import ships from EVEOFFLINE JSON data and generate them"""
    bl_idname = "mesh.import_eveoffline_ships"
    bl_label = "Import from EVEOFFLINE JSON"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        props = context.scene.spaceship_props
        json_path = bpy.path.abspath(props.eveoffline_json_path)

        if not json_path or not json_path.endswith('.json'):
            self.report({'ERROR'}, "Select a valid EVEOFFLINE ship JSON file")
            return {'CANCELLED'}

        import os
        if not os.path.isfile(json_path):
            self.report({'ERROR'}, f"File not found: {json_path}")
            return {'CANCELLED'}

        try:
            ships = atlas_exporter.load_ship_data(json_path)
        except Exception as e:
            self.report({'ERROR'}, f"Failed to load JSON: {e}")
            return {'CANCELLED'}

        count = 0
        for ship_id, ship_data in ships.items():
            config = atlas_exporter.parse_ship_config(ship_data)
            ship_generator.generate_spaceship(
                ship_class=config['ship_class'],
                seed=config['seed'],
                generate_interior=config['generate_interior'],
                module_slots=config['module_slots'],
                hull_complexity=config['hull_complexity'],
                symmetry=config['symmetry'],
                style=config['style'],
            )
            count += 1

        self.report({'INFO'}, f"Generated {count} ships from EVEOFFLINE data")
        return {'FINISHED'}


class SPACESHIP_OT_export_obj(bpy.types.Operator):
    """Export the selected ship as OBJ for the EVEOFFLINE/Atlas engine"""
    bl_idname = "mesh.export_eveoffline_obj"
    bl_label = "Export OBJ for Atlas"
    bl_options = {'REGISTER'}

    def execute(self, context):
        props = context.scene.spaceship_props
        export_dir = bpy.path.abspath(props.eveoffline_export_path)

        if not export_dir:
            self.report({'ERROR'}, "Set an export directory first")
            return {'CANCELLED'}

        import os
        os.makedirs(export_dir, exist_ok=True)

        obj = context.active_object
        if obj is None:
            self.report({'ERROR'}, "Select a ship object to export")
            return {'CANCELLED'}

        filename = obj.name.replace(' ', '_') + '.obj'
        filepath = os.path.join(export_dir, filename)

        # Select all children too
        bpy.ops.object.select_all(action='DESELECT')
        obj.select_set(True)
        for child in obj.children_recursive:
            child.select_set(True)

        atlas_exporter.export_obj(filepath)

        self.report({'INFO'}, f"Exported to {filepath}")
        return {'FINISHED'}


class SPACESHIP_OT_generate_station(bpy.types.Operator):
    """Generate a procedural space station"""
    bl_idname = "mesh.generate_station"
    bl_label = "Generate Station"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        props = context.scene.spaceship_props
        station_generator.generate_station(
            station_type=props.station_type,
            faction=props.station_faction,
            seed=props.seed,
        )
        self.report({'INFO'}, f"Generated {props.station_type} station ({props.station_faction})")
        return {'FINISHED'}


class SPACESHIP_OT_generate_asteroid_belt(bpy.types.Operator):
    """Generate a procedural asteroid belt"""
    bl_idname = "mesh.generate_asteroid_belt"
    bl_label = "Generate Asteroid Belt"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        props = context.scene.spaceship_props
        asteroid_generator.generate_asteroid_belt(
            layout=props.belt_layout,
            ore_types=[props.belt_ore_type],
            count=props.belt_count,
            seed=props.seed,
        )
        self.report({'INFO'}, f"Generated {props.belt_layout} belt with {props.belt_count} asteroids")
        return {'FINISHED'}


class SPACESHIP_OT_export_ship_dna(bpy.types.Operator):
    """Export Ship DNA JSON for the selected ship"""
    bl_idname = "mesh.export_ship_dna"
    bl_label = "Export Ship DNA"
    bl_options = {'REGISTER'}

    def execute(self, context):
        import os

        props = context.scene.spaceship_props
        export_path = bpy.path.abspath(props.ship_dna_export_path)

        if not export_path or not export_path.endswith('.json'):
            self.report({'ERROR'}, "Set a valid .json export path first")
            return {'CANCELLED'}

        obj = context.active_object
        if obj is None or "ship_dna" not in obj:
            self.report({'ERROR'}, "Select a generated ship (hull) with Ship DNA")
            return {'CANCELLED'}

        os.makedirs(os.path.dirname(export_path) or '.', exist_ok=True)
        with open(export_path, 'w') as f:
            f.write(obj["ship_dna"])

        self.report({'INFO'}, f"Ship DNA exported to {export_path}")
        return {'FINISHED'}


class SPACESHIP_PT_main_panel(bpy.types.Panel):
    """Main panel for spaceship generator"""
    bl_label = "Spaceship Generator"
    bl_idname = "SPACESHIP_PT_main_panel"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Spaceship'
    
    def draw(self, context):
        layout = self.layout
        props = context.scene.spaceship_props
        
        layout.label(text="Ship Configuration:")
        layout.prop(props, "ship_class")
        layout.prop(props, "style")
        layout.prop(props, "seed")
        
        layout.separator()
        layout.label(text="Generation Options:")
        layout.prop(props, "generate_interior")
        layout.prop(props, "module_slots")
        layout.prop(props, "hull_complexity")
        layout.prop(props, "symmetry")

        layout.separator()
        layout.label(text="Naming & Hardpoints:")
        layout.prop(props, "naming_prefix")
        layout.prop(props, "turret_hardpoints")

        layout.separator()
        layout.label(text="Hull Shaping:")
        layout.prop(props, "hull_taper")

        layout.separator()
        layout.label(text="Texture Options:")
        layout.prop(props, "generate_textures")
        if props.generate_textures:
            layout.prop(props, "weathering")
        
        layout.separator()
        layout.operator("mesh.generate_spaceship", icon='MESH_CUBE')

        layout.separator()
        layout.label(text="EVEOFFLINE / Atlas Integration:")
        layout.prop(props, "eveoffline_json_path")
        layout.operator("mesh.import_eveoffline_ships", icon='IMPORT')
        layout.prop(props, "eveoffline_export_path")
        layout.operator("mesh.export_eveoffline_obj", icon='EXPORT')

        layout.separator()
        layout.label(text="Station Generation:")
        layout.prop(props, "station_type")
        layout.prop(props, "station_faction")
        layout.operator("mesh.generate_station", icon='WORLD')

        layout.separator()
        layout.label(text="Asteroid Belt Generation:")
        layout.prop(props, "belt_layout")
        layout.prop(props, "belt_ore_type")
        layout.prop(props, "belt_count")
        layout.operator("mesh.generate_asteroid_belt", icon='OUTLINER_OB_POINTCLOUD')

        layout.separator()
        layout.label(text="Ship DNA Export:")
        layout.prop(props, "ship_dna_export_path")
        layout.operator("mesh.export_ship_dna", icon='FILE_TEXT')


# Registration
classes = (
    SpaceshipGeneratorProperties,
    SPACESHIP_OT_generate,
    SPACESHIP_OT_import_eveoffline,
    SPACESHIP_OT_export_obj,
    SPACESHIP_OT_generate_station,
    SPACESHIP_OT_generate_asteroid_belt,
    SPACESHIP_OT_export_ship_dna,
    SPACESHIP_PT_main_panel,
)


def register():
    for cls in classes:
        bpy.utils.register_class(cls)
    
    bpy.types.Scene.spaceship_props = bpy.props.PointerProperty(
        type=SpaceshipGeneratorProperties
    )
    
    # Register submodules
    ship_generator.register()
    ship_parts.register()
    interior_generator.register()
    module_system.register()
    atlas_exporter.register()
    station_generator.register()
    asteroid_generator.register()
    texture_generator.register()
    brick_system.register()


def unregister():
    # Unregister submodules
    brick_system.unregister()
    texture_generator.unregister()
    asteroid_generator.unregister()
    station_generator.unregister()
    atlas_exporter.unregister()
    module_system.unregister()
    interior_generator.unregister()
    ship_parts.unregister()
    ship_generator.unregister()
    
    del bpy.types.Scene.spaceship_props
    
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)


if __name__ == "__main__":
    register()
