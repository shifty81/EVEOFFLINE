"""
Entity Renderer for 3D Client
Renders game entities using Panda3D
"""

from panda3d.core import Vec3, Vec4, Point3, NodePath
from panda3d.core import AmbientLight, DirectionalLight
from panda3d.core import GeomNode
import os
from .ship_models import ShipModelGenerator


class EntityRenderer:
    """
    Renders game entities in 3D
    Handles ship models, effects, and visual representation
    """
    
    # Ship colors by faction (for placeholder rendering)
    FACTION_COLORS = {
        'Minmatar': Vec4(0.8, 0.5, 0.3, 1.0),  # Brown/rust
        'Caldari': Vec4(0.3, 0.5, 0.8, 1.0),   # Blue
        'Gallente': Vec4(0.3, 0.8, 0.5, 1.0),  # Green
        'Amarr': Vec4(0.9, 0.8, 0.3, 1.0),     # Gold
        'Serpentis': Vec4(0.6, 0.2, 0.6, 1.0), # Purple (NPC)
        'Guristas': Vec4(0.8, 0.2, 0.2, 1.0),  # Red (NPC)
        'Blood Raiders': Vec4(0.7, 0.1, 0.1, 1.0),  # Dark red (NPC)
        'default': Vec4(0.7, 0.7, 0.7, 1.0),   # Gray
    }
    
    def __init__(self, render, loader):
        self.render = render
        self.loader = loader
        self.entity_nodes = {}  # entity_id -> NodePath
        self.models_cache = {}  # model_name -> NodePath
        
        # Model paths
        self.model_dir = "client_3d/models/ships"
        
        # Procedural ship model generator
        self.ship_generator = ShipModelGenerator()
        
        # Setup lighting
        self._setup_lighting()
        
    def _setup_lighting(self):
        """Setup scene lighting"""
        # Ambient light (very low - space is dark!)
        ambient = AmbientLight('ambient')
        ambient.setColor(Vec4(0.15, 0.15, 0.2, 1.0))  # Slightly brighter dark blue
        ambient_np = self.render.attachNewNode(ambient)
        self.render.setLight(ambient_np)
        
        # Main directional light (sun/star)
        dlight = DirectionalLight('sun')
        dlight.setColor(Vec4(1.0, 0.95, 0.9, 1.0))  # Warm white light
        dlnp = self.render.attachNewNode(dlight)
        dlnp.setHpr(45, -45, 0)  # Angle the light
        self.render.setLight(dlnp)
        
        # Secondary fill light (softer, from opposite side)
        fill_light = DirectionalLight('fill')
        fill_light.setColor(Vec4(0.3, 0.35, 0.4, 1.0))  # Cool blue fill
        fill_np = self.render.attachNewNode(fill_light)
        fill_np.setHpr(-135, -30, 0)  # Opposite angle
        self.render.setLight(fill_np)
        
        print("[Renderer] Enhanced lighting setup complete")
    
    def _get_model_path(self, ship_type: str) -> str:
        """Get model file path for a ship type"""
        # Try different extensions
        base_name = ship_type.lower().replace(' ', '_')
        for ext in ['.bam', '.egg', '.gltf', '.obj']:
            path = f"{self.model_dir}/{base_name}{ext}"
            if os.path.exists(path):
                return path
        return None
    
    def _load_model(self, ship_type: str) -> NodePath:
        """
        Load 3D model for a ship type
        Returns None if model not found (will use placeholder)
        """
        # Check cache first
        if ship_type in self.models_cache:
            return self.models_cache[ship_type]
        
        # Try to load model
        model_path = self._get_model_path(ship_type)
        if model_path:
            try:
                model = self.loader.loadModel(model_path)
                if model:
                    self.models_cache[ship_type] = model
                    print(f"[Renderer] Loaded model: {ship_type} from {model_path}")
                    return model
            except Exception as e:
                print(f"[Renderer] Error loading model {ship_type}: {e}")
        
        # Model not found - will use placeholder
        return None
    
    def _create_placeholder(self, faction: str, ship_type: str) -> NodePath:
        """
        Create a procedural ship model
        Uses the ShipModelGenerator for detailed ship models
        """
        # Use procedural ship generator
        model = self.ship_generator.generate_ship_model(ship_type, faction)
        
        print(f"[Renderer] Generated procedural model for {faction} {ship_type}")
        return model
    
    def render_entity(self, entity):
        """
        Render or update an entity
        
        Args:
            entity: Entity object to render
        """
        entity_id = entity.id
        
        # Check if entity already has a node
        if entity_id in self.entity_nodes:
            # Update existing node position
            node = self.entity_nodes[entity_id]
            pos = entity.get_position()
            node.setPos(pos[0], pos[1], pos[2])
        else:
            # Create new node for entity
            # Try to load model
            model = self._load_model(entity.ship_type)
            
            if model:
                # Use actual model
                node = model.copyTo(self.render)
            else:
                # Use placeholder
                node = self._create_placeholder(entity.faction, entity.ship_type)
                node.reparentTo(self.render)
            
            # Set initial position
            pos = entity.get_position()
            node.setPos(pos[0], pos[1], pos[2])
            
            # Enable automatic shader generation for better lighting
            node.setShaderAuto()
            
            # Store node
            self.entity_nodes[entity_id] = node
            
            print(f"[Renderer] Created entity node: {entity_id} ({entity.ship_type})")
    
    def remove_entity(self, entity_id: str):
        """Remove entity from scene"""
        if entity_id in self.entity_nodes:
            node = self.entity_nodes[entity_id]
            node.removeNode()
            del self.entity_nodes[entity_id]
            print(f"[Renderer] Removed entity: {entity_id}")
    
    def update_entities(self, entities: dict):
        """
        Update all entities
        
        Args:
            entities: Dictionary of entity_id -> Entity
        """
        # Update existing entities
        current_ids = set(self.entity_nodes.keys())
        new_ids = set(entities.keys())
        
        # Remove entities that no longer exist
        for entity_id in current_ids - new_ids:
            self.remove_entity(entity_id)
        
        # Render/update all entities
        for entity_id, entity in entities.items():
            self.render_entity(entity)
    
    def clear(self):
        """Clear all rendered entities"""
        for entity_id in list(self.entity_nodes.keys()):
            self.remove_entity(entity_id)
        print("[Renderer] All entities cleared")
