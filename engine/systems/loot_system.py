"""
Loot System
Handles loot drops, loot tables, and loot containers
Based on EVE Online's loot mechanics
"""

from typing import Optional, Dict, List, Tuple
from dataclasses import dataclass, field
from enum import Enum
import random
from engine.core.ecs import World, Entity
from engine.components.game_components import Position


class LootQuality(Enum):
    """Loot quality/rarity"""
    COMMON = "common"
    UNCOMMON = "uncommon"
    RARE = "rare"
    VERY_RARE = "very_rare"
    OFFICER = "officer"  # EVE's highest quality NPC drops


@dataclass
class LootItem:
    """Individual loot item"""
    item_id: str
    name: str
    quantity: int = 1
    quality: LootQuality = LootQuality.COMMON
    value: float = 0.0  # ISK value
    volume: float = 1.0  # m3


@dataclass
class LootTable:
    """Loot table with weighted drops"""
    table_id: str
    items: List[Tuple[str, float]] = field(default_factory=list)  # [(item_id, drop_chance)]
    guaranteed_items: List[str] = field(default_factory=list)  # Always drop
    min_items: int = 1
    max_items: int = 3
    isk_min: float = 0.0
    isk_max: float = 0.0


@dataclass
class LootContainer:
    """Container that holds loot (wreck, cargo container, etc.)"""
    container_id: str
    container_type: str = "wreck"  # wreck, cargo, secure, audit
    contents: Dict[str, int] = field(default_factory=dict)  # {item_id: quantity}
    isk_contents: float = 0.0
    owner_id: Optional[str] = None
    is_looted: bool = False
    despawn_time: float = 120.0  # seconds until despawn
    created_at: float = 0.0


@dataclass
class LootContainers:
    """Component for tracking loot containers in space"""
    containers: Dict[str, LootContainer] = field(default_factory=dict)  # {container_id: container}


class LootSystem:
    """
    Handles loot generation and distribution
    """
    
    def __init__(self, world: World):
        self.world = world
        self.container_counter = 0
        
        # Loot tables: {table_id: LootTable}
        self.loot_tables: Dict[str, LootTable] = {}
        
        # Item database for lookups
        self.item_database: Dict[str, LootItem] = {}
    
    def register_loot_table(self, loot_table: LootTable):
        """Register a loot table"""
        self.loot_tables[loot_table.table_id] = loot_table
    
    def register_item(self, item: LootItem):
        """Register an item in the database"""
        self.item_database[item.item_id] = item
    
    def generate_loot(self, loot_table_id: str) -> Dict[str, int]:
        """
        Generate loot from a loot table
        
        Returns:
            Dict of {item_id: quantity}
        """
        loot_table = self.loot_tables.get(loot_table_id)
        if not loot_table:
            return {}
        
        loot = {}
        
        # Add guaranteed items
        for item_id in loot_table.guaranteed_items:
            loot[item_id] = loot.get(item_id, 0) + 1
        
        # Roll for random items
        num_items = random.randint(loot_table.min_items, loot_table.max_items)
        
        for _ in range(num_items):
            # Roll the table
            roll = random.random()
            cumulative_chance = 0.0
            
            for item_id, drop_chance in loot_table.items:
                cumulative_chance += drop_chance
                if roll <= cumulative_chance:
                    loot[item_id] = loot.get(item_id, 0) + 1
                    break
        
        return loot
    
    def create_loot_container(
        self,
        position: Tuple[float, float, float],
        loot_table_id: str,
        container_type: str = "wreck",
        owner_id: Optional[str] = None,
        current_time: float = 0.0
    ) -> Optional[str]:
        """
        Create a loot container in space
        
        Returns:
            container_id if successful
        """
        # Generate loot
        loot = self.generate_loot(loot_table_id)
        
        # Generate ISK
        loot_table = self.loot_tables.get(loot_table_id)
        isk = 0.0
        if loot_table and loot_table.isk_max > 0:
            isk = random.uniform(loot_table.isk_min, loot_table.isk_max)
        
        # Create container
        self.container_counter += 1
        container = LootContainer(
            container_id=f"container_{self.container_counter}",
            container_type=container_type,
            contents=loot,
            isk_contents=isk,
            owner_id=owner_id,
            created_at=current_time
        )
        
        # Create entity for the container
        container_entity = self.world.create_entity()
        pos = Position(x=position[0], y=position[1], z=position[2])
        container_entity.add_component(pos)
        
        # Add to world's container tracking
        loot_containers = LootContainers()
        loot_containers.containers[container.container_id] = container
        container_entity.add_component(loot_containers)
        
        return container.container_id
    
    def loot_container(
        self,
        looter_entity: Entity,
        container_id: str
    ) -> Optional[Dict[str, int]]:
        """
        Loot a container
        
        Returns:
            Dict of looted items {item_id: quantity}
        """
        # Find the container
        container = None
        for entity in self.world.get_entities(LootContainers):
            loot_containers = entity.get_component(LootContainers)
            if container_id in loot_containers.containers:
                container = loot_containers.containers[container_id]
                break
        
        if not container or container.is_looted:
            return None
        
        # Mark as looted
        container.is_looted = True
        
        # Transfer items to looter
        from engine.systems.industry_system import Inventory
        from engine.systems.market_system import Wallet
        
        inventory = looter_entity.get_component(Inventory)
        wallet = looter_entity.get_component(Wallet)
        
        looted_items = {}
        
        if inventory:
            for item_id, quantity in container.contents.items():
                if inventory.add_item(item_id, quantity):
                    looted_items[item_id] = quantity
        
        if wallet and container.isk_contents > 0:
            wallet.deposit(container.isk_contents)
        
        return looted_items
    
    def spawn_loot_from_npc(
        self,
        npc_entity: Entity,
        npc_type: str,
        current_time: float = 0.0
    ) -> Optional[str]:
        """
        Spawn loot when NPC is destroyed
        
        Args:
            npc_entity: The destroyed NPC
            npc_type: Type of NPC (e.g., 'serpentis_frigate')
            current_time: Current game time
            
        Returns:
            container_id if loot spawned
        """
        position = npc_entity.get_component(Position)
        if not position:
            return None
        
        # Determine loot table based on NPC type
        loot_table_id = self._get_loot_table_for_npc(npc_type)
        if not loot_table_id:
            return None
        
        # Create container at NPC position
        return self.create_loot_container(
            position=(position.x, position.y, position.z),
            loot_table_id=loot_table_id,
            container_type="wreck",
            current_time=current_time
        )
    
    def _get_loot_table_for_npc(self, npc_type: str) -> Optional[str]:
        """Map NPC type to loot table"""
        # Mapping of NPC types to loot tables
        npc_loot_mapping = {
            "serpentis_frigate": "serpentis_frigate_loot",
            "serpentis_cruiser": "serpentis_cruiser_loot",
            "guristas_frigate": "guristas_frigate_loot",
            "guristas_cruiser": "guristas_cruiser_loot",
            "blood_raider_frigate": "blood_raider_frigate_loot",
            "blood_raider_cruiser": "blood_raider_cruiser_loot",
            "pirate_frigate": "generic_pirate_frigate_loot",
            "pirate_cruiser": "generic_pirate_cruiser_loot",
        }
        
        return npc_loot_mapping.get(npc_type)
    
    def salvage_wreck(
        self,
        salvager_entity: Entity,
        container_id: str
    ) -> Optional[Dict[str, int]]:
        """
        Salvage a wreck for materials
        
        Returns:
            Dict of salvaged materials {item_id: quantity}
        """
        # Find the container
        container = None
        for entity in self.world.get_entities(LootContainers):
            loot_containers = entity.get_component(LootContainers)
            if container_id in loot_containers.containers:
                container = loot_containers.containers[container_id]
                break
        
        if not container or container.container_type != "wreck":
            return None
        
        # Roll for salvage materials
        salvage_items = {}
        
        # Common materials
        if random.random() < 0.7:  # 70% chance
            salvage_items["salvage_material"] = random.randint(1, 5)
        
        # Rare materials
        if random.random() < 0.2:  # 20% chance
            salvage_items["rare_salvage"] = random.randint(1, 2)
        
        # Add to inventory
        from engine.systems.industry_system import Inventory
        inventory = salvager_entity.get_component(Inventory)
        
        if inventory:
            for item_id, quantity in salvage_items.items():
                inventory.add_item(item_id, quantity)
        
        return salvage_items
    
    def update(self, delta_time: float, current_time: float):
        """Update loot system - despawn old containers"""
        containers_to_remove = []
        
        for entity in self.world.get_entities(LootContainers):
            loot_containers = entity.get_component(LootContainers)
            
            for container_id, container in list(loot_containers.containers.items()):
                # Check if container should despawn
                if current_time - container.created_at > container.despawn_time:
                    containers_to_remove.append((entity, container_id))
        
        # Remove despawned containers
        for entity, container_id in containers_to_remove:
            loot_containers = entity.get_component(LootContainers)
            del loot_containers.containers[container_id]
            
            # If no more containers, remove entity
            if not loot_containers.containers:
                self.world.destroy_entity(entity.id)
    
    def get_nearby_containers(
        self,
        position: Tuple[float, float, float],
        max_range: float = 100.0
    ) -> List[Tuple[str, float]]:
        """
        Get containers within range
        
        Returns:
            List of (container_id, distance) tuples
        """
        nearby = []
        px, py, pz = position
        
        for entity in self.world.get_entities(LootContainers, Position):
            loot_containers = entity.get_component(LootContainers)
            cont_pos = entity.get_component(Position)
            
            if not cont_pos:
                continue
            
            # Calculate distance
            dx = cont_pos.x - px
            dy = cont_pos.y - py
            dz = cont_pos.z - pz
            distance = (dx**2 + dy**2 + dz**2) ** 0.5
            
            if distance <= max_range:
                for container_id in loot_containers.containers:
                    nearby.append((container_id, distance))
        
        # Sort by distance
        nearby.sort(key=lambda x: x[1])
        
        return nearby


def create_default_loot_tables() -> Dict[str, LootTable]:
    """Create default loot tables for common NPCs"""
    tables = {}
    
    # Frigate loot tables
    tables["serpentis_frigate_loot"] = LootTable(
        table_id="serpentis_frigate_loot",
        items=[
            ("salvage_material", 0.5),
            ("ammo", 0.3),
            ("small_module", 0.15),
            ("datacores", 0.05),
        ],
        min_items=0,
        max_items=2,
        isk_min=5000.0,
        isk_max=15000.0
    )
    
    tables["guristas_frigate_loot"] = LootTable(
        table_id="guristas_frigate_loot",
        items=[
            ("salvage_material", 0.5),
            ("missiles", 0.3),
            ("small_module", 0.15),
            ("datacores", 0.05),
        ],
        min_items=0,
        max_items=2,
        isk_min=6000.0,
        isk_max=18000.0
    )
    
    # Cruiser loot tables
    tables["serpentis_cruiser_loot"] = LootTable(
        table_id="serpentis_cruiser_loot",
        items=[
            ("salvage_material", 0.4),
            ("medium_module", 0.25),
            ("datacores", 0.15),
            ("rare_salvage", 0.1),
            ("blueprint_copy", 0.05),
        ],
        min_items=1,
        max_items=4,
        isk_min=50000.0,
        isk_max=150000.0
    )
    
    tables["guristas_cruiser_loot"] = LootTable(
        table_id="guristas_cruiser_loot",
        items=[
            ("salvage_material", 0.4),
            ("medium_module", 0.25),
            ("datacores", 0.15),
            ("rare_salvage", 0.1),
            ("blueprint_copy", 0.05),
        ],
        min_items=1,
        max_items=4,
        isk_min=60000.0,
        isk_max=180000.0
    )
    
    return tables
