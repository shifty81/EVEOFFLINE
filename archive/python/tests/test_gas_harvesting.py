"""
Tests for Gas Harvesting System
Tests gas cloud harvesting mechanics, yield calculations, and gas storage
"""

import unittest
import sys
import os

# Add parent directory to path for imports
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from engine.core.ecs import World, Entity
from engine.components.game_components import (
    Position, GasHarvester, MiningYield, OreHold, Inventory,
    Capacitor, Skills, Player, Fitting
)
from engine.systems.gas_harvesting_system import (
    GasHarvestingSystem, GasCloud, GasCloudManager
)


class TestGasCloud(unittest.TestCase):
    """Test gas cloud data structure"""
    
    def test_create_gas_cloud(self):
        """Test creating a gas cloud"""
        cloud = GasCloud(
            cloud_id="cloud_1",
            gas_type="fullerite_c50",
            system_id="j123456",
            position=(1000.0, 0.0, 0.0),
            volume=5000.0
        )
        
        self.assertEqual(cloud.id, "cloud_1")
        self.assertEqual(cloud.gas_type, "fullerite_c50")
        self.assertEqual(cloud.remaining_volume, 5000.0)
        self.assertFalse(cloud.is_depleted())
    
    def test_harvest_gas(self):
        """Test harvesting gas from cloud"""
        cloud = GasCloud("c1", "fullerite_c50", "sys1", (0, 0, 0), volume=100.0)
        
        harvested = cloud.harvest(10.0)
        self.assertEqual(harvested, 10.0)
        self.assertEqual(cloud.remaining_volume, 90.0)
    
    def test_cannot_overharvest(self):
        """Test that you cannot harvest more than available"""
        cloud = GasCloud("c1", "fullerite_c50", "sys1", (0, 0, 0), volume=5.0)
        
        harvested = cloud.harvest(10.0)
        self.assertEqual(harvested, 5.0)
        self.assertEqual(cloud.remaining_volume, 0.0)
        self.assertTrue(cloud.is_depleted())
    
    def test_deplete_gas_cloud(self):
        """Test depleting a gas cloud"""
        cloud = GasCloud("c1", "fullerite_c50", "sys1", (0, 0, 0), volume=20.0)
        
        cloud.harvest(20.0)
        self.assertTrue(cloud.is_depleted())


class TestGasCloudManager(unittest.TestCase):
    """Test gas cloud manager"""
    
    def test_create_and_get_cloud(self):
        """Test creating and retrieving gas clouds"""
        manager = GasCloudManager()
        cloud = manager.create_gas_cloud(
            "cloud_1", "fullerite_c50", "sys1", (0, 0, 0), 5000.0
        )
        
        retrieved = manager.get_gas_cloud("cloud_1")
        self.assertIsNotNone(retrieved)
        self.assertEqual(retrieved.gas_type, "fullerite_c50")
    
    def test_get_clouds_in_system(self):
        """Test getting all clouds in a system"""
        manager = GasCloudManager()
        manager.create_gas_cloud("c1", "fullerite_c50", "sys1", (0, 0, 0))
        manager.create_gas_cloud("c2", "fullerite_c60", "sys1", (100, 0, 0))
        manager.create_gas_cloud("c3", "cytoserocin", "sys2", (200, 0, 0))
        
        sys1_clouds = manager.get_clouds_in_system("sys1")
        self.assertEqual(len(sys1_clouds), 2)
        
        sys2_clouds = manager.get_clouds_in_system("sys2")
        self.assertEqual(len(sys2_clouds), 1)
    
    def test_remove_cloud(self):
        """Test removing a gas cloud"""
        manager = GasCloudManager()
        manager.create_gas_cloud("c1", "fullerite_c50", "sys1", (0, 0, 0))
        
        manager.remove_cloud("c1")
        self.assertIsNone(manager.get_gas_cloud("c1"))
        self.assertEqual(len(manager.get_clouds_in_system("sys1")), 0)


class TestGasHarvestingSystem(unittest.TestCase):
    """Test gas harvesting system"""
    
    def setUp(self):
        """Set up test world and entities"""
        self.world = World()
        self.gas_cloud_manager = GasCloudManager()
        self.gas_system = GasHarvestingSystem(self.world, self.gas_cloud_manager)
        
        # Create test gas cloud
        self.test_cloud = self.gas_cloud_manager.create_gas_cloud(
            "test_cloud_1", "fullerite_c50", "test_system",
            (500.0, 0.0, 0.0), volume=5000.0
        )
        
        # Create harvester entity
        self.harvester = self.world.create_entity()
        self.harvester.add_component(Position(x=0.0, y=0.0, z=0.0))
        self.harvester.add_component(Capacitor(capacitor=5000.0, capacitor_max=5000.0))
        self.harvester.add_component(GasHarvester(
            harvester_type="Gas Cloud Harvester I",
            cycle_time=30.0,
            yield_amount=10.0,
            optimal_range=1500.0,
            capacitor_usage=200.0
        ))
        self.harvester.add_component(MiningYield())
        self.harvester.add_component(Inventory(cargo_capacity=500.0))
        self.harvester.add_component(Skills())
    
    def test_start_harvesting(self):
        """Test starting gas harvesting"""
        result = self.gas_system.start_harvesting(
            self.harvester, self.test_cloud.id
        )
        
        self.assertTrue(result)
        gas_harvester = self.harvester.get_component(GasHarvester)
        self.assertTrue(gas_harvester.is_active)
    
    def test_cannot_start_out_of_range(self):
        """Test that harvesting fails when out of range"""
        pos = self.harvester.get_component(Position)
        pos.x = 50000.0  # Far beyond range
        
        result = self.gas_system.start_harvesting(
            self.harvester, self.test_cloud.id
        )
        
        self.assertFalse(result)
    
    def test_cannot_start_without_capacitor(self):
        """Test that harvesting fails without enough capacitor"""
        cap = self.harvester.get_component(Capacitor)
        cap.capacitor = 50.0  # Less than 200 required
        
        result = self.gas_system.start_harvesting(
            self.harvester, self.test_cloud.id
        )
        
        self.assertFalse(result)
    
    def test_complete_harvesting_cycle(self):
        """Test completing a full harvesting cycle"""
        self.gas_system.start_harvesting(self.harvester, self.test_cloud.id)
        
        # Complete one cycle
        self.gas_system.update(30.0)
        
        # Check gas was stored
        inventory = self.harvester.get_component(Inventory)
        self.assertIn("fullerite_c50", inventory.items)
        self.assertGreater(inventory.items["fullerite_c50"], 0)
        
        # Check capacitor was consumed
        cap = self.harvester.get_component(Capacitor)
        self.assertLess(cap.capacitor, 5000.0)
        
        # Check cloud was depleted
        self.assertLess(self.test_cloud.remaining_volume, 5000.0)
    
    def test_stop_harvesting(self):
        """Test stopping gas harvesting"""
        self.gas_system.start_harvesting(self.harvester, self.test_cloud.id)
        
        result = self.gas_system.stop_harvesting(self.harvester)
        self.assertTrue(result)
        
        gas_harvester = self.harvester.get_component(GasHarvester)
        self.assertFalse(gas_harvester.is_active)
    
    def test_harvesting_with_skills(self):
        """Test yield increase with Gas Cloud Harvesting skill"""
        skills = self.harvester.get_component(Skills)
        skills.skills["gas_cloud_harvesting"] = 5  # 5% per level = 25% bonus
        
        self.gas_system.start_harvesting(self.harvester, self.test_cloud.id)
        self.gas_system.update(30.0)
        
        # Base 10 * 1.25 = 12.5 -> int = 12
        inventory = self.harvester.get_component(Inventory)
        harvested = inventory.items.get("fullerite_c50", 0)
        self.assertGreater(harvested, 10)  # More than base yield
    
    def test_multiple_harvest_cycles(self):
        """Test multiple harvesting cycles"""
        self.gas_system.start_harvesting(self.harvester, self.test_cloud.id)
        
        # Complete 5 cycles
        for _ in range(5):
            self.gas_system.update(30.0)
        
        # Should have harvested ~50 m3
        inventory = self.harvester.get_component(Inventory)
        harvested = inventory.items.get("fullerite_c50", 0)
        self.assertGreater(harvested, 40)
    
    def test_stop_when_cargo_full(self):
        """Test harvesting stops when cargo is full"""
        inventory = self.harvester.get_component(Inventory)
        inventory.cargo_capacity = 15.0  # Very small
        
        self.gas_system.start_harvesting(self.harvester, self.test_cloud.id)
        
        # First cycle should work (10 m3)
        self.gas_system.update(30.0)
        gas_harvester = self.harvester.get_component(GasHarvester)
        self.assertTrue(gas_harvester.is_active)
        
        # Second cycle should stop (no space for another 10 m3)
        self.gas_system.update(30.0)
        self.assertFalse(gas_harvester.is_active)
    
    def test_stop_when_cloud_depleted(self):
        """Test harvesting stops when cloud is depleted"""
        # Create a tiny cloud
        small_cloud = self.gas_cloud_manager.create_gas_cloud(
            "small_cloud", "fullerite_c50", "test_system",
            (500.0, 0.0, 0.0), volume=5.0
        )
        
        self.gas_system.start_harvesting(self.harvester, small_cloud.id)
        
        # First cycle should deplete the cloud
        self.gas_system.update(30.0)
        
        gas_harvester = self.harvester.get_component(GasHarvester)
        # Cloud is depleted, next cycle should stop
        self.gas_system.update(30.0)
        self.assertFalse(gas_harvester.is_active)
    
    def test_storage_in_ore_hold(self):
        """Test gas is stored in ore hold when available"""
        self.harvester.add_component(OreHold(ore_hold_capacity=5000.0))
        
        self.gas_system.start_harvesting(self.harvester, self.test_cloud.id)
        self.gas_system.update(30.0)
        
        ore_hold = self.harvester.get_component(OreHold)
        self.assertIn("fullerite_c50", ore_hold.gas)
        self.assertGreater(ore_hold.gas["fullerite_c50"], 0)
    
    def test_get_nearby_gas_clouds(self):
        """Test finding nearby gas clouds"""
        nearby = self.gas_system.get_nearby_gas_clouds(self.harvester)
        
        self.assertEqual(len(nearby), 1)
        self.assertEqual(nearby[0][0].id, self.test_cloud.id)
    
    def test_get_harvesting_info(self):
        """Test getting harvesting information"""
        info = self.gas_system.get_harvesting_info(self.harvester)
        
        self.assertIn("is_active", info)
        self.assertIn("base_yield", info)
        self.assertIn("yield_multiplier", info)
        self.assertIn("final_yield", info)
        
        self.assertEqual(info["base_yield"], 10.0)
        self.assertGreater(info["yield_multiplier"], 0)
    
    def test_harvesting_stats_tracking(self):
        """Test that gas harvesting statistics are tracked"""
        self.gas_system.start_harvesting(self.harvester, self.test_cloud.id)
        self.gas_system.update(30.0)
        
        mining_yield = self.harvester.get_component(MiningYield)
        self.assertIn("fullerite_c50", mining_yield.total_gas_harvested)
        self.assertGreater(mining_yield.total_gas_harvested["fullerite_c50"], 0)


if __name__ == "__main__":
    unittest.main()
