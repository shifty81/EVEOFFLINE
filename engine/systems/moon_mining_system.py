"""
Moon Mining System
Handles moon extraction operations, chunk fracturing, and moon ore belts
Based on EVE Online moon mining mechanics (Refinery-based)
"""

from typing import Optional, Dict, List, Tuple, Union
from engine.core.ecs import System, World, Entity
from engine.components.game_components import (
    MoonMiningOperation, MoonResource, Structure, Skills
)


# Moon ore types and their mineral contents
# These special ores are only found from moon mining
MOON_ORE_DATA = {
    # R4 (Common moon ores)
    "ubiquitous_moon_ore": {
        "rarity": "R4",
        "name": "Ubiquitous Moon Ore",
        "volume_per_unit": 0.1,
        "minerals": {"tritanium": 500, "pyerite": 200}
    },
    "common_moon_ore": {
        "rarity": "R4",
        "name": "Common Moon Ore",
        "volume_per_unit": 0.1,
        "minerals": {"tritanium": 400, "mexallon": 150}
    },
    # R8 (Uncommon moon ores)
    "uncommon_moon_ore": {
        "rarity": "R8",
        "name": "Uncommon Moon Ore",
        "volume_per_unit": 0.1,
        "minerals": {"pyerite": 300, "isogen": 100}
    },
    "sylvite": {
        "rarity": "R8",
        "name": "Sylvite",
        "volume_per_unit": 0.1,
        "minerals": {"mexallon": 200, "nocxium": 50}
    },
    # R16 (Rare moon ores)
    "rare_moon_ore": {
        "rarity": "R16",
        "name": "Rare Moon Ore",
        "volume_per_unit": 0.1,
        "minerals": {"isogen": 200, "nocxium": 100, "zydrine": 20}
    },
    "cobaltite": {
        "rarity": "R16",
        "name": "Cobaltite",
        "volume_per_unit": 0.1,
        "minerals": {"nocxium": 150, "zydrine": 30}
    },
    # R32 (Exceptional moon ores)
    "exceptional_moon_ore": {
        "rarity": "R32",
        "name": "Exceptional Moon Ore",
        "volume_per_unit": 0.1,
        "minerals": {"nocxium": 200, "zydrine": 50, "megacyte": 10}
    },
    "carnotite": {
        "rarity": "R32",
        "name": "Carnotite",
        "volume_per_unit": 0.1,
        "minerals": {"zydrine": 80, "megacyte": 20}
    },
    # R64 (Legendary moon ores - very rare)
    "legendary_moon_ore": {
        "rarity": "R64",
        "name": "Legendary Moon Ore",
        "volume_per_unit": 0.1,
        "minerals": {"zydrine": 100, "megacyte": 50, "morphite": 5}
    },
    "xenotime": {
        "rarity": "R64",
        "name": "Xenotime",
        "volume_per_unit": 0.1,
        "minerals": {"megacyte": 80, "morphite": 10}
    },
}

# Standard moon compositions by security status
MOON_COMPOSITIONS = {
    "highsec": {
        "ubiquitous_moon_ore": 0.50,
        "common_moon_ore": 0.35,
        "uncommon_moon_ore": 0.15,
    },
    "lowsec": {
        "ubiquitous_moon_ore": 0.20,
        "common_moon_ore": 0.25,
        "uncommon_moon_ore": 0.25,
        "rare_moon_ore": 0.20,
        "exceptional_moon_ore": 0.10,
    },
    "nullsec": {
        "uncommon_moon_ore": 0.15,
        "rare_moon_ore": 0.25,
        "exceptional_moon_ore": 0.30,
        "legendary_moon_ore": 0.20,
        "xenotime": 0.10,
    },
}


class MoonManager:
    """Manages moon resources across the game universe"""
    
    def __init__(self):
        self.moons: Dict[str, Dict] = {}  # moon_id -> moon data
    
    def register_moon(
        self,
        moon_id: str,
        moon_name: str,
        system_id: str,
        planet_id: str,
        security: float,
        composition: Optional[Dict[str, float]] = None,
        yield_multiplier: float = 1.0
    ) -> Dict:
        """
        Register a moon with its resource composition.
        
        Args:
            moon_id: Unique moon identifier
            moon_name: Display name
            system_id: Solar system ID
            planet_id: Parent planet ID
            security: System security status
            composition: Custom composition or None for auto-generation
            yield_multiplier: Quality multiplier (0.5-2.0)
            
        Returns:
            Moon data dict
        """
        if composition is None:
            composition = self._generate_composition(security)
        
        moon_data = {
            "moon_id": moon_id,
            "moon_name": moon_name,
            "system_id": system_id,
            "planet_id": planet_id,
            "security": security,
            "composition": composition,
            "yield_multiplier": yield_multiplier,
            "base_ore_volume": 20000.0,
        }
        
        self.moons[moon_id] = moon_data
        return moon_data
    
    def get_moon(self, moon_id: str) -> Optional[Dict]:
        """Get moon data by ID"""
        return self.moons.get(moon_id)
    
    def get_system_moons(self, system_id: str) -> List[Dict]:
        """Get all moons in a solar system"""
        return [m for m in self.moons.values() if m["system_id"] == system_id]
    
    def _generate_composition(self, security: float) -> Dict[str, float]:
        """Generate moon composition based on security status"""
        if security >= 0.5:
            return dict(MOON_COMPOSITIONS["highsec"])
        elif security > 0.0:
            return dict(MOON_COMPOSITIONS["lowsec"])
        else:
            return dict(MOON_COMPOSITIONS["nullsec"])


class MoonMiningSystem(System):
    """
    Manages moon mining operations:
    - Moon drill activation and extraction scheduling
    - Chunk extraction over time
    - Moon ore belt creation after fracturing
    - Ore distribution based on moon composition
    
    Process flow:
    1. Anchor a refinery near a moon
    2. Start extraction (moon drill pulls a chunk)
    3. Wait for extraction to complete (configurable, default 6h)
    4. Fracture the chunk to create a minable asteroid belt
    5. Fleet mines the moon ore belt
    6. Belt despawns after 48 hours or when depleted
    """
    
    def __init__(self, world: World, moon_manager: MoonManager):
        super().__init__(world)
        self.moon_manager = moon_manager
    
    def start_extraction(
        self,
        entity: Union[Entity, str],
        moon_id: str,
        duration: float = 21600.0
    ) -> bool:
        """
        Start a moon extraction operation.
        
        Args:
            entity: Entity with MoonMiningOperation component
            moon_id: ID of the moon to extract from
            duration: Extraction duration in seconds (default 6 hours)
            
        Returns:
            True if extraction started successfully
        """
        entity = self._get_entity(entity)
        if not entity:
            return False
        
        operation = entity.get_component(MoonMiningOperation)
        if not operation:
            return False
        
        # Can't start if already extracting or belt is active
        if operation.state not in ("idle",):
            return False
        
        # Verify moon exists
        moon_data = self.moon_manager.get_moon(moon_id)
        if not moon_data:
            return False
        
        # Must have minimum 1 hour extraction
        if duration < 3600.0:
            return False
        
        # Start extraction
        operation.moon_id = moon_id
        operation.system_id = moon_data["system_id"]
        operation.state = "extracting"
        operation.extraction_duration = duration
        operation.extraction_progress = 0.0
        operation.moon_composition = dict(moon_data["composition"])
        
        return True
    
    def cancel_extraction(self, entity: Union[Entity, str]) -> bool:
        """
        Cancel an ongoing extraction.
        
        Args:
            entity: Entity with MoonMiningOperation component
            
        Returns:
            True if extraction was cancelled
        """
        entity = self._get_entity(entity)
        if not entity:
            return False
        
        operation = entity.get_component(MoonMiningOperation)
        if not operation:
            return False
        
        if operation.state != "extracting":
            return False
        
        operation.state = "idle"
        operation.extraction_progress = 0.0
        return True
    
    def fracture_chunk(self, entity: Union[Entity, str]) -> Optional[Dict[str, float]]:
        """
        Fracture the extracted moon chunk to create a minable belt.
        
        Args:
            entity: Entity with MoonMiningOperation component
            
        Returns:
            Dict of {ore_type: m3_available} in the belt, or None on failure
        """
        entity = self._get_entity(entity)
        if not entity:
            return None
        
        operation = entity.get_component(MoonMiningOperation)
        if not operation:
            return None
        
        if operation.state != "ready":
            return None
        
        # Get moon data for yield calculation
        moon_data = self.moon_manager.get_moon(operation.moon_id)
        if not moon_data:
            return None
        
        yield_multiplier = moon_data.get("yield_multiplier", 1.0)
        base_volume = moon_data.get("base_ore_volume", 20000.0)
        
        # Generate belt ores based on composition
        belt_ores = {}
        for ore_type, proportion in operation.moon_composition.items():
            ore_volume = base_volume * proportion * yield_multiplier
            if ore_volume > 0:
                belt_ores[ore_type] = round(ore_volume, 1)
        
        operation.belt_ores = belt_ores
        operation.state = "belt_active"
        operation.total_extractions += 1
        operation.total_ore_generated += sum(belt_ores.values())
        
        return dict(belt_ores)
    
    def mine_from_belt(
        self,
        entity: Union[Entity, str],
        ore_type: str,
        amount: float
    ) -> Optional[float]:
        """
        Mine ore from an active moon ore belt.
        
        Args:
            entity: Entity with MoonMiningOperation component
            ore_type: Type of moon ore to mine
            amount: Amount of ore in m3 to mine
            
        Returns:
            Actual amount mined, or None on failure
        """
        entity = self._get_entity(entity)
        if not entity:
            return None
        
        operation = entity.get_component(MoonMiningOperation)
        if not operation:
            return None
        
        if operation.state != "belt_active":
            return None
        
        if ore_type not in operation.belt_ores:
            return None
        
        available = operation.belt_ores[ore_type]
        mined = min(amount, available)
        
        operation.belt_ores[ore_type] -= mined
        if operation.belt_ores[ore_type] <= 0:
            del operation.belt_ores[ore_type]
        
        # Check if belt is depleted
        if not operation.belt_ores:
            operation.state = "idle"
        
        return mined
    
    def get_operation_status(self, entity: Union[Entity, str]) -> Optional[Dict]:
        """
        Get the current status of a moon mining operation.
        
        Args:
            entity: Entity with MoonMiningOperation component
            
        Returns:
            Dict with operation status details
        """
        entity = self._get_entity(entity)
        if not entity:
            return None
        
        operation = entity.get_component(MoonMiningOperation)
        if not operation:
            return None
        
        status = {
            "state": operation.state,
            "moon_id": operation.moon_id,
            "system_id": operation.system_id,
            "total_extractions": operation.total_extractions,
            "total_ore_generated": operation.total_ore_generated,
        }
        
        if operation.state == "extracting":
            status["progress"] = operation.extraction_progress
            status["duration"] = operation.extraction_duration
            remaining = operation.extraction_duration * (1.0 - operation.extraction_progress)
            status["time_remaining"] = remaining
        elif operation.state == "ready":
            status["composition"] = dict(operation.moon_composition)
        elif operation.state == "belt_active":
            status["belt_ores"] = dict(operation.belt_ores)
            status["total_belt_volume"] = sum(operation.belt_ores.values())
        
        return status
    
    def get_moon_ore_info(self, ore_type: str) -> Optional[Dict]:
        """Get information about a moon ore type"""
        return MOON_ORE_DATA.get(ore_type)
    
    def get_all_moon_ores(self) -> List[str]:
        """Get list of all moon ore types"""
        return list(MOON_ORE_DATA.keys())
    
    def update(self, delta_time: float):
        """
        Update all active moon mining operations.
        
        Progresses extraction timers and handles belt expiration.
        """
        entities = self.get_entities(MoonMiningOperation)
        
        for entity in entities:
            operation = entity.get_component(MoonMiningOperation)
            
            if operation.state == "extracting":
                # Progress extraction
                progress_increment = delta_time / operation.extraction_duration
                operation.extraction_progress += progress_increment
                
                if operation.extraction_progress >= 1.0:
                    operation.extraction_progress = 1.0
                    operation.state = "ready"
            
            elif operation.state == "belt_active":
                # Track belt lifetime
                operation.belt_creation_time += delta_time
                if operation.belt_creation_time >= operation.belt_lifetime:
                    # Belt has expired
                    operation.belt_ores.clear()
                    operation.state = "idle"
                    operation.belt_creation_time = 0.0
    
    def _get_entity(self, entity: Union[Entity, str]) -> Optional[Entity]:
        """Get entity by reference or ID"""
        if isinstance(entity, Entity):
            return entity
        return self.world.get_entity(entity) if hasattr(self.world, 'get_entity') else None
