"""
Gas Harvesting System
Handles gas cloud harvester operations, gas extraction, and gas storage
Based on EVE Online gas harvesting mechanics
"""

import json
import math
import os
from typing import Optional, Dict, List, Tuple
from engine.core.ecs import System
from engine.components.game_components import (
    Position, GasHarvester, MiningYield, OreHold, Inventory,
    Capacitor, Skills, Player, Fitting
)


class GasCloud:
    """Represents a gas cloud for harvesting"""

    def __init__(self, cloud_id: str, gas_type: str, system_id: str,
                 position: Tuple[float, float, float], volume: float = 5000.0):
        self.id = cloud_id
        self.gas_type = gas_type
        self.system_id = system_id
        self.position = position
        self.remaining_volume = volume  # m3 of gas
        self.total_volume = volume
        self.spawn_time = 0.0

    def is_depleted(self) -> bool:
        """Check if gas cloud is depleted"""
        return self.remaining_volume <= 0

    def harvest(self, amount: float) -> float:
        """
        Harvest gas from the cloud

        Args:
            amount: m3 of gas to harvest

        Returns:
            Actual m3 harvested
        """
        harvested = min(amount, self.remaining_volume)
        self.remaining_volume -= harvested
        return harvested


class GasCloudManager:
    """Manages gas clouds in the game world"""

    def __init__(self):
        self.clouds: Dict[str, GasCloud] = {}
        self.system_clouds: Dict[str, List[str]] = {}  # system_id -> [cloud_ids]
        self.gas_types_data = self._load_gas_types()

    def _load_gas_types(self) -> Dict:
        """Load gas types from data file"""
        data_path = os.path.join(
            os.path.dirname(os.path.dirname(os.path.dirname(__file__))),
            'data', 'gas_types.json'
        )
        try:
            with open(data_path, 'r') as f:
                return json.load(f)
        except FileNotFoundError:
            return {}

    def create_gas_cloud(
        self,
        cloud_id: str,
        gas_type: str,
        system_id: str,
        position: Tuple[float, float, float],
        volume: float = 5000.0
    ) -> GasCloud:
        """
        Create a new gas cloud

        Args:
            cloud_id: Unique identifier for the cloud
            gas_type: Type of gas in this cloud
            system_id: Solar system containing the cloud
            position: (x, y, z) position of the cloud
            volume: Total volume of gas in m3

        Returns:
            Created GasCloud
        """
        cloud = GasCloud(cloud_id, gas_type, system_id, position, volume)
        self.clouds[cloud_id] = cloud

        if system_id not in self.system_clouds:
            self.system_clouds[system_id] = []
        self.system_clouds[system_id].append(cloud_id)

        return cloud

    def get_gas_cloud(self, cloud_id: str) -> Optional[GasCloud]:
        """Get a gas cloud by ID"""
        return self.clouds.get(cloud_id)

    def get_clouds_in_system(self, system_id: str) -> List[GasCloud]:
        """Get all gas clouds in a system"""
        cloud_ids = self.system_clouds.get(system_id, [])
        return [self.clouds[cid] for cid in cloud_ids if cid in self.clouds]

    def remove_cloud(self, cloud_id: str):
        """Remove a depleted or expired cloud"""
        if cloud_id in self.clouds:
            cloud = self.clouds[cloud_id]
            if cloud.system_id in self.system_clouds:
                self.system_clouds[cloud.system_id].remove(cloud_id)
            del self.clouds[cloud_id]


class GasHarvestingSystem(System):
    """
    Manages gas harvesting operations including:
    - Gas harvester cycles
    - Gas extraction from clouds
    - Cycle time calculations with skill bonuses
    - Gas storage in cargo/ore holds
    """

    def __init__(self, world, gas_cloud_manager: GasCloudManager):
        super().__init__(world)
        self.gas_cloud_manager = gas_cloud_manager

    def update(self, delta_time: float):
        """Update gas harvesting cycles for all active harvesters"""
        entities = self.get_entities(GasHarvester, Position, Capacitor)

        for entity in entities:
            gas_harvester = entity.get_component(GasHarvester)

            if not gas_harvester.is_active:
                continue

            # Update harvesting cycle
            gas_harvester.current_cycle += delta_time

            # Check if cycle complete
            if gas_harvester.current_cycle >= gas_harvester.cycle_time:
                self._complete_harvesting_cycle(entity)
                gas_harvester.current_cycle = 0.0

    def start_harvesting(
        self,
        entity,
        target_cloud_id: str
    ) -> bool:
        """
        Start harvesting gas from a cloud

        Args:
            entity: The harvesting entity
            target_cloud_id: ID of the gas cloud to harvest

        Returns:
            True if harvesting started successfully
        """
        gas_harvester = entity.get_component(GasHarvester)
        position = entity.get_component(Position)
        capacitor = entity.get_component(Capacitor)

        if not all([gas_harvester, position, capacitor]):
            return False

        # Find the gas cloud
        gas_cloud = self.gas_cloud_manager.get_gas_cloud(target_cloud_id)
        if not gas_cloud:
            return False

        # Check if cloud is depleted
        if gas_cloud.is_depleted():
            return False

        # Check range
        distance = self._calculate_distance(
            (position.x, position.y, position.z),
            gas_cloud.position
        )

        if distance > gas_harvester.optimal_range:
            return False

        # Check capacitor
        if capacitor.capacitor < gas_harvester.capacitor_usage:
            return False

        # Start harvesting
        gas_harvester.is_active = True
        gas_harvester.target_gas_cloud_id = target_cloud_id
        gas_harvester.current_cycle = 0.0

        # Update yield tracking
        mining_yield = entity.get_component(MiningYield)
        if mining_yield:
            mining_yield.active_mining = True
            mining_yield.target_gas_cloud_id = target_cloud_id

        return True

    def stop_harvesting(self, entity) -> bool:
        """Stop harvesting gas"""
        gas_harvester = entity.get_component(GasHarvester)
        mining_yield = entity.get_component(MiningYield)

        if not gas_harvester:
            return False

        gas_harvester.is_active = False
        gas_harvester.target_gas_cloud_id = None
        gas_harvester.current_cycle = 0.0

        if mining_yield:
            mining_yield.active_mining = False
            mining_yield.target_gas_cloud_id = None

        return True

    def _complete_harvesting_cycle(self, entity):
        """Complete one harvesting cycle and extract gas"""
        gas_harvester = entity.get_component(GasHarvester)
        capacitor = entity.get_component(Capacitor)
        mining_yield = entity.get_component(MiningYield)

        # Check capacitor
        if capacitor.capacitor < gas_harvester.capacitor_usage:
            self.stop_harvesting(entity)
            return

        # Consume capacitor
        capacitor.capacitor -= gas_harvester.capacitor_usage

        # Find the gas cloud
        gas_cloud = self.gas_cloud_manager.get_gas_cloud(
            gas_harvester.target_gas_cloud_id
        )
        if not gas_cloud or gas_cloud.is_depleted():
            self.stop_harvesting(entity)
            return

        # Calculate yield with skill bonuses
        base_yield = gas_harvester.yield_amount
        yield_multiplier = self._calculate_yield_multiplier(entity)
        final_yield = base_yield * yield_multiplier

        # Extract gas from cloud
        gas_harvested = gas_cloud.harvest(final_yield)

        if gas_harvested <= 0:
            self.stop_harvesting(entity)
            return

        # Store gas
        stored = self._store_gas(entity, gas_cloud.gas_type, gas_harvested)

        if not stored:
            self.stop_harvesting(entity)
            return

        # Update statistics
        if mining_yield:
            if gas_cloud.gas_type not in mining_yield.total_gas_harvested:
                mining_yield.total_gas_harvested[gas_cloud.gas_type] = 0.0
            mining_yield.total_gas_harvested[gas_cloud.gas_type] += gas_harvested

    def _calculate_yield_multiplier(self, entity) -> float:
        """Calculate yield multiplier from skills"""
        multiplier = 1.0

        skills = entity.get_component(Skills)
        if skills:
            # Gas Cloud Harvesting skill: 5% yield per level
            gas_level = skills.skills.get("gas_cloud_harvesting", 0)
            multiplier *= (1.0 + gas_level * 0.05)

        return multiplier

    def _store_gas(self, entity, gas_type: str, amount: float) -> bool:
        """
        Store harvested gas in ore hold or cargo

        Args:
            entity: Entity doing the harvesting
            gas_type: Type of gas harvested
            amount: m3 of gas to store

        Returns:
            True if storage successful
        """
        # Try ore hold first (mining barges/exhumers have gas storage)
        ore_hold = entity.get_component(OreHold)
        if ore_hold:
            available = ore_hold.ore_hold_capacity - ore_hold.ore_hold_used
            if available >= amount:
                if gas_type not in ore_hold.gas:
                    ore_hold.gas[gas_type] = 0.0
                ore_hold.gas[gas_type] += amount
                ore_hold.ore_hold_used += amount
                return True

        # Try regular cargo
        inventory = entity.get_component(Inventory)
        if inventory:
            available = inventory.cargo_capacity - inventory.cargo_used
            if available >= amount:
                if gas_type not in inventory.items:
                    inventory.items[gas_type] = 0
                inventory.items[gas_type] += int(amount)
                inventory.cargo_used += amount
                return True

        return False

    def _calculate_distance(
        self,
        pos1: Tuple[float, float, float],
        pos2: Tuple[float, float, float]
    ) -> float:
        """Calculate distance between two positions"""
        dx = pos1[0] - pos2[0]
        dy = pos1[1] - pos2[1]
        dz = pos1[2] - pos2[2]
        return math.sqrt(dx * dx + dy * dy + dz * dz)

    def get_nearby_gas_clouds(
        self,
        entity,
        max_distance: float = 100000.0
    ) -> List[Tuple[GasCloud, float]]:
        """
        Get list of gas clouds near an entity

        Args:
            entity: Entity to check from
            max_distance: Maximum distance to search

        Returns:
            List of (gas_cloud, distance) tuples, sorted by distance
        """
        position = entity.get_component(Position)
        if not position:
            return []

        nearby = []
        entity_pos = (position.x, position.y, position.z)

        for cloud in self.gas_cloud_manager.clouds.values():
            if cloud.is_depleted():
                continue

            distance = self._calculate_distance(entity_pos, cloud.position)
            if distance <= max_distance:
                nearby.append((cloud, distance))

        # Sort by distance
        nearby.sort(key=lambda x: x[1])
        return nearby

    def get_harvesting_info(self, entity) -> Dict:
        """Get current gas harvesting status and information"""
        gas_harvester = entity.get_component(GasHarvester)
        mining_yield = entity.get_component(MiningYield)

        if not gas_harvester:
            return {"error": "No gas harvester equipped"}

        yield_multiplier = self._calculate_yield_multiplier(entity)

        info = {
            "is_active": gas_harvester.is_active,
            "cycle_time": gas_harvester.cycle_time,
            "current_cycle": gas_harvester.current_cycle,
            "cycle_progress": (
                gas_harvester.current_cycle / gas_harvester.cycle_time * 100
            ) if gas_harvester.is_active else 0,
            "base_yield": gas_harvester.yield_amount,
            "yield_multiplier": yield_multiplier,
            "final_yield": gas_harvester.yield_amount * yield_multiplier,
            "optimal_range": gas_harvester.optimal_range,
            "capacitor_usage": gas_harvester.capacitor_usage,
            "target_cloud_id": gas_harvester.target_gas_cloud_id
        }

        if mining_yield:
            info["total_gas_harvested"] = mining_yield.total_gas_harvested
            info["active_harvesting"] = mining_yield.active_mining

        return info
