"""
PBR (Physically Based Rendering) Material System
Provides realistic materials using metallic/roughness workflow
"""

from panda3d.core import (
    Shader, ShaderAttrib, Texture, TextureStage,
    Material, Vec4, Vec3, NodePath, TexturePool,
    Filename, SamplerState
)
from typing import Optional, Dict, Tuple
import os


class PBRMaterial:
    """
    Represents a PBR material with metallic/roughness workflow
    """
    
    def __init__(self, name: str):
        """
        Initialize a PBR material
        
        Args:
            name: Material name
        """
        self.name = name
        
        # Base color (albedo)
        self.base_color = Vec4(1.0, 1.0, 1.0, 1.0)
        
        # PBR properties
        self.metallic = 0.0  # 0.0 = dielectric, 1.0 = metal
        self.roughness = 0.5  # 0.0 = smooth/glossy, 1.0 = rough/matte
        self.emission = Vec3(0.0, 0.0, 0.0)  # Emissive color
        self.emission_strength = 0.0
        
        # Texture maps
        self.albedo_texture: Optional[Texture] = None
        self.metallic_texture: Optional[Texture] = None
        self.roughness_texture: Optional[Texture] = None
        self.normal_texture: Optional[Texture] = None
        self.emission_texture: Optional[Texture] = None
        self.ao_texture: Optional[Texture] = None  # Ambient occlusion
        
        # Panda3D material
        self.material = Material()
        self.material.setMetallic(self.metallic)
        self.material.setRoughness(self.roughness)
        self.material.setBaseColor(self.base_color)
    
    def set_base_color(self, color: Vec4):
        """Set base color (albedo)"""
        self.base_color = color
        self.material.setBaseColor(color)
    
    def set_metallic(self, metallic: float):
        """Set metallic value (0.0 - 1.0)"""
        self.metallic = max(0.0, min(1.0, metallic))
        self.material.setMetallic(self.metallic)
    
    def set_roughness(self, roughness: float):
        """Set roughness value (0.0 - 1.0)"""
        self.roughness = max(0.0, min(1.0, roughness))
        self.material.setRoughness(self.roughness)
    
    def set_emission(self, color: Vec3, strength: float = 1.0):
        """Set emission color and strength"""
        self.emission = color
        self.emission_strength = strength
        self.material.setEmission(Vec4(
            color.x * strength,
            color.y * strength,
            color.z * strength,
            1.0
        ))
    
    def load_albedo_texture(self, texture_path: str, loader) -> bool:
        """Load albedo (base color) texture"""
        try:
            self.albedo_texture = loader.loadTexture(Filename.fromOsSpecific(texture_path))
            if self.albedo_texture:
                print(f"[PBRMaterial] Loaded albedo texture: {texture_path}")
                return True
        except Exception as e:
            print(f"[PBRMaterial] Error loading albedo texture: {e}")
        return False
    
    def load_metallic_texture(self, texture_path: str, loader) -> bool:
        """Load metallic texture"""
        try:
            self.metallic_texture = loader.loadTexture(Filename.fromOsSpecific(texture_path))
            if self.metallic_texture:
                print(f"[PBRMaterial] Loaded metallic texture: {texture_path}")
                return True
        except Exception as e:
            print(f"[PBRMaterial] Error loading metallic texture: {e}")
        return False
    
    def load_roughness_texture(self, texture_path: str, loader) -> bool:
        """Load roughness texture"""
        try:
            self.roughness_texture = loader.loadTexture(Filename.fromOsSpecific(texture_path))
            if self.roughness_texture:
                print(f"[PBRMaterial] Loaded roughness texture: {texture_path}")
                return True
        except Exception as e:
            print(f"[PBRMaterial] Error loading roughness texture: {e}")
        return False
    
    def load_normal_texture(self, texture_path: str, loader) -> bool:
        """Load normal map texture"""
        try:
            self.normal_texture = loader.loadTexture(Filename.fromOsSpecific(texture_path))
            if self.normal_texture:
                print(f"[PBRMaterial] Loaded normal texture: {texture_path}")
                return True
        except Exception as e:
            print(f"[PBRMaterial] Error loading normal texture: {e}")
        return False
    
    def load_emission_texture(self, texture_path: str, loader) -> bool:
        """Load emission texture"""
        try:
            self.emission_texture = loader.loadTexture(Filename.fromOsSpecific(texture_path))
            if self.emission_texture:
                print(f"[PBRMaterial] Loaded emission texture: {texture_path}")
                return True
        except Exception as e:
            print(f"[PBRMaterial] Error loading emission texture: {e}")
        return False
    
    def load_ao_texture(self, texture_path: str, loader) -> bool:
        """Load ambient occlusion texture"""
        try:
            self.ao_texture = loader.loadTexture(Filename.fromOsSpecific(texture_path))
            if self.ao_texture:
                print(f"[PBRMaterial] Loaded AO texture: {texture_path}")
                return True
        except Exception as e:
            print(f"[PBRMaterial] Error loading AO texture: {e}")
        return False
    
    def apply_to_node(self, node: NodePath):
        """
        Apply this material to a NodePath
        
        Args:
            node: NodePath to apply material to
        """
        # Apply material
        node.setMaterial(self.material)
        
        # Apply textures if available
        if self.albedo_texture:
            node.setTexture(self.albedo_texture)
        
        # For more advanced texture mapping, would need custom shaders
        # This is basic implementation
        
        print(f"[PBRMaterial] Applied material '{self.name}' to node")


class PBRMaterialLibrary:
    """
    Library of PBR materials
    Manages creation and caching of materials
    """
    
    def __init__(self, loader, textures_dir: str = "client_3d/assets/textures"):
        """
        Initialize material library
        
        Args:
            loader: Panda3D loader instance
            textures_dir: Directory containing textures
        """
        self.loader = loader
        self.textures_dir = textures_dir
        self.materials: Dict[str, PBRMaterial] = {}
        
        # Create some default materials
        self._create_default_materials()
        
        print(f"[PBRMaterialLibrary] Initialized with {len(self.materials)} default materials")
    
    def _create_default_materials(self):
        """Create default material presets"""
        # Metal materials
        self.materials['steel'] = self._create_metal_material(
            'steel',
            base_color=Vec4(0.75, 0.75, 0.75, 1.0),
            metallic=0.9,
            roughness=0.3
        )
        
        self.materials['gold'] = self._create_metal_material(
            'gold',
            base_color=Vec4(1.0, 0.85, 0.4, 1.0),
            metallic=1.0,
            roughness=0.2
        )
        
        self.materials['copper'] = self._create_metal_material(
            'copper',
            base_color=Vec4(0.95, 0.64, 0.54, 1.0),
            metallic=1.0,
            roughness=0.25
        )
        
        # Painted materials
        self.materials['matte_black'] = self._create_painted_material(
            'matte_black',
            base_color=Vec4(0.1, 0.1, 0.1, 1.0),
            metallic=0.0,
            roughness=0.9
        )
        
        self.materials['glossy_white'] = self._create_painted_material(
            'glossy_white',
            base_color=Vec4(0.95, 0.95, 0.95, 1.0),
            metallic=0.0,
            roughness=0.1
        )
        
        # Ship hull materials
        self.materials['ship_hull_minmatar'] = self._create_ship_material(
            'ship_hull_minmatar',
            base_color=Vec4(0.5, 0.35, 0.25, 1.0),  # Rust brown
            metallic=0.6,
            roughness=0.7
        )
        
        self.materials['ship_hull_caldari'] = self._create_ship_material(
            'ship_hull_caldari',
            base_color=Vec4(0.35, 0.45, 0.55, 1.0),  # Steel blue
            metallic=0.8,
            roughness=0.4
        )
        
        self.materials['ship_hull_gallente'] = self._create_ship_material(
            'ship_hull_gallente',
            base_color=Vec4(0.3, 0.4, 0.35, 1.0),  # Dark green
            metallic=0.7,
            roughness=0.5
        )
        
        self.materials['ship_hull_amarr'] = self._create_ship_material(
            'ship_hull_amarr',
            base_color=Vec4(0.6, 0.55, 0.45, 1.0),  # Gold-brass
            metallic=0.9,
            roughness=0.3
        )
        
        # Emissive materials for engines/lights
        self.materials['engine_glow_blue'] = self._create_emissive_material(
            'engine_glow_blue',
            base_color=Vec4(0.1, 0.3, 0.6, 1.0),
            emission=Vec3(0.3, 0.6, 1.0),
            strength=2.0
        )
        
        self.materials['engine_glow_orange'] = self._create_emissive_material(
            'engine_glow_orange',
            base_color=Vec4(0.6, 0.3, 0.1, 1.0),
            emission=Vec3(1.0, 0.5, 0.2),
            strength=2.0
        )
    
    def _create_metal_material(self, name: str, base_color: Vec4, 
                              metallic: float, roughness: float) -> PBRMaterial:
        """Create a metallic material"""
        mat = PBRMaterial(name)
        mat.set_base_color(base_color)
        mat.set_metallic(metallic)
        mat.set_roughness(roughness)
        return mat
    
    def _create_painted_material(self, name: str, base_color: Vec4,
                                metallic: float, roughness: float) -> PBRMaterial:
        """Create a painted (non-metallic) material"""
        mat = PBRMaterial(name)
        mat.set_base_color(base_color)
        mat.set_metallic(metallic)
        mat.set_roughness(roughness)
        return mat
    
    def _create_ship_material(self, name: str, base_color: Vec4,
                             metallic: float, roughness: float) -> PBRMaterial:
        """Create a ship hull material"""
        mat = PBRMaterial(name)
        mat.set_base_color(base_color)
        mat.set_metallic(metallic)
        mat.set_roughness(roughness)
        return mat
    
    def _create_emissive_material(self, name: str, base_color: Vec4,
                                 emission: Vec3, strength: float) -> PBRMaterial:
        """Create an emissive (glowing) material"""
        mat = PBRMaterial(name)
        mat.set_base_color(base_color)
        mat.set_metallic(0.0)
        mat.set_roughness(0.5)
        mat.set_emission(emission, strength)
        return mat
    
    def get_material(self, name: str) -> Optional[PBRMaterial]:
        """
        Get a material by name
        
        Args:
            name: Material name
        
        Returns:
            PBRMaterial or None if not found
        """
        return self.materials.get(name)
    
    def create_material(self, name: str, **kwargs) -> PBRMaterial:
        """
        Create a new custom material
        
        Args:
            name: Material name
            **kwargs: Material properties (base_color, metallic, roughness, etc.)
        
        Returns:
            Created PBRMaterial
        """
        mat = PBRMaterial(name)
        
        if 'base_color' in kwargs:
            mat.set_base_color(kwargs['base_color'])
        if 'metallic' in kwargs:
            mat.set_metallic(kwargs['metallic'])
        if 'roughness' in kwargs:
            mat.set_roughness(kwargs['roughness'])
        if 'emission' in kwargs:
            emission = kwargs['emission']
            strength = kwargs.get('emission_strength', 1.0)
            mat.set_emission(emission, strength)
        
        self.materials[name] = mat
        print(f"[PBRMaterialLibrary] Created material: {name}")
        return mat
    
    def apply_material_to_node(self, node: NodePath, material_name: str):
        """
        Apply a material to a node
        
        Args:
            node: NodePath to apply material to
            material_name: Name of material to apply
        """
        material = self.get_material(material_name)
        if material:
            material.apply_to_node(node)
        else:
            print(f"[PBRMaterialLibrary] Material not found: {material_name}")
    
    def list_materials(self) -> list:
        """Get list of all available materials"""
        return sorted(self.materials.keys())
    
    def get_faction_material(self, faction: str) -> Optional[PBRMaterial]:
        """
        Get appropriate material for a faction
        
        Args:
            faction: Faction name
        
        Returns:
            PBRMaterial for that faction
        """
        faction_map = {
            'Minmatar': 'ship_hull_minmatar',
            'Caldari': 'ship_hull_caldari',
            'Gallente': 'ship_hull_gallente',
            'Amarr': 'ship_hull_amarr',
        }
        
        material_name = faction_map.get(faction)
        if material_name:
            return self.get_material(material_name)
        
        # Default to steel for unknown factions
        return self.get_material('steel')


class PBRShaderManager:
    """
    Manages PBR shader compilation and application
    Provides custom PBR shaders if Panda3D's auto-shader isn't sufficient
    """
    
    def __init__(self):
        """Initialize shader manager"""
        self.shaders: Dict[str, Shader] = {}
        self.shader_dir = "client_3d/shaders"
        
        print("[PBRShaderManager] Initialized")
    
    def load_pbr_shader(self, name: str = "pbr") -> Optional[Shader]:
        """
        Load PBR shader
        
        Args:
            name: Shader name
        
        Returns:
            Compiled Shader or None
        """
        # Check cache
        if name in self.shaders:
            return self.shaders[name]
        
        # Try to load custom shader files
        vertex_path = os.path.join(self.shader_dir, f"{name}.vert")
        fragment_path = os.path.join(self.shader_dir, f"{name}.frag")
        
        if os.path.exists(vertex_path) and os.path.exists(fragment_path):
            try:
                shader = Shader.load(Shader.SL_GLSL,
                                    vertex=vertex_path,
                                    fragment=fragment_path)
                if shader:
                    self.shaders[name] = shader
                    print(f"[PBRShaderManager] Loaded custom shader: {name}")
                    return shader
            except Exception as e:
                print(f"[PBRShaderManager] Error loading shader: {e}")
        
        print(f"[PBRShaderManager] Custom shader not found, using auto-shader")
        return None
    
    def apply_pbr_shader(self, node: NodePath, shader_name: str = "pbr"):
        """
        Apply PBR shader to a node
        
        Args:
            node: NodePath to apply shader to
            shader_name: Name of shader to apply
        """
        shader = self.load_pbr_shader(shader_name)
        if shader:
            node.setShader(shader)
            print(f"[PBRShaderManager] Applied shader '{shader_name}' to node")
        else:
            # Fallback to auto-shader
            node.setShaderAuto()
            print(f"[PBRShaderManager] Applied auto-shader to node")
