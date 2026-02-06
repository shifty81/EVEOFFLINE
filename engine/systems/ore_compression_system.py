"""
Ore Compression System
Handles compression of ore and ice to reduce volume for transport
Based on EVE Online ore compression mechanics
"""

import json
import os
from typing import Optional, Dict, List, Tuple, Union
from engine.core.ecs import System, World, Entity
from engine.components.game_components import OreHold, Inventory, Skills


# Compression ratios by ore type (compressed volume / original volume)
# In EVE, compressed ore is much smaller than raw ore
ORE_COMPRESSION_DATA = {
    # High-sec ores
    "veldspar": {
        "batch_size": 100,  # m3 per compression batch
        "compressed_volume": 0.15,  # m3 per compressed unit
        "compressed_name": "Compressed Veldspar"
    },
    "scordite": {
        "batch_size": 100,
        "compressed_volume": 0.19,
        "compressed_name": "Compressed Scordite"
    },
    "pyroxeres": {
        "batch_size": 100,
        "compressed_volume": 0.16,
        "compressed_name": "Compressed Pyroxeres"
    },
    "plagioclase": {
        "batch_size": 100,
        "compressed_volume": 0.15,
        "compressed_name": "Compressed Plagioclase"
    },
    # Low-sec ores
    "kernite": {
        "batch_size": 100,
        "compressed_volume": 0.19,
        "compressed_name": "Compressed Kernite"
    },
    "omber": {
        "batch_size": 100,
        "compressed_volume": 0.07,
        "compressed_name": "Compressed Omber"
    },
    "jaspet": {
        "batch_size": 100,
        "compressed_volume": 0.15,
        "compressed_name": "Compressed Jaspet"
    },
    "hemorphite": {
        "batch_size": 100,
        "compressed_volume": 0.16,
        "compressed_name": "Compressed Hemorphite"
    },
    "hedbergite": {
        "batch_size": 100,
        "compressed_volume": 0.14,
        "compressed_name": "Compressed Hedbergite"
    },
    # Null-sec ores
    "gneiss": {
        "batch_size": 100,
        "compressed_volume": 0.10,
        "compressed_name": "Compressed Gneiss"
    },
    "dark_ochre": {
        "batch_size": 100,
        "compressed_volume": 0.15,
        "compressed_name": "Compressed Dark Ochre"
    },
    "crokite": {
        "batch_size": 100,
        "compressed_volume": 0.16,
        "compressed_name": "Compressed Crokite"
    },
    "bistot": {
        "batch_size": 100,
        "compressed_volume": 0.16,
        "compressed_name": "Compressed Bistot"
    },
    "arkonor": {
        "batch_size": 100,
        "compressed_volume": 0.16,
        "compressed_name": "Compressed Arkonor"
    },
    "mercoxit": {
        "batch_size": 100,
        "compressed_volume": 0.10,
        "compressed_name": "Compressed Mercoxit"
    },
}

ICE_COMPRESSION_DATA = {
    "clear_icicle": {
        "batch_size": 1,
        "compressed_volume": 10.0,
        "compressed_name": "Compressed Clear Icicle"
    },
    "white_glaze": {
        "batch_size": 1,
        "compressed_volume": 10.0,
        "compressed_name": "Compressed White Glaze"
    },
    "blue_ice": {
        "batch_size": 1,
        "compressed_volume": 10.0,
        "compressed_name": "Compressed Blue Ice"
    },
    "glacial_mass": {
        "batch_size": 1,
        "compressed_volume": 10.0,
        "compressed_name": "Compressed Glacial Mass"
    },
    "dark_glitter": {
        "batch_size": 1,
        "compressed_volume": 5.0,
        "compressed_name": "Compressed Dark Glitter"
    },
    "gelidus": {
        "batch_size": 1,
        "compressed_volume": 5.0,
        "compressed_name": "Compressed Gelidus"
    },
    "krystallos": {
        "batch_size": 1,
        "compressed_volume": 5.0,
        "compressed_name": "Compressed Krystallos"
    },
    "glare_crust": {
        "batch_size": 1,
        "compressed_volume": 5.0,
        "compressed_name": "Compressed Glare Crust"
    },
    "enriched_clear_icicle": {
        "batch_size": 1,
        "compressed_volume": 8.0,
        "compressed_name": "Compressed Enriched Clear Icicle"
    },
    "pristine_white_glaze": {
        "batch_size": 1,
        "compressed_volume": 8.0,
        "compressed_name": "Compressed Pristine White Glaze"
    },
    "thick_blue_ice": {
        "batch_size": 1,
        "compressed_volume": 8.0,
        "compressed_name": "Compressed Thick Blue Ice"
    },
    "smooth_glacial_mass": {
        "batch_size": 1,
        "compressed_volume": 8.0,
        "compressed_name": "Compressed Smooth Glacial Mass"
    },
}


class OreCompressionSystem(System):
    """
    Manages ore and ice compression operations.
    
    Compression reduces volume for efficient transport:
    - Ore: 100 m3 raw → ~0.15 m3 compressed (varies by ore type)
    - Ice: 1000 m3 raw → 10 m3 compressed (per unit)
    
    Requirements:
    - Must be performed at a station or Rorqual industrial ship
    - Ore Compression skill reduces waste
    - Ore is compressed in fixed batch sizes
    """
    
    def __init__(self, world: World):
        super().__init__(world)
    
    def compress_ore(
        self,
        entity: Union[Entity, str],
        ore_type: str,
        quantity: float,
        at_station: bool = True
    ) -> Optional[Dict[str, float]]:
        """
        Compress raw ore to reduce volume.
        
        Args:
            entity: Entity with OreHold or Inventory containing the ore
            ore_type: Type of ore to compress (e.g., 'veldspar')
            quantity: Amount of ore in m3 to compress
            at_station: Whether compression is being done at a station/structure
            
        Returns:
            Dict with compressed ore type and amount, or None on failure
        """
        entity = self._get_entity(entity)
        if not entity:
            return None
        
        if not at_station:
            return None
        
        # Look up compression data
        compression_info = ORE_COMPRESSION_DATA.get(ore_type)
        if not compression_info:
            return None
        
        batch_size = compression_info["batch_size"]
        compressed_volume = compression_info["compressed_volume"]
        compressed_name = compression_info["compressed_name"]
        
        # Find ore source
        ore_hold = entity.get_component(OreHold)
        inventory = entity.get_component(Inventory)
        
        ore_available = 0.0
        source = None
        
        if ore_hold and ore_type in ore_hold.ore:
            ore_available = ore_hold.ore[ore_type]
            source = "ore_hold"
        elif inventory and ore_type in getattr(inventory, 'items', {}):
            ore_available = float(inventory.items.get(ore_type, 0))
            source = "inventory"
        
        if ore_available < quantity:
            return None
        
        # Calculate batches
        batches = int(quantity / batch_size)
        if batches <= 0:
            return None
        
        actual_ore_used = batches * batch_size
        
        # Calculate waste reduction from skills
        skills = entity.get_component(Skills)
        waste_reduction = 0.0
        if skills:
            compression_level = skills.skills.get("ore_compression", 0)
            waste_reduction = compression_level * 0.02  # 2% waste reduction per level
        
        # Compressed output: each batch produces 1 compressed unit
        compressed_amount = batches * (1.0 + waste_reduction)
        compressed_total_volume = compressed_amount * compressed_volume
        
        # Consume raw ore
        if source == "ore_hold":
            ore_hold.ore[ore_type] -= actual_ore_used
            ore_hold.ore_hold_used -= actual_ore_used
            if ore_hold.ore[ore_type] <= 0:
                del ore_hold.ore[ore_type]
        elif source == "inventory":
            inventory.items[ore_type] -= int(actual_ore_used)
            if inventory.items[ore_type] <= 0:
                del inventory.items[ore_type]
        
        # Add compressed ore to inventory
        compressed_key = f"compressed_{ore_type}"
        if inventory:
            inventory.items[compressed_key] = inventory.items.get(compressed_key, 0) + int(compressed_amount)
        elif ore_hold:
            ore_hold.ore[compressed_key] = ore_hold.ore.get(compressed_key, 0.0) + compressed_amount
            ore_hold.ore_hold_used += compressed_total_volume
        
        return {
            "compressed_type": compressed_key,
            "compressed_amount": compressed_amount,
            "compressed_volume": compressed_total_volume,
            "ore_consumed": actual_ore_used,
            "batches": batches,
            "volume_saved": actual_ore_used - compressed_total_volume
        }
    
    def compress_ice(
        self,
        entity: Union[Entity, str],
        ice_type: str,
        quantity: int,
        at_station: bool = True
    ) -> Optional[Dict[str, float]]:
        """
        Compress ice blocks to reduce volume.
        
        Args:
            entity: Entity with OreHold or Inventory containing the ice
            ice_type: Type of ice to compress (e.g., 'clear_icicle')
            quantity: Number of ice units to compress
            at_station: Whether compression is being done at a station/structure
            
        Returns:
            Dict with compressed ice type and amount, or None on failure
        """
        entity = self._get_entity(entity)
        if not entity:
            return None
        
        if not at_station:
            return None
        
        compression_info = ICE_COMPRESSION_DATA.get(ice_type)
        if not compression_info:
            return None
        
        compressed_volume_per_unit = compression_info["compressed_volume"]
        
        # Find ice source
        ore_hold = entity.get_component(OreHold)
        inventory = entity.get_component(Inventory)
        
        ice_available = 0
        source = None
        
        if ore_hold and ice_type in ore_hold.ice:
            ice_available = ore_hold.ice[ice_type]
            source = "ore_hold"
        elif inventory and ice_type in getattr(inventory, 'items', {}):
            ice_available = inventory.items.get(ice_type, 0)
            source = "inventory"
        
        if ice_available < quantity:
            return None
        
        # Each ice unit compresses 1:1 but with reduced volume
        # Original ice volume: 1000 m3 per unit
        original_volume_per_unit = 1000.0
        compressed_amount = quantity
        compressed_total_volume = compressed_amount * compressed_volume_per_unit
        original_total_volume = quantity * original_volume_per_unit
        
        # Consume raw ice
        if source == "ore_hold":
            ore_hold.ice[ice_type] -= quantity
            ore_hold.ore_hold_used -= original_total_volume
            if ore_hold.ice[ice_type] <= 0:
                del ore_hold.ice[ice_type]
        elif source == "inventory":
            inventory.items[ice_type] -= quantity
            if inventory.items[ice_type] <= 0:
                del inventory.items[ice_type]
        
        # Add compressed ice
        compressed_key = f"compressed_{ice_type}"
        if inventory:
            inventory.items[compressed_key] = inventory.items.get(compressed_key, 0) + compressed_amount
        elif ore_hold:
            ore_hold.ice[compressed_key] = ore_hold.ice.get(compressed_key, 0) + compressed_amount
            ore_hold.ore_hold_used += compressed_total_volume
        
        return {
            "compressed_type": compressed_key,
            "compressed_amount": compressed_amount,
            "compressed_volume": compressed_total_volume,
            "ice_consumed": quantity,
            "volume_saved": original_total_volume - compressed_total_volume
        }
    
    def get_compression_info(self, material_type: str) -> Optional[Dict]:
        """
        Get compression information for an ore or ice type.
        
        Args:
            material_type: Ore or ice type name
            
        Returns:
            Dict with compression details or None if not compressible
        """
        if material_type in ORE_COMPRESSION_DATA:
            data = ORE_COMPRESSION_DATA[material_type]
            return {
                "type": "ore",
                "batch_size": data["batch_size"],
                "compressed_volume": data["compressed_volume"],
                "compressed_name": data["compressed_name"],
                "compression_ratio": data["batch_size"] / data["compressed_volume"]
            }
        elif material_type in ICE_COMPRESSION_DATA:
            data = ICE_COMPRESSION_DATA[material_type]
            return {
                "type": "ice",
                "batch_size": data["batch_size"],
                "compressed_volume": data["compressed_volume"],
                "compressed_name": data["compressed_name"],
                "compression_ratio": 1000.0 / data["compressed_volume"]  # Ice is 1000 m3/unit
            }
        return None
    
    def get_compressible_ores(self) -> List[str]:
        """Get list of all compressible ore types"""
        return list(ORE_COMPRESSION_DATA.keys())
    
    def get_compressible_ice(self) -> List[str]:
        """Get list of all compressible ice types"""
        return list(ICE_COMPRESSION_DATA.keys())
    
    def _get_entity(self, entity: Union[Entity, str]) -> Optional[Entity]:
        """Get entity by reference or ID"""
        if isinstance(entity, Entity):
            return entity
        return self.world.get_entity(entity) if hasattr(self.world, 'get_entity') else None
    
    def update(self, delta_time: float):
        """Compression is instant, no periodic update needed"""
        pass
