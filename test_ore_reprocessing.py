"""
Tests for Ore Reprocessing
Tests ore refining mechanics and mineral yields
"""

import unittest
import sys
import os

# Add parent directory to path for imports
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from engine.core.ecs import World
from engine.components.game_components import (
    OreHold, Inventory, Skills
)
from engine.systems.industry_system import IndustrySystem


class TestOreReprocessing(unittest.TestCase):
    """Test ore reprocessing functionality"""
    
    def setUp(self):
        """Set up test world and entities"""
        self.world = World()
        self.industry_system = IndustrySystem(self.world)
        
        # Sample ore data (Veldspar)
        self.veldspar_data = {
            "name": "Veldspar",
            "tier": "common",
            "volume_per_unit": 0.1,
            "reprocessing_base": 400,
            "minerals": {"tritanium": 415}
        }
        
        # Sample ore data (Scordite)
        self.scordite_data = {
            "name": "Scordite",
            "tier": "common",
            "volume_per_unit": 0.15,
            "reprocessing_base": 333,
            "minerals": {"tritanium": 346, "pyerite": 173}
        }
        
        # Create miner entity with inventory
        self.miner = self.world.create_entity()
        self.miner.add_component(Inventory(items={}, cargo_capacity=10000.0))
        self.miner.add_component(Skills())
    
    def test_reprocess_ore_basic(self):
        """Test basic ore reprocessing"""
        # Add ore to inventory
        inventory = self.miner.get_component(Inventory)
        inventory.items["veldspar"] = 1000
        
        # Reprocess ore (400 units = 1 batch)
        minerals = self.industry_system.reprocess_ore(
            self.miner,
            "veldspar",
            400,
            self.veldspar_data,
            station_efficiency=0.50
        )
        
        self.assertIsNotNone(minerals)
        self.assertIn("tritanium", minerals)
        
        # Should get 415 * 1 * 0.50 = 207.5 -> 207 tritanium
        self.assertGreater(minerals["tritanium"], 200)
        
        # Check ore was consumed
        self.assertEqual(inventory.items["veldspar"], 600)
    
    def test_reprocess_with_skills(self):
        """Test reprocessing with skill bonuses"""
        skills = self.miner.get_component(Skills)
        skills.skills["reprocessing"] = 5  # 3% per level = 15% bonus
        skills.skills["reprocessing_efficiency"] = 4  # 2% per level = 8% bonus
        # Total: 0.50 + 0.15 + 0.08 = 0.73 (73%)
        
        inventory = self.miner.get_component(Inventory)
        inventory.items["veldspar"] = 1000
        
        # Calculate expected efficiency
        efficiency = self.industry_system.calculate_reprocessing_efficiency(
            self.miner,
            station_base=0.50
        )
        self.assertAlmostEqual(efficiency, 0.73, places=2)
        
        # Reprocess with skills
        minerals = self.industry_system.reprocess_ore(
            self.miner,
            "veldspar",
            400,
            self.veldspar_data,
            station_efficiency=0.50,
            skills_bonus=0.23  # 15% + 8%
        )
        
        self.assertIsNotNone(minerals)
        # Should get 415 * 1 * 0.73 = 302.95 -> 302 tritanium
        self.assertGreater(minerals["tritanium"], 280)
        self.assertLess(minerals["tritanium"], 320)
    
    def test_reprocess_multiple_minerals(self):
        """Test reprocessing ore with multiple mineral outputs"""
        inventory = self.miner.get_component(Inventory)
        inventory.items["scordite"] = 1000
        
        # Reprocess scordite (produces tritanium and pyerite)
        minerals = self.industry_system.reprocess_ore(
            self.miner,
            "scordite",
            666,  # 2 batches (333 each)
            self.scordite_data,
            station_efficiency=0.50
        )
        
        self.assertIsNotNone(minerals)
        self.assertIn("tritanium", minerals)
        self.assertIn("pyerite", minerals)
        
        # Should get 2 batches worth
        # Tritanium: 346 * 2 * 0.50 = 346
        # Pyerite: 173 * 2 * 0.50 = 173
        self.assertGreater(minerals["tritanium"], 300)
        self.assertGreater(minerals["pyerite"], 150)
    
    def test_reprocess_insufficient_ore(self):
        """Test reprocessing fails with insufficient ore"""
        inventory = self.miner.get_component(Inventory)
        inventory.items["veldspar"] = 100  # Not enough for 1 batch (needs 400)
        
        minerals = self.industry_system.reprocess_ore(
            self.miner,
            "veldspar",
            400,
            self.veldspar_data
        )
        
        self.assertIsNone(minerals)
    
    def test_reprocess_from_ore_hold(self):
        """Test reprocessing from ore hold instead of cargo"""
        # Add ore hold
        self.miner.add_component(OreHold(ore_hold_capacity=10000.0))
        ore_hold = self.miner.get_component(OreHold)
        ore_hold.ore["veldspar"] = 1000
        ore_hold.ore_hold_used = 1000
        
        # Reprocess from ore hold
        minerals = self.industry_system.reprocess_ore(
            self.miner,
            "veldspar",
            800,  # 2 batches
            self.veldspar_data,
            station_efficiency=0.50
        )
        
        self.assertIsNotNone(minerals)
        self.assertIn("tritanium", minerals)
        
        # Check ore hold was depleted
        self.assertEqual(ore_hold.ore["veldspar"], 200)
        self.assertEqual(ore_hold.ore_hold_used, 200)
        
        # Minerals should be in inventory
        inventory = self.miner.get_component(Inventory)
        self.assertIn("tritanium", inventory.items)
    
    def test_reprocess_partial_batch(self):
        """Test that partial batches don't get processed"""
        inventory = self.miner.get_component(Inventory)
        inventory.items["veldspar"] = 1000
        
        # Try to reprocess 300 units (less than 1 batch of 400)
        minerals = self.industry_system.reprocess_ore(
            self.miner,
            "veldspar",
            300,
            self.veldspar_data
        )
        
        # Should fail - not enough for 1 batch
        self.assertIsNone(minerals)
        
        # Ore should not be consumed
        self.assertEqual(inventory.items["veldspar"], 1000)
    
    def test_reprocess_removes_depleted_ore(self):
        """Test that ore entry is removed when fully consumed"""
        inventory = self.miner.get_component(Inventory)
        inventory.items["veldspar"] = 400  # Exactly 1 batch
        
        minerals = self.industry_system.reprocess_ore(
            self.miner,
            "veldspar",
            400,
            self.veldspar_data
        )
        
        self.assertIsNotNone(minerals)
        
        # Ore should be completely removed from inventory
        self.assertNotIn("veldspar", inventory.items)
    
    def test_calculate_reprocessing_efficiency(self):
        """Test efficiency calculation"""
        skills = self.miner.get_component(Skills)
        
        # No skills
        efficiency = self.industry_system.calculate_reprocessing_efficiency(
            self.miner,
            station_base=0.50
        )
        self.assertEqual(efficiency, 0.50)
        
        # With Reprocessing 3
        skills.skills["reprocessing"] = 3
        efficiency = self.industry_system.calculate_reprocessing_efficiency(
            self.miner,
            station_base=0.50
        )
        self.assertEqual(efficiency, 0.59)  # 0.50 + 0.09
        
        # With both skills maxed
        skills.skills["reprocessing"] = 5
        skills.skills["reprocessing_efficiency"] = 5
        efficiency = self.industry_system.calculate_reprocessing_efficiency(
            self.miner,
            station_base=0.50
        )
        self.assertEqual(efficiency, 0.75)  # 0.50 + 0.15 + 0.10
    
    def test_reprocess_efficiency_cap(self):
        """Test that efficiency is capped at 100%"""
        skills = self.miner.get_component(Skills)
        skills.skills["reprocessing"] = 5  # +15%
        skills.skills["reprocessing_efficiency"] = 5  # +10%
        
        # Use high station base that would exceed 100%
        efficiency = self.industry_system.calculate_reprocessing_efficiency(
            self.miner,
            station_base=0.80  # 80% + 25% = 105%, should cap at 100%
        )
        
        self.assertEqual(efficiency, 1.0)  # Capped at 100%
    
    def test_reprocess_multiple_types(self):
        """Test reprocessing different ore types sequentially"""
        inventory = self.miner.get_component(Inventory)
        inventory.items["veldspar"] = 1000
        inventory.items["scordite"] = 1000
        
        # Reprocess veldspar
        minerals1 = self.industry_system.reprocess_ore(
            self.miner,
            "veldspar",
            400,
            self.veldspar_data,
            station_efficiency=0.50
        )
        
        # Reprocess scordite
        minerals2 = self.industry_system.reprocess_ore(
            self.miner,
            "scordite",
            333,
            self.scordite_data,
            station_efficiency=0.50
        )
        
        self.assertIsNotNone(minerals1)
        self.assertIsNotNone(minerals2)
        
        # Check both ores were consumed
        self.assertEqual(inventory.items["veldspar"], 600)
        self.assertEqual(inventory.items["scordite"], 667)
        
        # Check minerals accumulated
        self.assertIn("tritanium", inventory.items)
        self.assertIn("pyerite", inventory.items)


if __name__ == "__main__":
    unittest.main()
