"""
Exploration and Scanning System
Handles probe scanning, anomalies, and signatures
Based on EVE Online's exploration mechanics
"""

from typing import Optional, Dict, List, Tuple
from dataclasses import dataclass, field
from enum import Enum
from engine.core.ecs import World, Entity
from engine.components.game_components import Position


class SignatureType(Enum):
    """Types of cosmic signatures"""
    COMBAT = "combat"  # Combat sites
    RELIC = "relic"  # Relic sites (archaeology)
    DATA = "data"  # Data sites (hacking)
    GAS = "gas"  # Gas clouds
    WORMHOLE = "wormhole"  # Wormhole connections


@dataclass
class CosmicSignature:
    """Scannable signature in space"""
    signature_id: str
    signature_type: SignatureType
    name: str
    signal_strength: float = 100.0  # 0-100%
    scan_progress: float = 0.0  # 0-100%
    position: Tuple[float, float, float] = (0.0, 0.0, 0.0)
    is_scanned: bool = False
    is_completed: bool = False  # Has been completed/harvested
    
    # Site-specific data
    loot_table: Dict[str, int] = field(default_factory=dict)  # {item_id: quantity}
    npc_spawns: List[str] = field(default_factory=list)  # NPC IDs to spawn
    difficulty: int = 1  # 1-5 difficulty level


@dataclass
class ScannerProbe:
    """Individual scanner probe"""
    probe_id: str
    position: Tuple[float, float, float] = (0.0, 0.0, 0.0)
    scan_range: float = 4.0  # AU
    scan_strength: float = 100.0


@dataclass
class ProbeScanner:
    """Component for ships with probe scanning capability"""
    probes: List[ScannerProbe] = field(default_factory=list)
    max_probes: int = 8
    scan_strength_bonus: float = 0.0  # From skills/modules
    active_scan: Optional[str] = None  # Currently scanning signature_id


@dataclass
class ShipScanner:
    """Component for directional and ship scanner"""
    max_range: float = 14.3  # AU
    scan_angle: float = 360.0  # degrees (360 = full sphere)
    scan_resolution: float = 1.0  # Lower is better
    
    # Last scan results
    last_scan_time: float = 0.0
    scan_cooldown: float = 5.0  # seconds between scans
    detected_entities: List[Entity] = field(default_factory=list)


@dataclass
class ExplorationData:
    """Component for tracking exploration progress"""
    scanned_signatures: List[str] = field(default_factory=list)
    completed_sites: List[str] = field(default_factory=list)
    total_loot_value: float = 0.0


class ExplorationSystem:
    """
    Handles exploration mechanics including probe scanning and sites
    """
    
    def __init__(self, world: World):
        self.world = world
        self.probe_counter = 0
        self.signature_counter = 0
        
        # System signatures: {system_id: {signature_id: CosmicSignature}}
        self.signatures: Dict[str, Dict[str, CosmicSignature]] = {}
    
    def generate_signatures(self, system_id: str, count: int = 5):
        """Generate random signatures in a system"""
        import random
        
        if system_id not in self.signatures:
            self.signatures[system_id] = {}
        
        signature_types = list(SignatureType)
        
        for _ in range(count):
            self.signature_counter += 1
            sig_type = random.choice(signature_types)
            
            # Generate signature
            signature = CosmicSignature(
                signature_id=f"sig_{self.signature_counter:04d}",
                signature_type=sig_type,
                name=self._generate_signature_name(sig_type),
                signal_strength=random.uniform(80.0, 100.0),
                position=(
                    random.uniform(-10.0, 10.0),
                    random.uniform(-10.0, 10.0),
                    random.uniform(-10.0, 10.0)
                ),
                difficulty=random.randint(1, 5)
            )
            
            # Add site-specific data
            if sig_type == SignatureType.COMBAT:
                signature.npc_spawns = self._generate_npc_spawns(signature.difficulty)
            elif sig_type in [SignatureType.RELIC, SignatureType.DATA]:
                signature.loot_table = self._generate_loot_table(signature.difficulty)
            
            self.signatures[system_id][signature.signature_id] = signature
    
    def _generate_signature_name(self, sig_type: SignatureType) -> str:
        """Generate a lore-friendly signature name"""
        import random
        
        prefixes = ["Forgotten", "Ancient", "Abandoned", "Hidden", "Lost", "Ruined"]
        suffixes = {
            SignatureType.COMBAT: ["Hideout", "Base", "Outpost", "Fortress"],
            SignatureType.RELIC: ["Ruins", "Wreckage", "Monument", "Tomb"],
            SignatureType.DATA: ["Lab", "Research Facility", "Databank", "Archive"],
            SignatureType.GAS: ["Cloud", "Nebula", "Pocket", "Formation"],
            SignatureType.WORMHOLE: ["Wormhole", "Rift", "Passage", "Gateway"]
        }
        
        prefix = random.choice(prefixes)
        suffix = random.choice(suffixes.get(sig_type, ["Site"]))
        return f"{prefix} {suffix}"
    
    def _generate_npc_spawns(self, difficulty: int) -> List[str]:
        """Generate NPC spawns based on difficulty"""
        npc_types = {
            1: ["pirate_frigate", "pirate_destroyer"],
            2: ["pirate_destroyer", "pirate_cruiser"],
            3: ["pirate_cruiser", "pirate_battlecruiser"],
            4: ["pirate_battlecruiser", "pirate_battleship"],
            5: ["pirate_battleship", "pirate_battleship", "pirate_commander"]
        }
        
        return npc_types.get(difficulty, ["pirate_frigate"])
    
    def _generate_loot_table(self, difficulty: int) -> Dict[str, int]:
        """Generate loot based on difficulty"""
        base_value = 1000000 * difficulty  # 1M ISK per difficulty level
        
        return {
            "salvage_material": difficulty * 10,
            "datacores": difficulty * 2,
            "blueprint_copy": 1 if difficulty >= 3 else 0
        }
    
    def launch_probes(self, entity: Entity, count: int = 8) -> bool:
        """Launch scanner probes"""
        scanner = entity.get_component(ProbeScanner)
        position = entity.get_component(Position)
        
        if not scanner or not position:
            return False
        
        # Check if we have room
        if len(scanner.probes) + count > scanner.max_probes:
            return False
        
        # Launch probes at ship position
        for i in range(count):
            self.probe_counter += 1
            probe = ScannerProbe(
                probe_id=f"probe_{self.probe_counter}",
                position=(position.x, position.y, position.z),
                scan_range=4.0,
                scan_strength=100.0 + scanner.scan_strength_bonus
            )
            scanner.probes.append(probe)
        
        return True
    
    def recall_probes(self, entity: Entity) -> bool:
        """Recall all probes"""
        scanner = entity.get_component(ProbeScanner)
        if not scanner:
            return False
        
        scanner.probes.clear()
        scanner.active_scan = None
        return True
    
    def move_probe(
        self,
        entity: Entity,
        probe_id: str,
        position: Tuple[float, float, float]
    ) -> bool:
        """Move a probe to a new position"""
        scanner = entity.get_component(ProbeScanner)
        if not scanner:
            return False
        
        for probe in scanner.probes:
            if probe.probe_id == probe_id:
                probe.position = position
                return True
        
        return False
    
    def adjust_probe_range(self, entity: Entity, new_range: float) -> bool:
        """Adjust scan range for all probes"""
        scanner = entity.get_component(ProbeScanner)
        if not scanner:
            return False
        
        for probe in scanner.probes:
            probe.scan_range = new_range
        
        return True
    
    def scan(self, entity: Entity, system_id: str) -> List[str]:
        """
        Perform a scan and return detected signatures
        
        Returns:
            List of signature IDs that were detected/improved
        """
        scanner = entity.get_component(ProbeScanner)
        if not scanner or len(scanner.probes) == 0:
            return []
        
        if system_id not in self.signatures:
            return []
        
        detected = []
        
        # Check each signature
        for sig_id, signature in self.signatures[system_id].items():
            if signature.is_scanned:
                continue
            
            # Calculate scan strength based on probe positions
            scan_strength = self._calculate_scan_strength(
                scanner.probes,
                signature.position
            )
            
            # Improve scan progress
            if scan_strength > 0:
                signature.scan_progress = min(
                    100.0,
                    signature.scan_progress + scan_strength
                )
                detected.append(sig_id)
                
                # Mark as scanned if > 100%
                if signature.scan_progress >= 100.0:
                    signature.is_scanned = True
        
        return detected
    
    def _calculate_scan_strength(
        self,
        probes: List[ScannerProbe],
        target_position: Tuple[float, float, float]
    ) -> float:
        """Calculate effective scan strength based on probe formation"""
        if not probes:
            return 0.0
        
        total_strength = 0.0
        tx, ty, tz = target_position
        
        for probe in probes:
            px, py, pz = probe.position
            distance = ((tx - px)**2 + (ty - py)**2 + (tz - pz)**2) ** 0.5
            
            # Probe contributes if target is in range
            if distance <= probe.scan_range:
                # Strength decreases with distance
                strength_multiplier = 1.0 - (distance / probe.scan_range) ** 2
                total_strength += probe.scan_strength * strength_multiplier
        
        # More probes = better scan (diminishing returns)
        probe_count_multiplier = 1.0 + (len(probes) - 1) * 0.1
        
        # Return progress per scan (0-20%)
        return min(20.0, total_strength * 0.01 * probe_count_multiplier)
    
    def warp_to_signature(
        self,
        entity: Entity,
        system_id: str,
        signature_id: str
    ) -> bool:
        """Warp ship to a scanned signature"""
        if system_id not in self.signatures:
            return False
        
        signature = self.signatures[system_id].get(signature_id)
        if not signature or not signature.is_scanned:
            return False
        
        position = entity.get_component(Position)
        if not position:
            return False
        
        # Move ship to signature location
        position.x, position.y, position.z = signature.position
        
        return True
    
    def complete_site(
        self,
        entity: Entity,
        system_id: str,
        signature_id: str
    ) -> Optional[Dict]:
        """
        Complete an exploration site and receive rewards
        
        Returns:
            Dict with rewards (loot, ISK value, etc.)
        """
        if system_id not in self.signatures:
            return None
        
        signature = self.signatures[system_id].get(signature_id)
        if not signature or not signature.is_scanned or signature.is_completed:
            return None
        
        # Mark as completed
        signature.is_completed = True
        
        # Track exploration progress
        exp_data = entity.get_component(ExplorationData)
        if exp_data:
            exp_data.completed_sites.append(signature_id)
        
        # Return rewards
        rewards = {
            'loot': signature.loot_table.copy(),
            'signature_type': signature.signature_type.value,
            'difficulty': signature.difficulty
        }
        
        # Add loot to inventory if available
        from engine.systems.industry_system import Inventory
        inventory = entity.get_component(Inventory)
        if inventory:
            for item_id, quantity in signature.loot_table.items():
                inventory.add_item(item_id, quantity)
        
        return rewards
    
    def directional_scan(
        self,
        entity: Entity,
        current_time: float
    ) -> List[Entity]:
        """
        Perform a directional scan to detect nearby entities
        
        Returns:
            List of detected entity IDs
        """
        ship_scanner = entity.get_component(ShipScanner)
        position = entity.get_component(Position)
        
        if not ship_scanner or not position:
            return []
        
        # Check cooldown
        if current_time - ship_scanner.last_scan_time < ship_scanner.scan_cooldown:
            return ship_scanner.detected_entities
        
        # Perform scan
        detected = []
        scanner_pos = (position.x, position.y, position.z)
        
        # Check all entities with Position
        for other_entity in self.world.get_entities(Position):
            if other_entity == entity:
                continue
            
            other_pos = other_entity.get_component(Position)
            if not other_pos:
                continue
            
            # Calculate distance
            dx = other_pos.x - scanner_pos[0]
            dy = other_pos.y - scanner_pos[1]
            dz = other_pos.z - scanner_pos[2]
            distance = (dx**2 + dy**2 + dz**2) ** 0.5
            
            # Check if in range
            if distance <= ship_scanner.max_range:
                # TODO: Check angle if scan_angle < 360
                detected.append(other_entity)
        
        ship_scanner.detected_entities = detected
        ship_scanner.last_scan_time = current_time
        
        return detected
    
    def get_signatures(self, system_id: str, scanned_only: bool = False) -> List[CosmicSignature]:
        """Get all signatures in a system"""
        if system_id not in self.signatures:
            return []
        
        signatures = list(self.signatures[system_id].values())
        
        if scanned_only:
            signatures = [s for s in signatures if s.is_scanned]
        
        return signatures
