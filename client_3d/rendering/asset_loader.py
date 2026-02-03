"""
Asset Loader for External 3D Models
Loads and manages external 3D models (.obj, .gltf, .fbx)
"""

from panda3d.core import (
    NodePath, Loader, LoaderOptions,
    Filename, ModelRoot, TexturePool,
    Shader, ShaderAttrib
)
from typing import Optional, Dict, Tuple
import os
import json


class AssetLoader:
    """
    Manages loading and caching of external 3D assets
    Supports .obj, .gltf/.glb, and .fbx formats
    """
    
    # Supported file extensions
    SUPPORTED_FORMATS = ['.obj', '.gltf', '.glb', '.fbx', '.bam', '.egg']
    
    def __init__(self, loader: Loader, assets_dir: str = "client_3d/assets"):
        """
        Initialize the asset loader
        
        Args:
            loader: Panda3D loader instance
            assets_dir: Base directory for assets
        """
        self.loader = loader
        self.assets_dir = assets_dir
        self.model_cache: Dict[str, NodePath] = {}
        self.texture_cache: Dict[str, str] = {}
        
        # Create assets directory if it doesn't exist
        self._ensure_directories()
        
        print(f"[AssetLoader] Initialized with base directory: {assets_dir}")
    
    def _ensure_directories(self):
        """Create necessary asset directories"""
        dirs = [
            self.assets_dir,
            os.path.join(self.assets_dir, "models"),
            os.path.join(self.assets_dir, "textures"),
            os.path.join(self.assets_dir, "ships"),
            os.path.join(self.assets_dir, "effects")
        ]
        for dir_path in dirs:
            os.makedirs(dir_path, exist_ok=True)
    
    def load_model(self, filename: str, cache: bool = True) -> Optional[NodePath]:
        """
        Load a 3D model from file
        
        Args:
            filename: Model filename (relative to assets_dir or absolute path)
            cache: Whether to cache the loaded model
        
        Returns:
            NodePath containing the model, or None if loading failed
        """
        # Check cache first
        if cache and filename in self.model_cache:
            print(f"[AssetLoader] Loading '{filename}' from cache")
            return self.model_cache[filename].copyTo(NodePath())
        
        # Resolve full path
        model_path = self._resolve_path(filename)
        if not model_path:
            print(f"[AssetLoader] Model not found: {filename}")
            return None
        
        # Determine file format
        ext = os.path.splitext(model_path)[1].lower()
        if ext not in self.SUPPORTED_FORMATS:
            print(f"[AssetLoader] Unsupported format: {ext}")
            return None
        
        try:
            print(f"[AssetLoader] Loading model: {model_path}")
            
            # Load the model
            model = self._load_by_format(model_path, ext)
            
            if model:
                # Validate and optimize the model
                model = self._validate_model(model, filename)
                
                # Cache if requested
                if cache:
                    self.model_cache[filename] = model
                
                print(f"[AssetLoader] Successfully loaded: {filename}")
                return model.copyTo(NodePath()) if cache else model
            else:
                print(f"[AssetLoader] Failed to load: {filename}")
                return None
                
        except Exception as e:
            print(f"[AssetLoader] Error loading {filename}: {e}")
            return None
    
    def _resolve_path(self, filename: str) -> Optional[str]:
        """
        Resolve filename to full path
        Checks multiple locations for the file
        """
        # If absolute path and exists, use it
        if os.path.isabs(filename) and os.path.exists(filename):
            return filename
        
        # Try relative to assets directory
        paths_to_try = [
            os.path.join(self.assets_dir, filename),
            os.path.join(self.assets_dir, "models", filename),
            os.path.join(self.assets_dir, "ships", filename),
            filename  # Try as-is
        ]
        
        for path in paths_to_try:
            if os.path.exists(path):
                return path
        
        return None
    
    def _load_by_format(self, path: str, ext: str) -> Optional[NodePath]:
        """
        Load model using appropriate method based on format
        
        Args:
            path: Full path to model file
            ext: File extension
        
        Returns:
            Loaded NodePath or None
        """
        if ext in ['.bam', '.egg']:
            # Native Panda3D formats
            return self.loader.loadModel(Filename.fromOsSpecific(path))
        
        elif ext == '.obj':
            # Wavefront OBJ format
            return self._load_obj(path)
        
        elif ext in ['.gltf', '.glb']:
            # glTF format (requires panda3d-gltf plugin)
            return self._load_gltf(path)
        
        elif ext == '.fbx':
            # FBX format (requires conversion or plugin)
            return self._load_fbx(path)
        
        return None
    
    def _load_obj(self, path: str) -> Optional[NodePath]:
        """
        Load Wavefront OBJ file
        Panda3D has basic OBJ support
        """
        try:
            # Use Panda3D's built-in OBJ loader
            options = LoaderOptions()
            options.setFlags(LoaderOptions.LF_no_cache)
            
            model = self.loader.loadModel(Filename.fromOsSpecific(path), options)
            return model
        except Exception as e:
            print(f"[AssetLoader] OBJ load error: {e}")
            return None
    
    def _load_gltf(self, path: str) -> Optional[NodePath]:
        """
        Load glTF/glb file
        Requires panda3d-gltf plugin
        """
        try:
            # Try to use gltf loader if available
            try:
                from gltf.loader import GltfLoader
                loader = GltfLoader()
                return loader.load(path)
            except ImportError:
                print("[AssetLoader] panda3d-gltf not installed, falling back to basic loader")
                # Try Panda3D's built-in support (if any)
                model = self.loader.loadModel(Filename.fromOsSpecific(path))
                return model
        except Exception as e:
            print(f"[AssetLoader] glTF load error: {e}")
            return None
    
    def _load_fbx(self, path: str) -> Optional[NodePath]:
        """
        Load FBX file
        FBX requires conversion or external plugin
        """
        try:
            # FBX is not natively supported by Panda3D
            # Would need to convert to .bam or .egg first using fbx2bam or similar
            print("[AssetLoader] FBX support requires fbx2bam converter")
            print("[AssetLoader] Please convert FBX to .bam or .gltf format")
            return None
        except Exception as e:
            print(f"[AssetLoader] FBX load error: {e}")
            return None
    
    def _validate_model(self, model: NodePath, filename: str) -> NodePath:
        """
        Validate and optimize loaded model
        
        Args:
            model: Loaded model NodePath
            filename: Original filename for logging
        
        Returns:
            Validated/optimized model
        """
        if not model or model.isEmpty():
            print(f"[AssetLoader] Model is empty: {filename}")
            return model
        
        # Get model info
        bounds = model.getBounds()
        print(f"[AssetLoader] Model bounds: {bounds}")
        
        # Center the model at origin if needed
        # (optional, depends on your coordinate system preferences)
        
        # Flatten the model for better performance
        model.flattenStrong()
        
        # Enable automatic shader generation for better appearance
        model.setShaderAuto()
        
        return model
    
    def load_texture(self, filename: str) -> Optional[str]:
        """
        Load a texture file
        
        Args:
            filename: Texture filename
        
        Returns:
            Path to loaded texture or None
        """
        # Check cache
        if filename in self.texture_cache:
            return self.texture_cache[filename]
        
        # Resolve path
        texture_path = self._resolve_texture_path(filename)
        if not texture_path:
            print(f"[AssetLoader] Texture not found: {filename}")
            return None
        
        try:
            # Load texture using Panda3D's texture pool
            texture = self.loader.loadTexture(Filename.fromOsSpecific(texture_path))
            if texture:
                self.texture_cache[filename] = texture_path
                print(f"[AssetLoader] Loaded texture: {filename}")
                return texture_path
        except Exception as e:
            print(f"[AssetLoader] Error loading texture {filename}: {e}")
        
        return None
    
    def _resolve_texture_path(self, filename: str) -> Optional[str]:
        """Resolve texture filename to full path"""
        paths_to_try = [
            os.path.join(self.assets_dir, filename),
            os.path.join(self.assets_dir, "textures", filename),
            filename
        ]
        
        for path in paths_to_try:
            if os.path.exists(path):
                return path
        
        return None
    
    def clear_cache(self):
        """Clear all cached models and textures"""
        self.model_cache.clear()
        self.texture_cache.clear()
        print("[AssetLoader] Cache cleared")
    
    def get_cache_info(self) -> Dict[str, int]:
        """Get information about cached assets"""
        return {
            'models_cached': len(self.model_cache),
            'textures_cached': len(self.texture_cache)
        }
    
    def list_available_models(self) -> list:
        """
        List all available models in the assets directory
        
        Returns:
            List of model filenames
        """
        models = []
        
        # Check models directory
        models_dir = os.path.join(self.assets_dir, "models")
        if os.path.exists(models_dir):
            for filename in os.listdir(models_dir):
                ext = os.path.splitext(filename)[1].lower()
                if ext in self.SUPPORTED_FORMATS:
                    models.append(filename)
        
        # Check ships directory
        ships_dir = os.path.join(self.assets_dir, "ships")
        if os.path.exists(ships_dir):
            for filename in os.listdir(ships_dir):
                ext = os.path.splitext(filename)[1].lower()
                if ext in self.SUPPORTED_FORMATS:
                    models.append(os.path.join("ships", filename))
        
        return sorted(models)
    
    def save_asset_manifest(self, output_path: str = None):
        """
        Save a manifest of all available assets
        Useful for documentation and validation
        """
        if output_path is None:
            output_path = os.path.join(self.assets_dir, "manifest.json")
        
        manifest = {
            'models': self.list_available_models(),
            'cache_info': self.get_cache_info(),
            'supported_formats': self.SUPPORTED_FORMATS
        }
        
        try:
            with open(output_path, 'w') as f:
                json.dump(manifest, f, indent=2)
            print(f"[AssetLoader] Manifest saved to: {output_path}")
        except Exception as e:
            print(f"[AssetLoader] Error saving manifest: {e}")
