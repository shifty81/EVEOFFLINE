"""
Tests for Ore Compression System
Tests ore and ice compression mechanics, volume reduction, and skill bonuses
"""

import unittest
import sys
import os

# Add parent directory to path for imports
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from engine.core.ecs import World, Entity
from engine.components.game_components import (
    OreHold, Inventory, Skills
)
from engine.systems.ore_compression_system import (
    OreCompressionSystem, ORE_COMPRESSION_DATA, ICE_COMPRESSION_DATA
)


class TestOreCompressionSystem(unittest.TestCase):
    """Test ore compression system functionality"""
    
    def setUp(self):
        """Set up test world and entities"""
        self.world = World()
        self.compression_system = OreCompressionSystem(self.world)
        
        # Create miner entity with ore hold and inventory
        self.miner = self.world.create_entity()
        self.miner.add_component(OreHold(ore_hold_capacity=50000.0))
        self.miner.add_component(Inventory(cargo_capacity=50000.0))
        self.miner.add_component(Skills())
    
    def test_compress_ore_basic(self):
        """Test basic ore compression"""
        # Add raw veldspar to ore hold
        ore_hold = self.miner.get_component(OreHold)
        ore_hold.ore["veldspar"] = 500.0
        ore_hold.ore_hold_used = 500.0
        
        result = self.compression_system.compress_ore(
            self.miner, "veldspar", 500.0
        )
        
        self.assertIsNotNone(result)
        self.assertEqual(result["compressed_type"], "compressed_veldspar")
        self.assertEqual(result["ore_consumed"], 500.0)
        self.assertEqual(result["batches"], 5)
        self.assertGreater(result["volume_saved"], 0)
    
    def test_compress_ore_partial_batch(self):
        """Test compression with partial batches (only full batches processed)"""
        ore_hold = self.miner.get_component(OreHold)
        ore_hold.ore["veldspar"] = 250.0
        ore_hold.ore_hold_used = 250.0
        
        result = self.compression_system.compress_ore(
            self.miner, "veldspar", 250.0
        )
        
        self.assertIsNotNone(result)
        self.assertEqual(result["batches"], 2)
        self.assertEqual(result["ore_consumed"], 200.0)
        # 50 m3 should remain
        self.assertAlmostEqual(ore_hold.ore["veldspar"], 50.0)
    
    def test_compress_ore_insufficient(self):
        """Test compression fails with insufficient ore"""
        ore_hold = self.miner.get_component(OreHold)
        ore_hold.ore["veldspar"] = 50.0
        
        result = self.compression_system.compress_ore(
            self.miner, "veldspar", 200.0
        )
        
        self.assertIsNone(result)
    
    def test_compress_ore_below_batch_size(self):
        """Test compression fails with less than one batch"""
        ore_hold = self.miner.get_component(OreHold)
        ore_hold.ore["veldspar"] = 50.0
        ore_hold.ore_hold_used = 50.0
        
        result = self.compression_system.compress_ore(
            self.miner, "veldspar", 50.0
        )
        
        self.assertIsNone(result)
    
    def test_compress_ore_unknown_type(self):
        """Test compression fails for unknown ore type"""
        ore_hold = self.miner.get_component(OreHold)
        ore_hold.ore["unobtainium"] = 500.0
        
        result = self.compression_system.compress_ore(
            self.miner, "unobtainium", 500.0
        )
        
        self.assertIsNone(result)
    
    def test_compress_ore_not_at_station(self):
        """Test compression fails when not at a station"""
        ore_hold = self.miner.get_component(OreHold)
        ore_hold.ore["veldspar"] = 500.0
        
        result = self.compression_system.compress_ore(
            self.miner, "veldspar", 500.0, at_station=False
        )
        
        self.assertIsNone(result)
    
    def test_compress_ore_from_inventory(self):
        """Test compression from regular inventory"""
        inventory = self.miner.get_component(Inventory)
        inventory.items["scordite"] = 300
        
        # Remove ore hold to test inventory path
        self.miner.remove_component(OreHold)
        
        result = self.compression_system.compress_ore(
            self.miner, "scordite", 300.0
        )
        
        self.assertIsNotNone(result)
        self.assertEqual(result["batches"], 3)
        self.assertEqual(result["compressed_type"], "compressed_scordite")
    
    def test_compress_ore_with_skills(self):
        """Test compression with ore compression skill bonus"""
        ore_hold = self.miner.get_component(OreHold)
        ore_hold.ore["veldspar"] = 500.0
        ore_hold.ore_hold_used = 500.0
        
        # Set ore compression skill to level 5
        skills = self.miner.get_component(Skills)
        skills.skills["ore_compression"] = 5
        
        result = self.compression_system.compress_ore(
            self.miner, "veldspar", 500.0
        )
        
        self.assertIsNotNone(result)
        # 5 batches * (1.0 + 5*0.02) = 5 * 1.10 = 5.5
        self.assertAlmostEqual(result["compressed_amount"], 5.5)
    
    def test_compress_ore_volume_reduction(self):
        """Test that compression significantly reduces volume"""
        ore_hold = self.miner.get_component(OreHold)
        ore_hold.ore["veldspar"] = 1000.0
        ore_hold.ore_hold_used = 1000.0
        
        result = self.compression_system.compress_ore(
            self.miner, "veldspar", 1000.0
        )
        
        self.assertIsNotNone(result)
        # 1000 m3 raw → very small compressed volume
        self.assertGreater(result["volume_saved"], 900.0)
        self.assertLess(result["compressed_volume"], 100.0)
    
    def test_compress_all_ore_types(self):
        """Test compression for all supported ore types"""
        for ore_type in ORE_COMPRESSION_DATA:
            ore_hold = self.miner.get_component(OreHold)
            ore_hold.ore[ore_type] = 200.0
            ore_hold.ore_hold_used = 200.0
            
            result = self.compression_system.compress_ore(
                self.miner, ore_type, 200.0
            )
            
            self.assertIsNotNone(result, f"Compression failed for {ore_type}")
            self.assertEqual(result["batches"], 2, f"Wrong batch count for {ore_type}")
    
    def test_compress_ice_basic(self):
        """Test basic ice compression"""
        ore_hold = self.miner.get_component(OreHold)
        ore_hold.ice["clear_icicle"] = 5
        ore_hold.ore_hold_used = 5000.0  # 1000 m3 per ice unit
        
        result = self.compression_system.compress_ice(
            self.miner, "clear_icicle", 5
        )
        
        self.assertIsNotNone(result)
        self.assertEqual(result["compressed_type"], "compressed_clear_icicle")
        self.assertEqual(result["compressed_amount"], 5)
        self.assertEqual(result["ice_consumed"], 5)
        # 5 * 1000 m3 = 5000 m3 raw → 5 * 10 m3 = 50 m3 compressed
        self.assertAlmostEqual(result["volume_saved"], 4950.0)
    
    def test_compress_ice_insufficient(self):
        """Test ice compression fails with insufficient ice"""
        ore_hold = self.miner.get_component(OreHold)
        ore_hold.ice["clear_icicle"] = 2
        
        result = self.compression_system.compress_ice(
            self.miner, "clear_icicle", 5
        )
        
        self.assertIsNone(result)
    
    def test_compress_ice_not_at_station(self):
        """Test ice compression fails when not at station"""
        ore_hold = self.miner.get_component(OreHold)
        ore_hold.ice["clear_icicle"] = 5
        
        result = self.compression_system.compress_ice(
            self.miner, "clear_icicle", 5, at_station=False
        )
        
        self.assertIsNone(result)
    
    def test_compress_ice_unknown_type(self):
        """Test ice compression fails for unknown ice type"""
        ore_hold = self.miner.get_component(OreHold)
        ore_hold.ice["magic_ice"] = 5
        
        result = self.compression_system.compress_ice(
            self.miner, "magic_ice", 5
        )
        
        self.assertIsNone(result)
    
    def test_compress_all_ice_types(self):
        """Test compression for all supported ice types"""
        for ice_type in ICE_COMPRESSION_DATA:
            ore_hold = self.miner.get_component(OreHold)
            ore_hold.ice[ice_type] = 3
            ore_hold.ore_hold_used = 3000.0
            
            result = self.compression_system.compress_ice(
                self.miner, ice_type, 3
            )
            
            self.assertIsNotNone(result, f"Compression failed for {ice_type}")
            self.assertEqual(result["compressed_amount"], 3, f"Wrong amount for {ice_type}")
    
    def test_get_compression_info_ore(self):
        """Test getting compression info for ore"""
        info = self.compression_system.get_compression_info("veldspar")
        
        self.assertIsNotNone(info)
        self.assertEqual(info["type"], "ore")
        self.assertEqual(info["batch_size"], 100)
        self.assertGreater(info["compression_ratio"], 1)
    
    def test_get_compression_info_ice(self):
        """Test getting compression info for ice"""
        info = self.compression_system.get_compression_info("clear_icicle")
        
        self.assertIsNotNone(info)
        self.assertEqual(info["type"], "ice")
        self.assertGreater(info["compression_ratio"], 1)
    
    def test_get_compression_info_unknown(self):
        """Test getting compression info for unknown material"""
        info = self.compression_system.get_compression_info("unobtainium")
        self.assertIsNone(info)
    
    def test_get_compressible_lists(self):
        """Test getting lists of compressible materials"""
        ores = self.compression_system.get_compressible_ores()
        ice = self.compression_system.get_compressible_ice()
        
        self.assertEqual(len(ores), 15)
        self.assertEqual(len(ice), 12)
        self.assertIn("veldspar", ores)
        self.assertIn("clear_icicle", ice)
    
    def test_ore_consumed_after_compression(self):
        """Test that ore is properly consumed after compression"""
        ore_hold = self.miner.get_component(OreHold)
        ore_hold.ore["veldspar"] = 500.0
        ore_hold.ore_hold_used = 500.0
        
        self.compression_system.compress_ore(self.miner, "veldspar", 500.0)
        
        # All 500 m3 consumed (5 full batches of 100)
        self.assertNotIn("veldspar", ore_hold.ore)
    
    def test_compressed_ore_added_to_inventory(self):
        """Test that compressed ore is added to inventory"""
        ore_hold = self.miner.get_component(OreHold)
        ore_hold.ore["veldspar"] = 300.0
        ore_hold.ore_hold_used = 300.0
        
        self.compression_system.compress_ore(self.miner, "veldspar", 300.0)
        
        inventory = self.miner.get_component(Inventory)
        self.assertIn("compressed_veldspar", inventory.items)
        self.assertEqual(inventory.items["compressed_veldspar"], 3)


if __name__ == '__main__':
    unittest.main()
